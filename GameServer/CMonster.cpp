#include "pch.h"
#include "CMonster.h"
#include "CZone_Manager.h"
#include "CZone.h"
#include "Player.h"
#include "ConsoleMapViewer.h"
#include "RandomMove.h"
class CZone;
CMonster::CMonster(int nObjectID, int nZoneID,int nSectorID,  Protocol::D3DVECTOR vStartPos,bool bActivate) :
 m_ndistribute(Object::Idle, Object::Move),gen(rd())
{
	m_bActivate = bActivate;

	m_nObjectID = nObjectID;
	m_nZoneID = nZoneID;
	m_nSectorID = nSectorID;
	m_nObjectType = Object::Monster;
	m_vPos = vStartPos;

	m_eState = Object::Idle;

	m_nHP = 1;// Util::Random_HP();
	m_nLevel = 5;// Util::Random_Level();
	m_nExp = Util::Random_ExpGold(m_nLevel);
	m_nGold= Util::Random_ExpGold(m_nLevel);
	m_nAttack = 10;

	//cout << "���� ���� ID: " << m_nObjectID <<" : " << m_vPos.x() << ", " << m_vPos.y() << endl;
	

	//m_ndistribute(Object::Idle, Object::Move);;

}

CMonster::CMonster(Sector::MonsterData sData, bool bActivate):
	m_ndistribute(Object::Idle, Object::Move), gen(rd()), pTarget(nullptr)
{
	m_bActivate = bActivate;

	m_nObjectID = sData.nObjectID;
	m_nZoneID = sData.nZoneID;
	m_nSectorID = sData.nSectorID;
	m_vPos.set_x(sData.vPos.x);
	m_vPos.set_y(sData.vPos.y);
	m_vPos.set_z(sData.vPos.z);


	m_nObjectType = Object::Monster;
	m_eState = Object::Idle;
	m_nHP = 1;// Util::Random_HP();
	m_nLevel = 5;// Util::Random_Level();
	m_nExp = Util::Random_ExpGold(m_nLevel);
	m_nGold = Util::Random_ExpGold(m_nLevel);
	m_nAttack = 10;

}

CMonster::CMonster(const CMonster& other):
m_ndistribute(Object::Idle, Object::Move), gen(rd()), pTarget(nullptr)

{
	m_bActivate = other.m_bActivate;


	m_nObjectID = other.m_nObjectID;
	m_nZoneID = other.m_nZoneID;
	m_nSectorID = other.m_nSectorID;
	m_vPos.set_x(other.m_vPos.x());
	m_vPos.set_y(other.m_vPos.y());
	m_vPos.set_z(other.m_vPos.z());


	m_nObjectType = Object::Monster;
	m_eState = other.m_eState;
	m_nHP.store(other.m_nHP);// Util::Random_HP();
	m_nLevel = other.m_nLevel;// Util::Random_Level();
	m_nExp = other.m_nExp;
	m_nGold = other.m_nGold;  //Util::Random_ExpGold(m_nLevel);
	m_nAttack = 10;
}

CMonster& CMonster::operator=(const CMonster& other)
{
	m_bActivate = other.m_bActivate;


	m_nObjectID = other.m_nObjectID;
	m_nZoneID = other.m_nZoneID;
	m_nSectorID = other.m_nSectorID;
	m_vPos.set_x(other.m_vPos.x());
	m_vPos.set_y(other.m_vPos.y());
	m_vPos.set_z(other.m_vPos.z());


	m_nObjectType = Object::Monster;
	m_eState = other.m_eState;
	m_nHP.store(other.m_nHP);// Util::Random_HP();
	m_nLevel = other.m_nLevel;// Util::Random_Level();
	m_nExp = other.m_nExp;
	m_nGold = other.m_nGold;  //Util::Random_ExpGold(m_nLevel);
	m_nAttack = 10;

	if (this != &other) {

		CObject::operator=(other);

		// Copy members
	}
	return *this;
	// TODO: ���⿡ return ���� �����մϴ�.
}

