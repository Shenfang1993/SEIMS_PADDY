#include "api.h"
#include <iostream>
#include "ORYZA.h"
#include "MetadataInfo.h"
#include "ModelException.h"
#include "math.h"
#include "ClimateParams.h"
#include "util.h"
#include "PlantGrowthCommon.h"
#include <omp.h>
#include <cmath>

using namespace std;

ORYZA::ORYZA(void) : m_nCells(-1),m_co2(NODATA_VALUE), m_tMean(NULL), m_tMin(NULL),m_tMax(NULL),m_SR(NULL),
	//rice related parameters,they are all read from DB, single
	m_tbd(NODATA_VALUE),m_tod(NODATA_VALUE),m_tmd(NODATA_VALUE),m_dvrj(NODATA_VALUE),
	m_dvri(NODATA_VALUE),m_dvrp(NODATA_VALUE),m_dvrr(NODATA_VALUE),m_mopp(NODATA_VALUE),
	m_ppse(NODATA_VALUE),m_shckd(NODATA_VALUE),m_knf(NODATA_VALUE),m_rgrlMX(NODATA_VALUE),
	m_rgrlMN(NODATA_VALUE),m_nh(NODATA_VALUE),m_nplh(NODATA_VALUE),m_nplsb(NODATA_VALUE),
	m_lape(NODATA_VALUE),m_zrttr(NODATA_VALUE),m_tmpsb(NODATA_VALUE),m_aFsh(NODATA_VALUE),
	m_bFsh(NODATA_VALUE),m_aFlv(NODATA_VALUE),m_bFlv(NODATA_VALUE),m_aFso(NODATA_VALUE),
	m_bFso(NODATA_VALUE),m_aDrlv(NODATA_VALUE),m_bDrlv(NODATA_VALUE),m_tclstr(NODATA_VALUE),
	m_q10(NODATA_VALUE),m_tref(NODATA_VALUE),m_mainLV(NODATA_VALUE),m_mainST(NODATA_VALUE),
	m_mainSO(NODATA_VALUE),m_mainRT(NODATA_VALUE),m_crgLV(NODATA_VALUE),m_crgST(NODATA_VALUE),
	m_crgSTR(NODATA_VALUE),m_crgSO(NODATA_VALUE),m_crgRT(NODATA_VALUE),m_fstr(NODATA_VALUE), 
	m_lrstr(NODATA_VALUE), m_aSLA(NODATA_VALUE),m_bSLA(NODATA_VALUE),m_cSLA(NODATA_VALUE),
	m_dSLA(NODATA_VALUE),m_slaMX(NODATA_VALUE),m_fcRT(NODATA_VALUE),m_fcLV(NODATA_VALUE),
	m_fcST(NODATA_VALUE),m_fcSTR(NODATA_VALUE), m_fcSO(NODATA_VALUE),m_wgrMX(NODATA_VALUE),
	m_gzrt(NODATA_VALUE),m_zrtMCD(NODATA_VALUE), m_frpar(NODATA_VALUE),m_spgf(NODATA_VALUE),
	m_nMaxL(NODATA_VALUE),m_nMinL(NODATA_VALUE),m_rfnlv(NODATA_VALUE),m_rfnst(NODATA_VALUE),
	m_fntrt(NODATA_VALUE),m_tcntrf(NODATA_VALUE),m_nMaxSO(NODATA_VALUE),m_anMinSO(NODATA_VALUE),
	m_bnMinSO(NODATA_VALUE),m_shckl(NODATA_VALUE),m_sbdur(NODATA_VALUE), m_llls(NODATA_VALUE), 
	m_ulls(NODATA_VALUE), m_llle(NODATA_VALUE), m_ulle(NODATA_VALUE), m_lldl(NODATA_VALUE), 
	m_uldl(NODATA_VALUE),
	//soil related parameters
	m_soilLayers(-1),m_nSoilLayers(NULL), m_soilZMX(NULL), m_soilALB(NULL), m_soilDepth(NULL),
	m_soilAWC(NULL), m_totSoilAWC(NULL), m_totSoilSat(NULL),m_soilStorage(NULL), m_soilStorageProfile(NULL), 
	m_sol_cov(NULL),m_sol_rsd(NULL),m_ppt(NULL),m_soilNO3(NULL),  m_snowAcc(NULL),
	m_plantEPDay(NULL),m_plantUpTkN(NULL), m_ricePlantN(NULL), m_frStrsN(NULL), m_frStrsWa(NULL),
	m_albedo(NULL), m_sol_sat(NULL), m_sol_wpmm(NULL), m_sol_rsdin(NULL),
	// rice 
	m_cropsta(NULL), m_epco(NULL), m_ts(NULL), m_sla(NULL),
	// parameters related to the current day and latitude
	m_dayL(NULL), m_sinLD(NULL), m_cosLD(NULL), m_dsinbe(NULL), m_sinb(NULL), m_solcon(NULL), 
	m_rdpdf(NULL), m_rdpdr(NULL), m_tslvtr(NODATA_VALUE), m_tshckl(NODATA_VALUE),
	// parameters related to the growth of rice
	m_gaid(NULL), m_gai(NULL), m_rapshl(NULL), m_rapppl(NULL), m_fslla(NULL), m_nflv(NODATA_VALUE), m_redf(NODATA_VALUE), 
	m_eff(NODATA_VALUE), m_gpl(NULL), m_rapl(NULL), m_gpc(NULL), m_rapc(NULL), m_hour(NODATA_VALUE), m_gpcdt(NULL), 
	m_rapcdt(NULL), m_dtga(NULL), gnsp(NODATA_VALUE), m_gcr(NULL), m_coldTT(NULL), m_tfert(NULL), 
	m_ntfert(NODATA_VALUE), m_nsp(NULL), m_gngr(NULL), m_gLai(NODATA_VALUE), m_rwlvg(NULL), m_zrt(NULL), 
	m_sai(NODATA_VALUE), m_aLAI(NODATA_VALUE),m_fsh(NODATA_VALUE), m_frt(NODATA_VALUE), m_flv(NODATA_VALUE), m_fst(NODATA_VALUE), 
	m_fso(NODATA_VALUE), m_drlv(NODATA_VALUE), m_nsllv(NODATA_VALUE), m_lstr(NODATA_VALUE), m_teff(NODATA_VALUE), m_wlvd(NULL), 
	m_wsts(NULL), m_wstr(NULL), m_ngr(NULL), m_tnass(NULL), m_wlv(NULL), m_wagt(NULL), m_biomass(NULL), m_frRoot(NULL), glv(NODATA_VALUE), 
	gst(NODATA_VALUE), gso(NODATA_VALUE), m_wst(NULL), m_wso(NULL), m_wlvg(NULL), m_wrt(NULL), 
	// parameter related to the N of plant and soil
	m_anst(NULL), m_ancrf(NULL), m_anlv(NULL),
	// rice related parameters, output
	m_dvs(NULL), m_lai(NULL), m_wrr(NULL), m_sowDay(NULL), m_wlvgExs(NULL), m_laiExs(NULL), m_wlvgExp(NULL), m_laiExp(NULL),
	// p uptake
	m_PUpDis(NODATA_VALUE), m_frPlantP(NULL), m_frPlantP1(NULL),m_frPlantP2(NULL), m_frPlantP3(NULL), m_frPHUacc(NULL),
	m_PHUPlt(NULL), m_plantUpTkP(NULL), m_plantP(NULL), m_tBase(NULL), m_soilPsol(NULL), m_frStrsP(NULL)
{
}


ORYZA::~ORYZA(void)
{
	if (m_sol_cov != NULL) Release1DArray(m_sol_cov);
	if (m_sol_rsd != NULL) Release2DArray(m_nCells, m_sol_rsd);
	if (m_lastSoilRootDepth != NULL) Release1DArray(m_lastSoilRootDepth);
	if (m_plantEPDay != NULL) Release1DArray(m_plantEPDay);
	if (m_plantUpTkN != NULL) Release1DArray(m_plantUpTkN);
	if (m_ricePlantN != NULL) Release1DArray(m_ricePlantN);
	if (m_frStrsN != NULL) Release1DArray(m_frStrsN);
	if (m_frStrsWa != NULL) Release1DArray(m_frStrsWa);
	if (m_biomass != NULL) Release1DArray(m_biomass);
	if (m_wagt != NULL) Release1DArray(m_wagt);
	if (m_dvs != NULL) Release1DArray(m_dvs);
	if (m_wrr != NULL) Release1DArray(m_wrr);
	if (m_frPlantP != NULL) Release1DArray(m_frPlantP);
}

