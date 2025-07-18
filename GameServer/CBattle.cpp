#include "pch.h"
#include "CBattle.h"
shared_ptr<CBattle> GBattle = make_shared<CBattle>();

bool CBattle::Attack(int nAttack, OUT int& nKillcount , OUT ObjectInfo& stTarget)
{

	if (stTarget.bActivate == false)
		return false;

	stTarget.nHP-=(nAttack);
	if (stTarget.nHP < 0)
	{
		//m_bAlive = false;
		stTarget.bActivate = false;
		fill_n(stTarget.m_nStateTime, Object::End, 0);

		nKillcount++;

		//죽었을 경우, 죽음 상태 변경해 알림.
		/*CZoneRef Zone = GZoneManager->GetZone(m_nZoneID);

		CSectorRef Sector = Zone->GetSectorRef(m_nSectorID);
		Sector->Insert_DeadList(m_nObjectID);*/

		/*
			킬 값 DB로 업뎃
		*/


	}
	else
	{
		/*
			몹도 공격받은 상태 인지,  공격상태로 전환

		*/
		stTarget.m_nStateTime[Object::Idle] = 0;
		stTarget.m_nStateTime[Object::Move] = 0;
		stTarget.m_nStateTime[Object::Attack] = GetTickCount64();

	}
	return true;


}
