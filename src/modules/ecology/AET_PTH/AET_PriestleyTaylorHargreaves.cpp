#include "api.h"
#include <iostream>
#include "AET_PriestleyTaylorHargreaves.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "ClimateParams.h"

using namespace std;

AET_PT_H::AET_PT_H(void) : m_nCells(-1), m_soilLayers(-1), m_esco(NULL), m_nSoilLayers(NULL), m_soilDepth(NULL), 
	                       m_soilThick(NULL), m_solFC(NULL), m_landuse(NULL), m_cropsta(NULL),
						   /// input from other modules
                           m_tMean(NULL), m_lai(NULL), m_pet(NULL), m_canEvp(NULL), m_snowAcc(NULL), m_snowSB(NULL),
                           m_solCov(NULL), m_solNo3(NULL), m_soilStorage(NULL),m_soilStorageProfile(NULL),
						   m_impoundTrig(NULL), m_potVol(NULL),
						   /// output
                           m_ppt(NULL), m_soilESDay(NULL), m_no3Up(0.f)
{
}

AET_PT_H::~AET_PT_H(void)
{
    /// clean up output variables
    if (m_ppt != NULL) Release1DArray(m_ppt);
	if (m_soilESDay != NULL) Release1DArray(m_soilESDay);
}

void AET_PT_H::Set1DData(const char *key, int n, float *data)
{
    string sk(key);
    CheckInputSize(key, n);
    if (StringMatch(sk, VAR_ESCO)) m_esco = data;
    else if (StringMatch(sk, VAR_SOILLAYERS)) m_nSoilLayers = data;
    else if (StringMatch(sk, DataType_MeanTemperature)) m_tMean = data;
    else if (StringMatch(sk, VAR_LAIDAY)) m_lai = data;
    else if (StringMatch(sk, VAR_PET)) m_pet = data;
	else if (StringMatch(sk, VAR_INET)) m_canEvp = data;
    else if (StringMatch(sk, VAR_SNAC)) m_snowAcc = data;
    else if (StringMatch(sk, VAR_SNSB)) m_snowSB = data;
    else if (StringMatch(sk, VAR_SOL_COV)) m_solCov = data;
	else if (StringMatch(sk, VAR_SOL_SW)) m_soilStorageProfile = data;
	else if (StringMatch(sk, VAR_LANDUSE)){ m_landuse = data; }
	else if (StringMatch(sk, VAR_CROPSTA)) { m_cropsta = data; }
	else if (StringMatch(sk, VAR_IMPOUND_TRIG)) { m_impoundTrig = data; }
	else if (StringMatch(sk, VAR_POT_VOL)) { m_potVol = data; }
    else
        throw ModelException(MID_AET_PTH, "Set1DData", "Parameter " + sk + " does not exist.");
}

void AET_PT_H::Set2DData(const char *key, int n, int col, float **data)
{
    string sk(key);
    CheckInputSize(key, n);
    m_soilLayers = col;
    if (StringMatch(sk, VAR_SOILDEPTH)) m_soilDepth = data;
	else if(StringMatch(sk, VAR_SOILTHICK)) m_soilThick = data;
    else if (StringMatch(sk, VAR_SOL_AWC)) m_solFC = data;
    else if (StringMatch(sk, VAR_SOL_NO3)) m_solNo3 = data;
    else if (StringMatch(sk, VAR_SOL_ST)) m_soilStorage = data;
    else
        throw ModelException(MID_AET_PTH, "Set2DData", "Parameter " + sk + " does not exist.");
}