void ORYZA::SetValue(const char *key, float value)
{
	string sk(key);
	if (StringMatch(sk, VAR_CO2)) m_co2 = value;
	else if(StringMatch(sk, VAR_TBD)) m_tbd = value;
	else if(StringMatch(sk, VAR_TOD)) m_tod = value;
	else if(StringMatch(sk, VAR_TMD)) m_tmd = value;
	else if(StringMatch(sk, VAR_DVRJ)) m_dvrj = value;
	else if(StringMatch(sk, VAR_DVRI)) m_dvri = value;
	else if(StringMatch(sk, VAR_DVRP)) m_dvrp = value;
	else if(StringMatch(sk, VAR_DVRR)) m_dvrr = value;
	else if(StringMatch(sk, VAR_MOPP)) m_mopp = value;
	else if(StringMatch(sk, VAR_PPSE)) m_ppse = value;
	else if(StringMatch(sk, VAR_SHCKD)) m_shckd = value;
	else if(StringMatch(sk, VAR_KNF)) m_knf = value;
	else if(StringMatch(sk, VAR_RGRLMX)) m_rgrlMX = value;
	else if(StringMatch(sk, VAR_RGRLMN)) m_rgrlMN = value;
	else if(StringMatch(sk, VAR_NH)) m_nh = value;
	else if(StringMatch(sk, VAR_NPLH)) m_nplh = value;
	else if(StringMatch(sk, VAR_NPLSB)) m_nplsb = value;
	else if(StringMatch(sk, VAR_LAPE)) m_lape = value;
	else if(StringMatch(sk, VAR_ZRTTR)) m_zrttr = value;
	else if(StringMatch(sk, VAR_TMPSB)) m_tmpsb = value;
	else if(StringMatch(sk, VAR_AFSH)) m_aFsh = value;
	else if(StringMatch(sk, VAR_BFSH)) m_bFsh = value;
	else if(StringMatch(sk, VAR_AFLV)) m_aFlv = value;
	else if(StringMatch(sk, VAR_BFLV)) m_bFlv = value;
	else if(StringMatch(sk, VAR_AFSO)) m_aFso = value;
	else if(StringMatch(sk, VAR_BFSO)) m_bFso = value;
	else if(StringMatch(sk, VAR_ADRLV)) m_aDrlv = value;
	else if(StringMatch(sk, VAR_BDRLV)) m_bDrlv = value;
	else if(StringMatch(sk, VAR_TCLSTR)) m_tclstr = value;
	else if(StringMatch(sk, VAR_Q10)) m_q10 = value;
	else if(StringMatch(sk, VAR_TREF)) m_tref = value;
	else if(StringMatch(sk, VAR_MAINLV)) m_mainLV = value;
	else if(StringMatch(sk, VAR_MAINST)) m_mainST = value;
	else if(StringMatch(sk, VAR_MAINSO)) m_mainSO = value;
	else if(StringMatch(sk, VAR_MAINRT))m_mainRT = value;
	else if(StringMatch(sk, VAR_CRGLV)) m_crgLV = value;
	else if(StringMatch(sk, VAR_CRGST)) m_crgST = value;
	else if(StringMatch(sk, VAR_CRGSTR)) m_crgSTR = value;
	else if(StringMatch(sk, VAR_CRGSO)) m_crgSO = value;
	else if(StringMatch(sk, VAR_CRGRT)) m_crgRT = value;
	else if(StringMatch(sk, VAR_FSTR)) m_fstr = value;
	else if(StringMatch(sk, VAR_LRSTR)) m_lrstr = value;
	else if(StringMatch(sk, VAR_ASLA)) m_aSLA = value;
	else if(StringMatch(sk, VAR_BSLA)) m_bSLA = value;
	else if(StringMatch(sk, VAR_CSLA)) m_cSLA = value;
	else if(StringMatch(sk, VAR_DSLA)) m_dSLA = value;
	else if(StringMatch(sk, VAR_SLAMX)) m_slaMX = value;
	else if(StringMatch(sk, VAR_FCRT)) m_fcRT = value;
	else if(StringMatch(sk, VAR_FCST)) m_fcLV = value;
	else if(StringMatch(sk, VAR_FCLV)) m_fcST = value;
	else if(StringMatch(sk, VAR_FCSTR)) m_fcSTR = value;
	else if(StringMatch(sk, VAR_FCSO)) m_fcSO = value;
	else if(StringMatch(sk, VAR_WGRMX)) m_wgrMX = value;
	else if(StringMatch(sk, VAR_GZRT)) m_gzrt = value;
	else if(StringMatch(sk, VAR_ZRTMCD)) m_zrtMCD = value;
	else if(StringMatch(sk, VAR_FRPAR)) m_frpar = value;
	else if(StringMatch(sk, VAR_SPGF)) m_spgf = value;
	else if(StringMatch(sk, VAR_NMAXL)) m_nMaxL = value;
	else if(StringMatch(sk, VAR_NMINL)) m_nMinL = value;
	else if(StringMatch(sk, VAR_RFNLV)) m_rfnlv = value;
	else if(StringMatch(sk, VAR_RFNST)) m_rfnst = value;
	else if(StringMatch(sk, VAR_RFNRT)) m_fntrt = value;
	else if(StringMatch(sk, VAR_TCNTRF)) m_tcntrf = value;
	else if(StringMatch(sk, VAR_NMAXSO)) m_nMaxSO = value;
	else if(StringMatch(sk, VAR_ANMINSO)) m_anMinSO = value;
	else if(StringMatch(sk,VAR_BNMINSO)) m_bnMinSO = value;
	else if(StringMatch(sk,VAR_SHCKL)) m_shckl = value;
	else if(StringMatch(sk,VAR_SBDUR)) m_sbdur = value;
	else if(StringMatch(sk,VAR_LLLS)) m_llls = value;
	else if(StringMatch(sk,VAR_ULLS)) m_ulls = value;
	else if(StringMatch(sk,VAR_LLLE)) m_llle = value;
	else if(StringMatch(sk,VAR_ULLE)) m_ulle = value;
	else if(StringMatch(sk,VAR_LLDL)) m_lldl = value;
	else if(StringMatch(sk,VAR_ULDL)) m_uldl = value;
	else if (StringMatch(sk, VAR_PUPDIS)) m_PUpDis = value;
	else
		throw ModelException(MID_RICEGROW, "SetValue", "Parameter " + sk + " does not exist.");
}

bool ORYZA::CheckInputSize(const char *key, int n)
{
	if (n <= 0)
		throw ModelException(MID_RICEGROW, "CheckInputSize", "Input data for " + string(key) +
		" is invalid. The size could not be less than zero.");
	if (m_nCells != n)
	{
		if (m_nCells <= 0)
			m_nCells = n;
		else
			throw ModelException(MID_RICEGROW, "CheckInputSize", "Input data for " + string(key) +
			" is invalid. All the input raster data should have same size.");
	}
	return true;
}

void ORYZA::Set1DData(const char *key, int n, float *data)
{
	string sk(key);
	CheckInputSize(key, n);
	//// climate
	if (StringMatch(sk, DataType_MeanTemperature)) m_tMean = data;
	else if (StringMatch(sk, DataType_MinimumTemperature)) m_tMin = data;
	else if (StringMatch(sk, DataType_MaximumTemperature)) m_tMax = data;
	else if (StringMatch(sk, DataType_SolarRadiation)) m_SR = data;  
	//// soil properties and water related
	else if (StringMatch(sk, VAR_SOILLAYERS)) m_nSoilLayers = data;
	else if (StringMatch(sk, VAR_SOL_ZMX)) m_soilZMX = data;
	else if (StringMatch(sk, VAR_SOL_ALB)) m_soilALB = data;
	else if (StringMatch(sk, VAR_SOL_SW)) m_soilStorageProfile = data;
	else if (StringMatch(sk, VAR_SOL_SUMAWC)) m_totSoilAWC = data;
	else if (StringMatch(sk, VAR_SOL_SUMSAT)) m_totSoilSat = data;
	else if (StringMatch(sk, VAR_SOL_COV)) m_sol_cov = data;
	else if (StringMatch(sk, VAR_SNAC)) m_snowAcc = data;
	//// management
	else if (StringMatch(sk, VAR_EPCO)) m_epco = data;
	// rice
	else if (StringMatch(sk, VAR_CROPSTA)) m_cropsta = data;
	else if (StringMatch(sk, VAR_SOL_RSDIN)) m_sol_rsdin = data;
	else if (StringMatch(sk, VAR_PPT)) m_ppt = data;
	else if (StringMatch(sk, VAR_CELL_LAT)) m_celllat = data;
	else if (StringMatch(sk, VAR_LAIDAY)) m_lai = data;
	else if (StringMatch(sk, VAR_ANCRF)) m_ancrf = data;
	else if (StringMatch(sk, VAR_BP1)) m_frPlantP1 = data;
	else if (StringMatch(sk, VAR_BP2)) m_frPlantP2 = data;
	else if (StringMatch(sk, VAR_BP3)) m_frPlantP3 = data;
	else if (StringMatch(sk, VAR_PHUPLT)) m_PHUPlt = data;
	else if (StringMatch(sk, VAR_T_BASE)) m_tBase = data;
	
	else
		throw ModelException(MID_RICEGROW, "Set1DData", "Parameter " + sk + " does not exist.");
}

