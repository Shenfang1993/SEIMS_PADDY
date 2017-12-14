#include "api.h"
#include <iostream>
#include "pond.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "ClimateParams.h"

using namespace std;

POND::POND(void) : m_npond(-1)
{
	
}


POND::~POND(void)
{
}
	

bool POND::CheckInputSize(const char *key, int n)
{

	return true;
}

bool POND::CheckInputSize2D(const char *key, int n, int col)
{
	CheckInputSize(key, n);
	
	return true;
}

bool POND::CheckInputData()
{
	

	return true;
}

void POND::SetValue(const char *key, float value)
{
	string sk(key);
	
}

void POND::Set1DData(const char *key, int n, float *data)
{
	string sk(key);

}

void POND::Set2DData(const char *key, int n, int col, float **data)
{
	
}

void POND::initialOutputs()
{
	
}

int POND::Execute()
{
	CheckInputData();
	initialOutputs();

	///演算每个pond id，注意每个pond id 都由一个或多个pond cell id 组成，对于同一个pond id内的单元来说，其pond vol相等
	for (int i = 0; i < m_npond; i++){

	}

//	for (int iLayer = 0; iLayer < m_nRoutingLayers; ++iLayer)
//	{
//		// There are not any flow relationship within each routing layer.
//		// So parallelization can be done here.
//		int nCells = (int) m_routingLayers[iLayer][0];
//#pragma omp parallel for
//		for (int iCell = 1; iCell <= nCells; ++iCell)
//		{
//			int id = (int) m_routingLayers[iLayer][iCell]; // cell index
//			if (FloatEqual(m_impoundTrig[id], 0.f)){ /// if impounding trigger on
//				potholeSimulate(id);
//			}
//			else{
//				releaseWater(id);
//			}
//		}
	//}
	
    return true;
}

void POND::pondSimulate(int id)
{
	/// if overflow, then send the overflow to the cell's surface flow
	if(m_pondVol[id] > m_pondVolMax[id]){

	}

	/// if no overflow, compute settling and losses, surface inlet tile
	/// flow, evap, seepage, and redistribute soil water
	if (m_pondVol[id] > UTIL_ZERO){

	}

}

void POND::pondSurfaceArea(int id)
{
	/// compute surface area assuming a cone shape, ha
	float potVol_m3 = m_potVol[id] * m_cnv;
	m_potSurfaceArea[id] = PI * pow((3.f * potVol_m3 / (PI * m_slope[id])), 0.6666f);
	m_potSurfaceArea[id] /= 10000.f; /// convert to ha
	if (m_potSurfaceArea[id] <= UTIL_ZERO)
		m_potSurfaceArea[id] = 0.001f;
	if (m_potSurfaceArea[id] > m_cellArea)
		m_potSurfaceArea[id] = m_cellArea;
}

void POND::Get1DData(const char *key, int *n, float **data)
{
	initialOutputs();
	
	return;
}