bool AET_PT_H::CheckInputSize(const char *key, int n)
{
    if (n <= 0)
        throw ModelException(MID_AET_PTH, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    if (this->m_nCells != n)
    {
        if (this->m_nCells <= 0) this->m_nCells = n;
        else
            throw ModelException(MID_AET_PTH, "CheckInputSize", "Input data for " + string(key) +
                                                                " is invalid. All the input data should have same size.");
    }
    return true;
}

bool AET_PT_H::CheckInputData(void)
{
    if (this->m_date <= 0) throw ModelException(MID_AET_PTH, "CheckInputData", "You have not set the time.");
    if (this->m_nCells <= 0)
        throw ModelException(MID_AET_PTH, "CheckInputData", "The dimension of the input data can not be less than zero.");
    if (this->m_esco == NULL)
        throw ModelException(MID_AET_PTH, "CheckInputData", "The soil evaporation compensation factor can not be NULL.");
    if (this->m_nSoilLayers == NULL)
        throw ModelException(MID_AET_PTH, "CheckInputData", "The soil layers can not be NULL.");
    if (this->m_tMean == NULL)
        throw ModelException(MID_AET_PTH, "CheckInputData", "The mean temperature can not be NULL.");
    if (this->m_lai == NULL)
        throw ModelException(MID_AET_PTH, "CheckInputData", "The leaf area index can not be NULL.");
    if (this->m_pet == NULL)
        throw ModelException(MID_AET_PTH, "CheckInputData", "The potential evaportranspiration can not be NULL.");
    if (this->m_snowAcc == NULL)
        throw ModelException(MID_AET_PTH, "CheckInputData", "The snow accumulation can not be NULL.");
    /// If m_snowSB is not provided, it will be initialized in initialOutputs().
	//if (this->m_snowSB == NULL)
    //    throw ModelException(MID_AET_PTH, "CheckInputData", "The snow sublimation can not be NULL.");
    if (this->m_solCov == NULL)
        throw ModelException(MID_AET_PTH, "CheckInputData", "The residue on soil surface can not be NULL.");
    if (this->m_soilDepth == NULL)
        throw ModelException(MID_AET_PTH, "CheckInputData", "The soil depth can not be NULL.");
	if (this->m_soilThick == NULL)
		throw ModelException(MID_AET_PTH, "CheckInputData", "The soil thickness can not be NULL.");
    if (this->m_solFC == NULL)
        throw ModelException(MID_AET_PTH, "CheckInputData", "The available water capacity at field capacity can not be NULL.");
    if (this->m_solNo3 == NULL)
        throw ModelException(MID_AET_PTH, "CheckInputData", "Nitrogen stored in the nitrate pool can not be NULL.");
    if (this->m_soilStorage == NULL)
        throw ModelException(MID_AET_PTH, "CheckInputData", "The soil storage can not be NULL.");
	if (this->m_soilStorageProfile == NULL)
		throw ModelException(MID_AET_PTH, "CheckInputData", "The soil storage of soil profile can not be NULL.");
    return true;
}
void AET_PT_H::initialOutputs()
{
	/// initialize output variables
	if(this->m_ppt == NULL) Initialize1DArray(m_nCells, m_ppt, 0.f);
	if(this->m_soilESDay == NULL) Initialize1DArray(m_nCells, m_soilESDay, 0.f);
	if(this->m_snowSB == NULL) Initialize1DArray(m_nCells, m_snowSB, 0.f);
}

float AET_PT_H::SWAT_maxPET(float pet, int i){
	/// compute potential plant evapotranspiration (PPT) other than Penman-Monteith method
	if (m_lai[i] <= 3.f) {
			m_ppt[i] = m_lai[i] * pet / 3.f;
	}
	else {
		m_ppt[i] = pet;
	}
	if (m_ppt[i] < 0.f) m_ppt[i] = 0.f;
	/// compute potential soil evaporation
	float cej = -5.e-5f;
	float eaj = 0.f;
	float es_max = 0.f;  ///maximum amount of evaporation (soil et)
	float eos1 = 0.f;
	if (m_snowAcc[i] >= 0.5f) {
		eaj = 0.5f;
	}
	else {
		eaj = exp(cej * (m_solCov[i] + 0.1f));
		}
	es_max = pet * eaj;
	eos1 = pet / (es_max + m_ppt[i] + 1.e-10f);
	eos1 = es_max * eos1;
	es_max = min(es_max, eos1);
	es_max = max(es_max, 0.f);
	/// make sure maximum plant and soil ET doesn't exceed potential ET
	if (pet < es_max + m_ppt[i] && !FloatEqual(es_max + m_ppt[i], 0.f)) {
			es_max = pet * es_max / (es_max + m_ppt[i]);
			m_ppt[i] = pet * m_ppt[i] / (es_max + m_ppt[i]);
	}
	if (pet < es_max + m_ppt[i]) {
		es_max = pet - m_ppt[i] - UTIL_ZERO;
	}
	return es_max;
}

float AET_PT_H::ORYZA_maxPET(float pet, int i){
	// split the total pet to the radiation-driven part and drying power part
	float etrd = 0.f, etae = 0.f;
	etrd = 0.75f * pet;
	etae = pet - etrd;

	float es_max = 0.f;
	es_max = exp(-0.5f * m_lai[i]) * pet;
	es_max = max(es_max, 0);
		
	m_ppt[i] = etrd * (1.f - exp(-0.5f * m_lai[i])) + etae * min(2.f, m_lai[i]);
	return es_max;
}

int AET_PT_H::Execute()
{
    CheckInputData();
	initialOutputs();
//#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++)
    {
		/// define intermediate variables
		float esd = 0.f, etco = 0.f, effnup = 0.f;
		float no3up = 0.f, es_max = 0.f, eos1 = 0.f, xx = 0.f;
		float cej = 0.f, eaj = 0.f, pet = 0.f, esleft = 0.f;
		float evzp = 0.f, eosl = 0.f, dep = 0.f, evz = 0.f, sev = 0.f;
        pet = m_pet[i] - m_canEvp[i];
        esd = 500.f;
        etco = 0.8f;
        effnup = 0.1f;

		/*float day = JulianDay(this->m_date);
		if (day == 151){
			bool flag = true;
		}
		if (i == 70){
			ofstream fout;
			fout.open("j:\\pet.txt", ios::app);
			fout << pet << "\t" <<m_pet[70] << "\n";
			fout << flush;
			fout.close();
		}*/
		if (pet < UTIL_ZERO)
		{
			pet = 0.f;
			m_ppt[i] = 0.f; // i.e., ep_max
			es_max = 0.f;
		}
		else
        {
			if ((int)m_landuse[i] == LANDUSE_ID_PADDY && m_cropsta[i] == 4.f){
				// if the cell is paddy and rice in main field
				// add oryza method to compute soil evaporation and crop transpiration, by sf 2017.11.29
				es_max = ORYZA_maxPET(pet, i);
			}
			else{
				es_max = SWAT_maxPET(pet, i);
			}        

            /// initialize soil evaporation variables
            esleft = es_max;
            /// compute sublimation, using the input m_snowSB from snow sublimation module, if not provided, initialized as 0
            if (m_tMean[i] > 0.f)
            {
                if (m_snowAcc[i] >= esleft)
                {
                    /// take all soil evap from snow cover
                    m_snowAcc[i] -= esleft;
                    m_snowSB[i] += esleft;
                    esleft = 0.f;
                }
                else
                {
                    /// take all soil evap from snow cover then start taking from soil
                    esleft -= m_snowAcc[i];
                    m_snowSB[i] += m_snowAcc[i];
                    m_snowAcc[i] = 0.f;
                }
            }

			//if (FloatEqual(m_impoundTrig[i], 0.f)){
			//	// for impound paddy rice, source of water for evaporation can from pot and soil, by sf 2017.11.29
			//	if (m_potVol[i] >= esleft){
			//		/// take all soil evap from pot
			//		m_potVol[i] -= esleft;
			//		esleft = 0.f;
			//	}
			//	else{
			//		/// first taking from pot then start taking from soil
			//		esleft -= m_potVol[i];
			//		m_potVol[i] = 0.f;			
			//	}
			//}

			// take soil evap from each soil layer
            evzp = 0.f;
            eosl = esleft;
            for (int ly = 0; ly < (int)m_nSoilLayers[i]; ly++)
            {
				dep = 0.f;
                /// depth exceeds max depth for soil evap (esd, by default 500 mm)
                if (ly == 0)
                    dep = m_soilDepth[i][ly];
                else
                    dep = m_soilDepth[i][ly - 1];
                if (dep < esd)
                {
                    /// calculate evaporation from soil layer
                    evz = 0.f;
                    sev = 0.f;
                    xx = 0.f;
                    evz = eosl * m_soilDepth[i][ly] / (m_soilDepth[i][ly] + exp(2.374f - 0.00713f * m_soilDepth[i][ly]));
                    sev = evz - evzp * m_esco[i];
                    evzp = evz;
                    if (m_soilStorage[i][ly] < m_solFC[i][ly])
                    {
                        xx = 2.5f * (m_soilStorage[i][ly] - m_solFC[i][ly]) / m_solFC[i][ly]; /// non dimension
                        sev *= Expo(xx);
                    }
                    sev = min(sev, m_soilStorage[i][ly] * etco);
                    if (sev < 0.f || sev != sev) sev = 0.f;
                    if (sev > esleft) sev = esleft;
                    /// adjust soil storage, potential evap
                    if (m_soilStorage[i][ly] > sev)
                    {
                        esleft -= sev;
                        m_soilStorage[i][ly] = max(UTIL_ZERO, m_soilStorage[i][ly] - sev);
                    }
                    else
                    {
                        esleft -= m_soilStorage[i][ly];
                        m_soilStorage[i][ly] = 0.f;
                    }
                }
                /// compute no3 flux from layer 2 to 1 by soil evaporation
                if (ly == 1)  /// index of layer 2 is 1 (soil surface, 10mm)
                {
                    no3up = 0.f;
                    no3up = effnup * sev * m_solNo3[i][ly] / (m_soilStorage[i][ly] + UTIL_ZERO);
                    no3up = min(no3up, m_solNo3[i][ly]);
                    m_no3Up += no3up / m_nCells;
                    m_solNo3[i][ly] -= no3up;
                    m_solNo3[i][ly-1] += no3up;
                }
            }
            /// update total soil water content
            m_soilStorageProfile[i] = 0.f;
            for (int ly = 0; ly < (int)m_nSoilLayers[i]; ly++)
                m_soilStorageProfile[i] += m_soilStorage[i][ly];
            /// calculate actual amount of evaporation from soil / water
			if (es_max > esleft)
				m_soilESDay[i] = es_max - esleft;
			else
				m_soilESDay[i] = 0.f;
        }
    }
    return true;
}

void AET_PT_H::GetValue(const char *key, float *value)
{
	initialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SNO3UP)) *value = this->m_no3Up;
    else
        throw ModelException(MID_AET_PTH, "GetValue", "Result " + sk + " does not exist.");
}

void AET_PT_H::Get1DData(const char *key, int *n, float **data)
{
	initialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_PPT)) *data = this->m_ppt;
    else if (StringMatch(sk, VAR_SOET)) *data = this->m_soilESDay;
    else if (StringMatch(sk, VAR_SNAC)) *data = this->m_snowAcc;
	else if (StringMatch(sk, VAR_SNSB)) *data = m_snowSB;
	
    else
        throw ModelException(MID_AET_PTH, "Get1DData", "Result " + sk + " does not exist.");
    *n = this->m_nCells;
}
