#include "api.h"
#include <iostream>
#include "pond.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "ClimateParams.h"

using namespace std;

POND::POND(void) : m_nCells(-1), m_npond(-1), m_pond(NULL), m_pondVolMax(NULL), m_pondVol(NULL), m_landuse(NULL),
	               m_cellWidth(-1), m_pondSurfaceArea(NULL),
				   m_pondID1(NULL), m_pondID2(NULL), m_pondID3(NULL), m_reachID(NULL), m_paddyNum(-1),
				   m_chStorage(NULL), m_irrDepth(NULL), m_pet(NULL), m_ks(NULL), m_soilStorage(NULL),
				   m_pondCellEvap(NULL), m_pondCellSeep(NULL), m_flowInIndex(NULL),
				   m_surfaceRunoff(NULL), m_pe(NULL)
{
	
}


POND::~POND(void)
{
	if (m_pondVol != NULL) Release1DArray(m_pondVol);
	if (m_pondSurfaceArea != NULL) Release1DArray(m_pondSurfaceArea);
	
}
	

bool POND::CheckInputSize(const char *key, int n)
{
	if (n <= 0)
	{
		return false;
	}
	if (this->m_nCells != n)
	{
		if (this->m_nCells <= 0) this->m_nCells = n;
		else
			throw ModelException(MID_POND, "CheckInputSize", "Input data for " + string(key) +
			" is invalid. All the input data should have same size.");
	}
	return true;
}


bool POND::CheckInputData(void)
{
	return true;
}

void POND::SetValue(const char *key, float value)
{
	string sk(key);
	if(StringMatch(sk, Tag_CellWidth)){
		m_cellWidth = value;
		m_cellArea = m_cellWidth * m_cellWidth;
	}
	
}

void POND::Set1DData(const char *key, int n, float *data)
{
	string sk(key);
	CheckInputSize(key, n);
	if (StringMatch(sk, VAR_POND)) { m_pond = data; }
	else if (StringMatch(sk, VAR_IRRDEPTH)) { m_irrDepth = data; }
	else if (StringMatch(sk, VAR_CHST)) { m_chStorage = data; }		
	else if (StringMatch(sk, VAR_PET)) m_pet = data;
	else if (StringMatch(sk, VAR_OLFLOW)) m_surfaceRunoff = data;
	else if (StringMatch(sk, VAR_EXCP)) m_pe = data;
}

void POND::Set2DData(const char *key, int n, int col, float **data)
{
	string sk(key);
	CheckInputSize(key, n);
	m_soilLayers = col;

	if (StringMatch(sk, VAR_CONDUCT)) m_ks = data;
	else if (StringMatch(sk, VAR_SOL_ST)) m_soilStorage = data;
	else if (StringMatch(sk, Tag_FLOWIN_INDEX_D8))
	{
		CheckInputSize(key, n);
		m_flowInIndex = data;
	}	
}


void POND::initialOutputs()
{
	if (this->m_nCells <= 0)
	{
		throw ModelException(MID_POND, "CheckInputData", "The dimension of the input data can not be less than zero.");
	}
	// count all the pond id according to the pond raster and the grid cell of each pond
	if(m_pondIds.empty()){
		for (int i = 0; i < m_nCells; ++i){
			if(m_pond[i] != NODATA_POND){
				m_pondIds.push_back(m_pond[i]);
				m_pondIdInfo[m_pond[i]].push_back(i);
			}
		}
		// remove repeated id
		sort(m_pondIds.begin(),m_pondIds.end());
		m_pondIds.erase(unique(m_pondIds.begin(), m_pondIds.end()), m_pondIds.end());
		m_npond = m_pondIds.size();
	}
	
	if (m_npond <= 0)
		throw ModelException(MID_POND, "initialOutputs", "The pond number can not be less than zero.");

	// initialize some parameters,such as pond depth, max depth ...(mm)
	if (m_pondVolMax == NULL) Initialize1DArray(m_npond + 1, m_pondVolMax, 5000.f);
	if (m_pondVol == NULL) Initialize1DArray(m_npond + 1, m_pondVol, 3000.f);
	if (m_pondSurfaceArea == NULL) Initialize1DArray(m_npond + 1, m_pondSurfaceArea, 0.f);
	if (m_pondCellEvap == NULL) Initialize1DArray(m_nCells, m_pondCellEvap, 0.f);
	if (m_pondCellSeep == NULL) Initialize1DArray(m_nCells, m_pondCellSeep, 0.f);
	if (m_pondCellVol == NULL) Initialize1DArray(m_nCells, m_pondCellVol, 3000.f);
}

