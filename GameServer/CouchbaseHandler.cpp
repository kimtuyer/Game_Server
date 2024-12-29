#include "pch.h"
#include "CouchbaseHandler.h"
#include "CouchbaseClient.h"
#include "CPlayerManager.h"
#include "Player.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
shared_ptr<CouchbaseHandler> GCouchbaseHandler = make_shared<CouchbaseHandler>();

void CouchbaseHandler::HandleDBJob(const document doc, const json j)
{
	DB::Type e;
	//doc.type
	switch (doc.type)
	{
	case DB::PLAYER_KEY_REQ:
		Handle_PLAYER_KEY_REQ(doc, j);
		break;
	case DB::PLAYER_DATA_LOAD: //key_req�� ������ ��û�� �׿� ���� ����� ó��
		Handle_PLAYER_DATA_LOAD(doc, j);
		break;
	case DB::PLAYER_DATA_CREATE: //key_req�� ������ ȣ���� �׿����� ����� ó�� 
		Handle_PLAYER_DATA_CREATE(doc);
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

void CouchbaseHandler::Handle_PLAYER_KEY_REQ(const document& doc, const json& j)
{
	document doc_new;
	doc_new.key = doc.key;
	doc_new.threadID = doc.threadID;

	if (j[0].contains("$1"))
	{
		bool exists = j[0]["$1"].get<bool>();

		if (j[0]["$1"].get<bool>() == true) //�ش� idŰ�� ����
		{
			doc_new.type = DB::PLAYER_DATA_LOAD;
			doc_new.value = "SELECT* FROM `default` USE KEYS [\"" + doc.key + "\"]);";

			g_CouchbaseManager->GetConnection(doc.threadID)->QueryExecute(doc_new.value, doc_new);
			//��û�� get �ؿ��� ����� �÷��̾� �� Ŭ��� ����
		}
		else
		{
			//���� �÷��̾����� ������,
			int playertype = Util::Random_ClassType();
			json document1 = { {"playerID", doc.key}, {"Class",playertype },{"Level",1 }};

			doc_new.type = DB::PLAYER_DATA_CREATE;
			doc_new.value = document1.dump();

			g_CouchbaseManager->GetConnection(doc.threadID)->upsert(doc_new);

		}
	}
}

void CouchbaseHandler::Handle_PLAYER_DATA_CREATE(const document& doc)
{
	/*
	�÷��̾� ���� �ҷ��ͼ�, �ش� ���� �޸� ������, Ŭ�󿡰Ե� �˸�
	*/
	int playerid = stoi(doc.key);
	PlayerRef pPlayer= GPlayerManager->Player(playerid);
	if (pPlayer)
	{
		//std::string jsonString = R"({"name": "John", "age": 30})";	
		json jsonObject = json::parse(doc.value);
		PlayerInfo s = jsonObject.get<PlayerInfo>();


		pPlayer->playerId = s.id;
		//Player->name = player->name();
		//Player->type = player->playertype();
		pPlayer->SetObjectType(Object::Player);

		Protocol::S_LOGIN loginPkt;
		loginPkt.set_success(true);
		auto player = loginPkt.mutable_players();
		player->set_id(pPlayer->playerId);
		loginPkt.set_zoneid(pPlayer->GetZoneID());
		loginPkt.set_sectorid(pPlayer->GetSectorID());

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
		pPlayer->ownerSession.lock()->Send(sendBuffer);


	}

}

void CouchbaseHandler::Handle_PLAYER_DATA_LOAD(const document& doc, const json& j)
{
	/*
		DB�κ��� �÷��̾� ���� �޾Ƽ�, cas���� �÷��̾�Ŵ����� �÷��̾�� ���� ����,
		������ �÷��̾����� json ->����ü �Ľ���, �� �޸� ������ Ŭ�󿡰Ե� ����
	
	*/

}
