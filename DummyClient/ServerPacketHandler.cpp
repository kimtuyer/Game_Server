#include "pch.h"
#include "ServerPacketHandler.h"
#include "CPlayer.h"
#include "ClientSession.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

// ���� ������ �۾���

auto RTT = [](int64 nowtime, int64 arrivetime, string packetName)
{
	int64 responstime = nowtime - arrivetime;

	if (responstime >100)
	{

		cout << packetName << ":  RTT ���� �ð� :" << responstime <<"ms " << endl;
	}
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
	playerRef->SetObjectID(player->id());
	playerRef->ownerSession = gameSession;
	playerRef->SetZoneid(pkt.zoneid());
	playerRef->SetSectorID(pkt.sectorid());
	playerRef->nLevel=player->level();

	int zoneid = playerRef->GetZoneID();
	gameSession->_currentPlayer = playerRef;
	//gameSession->_currentPlayer->SetZoneid()



	// ���� UI ��ư ������ ���� ����
	Protocol::C_ENTER_ZONE enterGamePkt;
	enterGamePkt.set_sendtime(GetTickCount64());
	enterGamePkt.set_playerid(player->id());
	enterGamePkt.set_zoneid(pkt.zoneid());
	enterGamePkt.set_sectorid(pkt.sectorid());

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(enterGamePkt, zoneid);
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

	int nZoneid = pkt.zoneid();

	gameSession->_currentPlayer->m_eState = Object::Move;
	//�ӽ� �׽�Ʈ��, Ŭ�� ���� �Ŵ��� ���� �� �÷��̾� id �� �� id ���� ��ǥ��ġ ���� �ϰ��ο�
	//Protocol::D3DVECTOR vPos;
	//vPos.set_x(3);
	//vPos.set_y(3);
	gameSession->_currentPlayer->SetPos(pkt.pos());

	gameSession->_currentPlayer->DoTimer(Tick::SECOND_TICK, &CClientPlayer::AI_Move);

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

	/*
	 ���� ��ġ ����
	*/
	ClientSessionRef gameSession = static_pointer_cast<ClientSession>(session);

	gameSession->_currentPlayer->SetSectorID(pkt.sectorid());
	gameSession->_currentPlayer->SetZoneid(pkt.zoneid());


	//���� �̵���, ���� ���Ϳ� �����ϴ� targetlist�� �ʱ�ȭ.
	gameSession->_currentPlayer->Clear_TargetList();

	return true;
}


bool Handle_S_ATTACK_ACK(PacketSessionRef& session, Protocol::S_ATTACK_ACK& pkt)
{
	RTT(GetTickCount64(), pkt.sendtime(), "S_ATTACK_ACK");	
	/*
		������ Ÿ���� ����������, �ٽ� idle���� ��ȯ
	*/

	ClientSessionRef gameSession = static_pointer_cast<ClientSession>(session);
	if (pkt.success() && pkt.targetalive()==false)
	{
		gameSession->_currentPlayer->m_eState=Object::Attack;
		cout << "�ش� �� ų ����!" << endl;

	}
	else
	{

		gameSession->_currentPlayer->m_eState = Object::Idle;
		gameSession->_currentPlayer->SetSearchOn(false);

	}


	// TODO
	return true;
}


bool Handle_S_OBJ_LIST(PacketSessionRef& session, Protocol::S_OBJ_LIST& pkt)
{
	RTT(GetTickCount64(), pkt.sendtime(), "S_OBJ_LIST");
	
	ClientSessionRef gameSession = static_pointer_cast<ClientSession>(session);


	int size=pkt.pos_size();
	for (int i = 0; i < size; i++)
	{
		//Sector::ObjectInfo info;
		Protocol::Object_Pos object = pkt.pos(i);	
		gameSession->_currentPlayer->Insert_Target(object);
	}
	gameSession->_currentPlayer->SetSearchOn(true); //�ֺ� ������Ʈ Ž������
	/*
	
	
	
	
	*/


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


	ClientSessionRef gameSession = static_pointer_cast<ClientSession>(session);


	int size = pkt.pos_size();
	for (int i = 0; i < size; i++)
	{
		//Sector::ObjectInfo info;
		Protocol::Object_Pos object = pkt.pos(i);
		gameSession->_currentPlayer->Delete_Target(object);
	}


	return true;
}

bool Handle_S_MOVE_MONSTER(PacketSessionRef& session, Protocol::S_MOVE_MONSTER& pkt)
{
	RTT(GetTickCount64(), pkt.sendtime(), "S_MOVE_MONSTER");
	/*
		
	*/
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

bool Handle_S_PLAYER_REMOVE_ACK(PacketSessionRef& session, Protocol::S_PLAYER_REMOVE_ACK& pkt)
{
	RTT(GetTickCount64(), pkt.sendtime(), "S_PLAYER_REMOVE_ACK");

	// TODO
	return true;
}

bool Handle_S_ALL_OBJ_LIST(PacketSessionRef& session, Protocol::S_ALL_OBJ_LIST& pkt)
{
	RTT(GetTickCount64(), pkt.sendtime(), "Handle_S_ALL_OBJ_LIST");

	//cout << "Handle_S_ALL_OBJ_LIST" << endl;


	//�ٸ� ���� �̿��� ��� ���Ϳ� ��ġ�� �����鿡�� �ٸ� �� ��輽�� ���� ����ȭ

	ClientSessionRef gameSession = static_pointer_cast<ClientSession>(session);

	vector< Protocol::Object_Pos> vecObjectlist;

	int size = pkt.pos_size();
	for (int i = 0; i < size; i++)
	{
		//Sector::ObjectInfo info;
		Protocol::Object_Pos object = pkt.pos(i);
		
		vecObjectlist.emplace_back(object);
	}
	gameSession->_currentPlayer->Update_TargetList(vecObjectlist);

	gameSession->_currentPlayer->SetSearchOn(true); //�ֺ� ������Ʈ Ž������
	/*


	*/
	// TODO
	return true;
}

bool Handle_S_PLAYER_LIST(PacketSessionRef& session, Protocol::S_PLAYER_LIST& pkt)
{
	RTT(GetTickCount64(), pkt.sendtime(), "S_PLAYER_LIST");

	// TODO
	return true;
}

bool Handle_S_ATTACK_REACT_ACK(PacketSessionRef& session, Protocol::S_ATTACK_REACT_ACK& pkt)
{
	RTT(GetTickCount64(), pkt.sendtime(), "S_ATTACK_REACT_ACK");

	// TODO
	return true;
}