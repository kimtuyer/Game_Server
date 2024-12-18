#include "pch.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "Room.h"
#include "CMonster.h"
#include "CZone.h"
#include "CZone_Manager.h"
#include "GameSession.h"
#include "CPlayerManager.h"

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

	{
		auto player = loginPkt.mutable_players();
		//player->set_name(u8"DB에서긁어온이름1");
		player->set_playertype(Protocol::PLAYER_TYPE_KNIGHT);

		PlayerRef playerRef = MakeShared<CPlayer>();
		playerRef->playerId = idGenerator++;
		playerRef->name = player->name();
		playerRef->type = player->playertype();
		playerRef->SetObjectType(Object::Player);
		playerRef->ownerSession = gameSession;
		
		gameSession->_currentPlayer = playerRef;
		player->set_id(playerRef->playerId);
		//gameSession->_players.push_back(playerRef);
		
		GPlayerManager->Insert(playerRef->playerId, playerRef);

		//int nzoneid = GZoneManager->IssueZoneID();
		loginPkt.set_zoneid(GZoneManager->IssueZoneID());
	}

	//{
	//	auto player = loginPkt.add_players();
	//	//player->set_name(u8"DB에서긁어온이름2");
	//	player->set_playertype(Protocol::PLAYER_TYPE_MAGE);
	//
	//	PlayerRef playerRef = MakeShared<CPlayer>();
	//	playerRef->playerId = idGenerator++;
	//	playerRef->name = player->name();
	//	playerRef->type = player->playertype();
	//	playerRef->ownerSession = gameSession;
	//
	//	gameSession->_players.push_back(playerRef);
	//}

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(loginPkt);
	session->Send(sendBuffer);

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

	int nZoneid = pkt.zoneid();
	gameSession->_currentPlayer->SetObjectID(pkt.playerid());


	if (GZoneManager->Enter(nZoneid, gameSession->_currentPlayer) == false)
	{
		enterpkt.set_success(false);
	

	}
	else
	{
		enterpkt.set_success(true);
		enterpkt.set_zoneid(nZoneid);

	}

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(enterpkt);
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
	Protocol::S_MOVE_PLAYER movepkt;
	CZoneRef Zone = GZoneManager->GetZone(gameSession->_currentPlayer->GetZoneID());
	if (Zone == nullptr)
		return false;
	//ObjectList Playerlist = Zone->PlayerList();
	movepkt.set_playerid(pkt.playerid());
	movepkt.set_sendtime(pkt.sendtime());
	movepkt.set_allocated_pos(pkt.mutable_pos());
	Zone->BroadCasting(movepkt);
		

	S_MOVE_ACK moveackpkt;
	moveackpkt.set_sendtime(pkt.sendtime());
	moveackpkt.set_success(true);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(moveackpkt);
	session->Send(sendBuffer);

	return true;
}

bool Handle_C_ATTACK(PacketSessionRef& session, Protocol::C_ATTACK& pkt)
{
	return false;
}



bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	//std::cout << pkt.msg() << endl;

	Protocol::S_CHAT chatPkt;
	chatPkt.set_msg(pkt.msg());
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(chatPkt);

	//GRoom->DoTimer(3000, CM, sendBuffer);

	GRoom->DoAsync(&Room::Broadcast, sendBuffer);

	return true;
}
