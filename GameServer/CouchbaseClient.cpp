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
	if (lcb_respquery_is_final(resp)) { // lcb_respquery_is_final 사용
		lcb_STATUS rc = lcb_respquery_status(resp); // lcb_respquery_status 사용
		if (rc != LCB_SUCCESS) {
			std::cerr << "Query failed: " << lcb_strerror_short(rc) << std::endl;

			lcb_respquery_error_context(resp, &errstr); // lcb_respquery_error 사용
			if (errstr) {
				std::cerr << "Error string: " << errstr << std::endl;
			}
			return;
		}
	}

	const char* row;
	size_t nrow;
	lcb_respquery_cookie(resp, &cookie);
	lcb_respquery_row(resp, &row, &nrow); // lcb_respquery_row 사용

	if (row) {
		try {
			json j = json::parse(row);
			if (j.empty())
			{
				throw std::runtime_error("Json File is empty"); // 파일이 비어있을 경우 예외 발생
			}
			if (j.is_array() == false)
			{
				throw std::runtime_error("Json File is not array"); // 파일이 비어있을 경우 예외 발생
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
			//		if (j[0]["$1"].get<bool>() == true) //해당 id키값 존재
			//		{
			//			doc_new.type = DB::PLAYER_DATA_LOAD;
			//			doc_new.value = "SELECT* FROM `default` USE KEYS [\"" + doc->key + "\"]);";
			//
			//			g_CouchbaseManager->GetConnection(doc->threadID)->QueryExecute(doc_new.value, doc_new);
			//			//요청해 get 해오면 결과물 플레이어 및 클라로 전달
			//
			//		}
			//		else
			//		{
			//			//새로 플레이어정보 생성시,
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
	//std::lock_guard<std::mutex> lock(store_lock); // 뮤텍스로 공유 데이터 접근 보호

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
			std::cerr << "CAS 불일치! 다른 클라이언트가 문서를 수정했습니다." << std::endl;

			document doc;
			doc.cas = cas;
			doc.key = context->key;
			doc.value = context->value;

			//cas값 새로 받아서 업뎃 재시도
			CouchbaseClient* pDBConnect = g_CouchbaseManager->GetConnection(LThreadId);
			pDBConnect->DoAsync(&CouchbaseClient::upsert, doc);
			/*

			*/
		}
		return;
	}
	/*
		업뎃 성공

	
	
	*/



	//	printf("status: %s, key: %.*s, CAS: 0x%" PRIx64 "\n",
			//lcb_strerror_short(lcb_respstore_status(resp)), (int)nkey, key, cas);
}
static void get_callback(lcb_INSTANCE*, int cbtype, const lcb_RESPGET* resp)
{
	//std::lock_guard<std::mutex> lock(get_lock); // 뮤텍스로 공유 데이터 접근 보호
	CouchbaseClient* pDBConnect = g_CouchbaseManager->GetConnection(LThreadId);

	size_t nvalue;
	const char* value;
	uint64_t caskey = 0;
	void* cookie = nullptr;
	auto status = lcb_respget_status(resp);

	lcb_respget_cookie(resp, &cookie);
	auto cas_status = lcb_respget_cas(resp, &caskey);	//문서버전 키 가져오기
	lcb_respget_value(resp, &value, &nvalue);

	document* context = static_cast<document*>(cookie);
	document doc;

	if (status != LCB_SUCCESS) {
		if (status == LCB_ERR_DOCUMENT_NOT_FOUND)
		{
		}

		fprintf(stderr, "Failed to get document: %s\n", lcb_strerror_short(status));
		if (cas_status == LCB_ERR_CAS_MISMATCH) {
			std::cerr << "CAS 불일치! 다른 클라이언트가 문서를 수정했습니다." << std::endl;

			//document doc;
			document* context = static_cast<document*>(cookie);
			doc.cas = caskey;
			doc.key = context->key;
			//doc.value = context->value;

			//cas값 새로 받아서 업뎃 재시도
			//CouchbaseClient* pDBConnect = g_CouchbaseManager->GetConnection(LThreadId);
			pDBConnect->DoAsync(&CouchbaseClient::get, doc.key, doc);
		}
		return;
	}

	/*
	 get한 결과값을 큐로 넘겨서,다른 스레드에서 받아서 처리할수 있게
	 쿠키로 받아온 document문서의 내용에 따라서, 어떤 클래스가 해당 내용을 받아서 저장해야하는지도
	 결정해야함.

	 타입을 알면?
	 타입에 따라 처리할 함수를 호출

	*/
	doc.cas = caskey; //db로부터 cas값 받아온후에 각 유저별로 저장해야할듯.유저 문서라서
	doc.key = context->key;
	doc.type = context->type;

	string result= std::string(value, nvalue);
	//pDBConnect->resultValue.value = std::string(value, nvalue);

	json j = json::parse(value);
	GCouchbaseHandler->HandleDBJob(doc, j);

	//j.get_ptr<>
	//PlayerInfo s= j.get<PlayerInfo>();
	
	//context->type

	////// 기존 메모리 해제
	//if (doc_content.value.empty()==false) {
	//	doc_content.value.clear();
	//	//free(doc_content.value);
	//}

	//// 새로운 값 복사
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
			lcb_cmdstore_cas(cmd, doc.cas);//이전에 처음문서

		lcb_STATUS status = lcb_store(instance, &doc, cmd);
		lcb_cmdstore_destroy(cmd);

		if (status != LCB_SUCCESS) {
			std::cerr << "Couchbase 문서 업데이트 실패: " << std::string(lcb_strerror_short(status)) << std::endl;

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
	// N1QL 쿼리 실행
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