bool ORYZA::CheckInputSize2D(const char *key, int n, int col)
{
	CheckInputSize(key, n);
	if (col <= 0)
		throw ModelException(MID_RICEGROW, "CheckInputSize2D", "Input data for " + string(key) +
		" is invalid. The layer number could not be less than zero.");
	if (m_soilLayers != col)
	{
		if (m_soilLayers <= 0)
			m_soilLayers = col;
		else
			throw ModelException(MID_RICEGROW, "CheckInputSize2D", "Input data for " + string(key) +
			" is invalid. All the layers of input 2D raster data should have same size.");
	}
	return true;
}

void ORYZA::Set2DData(const char *key, int nRows, int nCols, float **data)
{
	string sk(key);
	CheckInputSize2D(key, nRows, nCols);
	if (StringMatch(sk, VAR_SOILDEPTH)) m_soilDepth = data;
	else if (StringMatch(sk, VAR_SOILTHICK)) m_soilThick = data;
	else if (StringMatch(sk, VAR_SOL_RSD)) m_sol_rsd = data;
	else if (StringMatch(sk, VAR_SOL_AWC)) m_soilAWC = data;
	else if (StringMatch(sk, VAR_SOL_ST)) m_soilStorage = data;
	else if (StringMatch(sk, VAR_SOL_NO3)) m_soilNO3 = data;
	else if (StringMatch(sk, VAR_SOL_UL)) m_sol_sat = data;
	else if (StringMatch(sk, VAR_SOL_WPMM)) m_sol_wpmm = data;
	else if (StringMatch(sk, VAR_SOL_SOLP)) m_soilPsol = data;
	else
		throw ModelException(MID_RICEGROW, "Set2DData", "Parameter " + sk + " does not exist.");
}

float ORYZA::CalHeatUnitDaily(int i)
{
	float tt = 0.f, hu = 0.f;

	for (int k = 1;k <= 24;k++)
	{
		float td = m_tMean[i] + 0.5f * abs(m_tMax[i] - m_tMin[i]) * cos(0.2618f * float(k - 14));
		if ((td > m_tbd) && (td < m_tmd))
		{
			if(td > m_tod)
			{
				td = m_tod - (td - m_tod) * (m_tod - m_tbd) / (m_tmd - m_tod);
			}
			tt = tt + (td - m_tbd) /24;
		}
	}
	hu = tt;
	return hu;
}

float ORYZA::CalDevelopmentRate(int i)
{
	float dl = 0.f, ppfac = 0.f;
	float dvr = 0.f;
	float hu = CalHeatUnitDaily(i);

	if((m_dvs[i] >= 0) && (m_dvs[i] < 0.4f))  dvr = m_dvrj * hu;
	else if(m_dvs[i] < 0.65f)
	{
		dl = m_dayL[i] + 0.9f;
		if(dl < m_mopp) 
			ppfac = 1.f;
		else 
			ppfac = 1.f - (dl - m_mopp) * m_ppse;
		ppfac = min(1.f, max(0, ppfac));
		dvr = m_dvri * hu * ppfac;
	}
	else if(m_dvs[i] < 1.f)  dvr = m_dvrp * hu;
	else if (m_dvs[i] > 1.f)  dvr = m_dvrr * hu;

	if(m_cropsta[i] == 3.f){
		m_tslvtr = m_ts[i];
		m_tshckl = m_shckl * m_tslvtr;
	}
	if((m_cropsta[i] > 3.f) && (m_ts[i] <(m_tslvtr + m_tshckl)))  dvr = 0.f;

	m_dvs[i] += dvr;
	return dvr;
}

void ORYZA::CalDayLengthAndSINB(int i)
{
	/// conversion factor from degrees to radians
	float DEGTRAD = 0.017453292f;
	/// compute declination according to lat and day number
	m_jday = JulianDay(this->m_date);
	float dec = -asin(sin(23.45f * DEGTRAD) * cos(2.f * PI * (m_jday + 10.f) / 365.f));
	m_sinLD[i] = sin (DEGTRAD * m_celllat[i]) * sin (dec);
	m_cosLD[i] = cos (DEGTRAD * m_celllat[i]) * cos (dec);
	float aob = m_sinLD[i] / m_cosLD[i];

	/// compute day length
	float zza, zzcos, zzsin;
	if(aob < -1)
	{
		/// above polar circle
		m_dayL[i] = 0;
		zzcos = 0;
		zzsin = 1;
	}
	else if(aob > 1)
	{
		/// within polar circle
		m_dayL[i] = 24;
		zzcos = 0;
		zzsin = -1;
	}
	else
	{
		m_dayL[i] = 12.f * (1.f + 2.f * asin(aob) / PI);
		zza = PI * (12.f + m_dayL[i]) / 24.f;
		zzcos = cos(zza);
		zzsin = sin(zza);
	}
	/// dsinbe used to calculate the actual radiation at a specific time of the day
	m_dsinbe[i] = 2.f * 3600.f * (m_dayL[i] * (0.5f * m_sinLD[i] + 0.2f * pow(m_sinLD[i], 2.f) + 0.1f * pow(m_cosLD[i], 2.f)) - 
		(12.f * m_cosLD[i] * zzcos + 9.6f * m_sinLD[i] * m_cosLD[i] * zzcos + 2.4f * pow(m_cosLD[i], 2.f) * zzcos * zzsin) / PI);
	m_solcon[i] = 1370.f * (1.f + 0.033f * cos(2.f * PI * m_jday/365.f));
}

void ORYZA::CalDirectRadiation(int i)
{
	float atmtr = 0.f, frdif = 0.f;
	m_sinb[i] = m_sinLD[i] + m_cosLD[i] * cos((m_hour - 12.f) * 0.2617993f);
	if (m_sinb[i] > 0)
	{
		/// sun is above the horizon
		//  SR : convert from MJ/m2/day to J/m2/day
		float tmpr1 = m_SR[i] * 1.e6 * m_sinb[i] * (1.f + 0.4f * m_sinb[i]) / m_dsinbe[i];
		atmtr = tmpr1 / (m_solcon[i] * m_sinb[i]);
		if(atmtr < 0.22f)  frdif = 1.f;
		else if (atmtr < 0.35f)  frdif = 1.f - 6.4f * pow((atmtr - 0.22f), 2.f);
		else  frdif = 1.47f - 1.66f * atmtr;

		float xx = 0.15f + 0.85f * (1.f - exp(-0.1f / m_sinb[i]));
		frdif = max(frdif, xx);

		m_rdpdf[i] = tmpr1 * m_frpar * frdif;
		m_rdpdr[i] = tmpr1 * m_frpar * (1.f - frdif);
	}
}

void ORYZA::CalLeafAbsorbRadiation(int i)
{
	/// Scattering coefficient of leaves for PAR
	float cslv = 0.2f;
	float tmpr1 = sqrt(1.f - cslv);
	float rflh = (1.f - tmpr1) / (1.f + tmpr1);
	float rfls = rflh * 2.f / (1.f + 2.f * m_sinb[i]);
	//compute Extinction coefficient
	float kdf = 0.f;
	if(m_dvs[i] > 0.f && m_dvs[i] < 0.65f) kdf = 0.4f;
	else if (m_dvs[i] < 1.f) kdf = 0.4f + (0.2f / 0.35f) * (m_dvs[i] - 0.65f);
	else kdf = 0.6f;
	float ecpdf = kdf;

	float clustf = ecpdf / (0.8f * tmpr1);
	float ecpbl = (0.5f / m_sinb[i]) * clustf;
	float ecptd = ecpbl * tmpr1;

	float rapdfl = (1.f - rflh) * m_rdpdf[i] * ecpdf * exp(-ecpdf * m_gaid[i]);
	float raptdl = (1.f - rfls) * m_rdpdr[i] * ecptd * exp(-ecptd * m_gaid[i]);
	float rapddl = (1.f - cslv) * m_rdpdr[i] * ecpbl * exp(-ecpbl * m_gaid[i]);

	m_rapshl[i] = rapdfl + raptdl - rapddl;
	m_rapppl[i] = (1.f - cslv) * m_rdpdr[i] / m_sinb[i];
	m_fslla[i] = clustf * exp(-ecpbl * m_gaid[i]);
}

