#include "pch.h"

#include "CouchbaseClient.h"
#include "CouchbaseHandler.h"
#ifdef _DEBUG
#pragma commment (lib,"libcouchbase_d.lib")
#else
#pragma comment (lib,"libcouchbase.lib")
#endif

//CouchbaseClient* g_pCouchbaseClient = NULL;
shared_ptr<CouchbaseManager> g_CouchbaseManager = make_shared<CouchbaseManager>();
using json = nlohmann::json;

static void query_callback(lcb_INSTANCE* instance, int cbtype, const lcb_RESPBASE* rb) {
	if (cbtype != LCB_CALLBACK_QUERY) {
		return;
	}

	const lcb_RESPQUERY* resp = (const lcb_RESPQUERY*)rb;
	void* cookie = nullptr;

	//lcb_respquery_row(resp,)
	const lcb_QUERY_ERROR_CONTEXT* errstr;
	if (lcb_respquery_is_final(resp)) { // lcb_respquery_is_final ���
		lcb_STATUS rc = lcb_respquery_status(resp); // lcb_respquery_status ���
		if (rc != LCB_SUCCESS) {
			std::cerr << "Query failed: " << lcb_strerror_short(rc) << std::endl;

			lcb_respquery_error_context(resp, &errstr); // lcb_respquery_error ���
			if (errstr) {
				std::cerr << "Error string: " << errstr << std::endl;
			}
			return;
		}
	}

	const char* row;
	size_t nrow;
	lcb_respquery_cookie(resp, &cookie);
	lcb_respquery_row(resp, &row, &nrow); // lcb_respquery_row ���

	if (row) {
		try {
			json j = json::parse(row);
			if (j.empty())
			{
				throw std::runtime_error("Json File is empty"); // ������ ������� ��� ���� �߻�
			}
			if (j.is_array() == false)
			{
				throw std::runtime_error("Json File is not array"); // ������ ������� ��� ���� �߻�
			}

			document* doc = static_cast<document*>(cookie);
			document doc_new;
			doc_new.key = doc->key;
			doc_new.threadID = doc->threadID;

			GCouchbaseHandler->HandleDBJob(doc_new, j);
			//if (doc->type == DB::PLAYER_KEY_REQ)
			//{
			//
			//	if (j[0].contains("$1"))
			//	{
			//		bool exists = j[0]["$1"].get<bool>();
			//
			//		if (j[0]["$1"].get<bool>() == true) //�ش� idŰ�� ����
			//		{
			//			doc_new.type = DB::PLAYER_DATA_LOAD;
			//			doc_new.value = "SELECT* FROM `default` USE KEYS [\"" + doc->key + "\"]);";
			//
			//			g_CouchbaseManager->GetConnection(doc->threadID)->QueryExecute(doc_new.value, doc_new);
			//			//��û�� get �ؿ��� ����� �÷��̾� �� Ŭ��� ����
			//
			//		}
			//		else
			//		{
			//			//���� �÷��̾����� ������,
			//			DB::PLAYER_DATA_CREATE
			//
			//		}
			//
			//
			//
			//	}
			//
			//}

			
		}
		catch (json::parse_error& e) {
			std::cerr << "JSON parse error: " << e.what() << "\nbyte position of error: " << e.byte << std::endl;
			std::cerr << "Offending JSON: " << std::string(row, nrow) << std::endl;
		}
	}
}

