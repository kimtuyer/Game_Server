#include "pch.h"
#include "CMonster.h"
#include "CZone_Manager.h"
#include "CZone.h"
#include "Player.h"
class CZone;
CMonster::CMonster() : m_nHP(100), m_nAttack(10), m_eState(Object::Idle)
{
	cout << "���� ����" << endl;

}

void CMonster::Update()
{	
	if (GetActivate() == false)
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

	CZoneRef pZone = CZone_Manager().GetZone(m_nZoneID);
	if (pZone == nullptr)
		return;

	CObject* pEnemy=pZone->SearchEnemy(this);
	if (pTarget == nullptr)
	{
		/*
		 �̵��� ���� ���, ���� ��
		 Ŭ��� �� �̵� ��Ŷ ����
		*/
		m_eState = Object::Move;
		//DoTimer(Tick::AI_TICK, &CMonster::AI_Move);

		//AI_Move();
		return;
		
	}

	pTarget = pEnemy;
	m_eState = Object::Attack;
//	DoTimer(Tick::AI_TICK, &CMonster::AI_Attack);
	//AI_Attack();
	/*
	 
	
	*/
	



	
}

void CMonster::AI_Move()
{
}

void CMonster::AI_Attack()
{
	CPlayer* pPlayer = static_cast<CPlayer*>(pTarget);

	/*
	 ���� ó����, ó�� ��� 
	
	
	*/


}