float ORYZA::CalLeafMaxAssimilationRate(float gai, float gaid, float nflv, float redf)
{
	/// AmaxIn shows in the oryza.for,but it is not in use everywhere,so delete it
	float slni = 0.f, Amax = 0.f;
	float AmaxCO2 = 49.57f / 34.26f * (1.f - exp(-0.208f * (m_co2-60.f)/49.57f));
	AmaxCO2 = max(0.f, AmaxCO2);

	if((gai > 0.01f) && (m_knf > 0.f))  slni = nflv * gai * m_knf * exp(-m_knf * gaid)/(1.f - exp(-m_knf * gai));
	else  slni = nflv;

	if(slni > 0.5f)  Amax = 9.5f + (22.f * slni) * redf * AmaxCO2;
	else  Amax = max(0.f, 68.33f * (slni - 0.2f) * redf *AmaxCO2);

	return Amax;
}

void ORYZA::Sgpl(int i)
{
	// Gauss weights for three point Gauss
	float gsx[3] = {0.112702, 0.500000, 0.887298};
	float gsw[3] = {0.277778, 0.444444, 0.277778};		

	CalLeafAbsorbRadiation(i);
	// calculate redf and nflv
	float redf = 0.f, nflv = 0.f;
	if (m_tMean[i] <= 10.f) redf = 0.f;
	else if (m_tMean[i] <= 20.f) redf = 0.1f * (m_tMean[i] - 10.f);
	else if (m_tMean[i] <= 37.f) redf = 1.f;
	else redf = max(0.f, (1.f - (m_tMean[i] - 37.f) * 0.2f));
	if (m_dvs[i] <= 0.16f) nflv = 0.54f;
	else if (m_dvs[i] <= 0.33f) nflv = 0.54f + (m_dvs[i] - 0.16f) * 6.25f;
	else if (m_dvs[i] <= 0.65f) nflv = 1.53f - (m_dvs[i] - 0.33f);
	else if (m_dvs[i] <= 0.8f) nflv = 1.222f + (m_dvs[i] - 0.65f) * 2.3f;
	else if (m_dvs[i] <= 1.f) nflv = 1.56f - (m_dvs[i] - 0.8f) * 1.25f;
	else if (m_dvs[i] <= 1.45f) nflv = 1.29f + (m_dvs[i] - 1.f) * 0.16f;
	else nflv = 1.36f - (m_dvs[i] - 1.45f);
	
	float gpshl = 0.f;
	float Amax2 = CalLeafMaxAssimilationRate(m_gai[i], m_gaid[i], nflv, redf);
	if(Amax2 > 0.f)  gpshl = Amax2 * (1.f - exp(-m_rapshl[i] * m_eff / Amax2));

	float gpsll = 0.f, rapsll = 0.f;
	for(int k = 0;k < 3;k++)
	{
		float tmpr1 = m_rapshl[i] + m_rapppl[i] * gsx[k];
		if(Amax2 > 0)  gpsll += Amax2 * (1.f - exp(-tmpr1 * m_eff / Amax2)) * gsw[k];

		rapsll += tmpr1 * gsw[k];
	}
	// local assimilation rate(GPL) and rate of absorption of PARby canopy(RAPL)
	m_gpl[i] = m_fslla[i] * gpsll + (1.f - m_fslla[i]) * gpshl;
	m_rapl[i] = m_fslla[i] * rapsll + (1.f - m_fslla[i]) * m_rapshl[i];
}

void ORYZA::CalCanopyAssimilationRate(int i)
{
	// Gauss weights for three point Gauss
	float gsx[3] = {0.112702, 0.500000, 0.887298};
	float gsw[3] = {0.277778, 0.444444, 0.277778};
	m_gai[i] = m_aLAI;

	for (int k = 0;k < 3 ;k++)
	{
		m_gaid[i] = m_gai[i] * gsx[k];
		Sgpl(i);
		// Integration of local assimilation rate to canopy assimilation (GPC) and absorption of PAR by canopy (RAPC)
		m_gpc[i] += m_gpl[i] * gsw[k];
		m_rapc[i] += m_rapl[i] * gsw[k];
	}
	m_gpc[i] = m_gpc[i] * m_gai[i];
	m_rapc[i] = m_rapc[i] * m_gai[i];
}

void ORYZA::CalDailyCanopyPhotosynthesisRate(int i)
{
	// Gauss weights for three point Gauss
	float gsx[3] = {0.112702, 0.500000, 0.887298};
	float gsw[3] = {0.277778, 0.444444, 0.277778};

	CalDayLengthAndSINB(i);

	// for every day, these variables should be initialized to 0	
	m_gpcdt[i] = 0.f;
	m_rapcdt[i] = 0.f;
	for (int k = 0;k < 3;k++)
	{
		m_hour = 12.f + m_dayL[i] * 0.5f * gsx[k];

		m_gpc[i] = 0.f;
		m_rapc[i] = 0.f;
		CalDirectRadiation(i);
		CalCanopyAssimilationRate(i);

		m_gpcdt[i] += m_gpc[i] * gsw[k];
		m_rapcdt[i] += m_rapc[i] * gsw[k];
	}
	// Integration of instantaneous assimilation/absorption rate to a daily total (GPCDT/RAPCDT)
	m_gpcdt[i] = m_gpcdt[i] * m_dayL[i];
	m_rapcdt[i] = m_rapcdt[i] * m_dayL[i] * 3600.f;
	m_dtga[i] = m_gpcdt[i];
}

void ORYZA::CalSpikeletAndGrainRate(int i)
{
	float tincr = 5.f * (1.f - m_frStrsWa[i]) * 1.6f;

	// Spikelet formation between PI and Flowering
	if((m_dvs[i] > 0.65f) && (m_dvs[i] < 1.f))  gnsp = m_gcr[i] * m_spgf;
	else  gnsp = 0;

	// Grain formation from spikelets,compute the reduction factors
	if ((m_dvs[i] > 0.75f) && (m_dvs[i] < 1.2f))
	{
		float ctt = max(0.f, 22.f - (m_tMean[i] - tincr));
		// Accumulated cold degree 
		m_coldTT[i] = m_coldTT[i] + ctt;
	}
	if ((m_dvs[i] > 0.96f) && (m_dvs[i] < 1.2f))
	{
		// Average daily maximum temperature during flowering
		m_tfert[i] += (m_tMax[i] + tincr);
		m_ntfert += 1;
	}

	// there is a second requirement that GRAINS (Fortran logical function whether grains are formed) is true
	if (m_dvs[i] > 1.2f)
	{
		float sf1 = 1.f - (4.6f + 0.054f * pow(m_coldTT[i], 1.56f)) / 100.f;
		sf1 = min(1.f, max(0.f, sf1));
		float xx = m_tfert[i] / m_ntfert;
		float sf2 = 1.f / (1.f + exp(0.853f * (xx - 36.6f)));
		sf2 = min(1.f, max(0.f,sf2));
		float spfert = min(sf1, sf2);
		m_gngr[i] = m_nsp[i] * spfert;
	}
	else  m_gngr[i] = 0;
}

void ORYZA::LAI(int i)
{
	// Actual relative growth rate of leaves ((oCd)-1)
	float rgrl = m_rgrlMX - (1.f - m_frStrsN[i]) * (m_rgrlMX - m_rgrlMN);
	float x = 1.f, testSet = 0.0001f;
	bool flag = false;
	float hu = CalHeatUnitDaily(i);

	// for transplanted rice
	if(m_cropsta[i] < 3.f)
	{
		// seedbed: no drought stress effects in seedbed
		if(m_lai[i] < 1.f)
		{
			m_gLai = m_lai[i] * rgrl * hu;
			m_wlvgExs[i] = m_wlvg[i];
			m_laiExs[i] = m_lai[i];
		}
		else
		{
			float test = abs((m_lai[i] / m_wlvg[i]) - m_sla[i]) / m_sla[i];
			if(test < testSet) flag = true;
			if(flag)  m_gLai = ((m_wlvg[i] + m_rwlvg[i]) * m_sla[i]) - m_lai[i];
			else
			{
				float gLai1 = ((m_wlvg[i] + m_rwlvg[i] - m_wlvgExs[i]) * m_sla[i] + m_laiExs[i]) - m_lai[i];
				float gLai2 = ((m_wlvg[i] + m_rwlvg[i]) * m_sla[i]) - m_lai[i];
				m_gLai = (gLai1 + gLai2) / (x + 1.f);
				x = x + 1.f;
			}
		}		
	}
	else if(m_cropsta[i] == 3)
	{
		// Transplanting effects: dilution and shock-setting
		m_gLai = (m_lai[i] * m_nh * m_nplh / m_nplsb) - m_lai[i];
	}
	else if(m_cropsta[i] == 4)
	{
		// After transplanting: main crop growth
		if (m_ts[i] < (m_tslvtr + m_tshckl)) m_gLai = 0;
		else
		{
			if((m_lai[i] < 1.f) && (m_dvs[i] < 1.f))
			{
				m_gLai = m_frStrsWa[i] * m_lai[i] * rgrl * hu;
				m_wlvgExp[i] = m_wlvg[i];
				m_laiExp[i] = m_lai[i];
			}
			else
			{
				float test = abs((m_lai[i] / m_wlvg[i]) - m_sla[i]) / m_sla[i];
				if(test < testSet) flag = true;
				if(flag)  m_gLai = ((m_wlvg[i] + m_rwlvg[i]) * m_sla[i]) - m_lai[i];
				else
				{
					float gLai1 = ((m_wlvg[i] + m_rwlvg[i] - m_wlvgExp[i]) * m_sla[i] + m_laiExp[i]) - m_lai[i];
					float gLai2 = ((m_wlvg[i] + m_rwlvg[i]) * m_sla[i]) - m_lai[i];
					m_gLai = (gLai1 + gLai2) / (x + 1.f);
					x = x + 1.f;
				}
			}
		}
	}
}

