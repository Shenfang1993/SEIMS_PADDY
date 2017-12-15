#include "api.h"
#include <iostream>
#include "pond.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "ClimateParams.h"

using namespace std;

POND::POND(void) : m_npond(-1), m_pondVolMax(NULL), m_pondVol(NULL)
{
	
}


POND::~POND(void)
{
	if (m_pondVol != NULL) Release1DArray(m_pondVol);
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
	if(StringMatch(sk, Tag_CellWidth)){
		m_cellWidth = value;
		m_cellArea = m_cellWidth * m_cellWidth * 1.e-4f; // m2 ==> ha
		m_cnv = 10.f * m_cellArea; // mm/ha => m^3
	}
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
	for (int i = 0; i < m_npond; i++){
		//��ÿ��pond��դ��Ԫ��һЩ������ʼ��
		if (m_pondVolMax == NULL) Initialize1DArray(m_nCells, m_pondVolMax, 5.f);
		if (m_pondVol == NULL) Initialize1DArray(m_nCells, m_pondVol, 3.f);
	}
	
}

int POND::Execute()
{
	CheckInputData();
	initialOutputs();

	///����ÿ��pond id��ע��ÿ��pond id ����һ������pond cell id ��ɣ�����ͬһ��pond id�ڵĵ�Ԫ��˵����pond vol���
	for (int i = 0; i < m_npond; i++){

	}
	
    return true;
}

void POND::pondSimulate(int id)
{
	/// if overflow, then send the overflow to the cell's surface flow
	if(m_pondVol[id] > m_pondVolMax[id]){

	}

	/// m_pondSurfaceArea������pond��ˮ�������������pond��ĳ��դ��Ԫ�����
	/// if no overflow
	if (m_pondVol[id] > UTIL_ZERO){
		/// compute evaporation from water surface
		float pondevap = 10.f * m_evap_coe * m_pondSurfaceArea[id] * m_pet[id] / m_cnv; /// mm/hr*ha/240=m3/cnv=mm
		pondevap = min(pondevap, m_potVol[id]);
		m_pondVol[id] -= pondevap;
		m_pondEvap[id] += pondevap;

		/// calculate seepage into soil
		float pondsep = m_ks[id][0] * m_pondSurfaceArea[id] * 240.f / m_cnv; /// mm/hr*ha/240=m3/cnv=mm
		pondsep = min(pondsep, m_potVol[id]);
		m_pondVol[id] -= pondsep;
		m_pondSeep[id] += pondsep;
		m_soilStorage[id][0] += pondsep; /// this will be handled in the next time step
	}

}

void POND::pondSurfaceArea(int id)
{
	float pondVol_m3 = m_pondVol[id] * m_cnv;
	float expsa = (log(m_surfaceAreaEM[id]) - log(m_surfaceAreaPR[id])) / (log(m_volEM[id]) - log(m_volPR[id]));
	float coe_beta = pow(m_surfaceAreaEM[id] / m_volEM[id], expsa);
	m_pondSurfaceArea[id] = coe_beta * pow(pondVol_m3, expsa);
	
}

void POND::Get1DData(const char *key, int *n, float **data)
{
	initialOutputs();
	
	return;
}
