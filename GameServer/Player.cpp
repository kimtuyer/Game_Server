#include "pch.h"
#include "Player.h"
#include "CZone.h"
#include "CZone_Manager.h"

CPlayer::CPlayer()
{
}

CPlayer::CPlayer(int playerid, int zoneid, int sectorid)
{
}

void CPlayer::Update()
{
}

void CPlayer::LeaveZone()
{
	/*
		해당 유저의 접속이 끊기는 와중에 다른 유저 및 몹과 상호작용 중이었다면?
	
	*/

	m_bActivate = false;

	CZoneRef Zone = GZoneManager->GetZone(m_nZoneID);
	Zone->Remove(Object::Player, playerId);


}