void ORYZA::CalRiceGrowth(int i)
{
	// 1.compute the heat unit and development rate of current day
	float hu = CalHeatUnitDaily(i);
	float dvr = CalDevelopmentRate(i);
	// 2.compute the development stage of current day
	if(m_dvs[i] < 2.f)    // rice growing
	{
		// re-initialize rooting depth at day of transplanting
		if(m_cropsta[i] == 3) m_zrt[i] = m_zrttr; 

		// compute light-use efficiency of a single leaf(EFF) use linear_interp
		float tmpCov = m_tmpsb; 
		float tav = (m_tMax[i] + tmpCov + m_tMin[i]) / 2.f;
		float tavD = (tav + m_tMax[i]) / 2.f;             
		float co2EFF = (1.f - exp(-0.00305f * m_co2 - 0.222f)) / (1.f - exp(-0.00305f * 340.f - 0.222f));
		if(tavD > 0.f && tavD <= 10.f) m_eff = 0.54f * co2EFF; 
		else m_eff = (0.54f - 0.006f * (tavD - 10.f)) * co2EFF;

		// Leaf rolling under drought stress (only for photosynthesis)
		float laiRol = m_lai[i] * (0.5f * m_frStrsWa[i] + 0.5f);
		float ssga = 0.f;
		if(m_dvs[i] >=0.f && m_dvs[i] < 0.9f) ssga = 0.0003f;
		else ssga = 0.0003f - 0.00025f * (m_dvs[i] - 0.9f);
		m_sai = ssga * m_wst[i];
		m_aLAI = laiRol + 0.5f * m_sai;

		CalDailyCanopyPhotosynthesisRate(i);

		float pari1 = m_rapcdt[i] / 1.e6;
		float dpari = m_rapcdt[i] / 1.e6;
		// compute the daily incoming photosynthetically active radiation
		float dpar = m_frpar * m_SR[i];
		// Unrolling of ALAI again
		m_aLAI = m_lai[i] + 0.5f * m_sai;
		// drought stress will decreases the rate 
		m_dtga[i] = m_dtga[i] * m_frStrsWa[i];
		//compute the fraction of dry matter to the shoots(FSH), leave(FLV), stems(FST), panicle(FSO), root(FRT), leaf death(DRLV)
		if(m_dvs[i] > 0.f && m_dvs[i] <= 1.f)
		{
			m_fsh = m_aFsh + m_bFsh * m_dvs[i];
			if(m_dvs[i] < 0.5f)  m_flv = m_aFlv;
			else m_flv = m_aFlv - m_bFlv * (m_dvs[i] - 0.5f);			
			if(m_dvs[i] < 0.75f){
				m_fst = 1.f - m_flv;
				m_fso = 0.f;
			}
			else{
				m_fst = 0.7f - (m_dvs[i] - 0.75f);
				m_fso = 1.f - m_flv - m_fst;
			}
			m_drlv = 0.f;
		}
		else if(m_dvs[i] < 1.2f)
		{
			m_fsh = 1.f;
			m_flv = 0.f;
			m_fst = 1.f - m_aFso - m_bFso * (m_dvs[i] - 1.f);
			m_fso = 1.f - m_fst;
			m_drlv = m_aDrlv + m_bDrlv * (m_dvs[i] - 1.f);
		}
		else
		{
			m_fsh = 1.f;
			m_flv = 0.f;
			m_fst = 0.f;
			m_fso = 1.f;
			m_drlv = m_aDrlv + m_bDrlv * (m_dvs[i] - 1.f);
		}
		m_frt = 1.f - m_fsh;
		// compute the loss of leaves and stems
		if(m_ricePlantN[i] >= 0.f && m_ricePlantN[i] <= 1.f) m_nsllv = 1.f;
		else if(m_ricePlantN[i] < 2.f) m_nsllv = 1.f + 0.2f * (m_ricePlantN[i] - 1.f);
		else m_nsllv = 1.5f;
		// the death or loss rate of leaves
		float llv = m_nsllv * m_wlvg[i] * m_drlv; 
		if(m_dvs[i] > 1.f)  m_lstr = m_wstr[i] / m_tclstr;
		else m_lstr = 0.f;
		// Maintenance requirements
		m_teff = pow(m_q10, (tav - m_tref) / 10.f);
		float mnDVS = m_wlvg[i] / (m_wlvg[i] + m_wlvd[i]);
		float rmcr = (m_wlvg[i] * m_mainLV + m_wst[i] * m_mainST + m_wso[i] * m_mainSO + m_wrt[i] * m_mainRT) * m_teff * mnDVS;
		// Carbohydrate requirement for dry matter production
		float crGCR = m_fsh * (m_crgLV * m_flv + m_crgST * m_fst * (1.f - m_fstr) + m_crgSTR * m_fstr * m_fst + m_crgSO * m_fso) + m_crgRT * m_frt;
		// Gross and net growth rate of crop (GCR, NGCR)
		m_gcr[i] = ((m_dtga[i] * 30.f / 44.f) - rmcr + (m_lstr * m_lrstr * m_fcSTR * 30.f / 12.f)) / crGCR;
		float xx = m_gcr[i] - m_lstr * m_lrstr *m_fcSTR * 30.f / 12.f;
		float ngcr = max(0, xx);
		// Intermediate variable for planting density, used to calculate the reduction in net weight 
		float pltr = 1.f;
		if(m_cropsta[i] == 3.f)  pltr = m_nplh * m_nh / m_nplsb;
		// Growth rates of crop organs at transplanting
		float rwlvg1 = m_wlvg[i] * (1.f - pltr);
		float gst1 = m_wsts[i] * (1.f - pltr);
		float rwstr1 = m_wstr[i] * (1.f - pltr);
		float grt1 = m_wrt[i] * (1.f - pltr);
		// Growth rates of crop organs
		float grt = m_gcr[i] * m_frt - grt1;
		glv = m_gcr[i] * m_fsh * m_flv - rwlvg1;
		float rwlvg = glv - llv;
		gst = m_gcr[i] * m_fsh * m_fst * (1.f - m_fstr) - gst1;
		float gstr = m_gcr[i] * m_fsh * m_fstr * m_fst - rwstr1;
		float rwstr = gstr - m_lstr;
		gso = m_gcr[i] * m_fsh * m_fso;
		float ggr = 0.f;
		if(m_dvs[i] > 0.95) ggr = gso;

		CalSpikeletAndGrainRate(i);

		m_sla[i] = m_aSLA + m_bSLA * exp(m_cSLA * (m_dvs[i] - m_dSLA));
		m_sla[i] = min(m_slaMX, m_sla[i]);
		LAI(i);

		// Growth respiration of the crop (RGCR)
		float co2RT = 44.f / 12.f * (m_crgRT *12.f / 30.f - m_fcRT);
		float co2LV = 44.f / 12.f * (m_crgLV *12.f / 30.f - m_fcLV);
		float co2ST = 44.f / 12.f * (m_crgST *12.f / 30.f - m_fcST);
		float co2STR = 44.f / 12.f * (m_crgSTR *12.f / 30.f - m_fcSTR);
		float co2SO = 44.f / 12.f * (m_crgSO *12.f / 30.f - m_fcSO);
		float rgcr = (grt + grt1) * co2RT + (glv + rwlvg1) * co2LV + (gst + gst1) * co2ST + gso * co2SO + 
			(gstr +rwstr1) * co2STR + (1.f - m_lrstr) * m_lstr * m_fcSTR * 44.f / 12.f;
		float ctrans = rwlvg1 * m_fcLV + gst1 * m_fcST + rwstr1 * m_fcSTR +grt1 * m_fcRT;
		float rtnass = ((m_dtga[i] * 30.f / 44.f - rmcr) * 44.f / 30.f) - rgcr - (ctrans * 44.f / 12.f);

		// update the state variables like dvs, lai, wso, wrr	
		// Integrate rate variables
		m_ts[i] += hu;
		m_wlvg[i] += rwlvg;
		m_wlvd[i] += llv;
		m_wsts[i] += gst;
		m_wstr[i] += rwstr;
		m_wso[i] += gso; // storage organs
		m_wrt[i] += grt; // root
		m_wrr[i] += ggr; // final yield
		m_ngr[i] += m_gngr[i];
		m_nsp[i] += gnsp;
		m_tnass[i] += rtnass;
		// Calculate sums of states
		m_wst[i] = m_wstr[i] + m_wsts[i]; // stem
		m_wlv[i] = m_wlvg[i] + m_wlvd[i]; // leaf
		m_wagt[i] = m_wlv[i] + m_wst[i] + m_wso[i]; // dry weight above ground
		/*m_biomass[i] = m_wagt[i] + m_wrt[i];
		m_frRoot[i] = m_wrt[i] / m_biomass[i];*/
		// Leaf area index and total area index (leaves + stems)
		m_lai[i] += m_gLai;
		m_aLAI = m_lai[i] + 0.5f * m_sai;
		// Root length
		m_zrt[i] += m_gzrt;
		m_zrt[i] = min(m_zrt[i], m_zrtMCD);		
	}
}

