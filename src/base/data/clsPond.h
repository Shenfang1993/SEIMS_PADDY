/*!
 * \file clsPond.h
 * \brief Class to store reach related parameters from Pond table
 *
 * \author Shen fang
 * \version 1.0
 * \date 2017.12
 *
 */
#pragma once

#include <string>
#include "MetadataInfoConst.h"
#include "mongoc.h"
#include "MongoUtil.h"
#include "ModelException.h"
#include <map>

using namespace std;

/*!
* \ingroup data
 * \class clsPond
 * \brief Class to store reach related parameters from Pond table
 */

class clsPond
{
public:
	clsPond(void);
	~clsPond(void);
};

class clsPonds
{
public:
	/*!
     * \brief Constructor
     *
     * Query pond table from MongoDB
     *
     * \param[in] conn mongoc client instance
     * \param[in] dbName Database name
     * \param[in] collectionName Pond collection name
     */
	clsPonds(mongoc_client_t *conn, string &dbName, string collectionName);

	/// Destructor
	~clsPonds();

	/// Get pond number
	int GetPondNumber() { return this->m_pondNum; }

private:
	/// ponds number
	int m_pondNum;
};