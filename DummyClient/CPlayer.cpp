#include "pch.h"
#include "CPlayer.h"
#include "ClientSession.h"
#include "ServerPacketHandler.h"
CClientPlayer::CClientPlayer():m_eState(Object::Idle)
{

}

CClientPlayer::~CClientPlayer()
{
	cout << "CClientPlayer �Ҹ�" << endl;
}

void CClientPlayer::Update()
{
}


void CClientPlayer::AI_Idle()
{
	Protocol::C_MOVE pkt;
	{
		pkt.set_sendtime(GetTickCount64());
		pkt.set_playerid(playerId);
		auto vPos = pkt.mutable_pos();
		vPos->set_x(m_vPos.x());
		vPos->set_y(m_vPos.y());
		vPos->set_z(m_vPos.z());
	}

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	ownerSession->Send(sendBuffer);


	DoTimer(Tick::AI_TICK, &CClientPlayer::AI_Move);

}

void CClientPlayer::AI_Move()
{

	/*
	  �� ����ũ�� 30, ���� 15  , 5x3 ����
	


	*/
	
	int second = 20;//1�ʿ� 1���̵�,  �Ѱ� �� ���������µ� 30�� �ҿ�
	float x =  m_vPos.x() + Zone::ZONE_WIDTH / Zone::ZONE_WIDTH * (Tick::SECOND_TICK * 0.001);
	m_vPos.set_x(x);
	//float y = m_vPos.y() + Zone::ZONE_WIDTH / second * (Tick::AI_TICK * 0.001);

	Protocol::C_MOVE pkt;
	{
		pkt.set_sendtime(GetTickCount64());
		pkt.set_playerid(playerId);
		auto vPos = pkt.mutable_pos();
	
		vPos->set_x(m_vPos.x());
		vPos->set_y(m_vPos.y());
		vPos->set_z(m_vPos.z());
	}

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	ownerSession->Send(sendBuffer);


	DoTimer(Tick::AI_TICK, &CClientPlayer::AI_Idle);
}

void CClientPlayer::AI_Attack()
{
}