static void store_callback(lcb_INSTANCE* instance, int cbtype, const lcb_RESPSTORE* resp)
{
	//std::lock_guard<std::mutex> lock(store_lock); // ���ؽ��� ���� ������ ���� ��ȣ

	const char* key;
	size_t nkey;
	uint64_t cas;
	void* cookie = nullptr;

	lcb_respstore_key(resp, &key, &nkey);
	lcb_respstore_cookie(resp, &cookie);
	auto cas_status = lcb_respstore_cas(resp, &cas);
	auto status = lcb_respstore_status(resp);
	document* context = static_cast<document*>(cookie);

	if (status != LCB_SUCCESS)
	{
		fprintf(stderr, "Failed to get document: %s\n", lcb_strerror_short(status));
		//lcb_strerror_short(lcb_respstore_status(resp)), (int)nkey, key, cas));
		if (cas_status == LCB_ERR_CAS_MISMATCH) {
			std::cerr << "CAS ����ġ! �ٸ� Ŭ���̾�Ʈ�� ������ �����߽��ϴ�." << std::endl;

			document doc;
			doc.cas = cas;
			doc.key = context->key;
			doc.value = context->value;

			//cas�� ���� �޾Ƽ� ���� ��õ�
			CouchbaseClient* pDBConnect = g_CouchbaseManager->GetConnection(LThreadId);
			pDBConnect->DoAsync(&CouchbaseClient::upsert, doc);
			/*

			*/
		}
		return;
	}
	/*
		���� ����

	
	
	*/



	//	printf("status: %s, key: %.*s, CAS: 0x%" PRIx64 "\n",
			//lcb_strerror_short(lcb_respstore_status(resp)), (int)nkey, key, cas);
}
static void get_callback(lcb_INSTANCE*, int cbtype, const lcb_RESPGET* resp)
{
	//std::lock_guard<std::mutex> lock(get_lock); // ���ؽ��� ���� ������ ���� ��ȣ
	CouchbaseClient* pDBConnect = g_CouchbaseManager->GetConnection(LThreadId);

	size_t nvalue;
	const char* value;
	uint64_t caskey = 0;
	void* cookie = nullptr;
	auto status = lcb_respget_status(resp);

	lcb_respget_cookie(resp, &cookie);
	auto cas_status = lcb_respget_cas(resp, &caskey);	//�������� Ű ��������
	lcb_respget_value(resp, &value, &nvalue);

	document* context = static_cast<document*>(cookie);
	document doc;

	if (status != LCB_SUCCESS) {
		if (status == LCB_ERR_DOCUMENT_NOT_FOUND)
		{
		}

		fprintf(stderr, "Failed to get document: %s\n", lcb_strerror_short(status));
		if (cas_status == LCB_ERR_CAS_MISMATCH) {
			std::cerr << "CAS ����ġ! �ٸ� Ŭ���̾�Ʈ�� ������ �����߽��ϴ�." << std::endl;

			//document doc;
			document* context = static_cast<document*>(cookie);
			doc.cas = caskey;
			doc.key = context->key;
			//doc.value = context->value;

			//cas�� ���� �޾Ƽ� ���� ��õ�
			//CouchbaseClient* pDBConnect = g_CouchbaseManager->GetConnection(LThreadId);
			pDBConnect->DoAsync(&CouchbaseClient::get, doc.key, doc);
		}
		return;
	}

	/*
	 get�� ������� ť�� �Ѱܼ�,�ٸ� �����忡�� �޾Ƽ� ó���Ҽ� �ְ�
	 ��Ű�� �޾ƿ� document������ ���뿡 ����, � Ŭ������ �ش� ������ �޾Ƽ� �����ؾ��ϴ�����
	 �����ؾ���.

	 Ÿ���� �˸�?
	 Ÿ�Կ� ���� ó���� �Լ��� ȣ��

	*/
	doc.cas = caskey; //db�κ��� cas�� �޾ƿ��Ŀ� �� �������� �����ؾ��ҵ�.���� ������
	doc.key = context->key;
	doc.type = context->type;

	string result= std::string(value, nvalue);
	//pDBConnect->resultValue.value = std::string(value, nvalue);

	json j = json::parse(value);
	GCouchbaseHandler->HandleDBJob(doc, j);

	//j.get_ptr<>
	//PlayerInfo s= j.get<PlayerInfo>();
	
	//context->type

	////// ���� �޸� ����
	//if (doc_content.value.empty()==false) {
	//	doc_content.value.clear();
	//	//free(doc_content.value);
	//}

	//// ���ο� �� ����
	//doc_content.value = malloc(nvalue + 1
	//printf("Document content: %.*s\n", (int)nvalue, value);
}

