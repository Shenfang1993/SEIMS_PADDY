/*!
 * \brief Simulates depressional areas that do not drain to the stream network (pothole) and impounded areas such as rice paddies
 * \author Liang-Jun Zhu
 * \date Sep 2016
 *           1. Source code of SWAT include: pothole.f
 *           2. Add the simulation of Ammonia n transported with surface runoff, 2016-9-27
 *           3. Add m_depEvapor and m_depStorage from DEP_LENSLEY module
 *           4. Using a simple model (first-order kinetics equation) to simulate N transformation in impounded area.
 * \data 2016-10-10
 * \description: 1. Update all related variables after the simulation of pothole.
 */
#pragma once

#include <string>
#include "api.h"
#include "util.h"
#include "SimulationModule.h"

using namespace std;

class POND : public SimulationModule
{
private:
	/// pond number
	int m_npond;
	/// pond volumn
	float *m_pondVol;
	/// max pond depth
	float *m_pondVolMax;
	/// 
	float *m_surfaceAreaEM;
	///
	float *m_surfaceAreaPR;
	///
	float *m_volEM;
	///
	float *m_volPR;
	///
	float *m_pondSurfaceArea;
	///
	float m_evap_coe;
	///
	float *m_pondEvap;
	///
	float *m_pondSeep;
	/// conversion factor (mm/ha => m^3)
	float m_cnv; 
    /// valid cells number
    int m_nCells;	
	/// cell width, m
	float m_cellWidth;
	/// cell area, ha
	float m_cellArea;
	
public:
    //! Constructor
    POND(void);

    //! Destructor
    ~POND(void);

    virtual int Execute();

	virtual void SetValue(const char *key, float data);

    virtual void Set1DData(const char *key, int n, float *data);

    virtual void Get1DData(const char *key, int *n, float **data);

    virtual void Set2DData(const char *key, int n, int col, float **data);

private:
    /*!
     * \brief check the input data. Make sure all the input data is available.
     * \return bool The validity of the input data.
     */
    bool CheckInputData(void);

    /*!
     * \brief check the input size. Make sure all the input data have same dimension.
     *
     *
     * \param[in] key The key of the input data
     * \param[in] n The input data dimension
     * \return bool The validity of the dimension
     */
    bool CheckInputSize(const char *, int);
	/*!
     * \brief check the input size of 2D data. Make sure all the input data have same dimension.
     *
     *
     * \param[in] key The key of the input data
     * \param[in] n The first dimension input data 
	 * \param[in] col The second dimension of input data 
     * \return bool The validity of the dimension
     */
    bool CheckInputSize2D(const char *key, int n, int col);
	/// initialize all possible outputs
	void initialOutputs();
	/*!
	 * \brief Simulates depressional areas that do not 
	 * drain to the stream network (pothole) and impounded areas such as rice paddies
	 * reWrite from pothole.f of SWAT
	 */
	void pondSimulate(int id);
	/*!
	 * compute surface area assuming a cone shape, ha
	 */
	void pondSurfaceArea(int id);
	
};
