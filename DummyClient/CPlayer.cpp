#include "pch.h"
#include "CPlayer.h"
#include "ClientSession.h"
#include "ServerPacketHandler.h"

CPlayer::CPlayer():m_eState(Object::Idle)
{

}

void CPlayer::Update()
{
}

void CPlayer::AI_Idle()
{
	Protocol::C_MOVE pkt;
	{
		pkt.set_sendtime(GetTickCount64());
		pkt.set_playerindex(playerId);
		auto vPos = pkt.add_pos();
		vPos->set_x(m_vPos.x);
		vPos->set_y(m_vPos.y);
		vPos->set_z(m_vPos.z);
	}

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	ownerSession->Send(sendBuffer);


	DoTimer(Tick::AI_TICK, &CPlayer::AI_Move);

}

void CPlayer::AI_Move()
{

	Protocol::C_MOVE pkt;
	{
		pkt.set_sendtime(GetTickCount64());
		pkt.set_playerindex(playerId);
		auto vPos = pkt.add_pos();
		vPos->set_x(m_vPos.x);
		vPos->set_y(m_vPos.y);
		vPos->set_z(m_vPos.z);
	}

	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	ownerSession->Send(sendBuffer);


	DoTimer(Tick::AI_TICK, &CPlayer::AI_Idle);
}

void CPlayer::AI_Attack()
{
}