void CouchbaseClient::connect()
{
	{
		lcb_CREATEOPTS* create_options = nullptr;
		lcb_createopts_create(&create_options, LCB_TYPE_BUCKET);
		lcb_createopts_connstr(create_options, connectionString.c_str(), connectionString.size());
		lcb_createopts_credentials(create_options, username.c_str(), username.size(),
			password.c_str(), password.size());

		lcb_STATUS status = lcb_create(&instance, create_options);
		lcb_createopts_destroy(create_options);

		if (status != LCB_SUCCESS) {
			throw std::runtime_error("Failed to create connection: " +
				std::string(lcb_strerror_short(status)));
		}

		status = lcb_connect(instance);
		if (status != LCB_SUCCESS) {
			throw std::runtime_error("Failed to connect: " +
				std::string(lcb_strerror_short(status)));
		}

		status = lcb_wait(instance, LCB_WAIT_DEFAULT);
		if (status != LCB_SUCCESS) {
			throw std::runtime_error("Failed to wait: " +
				std::string(lcb_strerror_short(status)));
		}

		lcb_install_callback(instance, LCB_CALLBACK_GET, (lcb_RESPCALLBACK)get_callback);
		lcb_install_callback(instance, LCB_CALLBACK_STORE, (lcb_RESPCALLBACK)store_callback);
		lcb_install_callback(instance, LCB_CALLBACK_QUERY, (lcb_RESPCALLBACK)query_callback);
	}
}

void CouchbaseClient::upsert(document doc)
{
	{
		lcb_CMDSTORE* cmd;
		lcb_cmdstore_create(&cmd, LCB_STORE_UPSERT);
		lcb_cmdstore_key(cmd, doc.key.c_str(), doc.key.size());
		lcb_cmdstore_value(cmd, doc.value.c_str(), doc.value.size());

		if (doc.cas != 0)
			lcb_cmdstore_cas(cmd, doc.cas);//������ ó������

		lcb_STATUS status = lcb_store(instance, &doc, cmd);
		lcb_cmdstore_destroy(cmd);

		if (status != LCB_SUCCESS) {
			std::cerr << "Couchbase ���� ������Ʈ ����: " << std::string(lcb_strerror_short(status)) << std::endl;

			//throw std::runtime_error("Failed to schedule upsert: " +
				//std::string(lcb_strerror_short(status)));
		}

		status = lcb_tick_nowait(instance);
		//status = lcb_wait(instance, LCB_WAIT_DEFAULT);
		if (status != LCB_SUCCESS) {
			throw std::runtime_error("Failed to upsert: " +
				std::string(lcb_strerror_short(status)));
		}
	}
}

void CouchbaseClient::get(const std::string key, document doc)
{
	//GetCallback cb;

	lcb_CMDGET* cmd;
	lcb_cmdget_create(&cmd);
	lcb_cmdget_key(cmd, key.c_str(), key.size());

	lcb_STATUS status = lcb_get(instance, &doc, cmd);
	lcb_cmdget_destroy(cmd);

	if (status != LCB_SUCCESS) {
		throw std::runtime_error("Failed to schedule get: " +
			std::string(lcb_strerror_short(status)));
	}

	status = lcb_tick_nowait(instance);

	//status = lcb_wait(instance, eWaitType);
	if (status != LCB_SUCCESS) {
		throw std::runtime_error("Failed to get: " +
			std::string(lcb_strerror_short(status)));
	}
}

void CouchbaseClient::QueryExecute(const std::string query, document doc)
{
	// N1QL ���� ����
	//std::string query = "SELECT * FROM `travel-sample` LIMIT 10;";
	//lcb_CMDGET* cmd;
	lcb_CMDQUERY* qrycmd;
	lcb_cmdquery_create(&qrycmd);
	lcb_cmdquery_statement(qrycmd, query.c_str(), query.length());
	//lcb_cmdquery_callback(qrycmd, query_callback);

	lcb_STATUS rc = lcb_query(instance, &doc, qrycmd);
	lcb_cmdquery_destroy(qrycmd);

	if (rc != LCB_SUCCESS) {
		std::cerr << "Failed to schedule query: " << lcb_strerror_short(rc) << std::endl;
		//lcb_destroy(instance);
		return;
	}

	rc = lcb_wait(instance, LCB_WAIT_DEFAULT);
	if (rc != LCB_SUCCESS) {
		throw std::runtime_error("Failed to get: " +
			std::string(lcb_strerror_short(rc)));
	}
	//lcb_destroy(instance);
}