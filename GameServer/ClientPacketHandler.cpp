#include "pch.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "Room.h"
#include "CMonster.h"
#include "CZone.h"
#include "CZone_Manager.h"
#include "GameSession.h"
#include "CPlayerManager.h"
#include "ConsoleMapViewer.h"
#include "CouchbaseClient.h"
PacketHandlerFunc GPacketHandler[UINT16_MAX];

// ���� ������ �۾���
using namespace Protocol;
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : Log
	return false;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	//cout << "C_LOGIN" << endl;
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	// TODO : Validation üũ

	Protocol::S_LOGIN loginPkt;
	loginPkt.set_success(true);
	
	// DB���� �÷��� ������ �ܾ�´�
	// GameSession�� �÷��� ������ ���� (�޸�)

	// ID �߱� (DB ���̵� �ƴϰ�, �ΰ��� ���̵�)
	static Atomic<uint64> idGenerator = 1;
	PlayerRef playerRef = MakeShared<CPlayer>();
	playerRef->ownerSession = gameSession;
	gameSession->_currentPlayer = playerRef;
	{
		

#ifdef  __COUCHBASE_DB__
		CouchbaseClient* pDBConnect = g_CouchbaseManager->GetConnection(LThreadId);
		document doc;
		doc.threadID = LThreadId;
		int key = idGenerator++;
		doc.key =to_string(key);
		doc.type = DB::PLAYER_KEY_REQ;
		doc.value = "SELECT EXISTS(SELECT 1 FROM `default` USE KEYS['"+doc.key+"'])";
		doc.cas = 0;
		doc.sendTime = GetTickCount64();
		//std::string query = "SELECT EXISTS(SELECT 1 FROM `default` USE KEYS [\"" + key_to_check + "\"]);";
		//pDBConnect->get(doc.key,doc);
		GPlayerManager->Insert(key, playerRef);

#endif  __COUCHBASE_DB__
		
#ifdef  __COUCHBASE_DB__
		pDBConnect->QueryExecute(doc.value, doc);
#endif
		
	}

#ifdef  __COUCHBASE_DB__
#else
	{
		auto player = loginPkt.mutable_players();
		int playertype=Util::Random_ClassType();
		player->set_playertype((PlayerType)playertype);

		//PlayerRef playerRef = MakeShared<CPlayer>();
		playerRef->ownerSession = gameSession;
		playerRef->playerId = idGenerator++;
		playerRef->name = player->name();
		playerRef->type = player->playertype();
		playerRef->SetObjectType(Object::Player);
		playerRef->SetZoneID(GZoneManager->IssueZoneID());

		{
			CZoneRef Zone = GZoneManager->GetZone(playerRef->GetZoneID());
			int nSectorID = Zone->GetInitSectorID();
			playerRef->SetSectorID(nSectorID);
		}

		//gameSession->_currentPlayer = playerRef;
		player->set_id(playerRef->playerId);
		
		GPlayerManager->Insert(playerRef->playerId, playerRef);

		//int nzoneid = GZoneManager->IssueZoneID();
		loginPkt.set_zoneid(playerRef->GetZoneID());
		loginPkt.set_sectorid(playerRef->GetSectorID());
	}

	
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
	session->Send(sendBuffer);
#endif
	return true;
}