void ORYZA::CalPlantETAndWStress(int i)
{
	// Only stress in main field after day of transplanting
	if(m_cropsta[i] == 4 && m_ppt[i] != 0.f){
		// compute the potential transpiration rate per unit of root length
		float trrm = m_ppt[i] / (m_zrt[i] + 1.0e-10);
		float trw = 0.f, lrav = 0.f, zll = 0.f, leav = 0.f, ldav = 0.f;
		float *fact(NULL), *musc(NULL), *mskpa(NULL);
		Initialize1DArray((int) m_nSoilLayers[i], fact, 0.f);
		Initialize1DArray((int) m_nSoilLayers[i], musc, 0.f);
		Initialize1DArray((int) m_nSoilLayers[i], mskpa, 0.f);
		float trr;

		for (int j = 0; j < (int) m_nSoilLayers[i]; j++){
			if (m_soilStorage[i][j] >= m_soilAWC[i][j]){
				fact[j] = max(0.f, min(1.f, (m_sol_sat[i][j] - m_soilStorage[i][j]) / (m_sol_sat[i][j] - m_soilAWC[i][j])));
				musc[j] = pow(10.f, fact[j] * 2.f);
			}else if (m_soilStorage[i][j] >= m_sol_wpmm[i][j] && (m_soilStorage[i][j] < m_soilAWC[i][j])){
				fact[j] = max(0.f, min(1.f, (m_soilStorage[i][j] - m_sol_wpmm[i][j]) / (m_soilAWC[i][j] - m_sol_wpmm[i][j])));
				musc[j] = pow(10.f, 4.2f - fact[j] * 2.2f);
			}else if (m_soilStorage[i][j] < m_sol_wpmm[i][j]){
				fact[j] = max(0.f, min(1.f, (m_soilStorage[i][j] - 0.01f) / (m_sol_wpmm[i][j] - 0.01f)));
				musc[j] = pow(10.f, 7.f - fact[j] * 2.8f);
			}

			// Root length in each soil layer
			float zrtl = min(m_soilThick[i][j]/1000.f, max(m_zrt[i] - zll/1000.f, 0.f));

			mskpa[j] = musc[j] / 10.f;
			// Leaf-rolling factor	
			float lr = (log10(mskpa[j]) - log10(m_llls)) / (log10(m_ulls) - log10(m_llls));
			lr = max(0.f, min(1.f, lr));
			lrav = lrav + zrtl / m_zrt[i] * lr;	
			// Relative leaf expansion rate factor
			float le = (log10(mskpa[j]) - log10(m_llle)) / (log10(m_ulle) - log10(m_llle));
			le = max(0.f, min(1.f, le));
			leav = leav + zrtl / m_zrt[i] * le;
			// Relative death rate factor
			float ld = (log10(mskpa[j]) - log10(m_lldl)) / (log10(m_uldl) - log10(m_lldl));
			ld = max(0.f, min(1.f, ld));
			ldav = ldav + zrtl / m_zrt[i] * ld;

			// Relative transpiration ratio (actual/potential)
			if (mskpa[j] >= 10000.f){
				trr = 0.f;
			}else{
				trr = 2.f / (1.f + exp(0.003297f * mskpa[j]));
			}
			trr = max(0.f, min(1.f, trr));

			float wla = max(0.f, (m_soilStorage[i][j] - m_sol_wpmm[i][j]) * zrtl * 1000.f);
			float trwl = min(wla, trr * zrtl * trrm);
			trw = trw + trwl;
			zll = zll + m_soilThick[i][j];
		}
		m_frStrsWa[i] = trw / m_ppt[i];
		m_plantEPDay[i] = trw;

		Release1DArray(fact);
		Release1DArray(musc);
		Release1DArray(mskpa);
	}
	else{
		// If crop is not in the main field, set stress factors at 1
		m_frStrsWa[i] = 1.f;
		m_plantEPDay[i] = 0.f;
	}

}

void ORYZA::CalPlantNUptake(int i)
{
	if(m_cropsta[i] == 4){
		// compute N demand of crop
		float nMaxL, nMinL, nMinSO;
		if (m_dvs[i] < 0.4f) nMaxL = 0.053f;
		else nMaxL = 0.053f - m_nMaxL * (m_dvs[i] - 0.4f);
		nMinL = 0.025f - m_nMinL * m_dvs[i];
		if (m_ancrf[i] < 50.f) nMinSO = 0.006f - m_anMinSO * m_ancrf[i];
		else if(m_ancrf[i] < 400.f) nMinSO = 0.0008f + m_bnMinSO * (m_ancrf[i] - 50.f);
		else nMinSO = 0.017f;
		float LeafDemandN = nMaxL * (m_wlvg[i] + glv) - m_anlv[i];
		float StemDemandN = nMaxL * 0.5f * (m_wst[i] + gst) - m_anst[i];
		float SODemandNMax = m_nMaxSO * gso;
		float SODemandNMin = nMinSO * gso;
		// Calculate translocation of N from organs, No translocation before DVS = 0.95
		float aTNlv, aTNst, aTN, ntso, aTnrt;
		if (m_dvs[i] < 0.95f)
		{
			aTNlv = 0.f;
			aTNst = 0.f;
			aTN = 1.f;
			ntso = 0.f;
			aTnrt = 0.f;
		}
		else
		{
			aTNlv = max(0.f, m_anlv[i] - m_wlvg[i] * m_rfnlv);
			aTNst = max(0.f, m_anst[i] - m_wst[i] * m_rfnst);
			aTnrt = (aTNlv + aTNst) * m_fntrt;
			aTN = aTNlv + aTNst + aTnrt;
			ntso = aTN / m_tcntrf;
			ntso = max(ntso, SODemandNMin);
			ntso = min(ntso, SODemandNMax);
		}
		// Actual N translocation rates from plant organs, ATN should not be 0
		float ntlv = ntso * aTNlv / aTN;
		float ntst = ntso * aTNst / aTN;
		float ntrt = ntso * aTnrt / aTN;
		// Calculate nitrogen uptake
		// float n_reduc = 300.f; /// nitrogen uptake reduction factor (not currently used; defaulted 300.)
		float tnsoil = 0.f;
		for (int k = 0; k < m_nSoilLayers[i]; k++){
			tnsoil += m_soilNO3[i][k];
		}
		// Available N uptake is mimimum of soil supply and maximum crop uptake
		float nupp = min(8.f, tnsoil);
		float totNDemand = (LeafDemandN + ntlv) + (StemDemandN + ntst) + (SODemandNMax - ntso);
		float naLV = max(0.f, min(LeafDemandN + ntlv, nupp * (LeafDemandN + ntlv) / totNDemand));
		float naST = max(0.f, min(StemDemandN + ntst, nupp * (StemDemandN + ntst) / totNDemand));
		float naSO = max(0.f, min(SODemandNMax - ntso, nupp* (SODemandNMax - ntso) / totNDemand));	
		m_anst[i] += (naST - ntst);
		m_plantUpTkN[i] = naLV + naST + naSO;
		if (m_plantUpTkN[i] < 0.f) m_plantUpTkN[i] = 0.f;
		m_ricePlantN[i] += m_plantUpTkN[i];
		// take up N from soil
		float tmp = m_plantUpTkN[i];
		for (int l = 0; l < (int)m_nSoilLayers[i]; l++)
		{
			float uno3l = min(tmp, m_soilNO3[i][l]);
			m_soilNO3[i][l] -= uno3l;
			tmp -= uno3l;
		}
		float nstan = 0.f, nlvan = 0.f;
		if(m_dvs[i] < 1.f)
		{
			nstan = naST - ntst;
			nlvan = naLV - ntlv;
		}
		m_ancrf[i] += (nstan + nlvan);
		m_anlv[i] += naLV;
		float fnlv = max(m_anlv[i] / m_wlvg[i], 0.5 * m_nMinL);
		m_frStrsN[i] = (fnlv - 0.9 * nMaxL) / (nMaxL - 0.9 * nMaxL);
		m_frStrsN[i] = min(max(m_frStrsN[i], 0), 1.f);
	}
	else{
		m_frStrsN[i] = 1.f;
	}
	
}

