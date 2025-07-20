#include "pch.h"
#include "ServerPacketHandler.h"
#include "CPlayer.h"
#include "ClientSession.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

// 직접 컨텐츠 작업자

auto RTT = [](int64 nowtime, int64 arrivetime, string packetName)
{
	int64 responstime = nowtime - arrivetime;

	if (responstime >100)
	{

		cout << packetName << ":  RTT 응답 시간 :" << responstime <<"ms " << endl;
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
	//	// 캐릭터 생성창
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



	// 입장 UI 버튼 눌러서 게임 입장
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
	 입장하려는 존 정보
	
	*/
	ClientSessionRef gameSession = static_pointer_cast<ClientSession>(session);

	gameSession->_currentPlayer->SetZoneid(pkt.zoneid());

	int nZoneid = pkt.zoneid();

	gameSession->_currentPlayer->m_eState = Object::Move;
	//임시 테스트용, 클라도 따로 매니저 만들어서 각 플레이어 id 및 존 id 따라서 좌표위치 랜덤 일괄부여
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
	 섹터 위치 변경
	*/
	ClientSessionRef gameSession = static_pointer_cast<ClientSession>(session);

	gameSession->_currentPlayer->SetSectorID(pkt.sectorid());
	gameSession->_currentPlayer->SetZoneid(pkt.zoneid());


	//섹터 이동후, 기존 섹터에 존재하던 targetlist는 초기화.
	gameSession->_currentPlayer->Clear_TargetList();

	return true;
}


bool Handle_S_ATTACK_ACK(PacketSessionRef& session, Protocol::S_ATTACK_ACK& pkt)
{
	RTT(GetTickCount64(), pkt.sendtime(), "S_ATTACK_ACK");	
	/*
		공격한 타겟을 제거했을시, 다시 idle상태 전환
	*/

	ClientSessionRef gameSession = static_pointer_cast<ClientSession>(session);
	if (pkt.success() && pkt.targetalive()==false)
	{
		gameSession->_currentPlayer->m_eState=Object::Attack;
		cout << "해당 몹 킬 성공!" << endl;

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
	gameSession->_currentPlayer->SetSearchOn(true); //주변 오브젝트 탐색가능
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
	//	cout << "위치 x:" << sData.vpos().x() << endl;
	//	/*
	//	
	//	 같은 섹터내 위치한 다른 유저들 정보 받아서 업데이트
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


	//다른 존과 이웃한 경계 섹터에 위치한 유저들에게 다른 존 경계섹터 정보 동기화

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

	gameSession->_currentPlayer->SetSearchOn(true); //주변 오브젝트 탐색가능
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