bool Handle_C_ENTER_ZONE(PacketSessionRef& session, Protocol::C_ENTER_ZONE& pkt)
{
	//cout << "C_ENTER_ZONE" << endl;

	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (GPlayerManager->Find(pkt.playerid())==false)
		return false;


	S_ENTER_ACK enterpkt;
	enterpkt.set_sendtime(pkt.sendtime());

	int nZoneid = 1;

	gameSession->_currentPlayer->SetObjectID(pkt.playerid());
	gameSession->_currentPlayer->SetActivate(true);


	if (GZoneManager->Enter(nZoneid, gameSession->_currentPlayer) == false)
	{
		enterpkt.set_success(false);
	

	}
	else
	{
		/* ���� �� ���� ��ġ ����.	*/
		Protocol::D3DVECTOR* vPos = enterpkt.mutable_pos();
		CSectorRef Sector = GZoneManager->GetZone(nZoneid)->GetSector(pkt.sectorid());
		if (Sector == nullptr)
		{

			enterpkt.set_success(false);
		}
		else
		{
			vPos->set_x(Sector->StartPos().x());
			vPos->set_y(Sector->StartPos().y());
			vPos->set_z(Sector->StartPos().z());

			//if (GZoneManager->GetStartPos(nZoneid, vPos) == false)
			//	return false;

			enterpkt.set_success(true);
			enterpkt.set_zoneid(nZoneid);
			enterpkt.set_sectorid(enterpkt.sectorid());

			Sector->Insert(Object::Player, gameSession->_currentPlayer); //���� ObjectRef �־���ϴµ�..

		}
	}

#ifdef __ZONE_THREAD__
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterpkt, nZoneid);
#else
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterpkt, nZoneid);
#endif // __ZONE_THREAD__	
	session->Send(sendBuffer);

	/*
		�ش� �� ������ �ٸ��������� �˸�?
	
	
	*/


	return true;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	//cout << "C_MOVE" << endl;

	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	if (GPlayerManager->Find(pkt.playerid()) == false)
		return false;
	if (gameSession->_currentPlayer == nullptr)
		return false;

	//gameSession->_currentPlayer->
	//PlayerRef Player=

	/*
		���� ��ġ���� �̵������� ��ġ���� Ȯ��	
		�ش� ��, ���Ϳ� ��ġ�� �ٸ� �����鿡�� ��ε� ĳ����
	*/

	//���߿� �ش� ������ ������ġ�� ��, ������������ ��ε�ĳ����!
	//Protocol::S_MOVE_PLAYER movepkt;
	CZoneRef Zone = GZoneManager->GetZone(gameSession->_currentPlayer->GetZoneID());
	if (Zone == nullptr)
		return false;

	int nZoneid = gameSession->_currentPlayer->GetZoneID();

	//�ϴ� ������ ��ġ �ٲ���� ����
	Protocol::D3DVECTOR vPos = pkt.pos();
	gameSession->_currentPlayer->UpdatePos(true);

	//�񵿱�� ȣ���ؼ� �׸���,�ٷ� �׸��� ������..
#ifdef __CONSOLE_UI__
	GConsoleViewer->queuePlayerUpdate(pkt.playerid(), Zone->ZoneID(), vPos.x(), vPos.y());
