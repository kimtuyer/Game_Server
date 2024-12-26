#include "pch.h"
#include "CPlayer.h"
#include "ClientSession.h"
#include "ServerPacketHandler.h"
#include "RandomMove.h"
CClientPlayer::CClientPlayer() :m_eState(Object::Idle)
{
	//m_RandomMovement = MakeShared<RandomMove>();
}

CClientPlayer::~CClientPlayer()
{
	cout << "CClientPlayer 소멸" << endl;
}

void CClientPlayer::Update()
{
	switch (m_eState)
	{
	case Object::Idle:
		break;
	case Object::Move:
		break;
	case Object::Attack:
		break;
	case Object::End:
		break;
	default:
		break;
	}
}

void CClientPlayer::AI_Idle()
{
	/*
		공격 가능한 몹 탐색

	*/

	if (m_bSearch == false)
	{
		Protocol::C_MOVE pkt;
		{
			pkt.set_sendtime(GetTickCount64());
			pkt.set_playerid(m_nObjectID);
			auto vPos = pkt.mutable_pos();
			vPos->set_x(m_vPos.x());
			vPos->set_y(m_vPos.y());
			vPos->set_z(m_vPos.z());
		}

		auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		ownerSession->Send(sendBuffer);

		m_eState = Object::Move;
		DoTimer(Tick::AI_TICK, &CClientPlayer::AI_Move);

		return;
	}

	//auto distance = [](float source_x, float source_y, float target_x, float target_y)->float
	//	{
	//		return sqrt(pow(target_x - source_x, 2) + pow(target_y - source_y, 2));
	//	};

	bool bAttack = false;
	//Sector::ObjectInfo targetInfo;

	/* Move후 AI_IDLE 이 호출된 이 시점에선, 섹터 이동이 있을경우,서버로부터 해당 섹터 오브젝트리스트를 이미 받았어야함 */
	//OBJ_LIST를 받고있을때 여기로 들어온다면???
	//move_ack를 받고 타겟리스트를 지웠을경우, 그 다음 obj_list를 받을때까진 타겟검색을 할수없다!
	for (auto [ObjectID, ObjectInfo] : m_listTarget)
	{
		float dist = Util::distance(ObjectInfo.vpos().x(), ObjectInfo.vpos().y(), m_vPos.x(), m_vPos.y());

		if (dist > Zone::BroadCast_Distance)
		{
			continue;
		}

		bAttack = true;
		m_targetInfo = ObjectInfo;
		break;
		/* 해당 타겟 공격 시도*/
	}
	if (bAttack == true)
	{
		m_eState = Object::Attack;

		Protocol::C_ATTACK pkt;
		{
			pkt.set_playerid(m_nObjectID);
			pkt.set_sendtime(GetTickCount64());
			pkt.set_skillid(1);
			pkt.set_targetid(m_targetInfo.id());
		}
		auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		ownerSession->Send(sendBuffer);

		DoTimer(Tick::AI_TICK, &CClientPlayer::AI_Attack);
		/*후속 행동 예약후 종료*/

		return;
	}
}


void CClientPlayer::AI_Search()
{

}

void CClientPlayer::AI_Move()
{
	/*
	  존 가로크기 30, 세로 15  , 5x3 형태

	*/
	int zoneid = m_nZoneID;
	int second = 20;//1초에 1씩이동,  한개 존 끝까지가는데 30초 소요
	//float x =  m_vPos.x() + Zone::ZONE_WIDTH / Zone::ZONE_WIDTH * (Tick::SECOND_TICK * 0.001);
	//m_vPos.set_x(x);
	//float y = m_vPos.y() + Zone::ZONE_WIDTH / second * (Tick::AI_TICK * 0.001);
	auto vNextPos = GRandomMove->getNextPosition(m_nZoneID, m_vPos.x(), m_vPos.y());
	m_vPos.set_x(vNextPos.first);
	m_vPos.set_y(vNextPos.second);

	Protocol::C_MOVE pkt;
	{
		pkt.set_sendtime(GetTickCount64());
		pkt.set_playerid(m_nObjectID);
		auto vPos = pkt.mutable_pos();

		vPos->set_x(m_vPos.x());
		vPos->set_y(m_vPos.y());
		vPos->set_z(m_vPos.z());
	}

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	ownerSession->Send(sendBuffer);
	m_eState = Object::Idle;
	DoTimer(Tick::AI_TICK, &CClientPlayer::AI_Idle);
}

void CClientPlayer::AI_Attack()
{
	if (m_eState != Object::Attack)
	{
		DoTimer(Tick::AI_TICK, &CClientPlayer::AI_Idle);

		return;
	}
	if (m_listTarget.contains(m_targetInfo.id()) == false)
	{
		DoTimer(Tick::AI_TICK, &CClientPlayer::AI_Idle);

		return;
	}

	/*타겟이 계속 살아있다 가정하에 공격시도 */
	Protocol::C_ATTACK pkt;
	{
		pkt.set_playerid(m_nObjectID);
		pkt.set_sendtime(GetTickCount64());
		pkt.set_skillid(1);
		pkt.set_targetid(m_targetInfo.id());
	}
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	ownerSession->Send(sendBuffer);

	DoTimer(Tick::SECOND_TICK, &CClientPlayer::AI_Attack);

	/*

	*/
}

void CClientPlayer::Insert_Target(Protocol::Object_Pos info)
{
	WRITE_LOCK;
	m_listTarget.insert({ info.id(),info });
}

void CClientPlayer::Insert_TargetList(vector<Sector::ObjectInfo> list)
{
}

void CClientPlayer::Delete_Target(Protocol::Object_Pos info)
{
	if (m_listTarget.contains(info.id()) == false)
		return;

	WRITE_LOCK;
	m_listTarget.erase(info.id());
}

void CClientPlayer::Delete_TargetList(vector<Sector::ObjectInfo> list)
{
}

void CClientPlayer::Clear_TargetList()
{
	WRITE_LOCK;
	m_bSearch = false;
	m_listTarget.clear();
}