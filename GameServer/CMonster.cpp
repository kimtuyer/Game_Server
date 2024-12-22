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
	//cout << "몬스터 생성 ID: " << m_nObjectID <<" : " << m_vPos.x() << ", " << m_vPos.y() << endl;
	

	//m_ndistribute(Object::Idle, Object::Move);;

}

void CMonster::Update()
{	
	if (GetActivate() == false)
		return;

	//공격 당했을땐, move, idle 시간 초기화 필요.
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
	//플레이어 탐색
	/*
	플레이어 위치는 어디서,누구를 통해서?
	플레이어 위치 정보는 누가 들고있나 ->플레이어 객체가 ->어떻게 접근하나 ->존이 들고있음 ->존 정보는 존 매니저 관리
	나(몬스터)가 위치한 존에서 -> 같은 존에 속한 유저리스트를 탐색


	존 도 내부에선 섹터단위로 나뉘어서 섹터 단위로 객체 탐색, 여기선 존 단위로 검색
		
	*/

	CZoneRef pZone = GZoneManager->GetZone(m_nZoneID);
	if (pZone == nullptr)
		return;

	CObject* pEnemy=pZone->SearchEnemy(this);
	if (pTarget == nullptr)
	{
		/*
		 이동할 지점 계산, 정한 후
		 클라로 몹 이동 패킷 전송
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

		//이전에 위치했던 섹터id로 변경후 제거
		info.nSectorID = curSectorID;
		Zone->Remove_ObjecttoSector(info);
		//섹터 위치 변경

	}


	m_eState = Object::Idle;

}

void CMonster::AI_Attack()
{
	CPlayer* pPlayer = static_cast<CPlayer*>(pTarget);

	/*
	 공격 처리후, 처리 결과 
	
	
	*/


}