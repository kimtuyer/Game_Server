#include "pch.h"
#include "CMonster.h"
#include "CZone_Manager.h"

CMonster::CMonster() : m_nHP(100), m_nAttack(10), m_eState(Idle)
{


}

void CMonster::Update()
{	
	switch (m_eState)
	{
		case Idle:
		{
			AI_Idle();

		}
		break;
		case Move:
		{

			AI_Move();
		}
		break;
		case Attack:
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
	//플레이어 탐색
	/*
	플레이어 위치는 어디서,누구를 통해서?
	플레이어 위치 정보는 누가 들고있나 ->플레이어 객체가 ->어떻게 접근하나 ->존이 들고있음 ->존 정보는 존 매니저 관리
	나(몬스터)가 위치한 존에서 -> 같은 존에 속한 유저리스트를 탐색


	존 도 내부에선 섹터단위로 나뉘어서 섹터 단위로 객체 탐색, 여기선 존 단위로 검색
		
	*/

	CZoneRef pZone = ZoneManager()->GetZone(m_nZoneID);
	if (pZone == nullptr)
		return;

	CObject* pEnemy=pZone->SearchEnemy(this);
	if (pTarget == nullptr)
	{
		/*
		 이동할 지점 계산, 정한 후
		 클라로 몹 이동 패킷 전송
		*/
		m_eState = Move;
		DoTimer(Tick::AI_TICK, &CMonster::AI_Move);

		//AI_Move();
		return;
		
	}

	pTarget = pEnemy;
	DoTimer(Tick::AI_TICK, &CMonster::AI_Attack);
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
	 공격 처리후, 처리 결과 
	
	
	*/


}