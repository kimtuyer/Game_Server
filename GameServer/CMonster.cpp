#include "pch.h"
#include "CMonster.h"
#include "CZone_Manager.h"
#include "CZone.h"
#include "Player.h"
#include "ConsoleMapViewer.h"
#include "RandomMove.h"
class CZone;
CMonster::CMonster(int nObjectID, int nZoneID,int nSectorID,  Protocol::D3DVECTOR vStartPos,bool bActivate) : m_nHP(100), m_nAttack(10), m_eState(Object::Idle)
, m_ndistribute(Object::Idle, Object::Move),gen(rd())
{
	m_bActivate = bActivate;

	m_nObjectID = nObjectID;
	m_nZoneID = nZoneID;
	m_nSectorID = nSectorID;

	m_vPos = vStartPos;
	//cout << "���� ���� ID: " << m_nObjectID <<" : " << m_vPos.x() << ", " << m_vPos.y() << endl;
	

	//m_ndistribute(Object::Idle, Object::Move);;

}

void CMonster::Update()
{	
	if (GetActivate() == false)
		return;

	//���� ��������, move, idle �ð� �ʱ�ȭ �ʿ�.
	if (m_nStateTime[m_eState] > GetTickCount64())
		return;

	switch (m_eState)
	{
	case Object::Idle:
		{
			AI_Idle();

		}
		break;
	case Object::Move:
		{

			AI_Move();
		}
		break;
	case Object::Attack:
		{

			AI_Attack();

		}
			break;
		default:
			break;
	}
}

void CMonster::AI_Idle()
{
	//�÷��̾� Ž��
	/*
	�÷��̾� ��ġ�� ���,������ ���ؼ�?
	�÷��̾� ��ġ ������ ���� ����ֳ� ->�÷��̾� ��ü�� ->��� �����ϳ� ->���� ������� ->�� ������ �� �Ŵ��� ����
	��(����)�� ��ġ�� ������ -> ���� ���� ���� ��������Ʈ�� Ž��


	�� �� ���ο��� ���ʹ����� ����� ���� ������ ��ü Ž��, ���⼱ �� ������ �˻�
		
	*/

	CZoneRef pZone = GZoneManager->GetZone(m_nZoneID);
	if (pZone == nullptr)
		return;

	CObject* pEnemy=pZone->SearchEnemy(this);
	if (pTarget == nullptr)
	{
		/*
		 �̵��� ���� ���, ���� ��
		 Ŭ��� �� �̵� ��Ŷ ����
		*/

		int64 ActionEndTime = GetTickCount64() + Tick::SECOND_TICK;
		m_nStateTime[Object::Idle] = (ActionEndTime);

		m_eState = Object::Move;
		//DoTimer(Tick::AI_TICK, &CMonster::AI_Move);

		//AI_Move();
		return;
		
	}

	//pTarget = pEnemy;
	//m_eState = Object::Attack;
//	DoTimer(Tick::AI_TICK, &CMonster::AI_Attack);
	//AI_Attack();
	/*
	 
	
	*/
	



	
}

void CMonster::AI_Move()
{
	//int nextState = m_ndistribute(gen);
	//
	//if (nextState == Object::Idle)
	//{
	//	m_eState = Object::Idle;
	//	return;
	//}
	int curSectorID = m_nSectorID;

	auto vNextPos = GRandomMove->getNextPosition(m_nZoneID, m_vPos.x(), m_vPos.y());
	m_vPos.set_x(vNextPos.first);
	m_vPos.set_y(vNextPos.second);

	int64 ActionEndTime= GetTickCount64() + Tick::SECOND_TICK;
	m_nStateTime[m_eState]=(ActionEndTime);

	GConsoleViewer->queuePlayerUpdate(m_nObjectID, m_nZoneID, m_vPos.x(), m_vPos.y());
	CZoneRef Zone = GZoneManager->GetZone(m_nZoneID);

	if (Zone->UpdateSectorID(m_nSectorID, m_vPos))
	{
		Sector::ObjectInfo info;
		{
			info.nSectorID = m_nSectorID;
			info.nObjectID = m_nObjectID;
			info.vPos.x = m_vPos.x();
			info.vPos.y = m_vPos.y();
			info.nObjectType = ObjectType();

			Zone->Insert_ObjecttoSector(info);
		}

		//������ ��ġ�ߴ� ����id�� ������ ����
		info.nSectorID = curSectorID;
		Zone->Remove_ObjecttoSector(info);
		//���� ��ġ ����

	}


	m_eState = Object::Idle;

}

void CMonster::AI_Attack()
{
	CPlayer* pPlayer = static_cast<CPlayer*>(pTarget);

	/*
	 ���� ó����, ó�� ��� 
	
	
	*/


}