int POND::Execute()
{
	CheckInputData();
	initialOutputs();	

	// the module only simulate the pond cell
	// for each pond id, it has >= 1 cells, we simulate evap, seepage .et of each cell
	for (vector<int>::iterator it = m_pondIds.begin(); it != m_pondIds.end(); it++){
		int id = *it;
		float totdepth = 0.f;
		// compute each pond area
		pondSurfaceArea(id);
		for(vector<int>::iterator i = m_pondIdInfo[id].begin(); i != m_pondIdInfo[id].end(); i++){
			int cellId = *i;								
			// for each cell grid, the pond depth is equal to the pond itself
			pondSimulate(id, cellId);
			// sum all pond cell depth to update the whole pond depth
			totdepth += m_pondCellVol[cellId];
		}
		m_pondVol[id] = totdepth / m_pondIdInfo[id].size();
	}						
    return true;
} 

void POND::findFlowInCell(int id, int cellId, vector<int> flowInCell){
	// for a grid cell, find the upstream cell recursively until the cell which has no upstream cell
	int nUpstream = (int) m_flowInIndex[cellId][0];
	
	for (int upIndex = 1; upIndex <= nUpstream; ++upIndex)
	{
		int flowInID = (int) m_flowInIndex[cellId][upIndex];
		flowInCell.push_back(flowInID);
		if ((int) m_flowInIndex[flowInID][0] != 0){
			findFlowInCell(id, flowInID, flowInCell);
		}		
	}
}

void POND::pondSimulate(int id, int cellId)
{
	float qIn = 0.f;
	// 1. add m_pe in SUR_MR
	qIn += m_pe[cellId];
	// 2. add surface runoff from upstream cell
	vector<int> flowInCell;
	// for a grid cell, find the upstream cell recursively until the cell which has no upstream cell
	findFlowInCell(id, cellId, flowInCell);

	if(flowInCell.size() != 0){
		// compute the surface runoff from upstream cell to the pond cell
		for (vector<int>::iterator iter = flowInCell.begin(); iter != flowInCell.end(); iter++){
			int k = *iter;
			qIn += m_surfaceRunoff[k];
			m_surfaceRunoff[k] = 0.f;
		}
	}
	// 3. update cell depth
	m_pondCellVol[cellId] += qIn;	

	/// if overflow, then send the overflow to the cell's surface flow
	if(m_pondCellVol[cellId] > m_pondVolMax[id]){

	}

	/// if no overflow
	if (m_pondCellVol[cellId] > UTIL_ZERO){
		// for a cell grid, compute evaporation from water surface
		float pondevap = m_evap_coe * m_pet[cellId]; // mm
		pondevap = min(pondevap, m_pondCellVol[cellId]);
		m_pondCellVol[cellId] -= pondevap;
		m_pondCellEvap[cellId] += pondevap;

		// calculate seepage into soil
		float pondsep = m_ks[cellId][0] * 240.f; // mm
		pondsep = min(pondsep, m_pondCellVol[cellId]);
		m_pondCellVol[cellId] -= pondsep;
		m_pondCellSeep[cellId] += pondsep;
		m_soilStorage[cellId][0] += pondsep; 
	}
}

void POND::pondSurfaceArea(int id)
{
	// now, we assume if the cell is pond, then the cell area is pond area, the whole pond area is the sun of all cell area
	float cellArea = m_cellWidth * m_cellWidth;
	float cellNum = m_pondIdInfo[id].size();
	m_pondSurfaceArea[id] = cellNum * cellArea;	
}

void POND::Get1DData(const char *key, int *n, float **data)
{
	initialOutputs();
	string sk(key);
	if (StringMatch(sk, VAR_POND_VOL))*data = m_pondVol;
	else if (StringMatch(sk, VAR_POND_SA))*data = m_pondSurfaceArea;
	else 
		throw ModelException(MID_POND, "Get1DData","Parameter" + sk + "does not exist.");
	*n = m_nCells;
	
	return;
}
