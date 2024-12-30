#include "pch.h"
#include "CObject.h"
#include "CZone.h"
#include "CZone_Manager.h"
CObject::CObject():m_nZoneID(0),m_bActivate(false),m_nGold(0),m_nExp(0),m_nLevel(0)
{


}

void CObject::Update()
{
}

void CObject::LeaveZone()
{
}

void CObject::AI_Idle()
{
}

void CObject::AI_Move()
{
}

void CObject::AI_Attack()
{
}

bool CObject::Attacked(int nAttack, OUT int& nKillcount)
{
	WRITE_LOCK;
	//_battleock.WriteLock("Battle");

	if (m_bActivate == false)
		return false;

	m_nHP.fetch_sub(nAttack);
	if (m_nHP < 0)
	{
		//m_bAlive = false;
		m_bActivate = false;
		fill_n(m_nStateTime, Object::End, 0);
	

		nKillcount++;

		CZoneRef Zone = GZoneManager->GetZone(m_nZoneID);
		CSectorRef Sector = Zone->GetSector(m_nSectorID);
		Sector->Insert_DeadList(m_nObjectID);

		/*
			킬 값 DB로 업뎃
		*/


	}
	else
	{
		/*
			몹도 공격받은 상태 인지,  공격상태로 전환
		
		*/
		m_nStateTime[Object::Idle] = 0;
		m_nStateTime[Object::Move] = 0;
		m_nStateTime[Object::Attack] = GetTickCount64();
		m_eState = Object::Attack;



	}




	return true;
}