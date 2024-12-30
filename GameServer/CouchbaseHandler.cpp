#include "pch.h"
#include "CouchbaseHandler.h"
#include "CouchbaseClient.h"
#include "CPlayerManager.h"
#include "Player.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "CZone_Manager.h"
#include "CZone.h"
shared_ptr<CouchbaseHandler> GCouchbaseHandler = make_shared<CouchbaseHandler>();

void CouchbaseHandler::HandleDBJob(const document doc, const json j,const string s)
{
	DB::Type e;
	//doc.type
	switch (doc.type)
	{
	case DB::PLAYER_KEY_REQ:
		Handle_PLAYER_KEY_REQ_ACK(doc, j,s);
		break;
	case DB::PLAYER_DATA_LOAD: //key_req후 있으면 요청후 그에 대한 결과값 처리
		Handle_PLAYER_DATA_LOAD_ACK(doc, j);
		break;
	case DB::PLAYER_DATA_CREATE: //key_req후 없으면 호출후 그에대한 결과값 처리 
		Handle_PLAYER_DATA_CREATE_ACK(doc);
		break;
	case DB::PLAYER_EXP_MONEY_UPDATE:
		break;
	case DB::PLAYER_LEVEL_UPDATE:
		break;
	case DB::PLAYER_ITEM_ADD:
		break;
	case DB::PLAYER_ITEM_REMOVE:
		break;
	case DB::PLAYER_EQUIP_ITEM:
		break;
	case DB::PLAYER_UNEQUIP_ITEM:
		break;
	default:
		break;
	}
}

void CouchbaseHandler::Handle_PLAYER_KEY_REQ_ACK(const document& doc, const json& j,const string s)
{
	document* doc_new=new document;
	doc_new->key = doc.key;
	doc_new->threadID = doc.threadID;

	if(j.contains("$1"))
	{

		bool exists = j["$1"].get<bool>();

		if (exists == true) //해당 id키값 존재
		{
			doc_new->type = DB::PLAYER_DATA_LOAD;
			doc_new->value = "SELECT* FROM `default` USE KEYS ["+ doc.key +"];";

			g_CouchbaseManager->GetConnection(doc.threadID)->get(doc_new->key, doc_new);

			//g_CouchbaseManager->GetConnection(doc.threadID)->QueryExecute(doc_new.value, doc_new);
			//요청해 get 해오면 결과물 플레이어 및 클라로 전달
		}
		else
		{
			//새로 플레이어정보 생성시,
			int playertype = Util::Random_ClassType();
			int id = stoi(doc.key);
			json document1 = { {"playerid",id } ,{"type",playertype },{"Level",1 }, { "Exp",0 }, {"Gold",0 },{"Kill",0 } };

			doc_new->type = DB::PLAYER_DATA_CREATE;
			doc_new->value = document1.dump();
			doc_new->cas = 0;
			

			g_CouchbaseManager->GetConnection(doc.threadID)->upsert(doc_new);

		}
	}
}

void CouchbaseHandler::Handle_PLAYER_DATA_CREATE_ACK(const document& doc)
{
	/*
	플레이어 정보 불러와서, 해당 정보 메모리 삽입후, 클라에게도 알림
	*/
	json jsonObject = json::parse(doc.value);
	//std::string jsonString = R"({"name": "John", "age": 30})";	
	//json jsonObject = json::parse(doc.value);
	PlayerInfo s = jsonObject.get<PlayerInfo>();

	int playerid = stoi(doc.key);
	Protocol::S_LOGIN loginPkt;
	auto playerRef = GPlayerManager->Player(playerid);
	if (playerRef == nullptr)
	{
		return;
		//loginPkt.set_success(false);
		//auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
		//pPlayer->ownerSession.lock()->Send(sendBuffer);
	}
	else
	{
		playerRef->nCas = doc.cas;

		loginPkt.set_success(true);

		auto player = loginPkt.mutable_players();
		int playertype = Util::Random_ClassType();
		player->set_playertype((Protocol::PlayerType)playertype);
		playerRef->playerId = playerid;
		playerRef->type = player->playertype();
		playerRef->SetObjectType(Object::Player);
		playerRef->SetZoneID(GZoneManager->IssueZoneID());

		
		CZoneRef Zone = GZoneManager->GetZone(playerRef->GetZoneID());
		int nSectorID = Zone->GetInitSectorID();
		playerRef->SetSectorID(nSectorID);
		

		player->set_id(playerRef->playerId);
		loginPkt.set_zoneid(playerRef->GetZoneID());
		loginPkt.set_sectorid(playerRef->GetSectorID());
		//loginPkt.set_
	}


	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
	playerRef->ownerSession.lock()->Send(sendBuffer);

}

void CouchbaseHandler::Handle_PLAYER_DATA_LOAD_ACK(const document& doc, const json& j)
{
	/*
		DB로부터 플레이어 정보 받아서, cas값은 플레이어매니저에 플레이어마다 별도 저장,
		나머지 플레이어정보 json ->구조체 파싱후, 내 메모리 저장후 클라에게도 전송
	
	*/
	//std::string jsonString = R"({"name": "John", "age": 30})";	
	//json jsonObject = json::parse(doc.value);
	PlayerInfo s = j.get<PlayerInfo>();
	Protocol::S_LOGIN loginPkt;

	auto playerRef = GPlayerManager->Player(s.id);
	if (playerRef == nullptr)
	{
		return;
		//loginPkt.set_success(false);
		//auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
		//pPlayer->ownerSession.lock()->Send(sendBuffer);
	}
	else
	{
		loginPkt.set_success(true);

		auto player = loginPkt.mutable_players();
		player->set_playertype((Protocol::PlayerType)s.type);
		
		playerRef->nCas = doc.cas;
		playerRef->playerId = s.id;
		playerRef->nLevel = s.level;
		playerRef->SetExp(s.Exp);
		playerRef->SetGold(s.Gold);
		playerRef->type = player->playertype();
		playerRef->SetObjectType(Object::Player);
		playerRef->SetZoneID(GZoneManager->IssueZoneID());


		CZoneRef Zone = GZoneManager->GetZone(playerRef->GetZoneID());
		int nSectorID = Zone->GetInitSectorID();
		playerRef->SetSectorID(nSectorID);


		player->set_id(s.id);
		loginPkt.set_zoneid(playerRef->GetZoneID());
		loginPkt.set_sectorid(playerRef->GetSectorID());
		//loginPkt.set_
	}


	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
	playerRef->ownerSession.lock()->Send(sendBuffer);



}