#endif // __CONSOLE_UI__

	//GConsoleViewer->Concurrent_queueUpdate(pkt.playerid(), Zone->ZoneID(), vPos.x(), vPos.y());
	//GConsoleViewer->updatePlayerPosition(pkt.playerid(), Zone->ZoneID(), vPos.x(), vPos.y());
	Zone->Update_Pos(Object::Player,pkt.playerid(), pkt.pos());


	int nCurSectorID = gameSession->_currentPlayer->GetSectorID();
	int nPrevSectorID = nCurSectorID;
	if (Zone->UpdateSectorID(nCurSectorID, vPos))
	{
		//Sector ����
		
		Sector::ObjectInfo info;
		{
			info.nSectorID = nCurSectorID;
			info.nObjectID = pkt.playerid();
			info.vPos.x = vPos.x();
			info.vPos.y = vPos.y();
			info.nObjectType = Object::Player;

#ifdef __NOT_SECTOR_OBJLIST_PLAYER__
			Zone->Insert_PlayertoSector(info);
#else			
			//Protocol::S_PLAYER_LIST objpkt; //���� ���� ������ ��ó �����鿡�� �˸�.
			//CSectorRef Sector = Zone->GetSector(nCurSectorID);
			//Sector->Insert(info.nObjectType, gameSession->_currentPlayer);
			//Sector->BroadCast_Player( objpkt,info);
			Zone->Insert_PlayertoSector(info);
			//Zone->DoTimer(Tick::BROADCAST_TICK, &CZone::Send_SectorInsertPlayer, info);
#endif		
		}

		//������ ��ġ�ߴ� ����id�� ������ ����
		info.nSectorID = nPrevSectorID;
#ifdef __NOT_SECTOR_OBJLIST_PLAYER__
		Zone->Remove_PlayertoSector(info);
#else	
		//Protocol::S_PLAYER_REMOVE_ACK objpkt;
		//CSectorRef Sector = Zone->GetSector(nPrevSectorID);
		//Sector->Delete(info.nObjectType, gameSession->_currentPlayer);
		//Sector->BroadCast_Player(objpkt,info);
		Zone->Remove_PlayertoSector(info);
		//Zone->DoTimer(Tick::BROADCAST_TICK, &CZone::Send_SectorInsertPlayer, info);

		//���� ��ġ ����
#endif
		//���� ��ġ ����ÿ��� �˷���.
		Protocol::S_MOVE_ACK moveackpkt;
		moveackpkt.set_sendtime(pkt.sendtime());
		moveackpkt.set_success(true);
		moveackpkt.set_sectorid(nCurSectorID);

#ifdef __ZONE_THREAD__
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(moveackpkt, nZoneid);
#else
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(moveackpkt, nZoneid);
#endif // __ZONE_THREAD__	
		session->Send(sendBuffer);
	}

		
	//Protocol::D3DVECTOR* vec = movepkt.mutable_pos();
	//vec->set_x(pkt.pos().x());
	//vec->set_y(pkt.pos().y());
	//vec->set_z(pkt.pos().z());

	//movepkt.set_sendtime(pkt.sendtime());

	//c_move��Ŷ ���� �����尡 ��ε�ĳ������ �ѹ��� �� ó���ؾ��ϳ� �ƴϸ� �۷ι�ť�� �Ѱܼ� �д�ó��?
	//��ε��ؾ��ϴ� ������ �������̻��ϰ��,�۷ι��� �Ѱ� �ٸ������忡 �й��û?

	//Zone->DoBroadTimer(Tick::AI_TICK, &CZone::BroadCasting,movepkt);
	//Zone->DoTimer(Tick::AI_TICK, &CZone::BroadCasting, movepkt);

		

	//Protocol::S_MOVE_ACK moveackpkt;
	//moveackpkt.set_sendtime(pkt.sendtime());
	//moveackpkt.set_success(true);
	//
	//auto sendBuffer = ClientPacketHandler::MakeSendBuffer(moveackpkt);
	//session->Send(sendBuffer);

	return true;
}

bool Handle_C_ATTACK(PacketSessionRef& session, Protocol::C_ATTACK& pkt)
{

	Protocol::S_ATTACK_ACK ackpkt;
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);


	 gameSession->_currentPlayer->Attack(pkt);





	/*
		������ �� ������Ʈ�� ���� ���� ���ϴ���
		������ ������ �Ÿ����� �ٽ� Ȯ����,
		�����ϸ� ���� ���� ó�� ->Ÿ���� �ϴ� �ѹ��� ����.-> 3�ʵ� ������ Ÿ�̸�
		�Ұ����ϸ� S_ATTACK_ACK ���� false ����.
	
	
	*/


	return false;
}



bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	//std::cout << pkt.msg() << endl;

	Protocol::S_CHAT chatPkt;
	chatPkt.set_msg(pkt.msg());
#ifdef __ZONE_THREAD__
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(chatPkt, 0);
#else
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(chatPkt,0);
#endif // __ZONE_THREAD__
	//Room->DoTimer(3000, CM, sendBuffer);
	//GRoom->DoTimer(3000 ,&Room::Broadcast, sendBuffer);

	GRoom->DoAsync(&Room::Broadcast, sendBuffer);

	return true;
}
