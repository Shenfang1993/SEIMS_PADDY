#include "api.h"
#include <iostream>
#include "pond.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "ClimateParams.h"

using namespace std;

POND::POND(void) : m_nCells(-1), m_npond(-1), m_pond(NULL), m_pondVolMax(NULL), m_pondVol(NULL), m_landuse(NULL),
	               m_cellWidth(-1), m_pondSurfaceArea(NULL),
				   m_pondID1(NULL), m_pondID2(NULL), m_pondID3(NULL), m_reachID(NULL), m_paddyNum(-1)
{
	
}


POND::~POND(void)
{
	if (m_pondVol != NULL) Release1DArray(m_pondVol);
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
	else if (StringMatch(sk, VAR_CHST)) { m_chStorage = data; }		

}

void POND::Set2DData(const char *key, int n, int col, float **data)
{
	
}

void POND::SetPonds(clsPonds *ponds){
	if(ponds != NULL){
		m_paddyNum = ponds->GetPaddyNumber();
		m_paddyIDs = ponds->GetPaddyIDs();
		if(m_pondID1 == NULL){
			Initialize1DArray(m_paddyNum + 1,m_pondID1, 0.f);
			Initialize1DArray(m_paddyNum + 1,m_pondID2, 0.f);
			Initialize1DArray(m_paddyNum + 1,m_pondID3, 0.f);
			Initialize1DArray(m_paddyNum + 1,m_reachID, 0.f);
		}
		
		for (vector<int>::iterator it = m_paddyIDs.begin(); it != m_paddyIDs.end(); it++){
			int i = *it;
			clsPond* tmpPond = ponds->GetPondByID(i);
			m_pondID1[i] = tmpPond->GetPondID1();
			m_pondID2[i] = tmpPond->GetPondID2();
			m_pondID3[i] = tmpPond->GetPondID3();
			m_reachID[i] = tmpPond->GetReachID();
		}
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
	if (m_pondVolMax == NULL) Initialize1DArray(m_nCells, m_pondVolMax, 5000.f);
	if (m_pondVol == NULL) Initialize1DArray(m_nCells, m_pondVol, 3000.f);
	if (m_pondSurfaceArea == NULL) Initialize1DArray(m_nCells, m_pondSurfaceArea, 0.f);
}

int POND::Execute()
{
	CheckInputData();
	initialOutputs();	

	// for all cell, the module only simulate the pond cell
	// simulate the pond as a whole, here is not care about the specific cell
	for (vector<int>::iterator it = m_pondIds.begin(); it != m_pondIds.end(); it++){
		int id = *it;
		// compute each pond area
		pondSurfaceArea(id);
	}

	// first, if the paddy auto-irrigate current day
	for (vector<int>::iterator it = m_paddyIDs.begin(); it != m_paddyIDs.end(); it++){
		int paddyId = *it;
		// if the paddy cell auto-irr happened
		if (m_irrDepth[paddyId] != 0.f){
			float irrWater = m_irrDepth[paddyId] * m_cellArea * (1.f - m_embnkfr_pr); // mm * m2
			int tmp[] = {m_pondID1[paddyId], m_pondID2[paddyId], m_pondID3[paddyId]};
			vector<int> irrSource;
			// remove -9999 from irrigation source
			for (int i = 0; i < sizeof(tmp); i++){
				if (tmp[i] != NODATA_VALUE){
					irrSource.push_back(tmp[i]);
				}
			}
			// compute the depth should remove from corresponding pond vol in order
			for (vector<int>::iterator i = irrSource.begin(); i != irrSource.end(); i++){
				int j = *i;
				// from pond
				m_pondVol[j] -= min(m_pondVol[j], irrWater / m_pondSurfaceArea[j]);
				irrWater -= m_pondVol[j] * m_pondSurfaceArea[j];
				// if enough, then stop search nest irrigation source
				if (irrWater <= 0.f){
					m_irrDepth[paddyId] = 0.f;
					break;
				}							
			}
			// if not enough, from reach
			if (irrWater > 0.f){
				irrWater = irrWater / 1000.f; // m *m2
				int reachId = m_reachID[paddyId];
				m_chStorage[reachId] -= irrWater;
				m_chStorage[reachId] = max(0.f, m_chStorage[reachId]);
				m_irrDepth[paddyId] = 0.f;
			}
		}
	}

	// then, for each pond id, it has >= 1 cells, we simulate evap, seepage .et of each cell
	for (vector<int>::iterator it = m_pondIds.begin(); it != m_pondIds.end(); it++){
		int id = *it;
		for(vector<int>::iterator i = m_pondIdInfo[id].begin(); i != m_pondIdInfo[id].end(); i++){
			int cellId = *i;
			// for each cell grid, the pond depth is equal to the pond itself
			m_pondVol[cellId] = m_pondVol[id];
			pondSimulate(cellId);
		}
	}						
    return true;
}

void POND::pondSimulate(int cellId)
{
	/// if overflow, then send the overflow to the cell's surface flow
	if(m_pondVol[cellId] > m_pondVolMax[cellId]){

	}

	/// if no overflow
	if (m_pondVol[cellId] > UTIL_ZERO){
		// for a cell grid, compute evaporation from water surface
		float pondevap = m_evap_coe * m_pet[cellId]; // mm
		pondevap = min(pondevap, m_pondVol[cellId]);
		m_pondVol[cellId] -= pondevap;
		m_pondEvap[cellId] += pondevap;

		// calculate seepage into soil
		float pondsep = m_ks[cellId][0] * 240.f; // mm
		pondsep = min(pondsep, m_pondVol[cellId]);
		m_pondVol[cellId] -= pondsep;
		m_pondSeep[cellId] += pondsep;
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
	else 
		throw ModelException(MID_POND, "Get1DData","Parameter" + sk + "does not exist.");
	*n = m_nCells;
	
	return;
}