void ORYZA::PlantPhosphorusUptake(int i)
{
	float uobp = PGCommon::getNormalization(m_PUpDis);
	/// Update accumulated heat units for the plant
	float delg = 0.f;
	if (m_PHUPlt[i] > 0.1)
		delg = (m_tMean[i] - m_tBase[i]) / m_PHUPlt[i];
	if (delg < 0.f)
		delg = 0.f;
	m_frPHUacc[i] += delg;
	//// determine shape parameters for plant phosphorus uptake equation, from readplant.f
	m_frPlantP[i] = PGCommon::NPBiomassFraction(m_frPlantP1[i], m_frPlantP2[i], m_frPlantP3[i], m_frPHUacc[i]);
	float up2 = 0.f; /// optimal plant phosphorus content
	float uapd = 0.f; /// plant demand of phosphorus
	float upmx = 0.f; /// maximum amount of phosphorus that can be removed from the soil layer
	float uapl = 0.f; /// amount of phosphorus removed from layer
	float gx = 0.f; /// lowest depth in layer from which phosphorus may be removed
	up2 = m_frPlantP[i] * (m_wagt[i] + m_wrt[i]);
	if (up2 < m_plantP[i]) up2 = m_plantP[i];
	uapd = up2 - m_plantP[i];
	uapd *= 1.5f;   /// luxury p uptake
	m_frStrsP[i] = 1.f;
	int ir = 0;
	if (uapd < UTIL_ZERO) return;
	for (int l = 0; l < m_nSoilLayers[i]; l++)
	{
		if (ir > 0) break;
		if (m_zrt[i] <= m_soilDepth[i][l])
		{
			gx = m_zrt[i];
			ir = 1;
		}
		else
			gx = m_soilDepth[i][l];
		upmx = uapd * (1.f - exp(-m_PUpDis * gx / m_zrt[i])) / uobp;
		uapl = min(upmx - m_plantUpTkP[i], m_soilPsol[i][l]);
		m_plantUpTkP[i] += uapl;
		m_soilPsol[i][l] -= uapl;
	}
	if (m_plantUpTkP[i] < 0.f) m_plantUpTkP[i] = 0.f;
	m_plantP[i] += m_plantUpTkP[i];
	/// compute phosphorus stress
	PGCommon::calPlantStressByLimitedNP(m_plantP[i], up2, &m_frStrsP[i]);
}

int ORYZA::Execute()
{
	CheckInputData();
	initialOutputs();

//#pragma omp parallel for
	for (int i = 0; i < m_nCells; i++)
	{
		/// calculate albedo in current day
		float cej = -5.e-5f, eaj = 0.f;
		eaj = exp(cej * (m_sol_cov[i] + 0.1f));
		if (m_snowAcc[i] < 0.5f)
		{
			m_albedo[i] = m_soilALB[i];
			if (m_lai[i] > 0.f)
				m_albedo[i] = 0.23f * (1.f - eaj) + m_soilALB[i] * eaj;
		}
		else
			m_albedo[i] = 0.8f;
		/// calculate residue on soil surface for current day
		m_sol_cov[i] = max((m_wagt[i] + m_sol_rsd[i][0]), 0.f);		

		if (m_cropsta[i] > 0.f)      /// rice growing
		{
			float day = JulianDay(this->m_date);
			if (day == 200){
				bool flag = true;
			}
			/// calculate transpiration and water stress
			CalPlantETAndWStress(i);
			/// calculate rice growth, dvs, yield .et
			CalRiceGrowth(i);
			/// calculate N uptake
			CalPlantNUptake(i);
			PlantPhosphorusUptake(i);

			/// update crop status code for transplanted rice
			if (m_cropsta[i] == 1.f){
				m_sowDay[i] = JulianDay(this->m_date);
			}
			
			if (day < (m_sowDay[i] + m_sbdur)){
				m_cropsta[i] = 2.f;
			}
			else if (day == (m_sowDay[i] + m_sbdur)){
				m_cropsta[i] = 3.f;
			}
			else m_cropsta[i] = 4.f;
		}
		/*if (i == 70){
		ofstream fout;
		fout.open("j:\\cropsta.txt", ios::app);
		fout << m_cropsta[70] << "\n";
		fout << flush;
		fout.close();
		}*/
	}
	return true;
}

