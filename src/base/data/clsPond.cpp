/*!
 * \file clsPond.h
 * \brief Class to store reach related parameters from Pond table
 *
 * \author Shen fang
 * \version 1.0
 * \date 2017.12
 *
 */
#include "util.h"
#include "clsPond.h"

using namespace std;



clsPond::clsPond(void)
{
}

clsPond::~clsPond(void)
{
}

clsPonds::clsPonds(mongoc_client_t *conn, string &dbName, string collectionName){
	bson_t *b = bson_new();
	bson_t *child1 = bson_new();
	bson_t *child2 = bson_new();
	BSON_APPEND_DOCUMENT_BEGIN(b, "$query", child1);  /// query all records
	bson_append_document_end(b, child1);
	BSON_APPEND_DOCUMENT_BEGIN(b, "$orderby", child2); /// and order by pond ID
	BSON_APPEND_INT32(child2, POND_PONDID, 1);
	bson_append_document_end(b, child2);
	bson_destroy(child1);
	bson_destroy(child2);

	mongoc_cursor_t *cursor;
	const bson_t *bsonTable;
	mongoc_collection_t *collection = NULL;
	bson_error_t *err = NULL;

	collection = mongoc_client_get_collection(conn, dbName.c_str(), collectionName.c_str());
	if (collection == NULL)
		throw ModelException("clsReaches", "ReadAllReachInfo", "Failed to get collection: " + collectionName + ".\n");
	const bson_t *qCount = bson_new();
	this->m_pondNum = (int) mongoc_collection_count(collection, MONGOC_QUERY_NONE, qCount, 0, 0, NULL, err);
}
clsPonds::~clsPonds(void)
{
}
