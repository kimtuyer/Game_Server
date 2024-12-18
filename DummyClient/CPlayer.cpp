#include "pch.h"
#include "CPlayer.h"
#include "ClientSession.h"
#include "ServerPacketHandler.h"

CClientPlayer::CClientPlayer():m_eState(Object::Idle)
{

}

CClientPlayer::~CClientPlayer()
{
	cout << "CClientPlayer ¼Ò¸ê" << endl;
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
