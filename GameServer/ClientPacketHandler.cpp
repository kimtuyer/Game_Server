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

// 직접 컨텐츠 작업자
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

	// TODO : Validation 체크

	Protocol::S_LOGIN loginPkt;
	loginPkt.set_success(true);
	
	// DB에서 플레이 정보를 긁어온다
	// GameSession에 플레이 정보를 저장 (메모리)

	// ID 발급 (DB 아이디가 아니고, 인게임 아이디)
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
		/* 입장 존 최초 위치 받음.	*/
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

			Sector->Insert(Object::Player, gameSession->_currentPlayer); //원래 ObjectRef 넣어야하는데..

		}
	}

#ifdef __ZONE_THREAD__
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterpkt, nZoneid);
#else
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterpkt, nZoneid);
#endif // __ZONE_THREAD__	
	session->Send(sendBuffer);

	/*
		해당 존 접속한 다른유저에게 알림?
	
	
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
		현재 위치에서 이동가능한 위치인지 확인	
		해당 존, 섹터에 위치한 다른 유저들에게 브로드 캐스팅
	*/

	//나중엔 해당 유저가 이전위치와 비교, 움직였을때만 브로드캐스팅!
	//Protocol::S_MOVE_PLAYER movepkt;
	CZoneRef Zone = GZoneManager->GetZone(gameSession->_currentPlayer->GetZoneID());
	if (Zone == nullptr)
		return false;

	int nZoneid = gameSession->_currentPlayer->GetZoneID();

	//일단 무조건 위치 바뀌었다 가정
	Protocol::D3DVECTOR vPos = pkt.pos();
	gameSession->_currentPlayer->UpdatePos(true);

	//비동기로 호출해서 그릴지,바로 그리고 나올지..
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
		//Sector 변경
		
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
			//Protocol::S_PLAYER_LIST objpkt; //새로 들어온 섹터의 근처 유저들에게 알림.
			//CSectorRef Sector = Zone->GetSector(nCurSectorID);
			//Sector->Insert(info.nObjectType, gameSession->_currentPlayer);
			//Sector->BroadCast_Player( objpkt,info);
			Zone->Insert_PlayertoSector(info);
			//Zone->DoTimer(Tick::BROADCAST_TICK, &CZone::Send_SectorInsertPlayer, info);
#endif		
		}

		//이전에 위치했던 섹터id로 변경후 제거
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

		//섹터 위치 변경
#endif
		//섹터 위치 변경시에만 알려줌.
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

	//c_move패킷 받은 스레드가 브로드캐스팅을 한번에 다 처리해야하나 아니면 글로벌큐로 넘겨서 분담처리?
	//브로드해야하는 유저가 일정수이상일경우,글로벌에 넘겨 다른스레드에 분배요청?

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
		존에서 두 오브젝트가 같은 존에 속하는지
		공격이 가능한 거리인지 다시 확인후,
		가능하면 공격 성공 처리 ->타겟은 일단 한번에 제거.-> 3초뒤 리스폰 타이머
		불가능하면 S_ATTACK_ACK 에서 false 전송.
	
	
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
