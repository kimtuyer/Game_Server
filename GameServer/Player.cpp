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
		�ش� ������ ������ ����� ���߿� �ٸ� ���� �� ���� ��ȣ�ۿ� ���̾��ٸ�?
	
	*/

	m_bActivate = false;

	CZoneRef Zone = GZoneManager->GetZone(m_nZoneID);
	Zone->Remove(Object::Player, playerId);


}
