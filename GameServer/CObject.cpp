#include "pch.h"
#include "CObject.h"
#include "CZone.h"
#include "CZone_Manager.h"
CObject::CObject():m_nZoneID(0),m_bActivate(false),m_nGold(0),m_nExp(0),m_nLevel(0)
{


}

CObject::CObject(const CObject& other)
{
	m_bActivate = other.m_bActivate;
	m_eState = other.m_eState;
	m_nGold = other.m_nGold;
	m_nExp = other.m_nExp;
	m_nLevel = other.m_nLevel;
	for(int i=0; i<4; i++)
		m_nStateTime[i] = other.m_nStateTime[i];
}

CObject& CObject::operator=(const CObject& other)
{
	// TODO: 여기에 return 문을 삽입합니다.
	return *this;

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

void CObject::Update(Sector::ObjectInfo info)
{
	m_bActivate = info.bActivate;
	if (m_bActivate == false)
		m_bAlive = false;

	m_nHP = info.nHP;
	m_nZoneID = info.nZoneID;
	m_nSectorID = info.nSectorID;

	for (int i = 0; i < 4; i++)
		m_nStateTime[i] = info.m_nStateTime[i];

	m_vPos.set_x(info.vPos.x);
	m_vPos.set_y(info.vPos.y);
	m_vPos.set_z(info.vPos.z);


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
#ifdef __DOP__
		auto Sector = Zone->GetSector(m_nSectorID);
		if (Sector == nullptr)
			return false;
#else
		CSectorRef Sector = Zone->GetSectorRef(m_nSectorID);
#endif
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