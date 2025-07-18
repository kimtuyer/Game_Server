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

		//�׾��� ���, ���� ���� ������ �˸�.
		/*CZoneRef Zone = GZoneManager->GetZone(m_nZoneID);

		CSectorRef Sector = Zone->GetSectorRef(m_nSectorID);
		Sector->Insert_DeadList(m_nObjectID);*/

		/*
			ų �� DB�� ����
		*/


	}
	else
	{
		/*
			���� ���ݹ��� ���� ����,  ���ݻ��·� ��ȯ

		*/
		stTarget.m_nStateTime[Object::Idle] = 0;
		stTarget.m_nStateTime[Object::Move] = 0;
		stTarget.m_nStateTime[Object::Attack] = GetTickCount64();

	}
	return true;


}