bool ORYZA::CheckInputData(void)
{
	/// DT_Single
	if (m_date <= 0) throw ModelException(MID_BIO_EPIC, "CheckInputData", "You have not set the time.");
	if (m_nCells <= 0)
		throw ModelException(MID_RICEGROW, "CheckInputData",
		"The dimension of the input data can not be less than zero.");
	if (m_soilLayers <= 0)
		throw ModelException(MID_RICEGROW, "CheckInputData",
		"The layer number of the input 2D raster data can not be less than zero.");
	if (FloatEqual(m_co2, NODATA_VALUE))
		throw ModelException(MID_RICEGROW, "CheckInputData",
		"The ambient atmospheric CO2 concentration must be provided.");
	/// DT_Raster1D
	if (m_tMin == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData", "The min temperature data can not be NULL.");
	if (m_tMax == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData", "The max temperature data can not be NULL.");
	if (m_tMean == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData", "The mean temperature data can not be NULL.");
	if (m_SR == NULL) throw ModelException(MID_BIO_EPIC, "CheckInputData", "The solar radiation data can not be NULL.");
	if (m_nSoilLayers == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData", "The soil layers data can not be NULL.");
	if (m_soilZMX == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData",
		"The maximum rooting depth in soil profile can not be NULL.");
	if (m_soilALB == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData", "The albedo when soil is moist can not be NULL.");
	if (m_soilStorageProfile == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData",
		"The amount of water stored in the soil profile can not be NULL.");
	if (m_totSoilAWC == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData",
		"The amount of water held in soil profile at field capacity can not be NULL.");
	if (m_totSoilSat == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData",
		"The amount of water held in soil profile at saturation can not be NULL.");
	/// DT_Raster2D
	if (m_soilDepth == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData", "The soil depth data can not be NULL.");
	if (m_soilThick == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData", "The soil thickness data can not be NULL.");
	if (m_soilAWC == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData",
		"The water available to plants in soil layer at field capacity can not be NULL.");
	if (m_soilStorage == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData", "The soil moisture in soil layers can not be NULL.");
	if (m_soilNO3 == NULL)
		throw ModelException(MID_RICEGROW, "CheckInputData",
		"The nitrogen stored in the nitrate pool in soil layer can not be NULL.");
	return true;
}
void ORYZA::initialOutputs()
{
	if(m_albedo == NULL) Initialize1DArray(m_nCells, m_albedo, 0.f);
	if(m_sol_cov == NULL || m_sol_rsd == NULL)
	{
		Initialize1DArray(m_nCells, m_sol_cov, m_sol_rsdin);
		Initialize2DArray(m_nCells, m_soilLayers, m_sol_rsd, 0.f);
#pragma omp parallel for
		for (int i = 0; i < m_nCells; i++)
			m_sol_rsd[i][0] = m_sol_cov[i];
	}
	if (m_frStrsWa == NULL)
		Initialize1DArray(m_nCells, m_frStrsWa, 1.f);
	if(m_frStrsN == NULL)
		Initialize1DArray(m_nCells, m_frStrsN, 1.f);
	if(m_plantEPDay == NULL)
		Initialize1DArray(m_nCells, m_plantEPDay, 0.f);
	if(m_ts == NULL)
		Initialize1DArray(m_nCells, m_ts, 0.f);
	if(m_dayL == NULL)
		Initialize1DArray(m_nCells, m_dayL, 0.f);
	if(m_sinLD == NULL)
		Initialize1DArray(m_nCells, m_sinLD, 0.f);
	if(m_cosLD == NULL)
		Initialize1DArray(m_nCells, m_cosLD, 0.f);
	if(m_dsinbe == NULL)
		Initialize1DArray(m_nCells, m_dsinbe, 0.f);
	if(m_sinb == NULL)
		Initialize1DArray(m_nCells, m_sinb, 0.f);
	if(m_solcon == NULL)
		Initialize1DArray(m_nCells, m_solcon, 0.f);
	if(m_rdpdf == NULL)
		Initialize1DArray(m_nCells, m_rdpdf, 0.f);
	if(m_rdpdr == NULL)
		Initialize1DArray(m_nCells, m_rdpdr, 0.f);
	if(m_wlvg == NULL)
		Initialize1DArray(m_nCells, m_wlvg, 0.01f);
	if(m_wlvd == NULL)
		Initialize1DArray(m_nCells, m_wlvd, 0.f);
	if(m_wsts = NULL)
		Initialize1DArray(m_nCells, m_wsts, 0.f);
	if(m_wstr == NULL)
		Initialize1DArray(m_nCells, m_wstr, 0.f);
	if(m_wso == NULL)
		Initialize1DArray(m_nCells, m_wso, 0.f);
	if(m_wrt == NULL)
		Initialize1DArray(m_nCells, m_wrt, 0.f);
	if(m_wrr == NULL)
		Initialize1DArray(m_nCells, m_wrr, 0.f);
	if(m_ngr == NULL)
		Initialize1DArray(m_nCells, m_ngr, 0.f);
	if(m_nsp == NULL)
		Initialize1DArray(m_nCells, m_nsp, 0.f);
	if(m_tnass == NULL)
		Initialize1DArray(m_nCells, m_tnass, 0.f);
	if(m_wst == NULL)
		Initialize1DArray(m_nCells, m_wst, 0.f);
	if(m_wlv == NULL)
		Initialize1DArray(m_nCells, m_wlv, 0.f);
	if(m_wagt == NULL)
		Initialize1DArray(m_nCells, m_wagt, 0.f);
	if(m_zrt == NULL)
		Initialize1DArray(m_nCells, m_zrt, 0.f);
	if(m_dvs == NULL)
		Initialize1DArray(m_nCells, m_dvs, 0.f);
	if(m_ancrf == NULL)
		Initialize1DArray(m_nCells, m_ancrf, 0.f);
	if(m_gai == NULL)
		Initialize1DArray(m_nCells, m_gai, 0.f);
	if(m_gaid == NULL)
		Initialize1DArray(m_nCells, m_gaid, 0.f);
	if(m_rapshl ==NULL)
		Initialize1DArray(m_nCells, m_rapshl, 0.f);
	if(m_rapppl ==NULL)
		Initialize1DArray(m_nCells, m_rapppl, 0.f);
	if(m_fslla ==NULL)
		Initialize1DArray(m_nCells, m_fslla, 0.f);
	if(m_gpl ==NULL)
		Initialize1DArray(m_nCells, m_gpl, 0.f);
	if(m_rapl ==NULL)
		Initialize1DArray(m_nCells, m_rapl, 0.f);
	if(m_gpc ==NULL)
		Initialize1DArray(m_nCells, m_gpc, 0.f);
	if(m_rapc ==NULL)
		Initialize1DArray(m_nCells, m_rapc, 0.f);
	if(m_gpcdt ==NULL)
		Initialize1DArray(m_nCells, m_gpcdt, 0.f);
	if(m_rapcdt ==NULL)
		Initialize1DArray(m_nCells, m_rapcdt, 0.f);
	if(m_dtga ==NULL)
		Initialize1DArray(m_nCells, m_dtga, 0.f);
	if (m_ricePlantN == NULL)
		Initialize1DArray(m_nCells, m_ricePlantN, 0.f);
	if (m_gcr == NULL)
		Initialize1DArray(m_nCells, m_gcr, 0.f);
	if (m_wsts == NULL)
		Initialize1DArray(m_nCells, m_wsts, 0.01f);
	if (m_anlv == NULL)
		Initialize1DArray(m_nCells, m_anlv, 0.f);
	if (m_anst == NULL)
		Initialize1DArray(m_nCells, m_anst, 0.f);
	if (m_coldTT == NULL)
		Initialize1DArray(m_nCells, m_coldTT, 0.f);
	if (m_tfert == NULL)
		Initialize1DArray(m_nCells, m_tfert, 0.f);
	if (m_gngr == NULL)
		Initialize1DArray(m_nCells, m_gngr, 0.f);
	if (m_sla == NULL)
		Initialize1DArray(m_nCells, m_sla, 0.f);
	if (m_biomass == NULL)
		Initialize1DArray(m_nCells, m_biomass, 0.f);
	if (m_frRoot == NULL)
		Initialize1DArray(m_nCells, m_frRoot, 0.f);
	if (m_plantUpTkN == NULL)
		Initialize1DArray(m_nCells, m_plantUpTkN, 0.f);
	if (m_rwlvg == NULL)
		Initialize1DArray(m_nCells, m_rwlvg, 0.f);
	if (m_sowDay == NULL)
		Initialize1DArray(m_nCells, m_sowDay, 0.f);
	if (m_wlvgExs == NULL)
		Initialize1DArray(m_nCells, m_wlvgExs, 0.f);
	if (m_laiExs == NULL)
		Initialize1DArray(m_nCells, m_laiExs, 0.f);
	if (m_wlvgExp == NULL)
		Initialize1DArray(m_nCells, m_wlvgExp, 0.f);
	if (m_laiExp == NULL)
		Initialize1DArray(m_nCells, m_laiExp, 0.f);
	if (m_frPlantP == NULL)
		Initialize1DArray(m_nCells, m_frPlantP, 0.f);
	if (m_frPHUacc == NULL)
		Initialize1DArray(m_nCells, m_frPHUacc, 0.f);
	if (m_plantP == NULL)
		Initialize1DArray(m_nCells, m_plantP, 0.f);
	if(m_plantUpTkP == NULL)
		Initialize1DArray(m_nCells, m_plantUpTkP, 0.f);
	if (m_frStrsP == NULL)
		Initialize1DArray(m_nCells, m_frStrsP, 1.f);
}
void ORYZA::Get1DData(const char *key, int *n, float **data)
{
	initialOutputs();
	string sk(key);
	*n = m_nCells;
	if (StringMatch(sk, VAR_LAST_SOILRD)) *data = m_lastSoilRootDepth;
	else if (StringMatch(sk, VAR_RICE_PLANT_N)) *data = m_ricePlantN;
	else if (StringMatch(sk, VAR_FR_STRSWTR)) *data = m_frStrsWa;
	else if (StringMatch(sk, VAR_SOL_COV)) *data = m_sol_cov;
	else if (StringMatch(sk, VAR_SOL_SW)) *data = m_soilStorageProfile;
	else if (StringMatch(sk, VAR_SOWDAY)) *data = m_sowDay;
	else if (StringMatch(sk, VAR_CROPSTA)) *data = m_cropsta;
	else if (StringMatch(sk, VAR_ALBDAY)) *data = m_albedo;
	else if (StringMatch(sk, VAR_AET_PLT)) *data = m_plantEPDay;
	else if (StringMatch(sk, VAR_TS)) *data = m_ts;
	else if (StringMatch(sk, VAR_WLVG)) *data = m_wlvg;
	else if (StringMatch(sk, VAR_WLVD)) *data = m_wlvd;
	else if (StringMatch(sk, VAR_WSTS)) *data = m_wsts;
	else if (StringMatch(sk, VAR_WSTR)) *data = m_wstr;
	else if (StringMatch(sk, VAR_WSO)) *data = m_wso;
	else if (StringMatch(sk, VAR_WRT)) *data = m_wrt;
	else if (StringMatch(sk, VAR_WRR)) *data = m_wrr;
	else if (StringMatch(sk, VAR_NGR)) *data = m_ngr;
	else if (StringMatch(sk, VAR_NSP)) *data = m_nsp;
	else if (StringMatch(sk, VAR_TNASS)) *data = m_tnass;
	else if (StringMatch(sk, VAR_WST)) *data = m_wst;
	else if (StringMatch(sk, VAR_WLV)) *data = m_wlv;
	else if (StringMatch(sk, VAR_WAGT)) *data = m_wagt;
	else if (StringMatch(sk, VAR_ZRT)) *data = m_zrt;
	else if (StringMatch(sk, VAR_BIOMASS)) *data = m_biomass;
	else if (StringMatch(sk, VAR_DVS)) *data = m_dvs;
	else if (StringMatch(sk, VAR_ANCRF)) *data = m_ancrf;
	else if (StringMatch(sk, VAR_FR_PLANT_P)) *data = m_frPlantP;
	else if (StringMatch(sk, VAR_FR_PHU_ACC)) *data = m_frPHUacc;
	else if (StringMatch(sk, VAR_PLANT_P)) *data = m_plantP;
	else
		throw ModelException(MID_RICEGROW, "Get1DData", "Result " + sk +
		" does not exist in current module. Please contact the module developer.");
}

void ORYZA::Get2DData(const char *key, int *n, int *col, float ***data)
{
	initialOutputs();
	string sk(key);
	*n = m_nCells;
	*col = m_soilLayers;
	if (StringMatch(sk, VAR_SOL_RSD)) *data = m_sol_rsd;
	else
		throw ModelException(MID_BIO_EPIC, "Get2DData", "Result " + sk + " does not exist.");
}