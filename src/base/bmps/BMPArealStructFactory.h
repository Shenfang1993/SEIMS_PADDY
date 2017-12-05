/*!
 * \brief Areal struct BMP factory
 * \author GAO Huiran
 * \date Feb. 2017
 *
 */
#pragma once

#include <stdlib.h>
#include "tinyxml.h"
#include "clsRasterData.h"
#include "BMPFactory.h"

using namespace MainBMP;

namespace MainBMP
{
	class BMPArealStructFactory: public BMPFactory
	{
		public:
			BMPArealStructFactory(int scenarioId, int bmpId, int subScenario, int bmpType, int bmpPriority,
				string distribution, string collection, string location);

			~BMPArealStructFactory(void);

			vector<string> m_BMPparam;

			struct Param
			{
				char BMPName[30];
				int  BMPID;
				char ParamName[30];
				char Method;
				double Value;
			};
			//vector, storage of every parameter value
			vector<struct Param> Params;
			//read Xml file
			bool ReadXmlFile(string& szFileName,string moduleName);

			/// Load BMP parameters from MongoDB
			void loadBMP(mongoc_client_t *conn, string &bmpDBName);

			/// Output
			void Dump(ostream *fs);

			void BMPParametersPreUpdate(map<string, clsRasterData<float>*> rsMap, int nSubbasin, mongoc_gridfs_t *spatialData);

		private:
			/// field index for where to apply the subScenario
			string m_bmpDBName;
			/// cell number
			int m_nCell;
			/// field number
			int m_nField;

            /*
            ** Key is the subScenario Id
            ** Value i
			string m_bmpDBName;       */
            map<int, vector<int>>  m_bmpArealStrOps;

			/// BMP index in each field
			int* m_fieldBMPid;

			clsRasterData<float>* templateRaster;

			/// field raster data
			float* m_fieldmap;

			//! 
			/*!
			 * Get BMP id in each field. 
			 * !!!This function is Not used!!!
			 */
			void getFieldBMPid(map<int, vector<int>>  m_bmpArealStrOps);

			//! 
			/*!
			 * Get field raster data. 
			 * 
			 */
			void readFieldRasterfile(int nSubbasin, mongoc_gridfs_t* spatialData, clsRasterData<float>* templateRaster);

			//! 
			/*!
			 * Update 1D parameters. 
			 * 
			 */
			void Update(string paraName, int n, float* data, int subScenarioid, vector<int> location);

			//! 
			/*!
			 * Update 2D parameters. 
			 * 
			 */
			void Update2D(string paraName, int n, int lyr, float** data2D, int subScenarioid, vector<int> location);

	};
}
