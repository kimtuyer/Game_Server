#include "pch.h"

#include "CouchbaseClient.h"
#include "CouchbaseHandler.h"
#include "ConsoleMapViewer.h"
#ifdef _DEBUG
#pragma commment (lib,"libcouchbase_d.lib")
#else
#pragma comment (lib,"libcouchbase.lib")
#endif

//CouchbaseClient* g_pCouchbaseClient = NULL;
shared_ptr<CouchbaseManager> g_CouchbaseManager = make_shared<CouchbaseManager>();
using json = nlohmann::json;
std::mutex cookie_mutex; // 뮤텍스

static void query_callback(lcb_INSTANCE* instance, int cbtype, const lcb_RESPQUERY* rb) {
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
	string ret = string(row, nrow);
	if (row) {
		try {

			json j = json::parse(ret);
			if (j.empty())
			{
				throw std::runtime_error("Json File is empty"); // 파일이 비어있을 경우 예외 발생
			}

			document* doc = static_cast<document*>(cookie);
			document doc_new;
			doc_new.key = doc->key;
			doc_new.type = doc->type;
			doc_new.threadID = doc->threadID;

			GCouchbaseHandler->HandleDBJob(doc_new, j,ret);
			

			
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
	int nowtime = GetTickCount64();

	const char* key;
	size_t nkey;
	uint64_t cas;
	void* cookie = nullptr;

	lcb_respstore_key(resp, &key, &nkey);
	lcb_respstore_cookie(resp, &cookie);
	auto cas_status = lcb_respstore_cas(resp, &cas);
	auto status = lcb_respstore_status(resp);
	document* context = static_cast<document*>(cookie);
	document doc;
	if (status != LCB_SUCCESS)
	{
		fprintf(stderr, "Failed to get document: %s\n", lcb_strerror_short(status));
		//lcb_strerror_short(lcb_respstore_status(resp)), (int)nkey, key, cas));
		if (cas_status == LCB_ERR_CAS_MISMATCH) {
			std::cerr << "CAS 불일치! 다른 클라이언트가 문서를 수정했습니다." << std::endl;

			context->cas = cas;
	
			//cas값 새로 받아서 업뎃 재시도
			CouchbaseClient* pDBConnect = g_CouchbaseManager->GetConnection(LThreadId);
			pDBConnect->DoAsync(&CouchbaseClient::upsert, context);
			/*

			*/
		}
		return;
	}

	//int RTT = context->sendTime - nowtime;
	//cout << "DB RTT:" << RTT;
	doc.cas = cas; //db로부터 cas값 받아온후에 각 유저별로 저장해야할듯.유저 문서라서
	doc.key = context->key;
	doc.type = context->type;
	doc.value = context->value;
	string ret = "0";
	json j = json::parse(ret);
	GCouchbaseHandler->HandleDBJob(doc, j, ret);
	delete context;
	//GConsoleViewer->DBRTT.push_back(RTT);
	/*
		업뎃 성공
	*/
	
}
static void get_callback(lcb_INSTANCE*, int cbtype, const lcb_RESPGET* resp)
{
	CouchbaseClient* pDBConnect = g_CouchbaseManager->GetConnection(LThreadId);

	size_t nvalue;
	const char* value;
	uint64_t caskey = 0;
	auto status = lcb_respget_status(resp);
	void* cookie = nullptr;

	{
		std::lock_guard<std::mutex> lock(cookie_mutex); // 뮤텍스 잠금
		lcb_respget_cookie(resp, &cookie);
	}
	document* context = reinterpret_cast<document*>(cookie);
	auto cas_status = lcb_respget_cas(resp, &caskey);	//문서버전 키 가져오기
	lcb_respget_value(resp, &value, &nvalue);

	document doc;

	if (status != LCB_SUCCESS) {
		
		fprintf(stderr, "Failed to get document: %s\n", lcb_strerror_short(status));
		if (cas_status == LCB_ERR_CAS_MISMATCH) {
			std::cerr << "CAS 불일치! 다른 클라이언트가 문서를 수정했습니다." << std::endl;

			document* context = static_cast<document*>(cookie);
			context->cas = caskey;
		
			//cas값 새로 받아서 업뎃 재시도
			pDBConnect->DoAsync(&CouchbaseClient::get, context->key, context);
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

	//int RTT = context->sendTime - GetTickCount64();
	//cout << "DB RTT:" << RTT;
	//GConsoleViewer->DBRTT.push_back(RTT);


	doc.cas = caskey; //db로부터 cas값 받아온후에 각 유저별로 저장해야할듯.유저 문서라서
	doc.key = context->key;
	doc.type = context->type;

	string result= std::string(value, nvalue);

	json j = json::parse(result);
	GCouchbaseHandler->HandleDBJob(doc, j, result);
	
	delete context;

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

		lcb_install_callback(instance, LCB_CALLBACK_QUERY, (lcb_RESPCALLBACK)query_callback);
		
		lcb_install_callback(instance, LCB_CALLBACK_GET, (lcb_RESPCALLBACK)get_callback);
		lcb_install_callback(instance, LCB_CALLBACK_STORE, (lcb_RESPCALLBACK)store_callback);
		//lcb_install_callback(instance, LCB_CALLBACK_QUERY, (lcb_RESPCALLBACK)query_callback);

	}
}

void CouchbaseClient::upsert(document* doc)
{
	doc->sendTime = GetTickCount64();
	{
		lcb_CMDSTORE* cmd;
		lcb_cmdstore_create(&cmd, LCB_STORE_UPSERT);
		lcb_cmdstore_key(cmd, doc->key.c_str(), doc->key.size());
		lcb_cmdstore_value(cmd, doc->value.c_str(), doc->value.size());

		if (doc->cas != 0)
			lcb_cmdstore_cas(cmd, doc->cas);//이전에 처음문서
	
		lcb_STATUS status = lcb_store(instance, doc, cmd);
		lcb_cmdstore_destroy(cmd);

		if (status != LCB_SUCCESS) {
			std::cerr << "Couchbase 문서 업데이트 실패: " << std::string(lcb_strerror_short(status)) << std::endl;

			//throw std::runtime_error("Failed to schedule upsert: " +
				//std::string(lcb_strerror_short(status)));
		}

		status = lcb_wait(instance, LCB_WAIT_DEFAULT);
		//status = lcb_wait(instance, LCB_WAIT_DEFAULT);
		if (status != LCB_SUCCESS) {
			throw std::runtime_error("Failed to upsert: " +
				std::string(lcb_strerror_short(status)));
		}
	}
}

void CouchbaseClient::get(const std::string key, document* doc)
{
	CouchbaseClient* pDBConnect = g_CouchbaseManager->GetConnection(LThreadId);

	doc->sendTime = GetTickCount64();

	lcb_CMDGET* cmd;
	lcb_cmdget_create(&cmd);
	lcb_cmdget_key(cmd, key.c_str(), key.size());
	lcb_STATUS status;
	{
		std::lock_guard<std::mutex> lock(cookie_mutex); // 뮤텍스 잠금
		status = lcb_get(instance, doc, cmd);
	}
	lcb_cmdget_destroy(cmd);

	if (status != LCB_SUCCESS) {
		throw std::runtime_error("Failed to schedule get: " +
			std::string(lcb_strerror_short(status)));
	}

	status = lcb_wait(instance, LCB_WAIT_DEFAULT);

	//status = lcb_wait(instance, eWaitType);
	if (status != LCB_SUCCESS) {
		throw std::runtime_error("Failed to get: " +
			std::string(lcb_strerror_short(status)));
	}
}

void CouchbaseClient::QueryExecute(const std::string query, document doc)
{
	
	lcb_CMDQUERY* qrycmd;
	auto rc=lcb_cmdquery_create(&qrycmd);
	if (rc != LCB_SUCCESS)
	{
		return;
	}
	
	rc=lcb_cmdquery_statement(qrycmd, query.c_str(), query.length());
	if (rc != LCB_SUCCESS)
	{
		return;
	}

	lcb_cmdquery_callback(qrycmd, query_callback);

	rc = lcb_query(instance, &doc, qrycmd);
	lcb_cmdquery_destroy(qrycmd);

	if (rc != LCB_SUCCESS) {
		std::cerr << "Failed to schedule query: " << lcb_strerror_short(rc) << std::endl;
		//lcb_destroy(instance);
		return;
	}

	//rc = lcb_tick_nowait(instance);
	rc = lcb_wait(instance, LCB_WAIT_DEFAULT);
	if (rc != LCB_SUCCESS) {
		throw std::runtime_error("Failed to get: " +
			std::string(lcb_strerror_short(rc)));
	}
}