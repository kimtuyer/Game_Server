#include "pch.h"
#include "ServerPacketHandler.h"
#include "CPlayer.h"
#include "ClientSession.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

// ���� ������ �۾���

auto RTT = [](int64 nowtime, int64 arrivetime, string packetName)
{
	int64 responstime = nowtime - arrivetime;

	cout << packetName << ":  RTT ���� �ð� :" << responstime <<"ms" << endl;
};

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : Log
	return false;
}

bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt)
{
	if (pkt.success() == false)
		return false;

	//if (pkt.players().size)
	//{
	//	// ĳ���� ����â
	//}
	auto player = pkt.mutable_players();

	ClientSessionRef gameSession = static_pointer_cast<ClientSession>(session);
	PlayerRef playerRef = MakeShared<CClientPlayer>();
	playerRef->playerId = player->id();
	playerRef->ownerSession = gameSession;
	gameSession->_currentPlayer = playerRef;



	// ���� UI ��ư ������ ���� ����
	Protocol::C_ENTER_ZONE enterGamePkt;
	enterGamePkt.set_sendtime(GetTickCount64());
	enterGamePkt.set_playerid(player->id());
	
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(enterGamePkt);
	session->Send(sendBuffer);

	return true;
}

bool Handle_S_ENTER_ACK(PacketSessionRef& session, Protocol::S_ENTER_ACK& pkt)
{	
	if (pkt.success() == false)
		return false;

	RTT(GetTickCount64(), pkt.sendtime(), "S_ENTER_ACK");
	/*
	 �����Ϸ��� �� ����
	
	*/
	ClientSessionRef gameSession = static_pointer_cast<ClientSession>(session);

	gameSession->_currentPlayer->SetZoneid(pkt.zoneid());
	gameSession->_currentPlayer->m_eState = Object::Move;

	gameSession->_currentPlayer->DoTimer(Tick::AI_TICK, &CClientPlayer::AI_Move);

	//Player->m_nZoneid= pkt.


	//auto player=pkt

	//Protocol::C_ENTER_ZONE enterGamePkt;
	//enterGamePkt.set_sendtime(GetTickCount64());
	//enterGamePkt.set_playerid(player->id());

	//auto sendBuffer = ServerPacketHandler::MakeSendBuffer(enterGamePkt);
	//session->Send(sendBuffer);





	// TODO
	return true;
}

bool Handle_S_MOVE_ACK(PacketSessionRef& session, Protocol::S_MOVE_ACK& pkt)
{
	// TODO

	RTT(GetTickCount64(), pkt.sendtime(), "S_MOVE_ACK");


	return true;
}


bool Handle_S_ATTACK_ACK(PacketSessionRef& session, Protocol::S_ATTACK_ACK& pkt)
{
	RTT(GetTickCount64(), pkt.sendtime(), "S_ATTACK_ACK");

	// TODO
	return true;
}


bool Handle_S_OBJ_LIST(PacketSessionRef& session, Protocol::S_OBJ_LIST& pkt)
{
	RTT(GetTickCount64(), pkt.sendtime(), "S_OBJ_LIST");

	// TODO
	return true;
}


bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt)
{
	std::cout << pkt.msg() << endl;
	return true;
}
bool Handle_S_OBJ_REMOVE_ACK(PacketSessionRef& session, Protocol::S_OBJ_REMOVE_ACK& pkt)
{
	RTT(GetTickCount64(), pkt.sendtime(), "S_OBJ_REMOVE_ACK");

	// TODO
	return true;
}

bool Handle_S_MOVE_MONSTER(PacketSessionRef& session, Protocol::S_MOVE_MONSTER& pkt)
{
	RTT(GetTickCount64(), pkt.sendtime(), "S_MOVE_MONSTER");

	// TODO
	return true;
}

bool Handle_S_MOVE_PLAYER(PacketSessionRef& session, Protocol::S_MOVE_PLAYER& pkt)
{
	RTT(GetTickCount64(), pkt.sendtime(), "S_MOVE_PLAYER");

	//for (int i = 0; i < pkt.pos().size(); i++)
	//{
	//	const Protocol::Player_Pos sData = pkt.pos(i);
	//
	//	cout << "ID:" << sData.id() << endl;
	//	cout << "��ġ x:" << sData.vpos().x() << endl;
	//	/*
	//	
	//	 ���� ���ͳ� ��ġ�� �ٸ� ������ ���� �޾Ƽ� ������Ʈ
	//	
	//	*/
	//}

	// TODO
	return true;
}