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
	cout << "CClientPlayer �Ҹ�" << endl;
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
		���� ������ �� Ž��

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

	/* Move�� AI_IDLE �� ȣ��� �� ��������, ���� �̵��� �������,�����κ��� �ش� ���� ������Ʈ����Ʈ�� �̹� �޾Ҿ���� */
	//OBJ_LIST�� �ް������� ����� ���´ٸ�???
	//move_ack�� �ް� Ÿ�ٸ���Ʈ�� ���������, �� ���� obj_list�� ���������� Ÿ�ٰ˻��� �Ҽ�����!
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
		/* �ش� Ÿ�� ���� �õ�*/
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
		/*�ļ� �ൿ ������ ����*/

		return;
	}
}


void CClientPlayer::AI_Search()
{

}

void CClientPlayer::AI_Move()
{
	/*
	  �� ����ũ�� 30, ���� 15  , 5x3 ����

	*/
	int zoneid = m_nZoneID;
	int second = 20;//1�ʿ� 1���̵�,  �Ѱ� �� ���������µ� 30�� �ҿ�
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

	/*Ÿ���� ��� ����ִ� �����Ͽ� ���ݽõ� */
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