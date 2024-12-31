#pragma once
#include <libcouchbase/couchbase.h>
#include <libcouchbase/error.h>
#include <nlohmann/json.hpp>
#include <nlohmann/detail/input/parser.hpp>
#include "JobQueue.h"


//#include <libcouchbase/couchbase.h>
// 콜백 함수에서 사용할 구조체
//extern class CouchbaseClient* g_pCouchbaseClient;

//typedef struct {
//	std::string value;
//	size_t nvalue;
//} document_content;
//

//static document_content doc_content;


//struct lcb_RESPGET;
//static void get_callback(lcb_INSTANCE*, int cbtype, const lcb_RESPGET* resp);
//std::mutex store_lock;
//std::mutex get_lock;

class CouchbaseClient : public JobQueue
{
private:
	lcb_INSTANCE* instance;
	std::string bucket;
	std::string connectionString;
	std::string username;
	std::string password;


public:
	document_content resultValue;
	CouchbaseClient(const std::string& connStr,
		const std::string& bucket,
		const std::string& user,
		const std::string& pass)
		: connectionString(connStr), bucket(bucket),
		username(user), password(pass) {
		connect();
	}

	//void get_callback(lcb_INSTANCE*, int cbtype, const lcb_RESPGET* resp);

	void connect();

	void upsert(document* doc);

	void get(const std::string key, document* doc);

	void QueryExecute(const std::string query, document doc);

	~CouchbaseClient() {
		if (instance) {
			lcb_destroy(instance);
		}
	}
};

class CouchbaseManager
{
public:
	void Init(int threadCnt)
	{
		for (int32 i = 0; i <= threadCnt; i++)
		{
			connections.push_back(std::make_unique<CouchbaseClient>("couchbase://localhost",
				"default",//"default",
				"admin",
				"552123"));
		}
	}

	CouchbaseClient* GetConnection(int threadId) {


		return connections[threadId-1].get();
	}
private:
	std::vector<std::unique_ptr<CouchbaseClient>> connections;

};
extern shared_ptr<CouchbaseManager> g_CouchbaseManager;