void CMonster::Update()
{	
	if (GetActivate() == false)
	{
		//�ش� ������Ʈ ���̵� ã�Ƽ� ���� ��ġ �����ֵ���!
		GConsoleViewer->queuePlayerUpdate(m_nObjectID, m_nZoneID, m_vPos.x(), m_vPos.y(),false);
		return;
	}

	int64 nowTime = GetTickCount64();
	//���� ��������, move, idle �ð� �ʱ�ȭ �ʿ�.
	if ( m_nStateTime[Object::Idle] > nowTime || m_nStateTime[Object::Move] > nowTime)
		return;
	if (m_nStateTime[Object::Attack] > 0)
		m_eState = Object::Attack;

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
#ifdef __DOP__
	auto Sector = pZone->GetSector(m_nSectorID);
	if (Sector == nullptr)
		return;
#else
	auto Sector = pZone->GetSectorRef(m_nSectorID);
#endif
	CObject* pEnemy=Sector->SearchEnemy(this);
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

	auto vNextPos = GRandomMove->getNextPosition(m_nZoneID, m_vPos.x(), m_vPos.y(),Object::Monster);
	m_vPos.set_x(vNextPos.first);
	m_vPos.set_y(vNextPos.second);

	int64 ActionEndTime= GetTickCount64() + Tick::SECOND_TICK;
	m_nStateTime[m_eState]=(ActionEndTime);
//#ifdef __CONSOLE_UI__
	GConsoleViewer->queuePlayerUpdate(m_nObjectID, m_nZoneID, m_vPos.x(), m_vPos.y());
//#endif

	CZoneRef Zone = GZoneManager->GetZone(m_nZoneID);
	int nPrevZone = m_nZoneID;
	Zone->Update_Pos(Object::Monster, m_nObjectID, m_vPos);
	if (Zone->UpdateSectorID(m_nSectorID, m_nZoneID,Object::Monster, m_vPos))
	{

#ifdef __SEAMLESS__
		Sector::ObjectInfo info;
		{
			info.nSectorID = m_nSectorID;
			info.nObjectID = m_nObjectID;
			info.vPos.x = m_vPos.x();
			info.vPos.y = m_vPos.y();
			info.nObjectType = ObjectType();

			info.nZoneID = m_nZoneID;
			if (info.nObjectType == 0)
				cout << "" << endl;

		}
		if (nPrevZone != m_nZoneID)
		{
			ObjectRef pMonster = Zone->Object(Object::Monster, m_nObjectID);
			if (pMonster == nullptr)
			{
				//Send_SectorRemoveObject ���� �ش� ������ ������ ���⶧����
				//��������Ʈ for���� Ÿ���ʾ�, �ش� ���Ϳ��� ���� ���������ʾ� ���⸦ Ž
				//cout << "AI_Move Error" << endl;
				cout << "AI_Move Error" << endl;

			}
			else
			{
				//cout << "�� ����!" << endl;

			}
			CZoneRef newZone = GZoneManager->GetZone(m_nZoneID);
			newZone->DoZoneJobTimer(0,m_nZoneID, &CZone::_EnterMonster, (int)Object::Monster, pMonster);
			newZone->DoZoneJobTimer(0,m_nZoneID, &CZone::Insert_ObjecttoSector, info);

			Zone->Remove(Object::Monster, info.nObjectID);
		}
		else
		{
			
			Zone->Insert_ObjecttoSector(info);

		}
		//������ ��ġ�ߴ� ����id�� ������ ����
		info.nSectorID = curSectorID;
		info.nZoneID = nPrevZone;

		Zone->Remove_ObjecttoSector(info);
		//���� ��ġ ����

#else
		Sector::ObjectInfo info;
		{
			info.nSectorID = m_nSectorID;
			info.nObjectID = m_nObjectID;
			info.vPos.x = m_vPos.x();
			info.vPos.y = m_vPos.y();
			info.nObjectType = ObjectType();

			info.nZoneID = m_nZoneID;

			Zone->Insert_ObjecttoSector(info);
		}

		//������ ��ġ�ߴ� ����id�� ������ ����
		info.nSectorID = curSectorID;
		Zone->Remove_ObjecttoSector(info);
		//���� ��ġ ����




#endif



	}




	m_eState = Object::Idle;

}

void CMonster::AI_Attack()
{
	//CPlayer* pPlayer = static_cast<CPlayer*>(pTarget);




	/*
	 ���� ó����, ó�� ��� 
	
	
	*/


}