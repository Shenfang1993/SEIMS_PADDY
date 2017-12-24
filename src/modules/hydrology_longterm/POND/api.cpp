#include <stdio.h>
#include <string>
#include "util.h"
#include "api.h"
#include "pond.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new POND();
}

/// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;
    string res;

    mdi.SetAuthor("ShenFang");
    mdi.SetClass(MCLS_PADDY, MCLSDESC_PADDY);
    mdi.SetDescription(MDESC_POND);
    mdi.SetID(MID_POND);
    mdi.SetName(MID_POND);
    mdi.SetVersion("1.2");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    /// set parameters from database
	mdi.AddParameter(VAR_POND_PARAM, UNIT_NON_DIM, DESC_POND_PARAM, Source_ParameterDB, DT_Pond);
	mdi.AddParameter(VAR_LANDUSE, UNIT_NON_DIM, DESC_LANDUSE, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_POND, UNIT_NON_DIM, DESC_POND, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
	mdi.AddInput(VAR_IRRDEPTH, UNIT_DEPTH_MM, DESC_IRRDEPTH, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_CHST, UNIT_VOL_M3, DESC_CHST, Source_Module, DT_Array1D);
	/// set the output variables
	mdi.AddOutput(VAR_POND_VOL, UNIT_DEPTH_MM, DESC_POND_VOL, DT_Raster1D);

    /// write out the XML file.
    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}