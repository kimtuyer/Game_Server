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

	{
		auto player = loginPkt.mutable_players();
		//player->set_name(u8"DB�����ܾ���̸�1");
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
	//	//player->set_name(u8"DB�����ܾ���̸�2");
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
	gameSession->_currentPlayer->SetActivate(true);


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

	//�ϴ� ������ ��ġ �ٲ���� ����
	gameSession->_currentPlayer->UpdatePos(true);
	Zone->Update_Pos(pkt.playerid(), pkt.pos());
		
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
	return false;
}



bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	//std::cout << pkt.msg() << endl;

	Protocol::S_CHAT chatPkt;
	chatPkt.set_msg(pkt.msg());
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(chatPkt);

	//Room->DoTimer(3000, CM, sendBuffer);
	//GRoom->DoTimer(3000 ,&Room::Broadcast, sendBuffer);

	GRoom->DoAsync(&Room::Broadcast, sendBuffer);

	return true;
}
