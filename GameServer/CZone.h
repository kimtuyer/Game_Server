#pragma once
#include "CObject.h"
#include "Struct.pb.h"
#include "Protocol.pb.h"
class CObject;
using ObjectType = int;
using ObjectID = int;
typedef  map<ObjectID, ObjectRef> ObjectList;
class CZone : public JobQueue
{

public:

	CZone(int nMaxUserCnt, int m_nZoneID, Protocol::D3DVECTOR vPos);
	~CZone();


	int		ZoneID()
	{
		return m_nZoneID;
	}
	//������Ʈ ����Ʈ�� ��ü ����
	bool _Enter(ObjectType, ObjectRef);

	//������Ʈ ����Ʈ ��ü ����
	void Remove(ObjectType, int objectID);

	//����Ʈ ��ȸ, ��ü Ÿ�̸� 
	void Update();
	
	//���� ��ü �޾Ƽ� �÷��̾� Ž��
	CObject* SearchEnemy(CObject* pMonster);//or MonsterID



	void	Update_Pos(int nObjectID, const Protocol::D3DVECTOR& vPos);



	void	SetActivate(bool bFlag)
	{
		m_bActivate = bFlag;
	}
	bool	GetActivate()
	{
		return m_bActivate;
	}

	

	bool	Enter();

	ObjectList& PlayerList()
	{
		READ_LOCK;
		{
			return m_nlistObject[Object::Player];
		}
		

	}

	ObjectList& MonsterList()
	{
		READ_LOCK;
		//if (m_nlistObject.contains(Object::Monster))
		{
			return m_nlistObject[Object::Monster];
		}
	}

	void Send_MonsterUpdateList();

	void Send_PlayerUpdateList();


	void BroadCast_Monster(Protocol::S_MOVE_MONSTER& movepkt);

	void BroadCast_Player(Protocol::S_MOVE_PLAYER& movepkt);



	void	SetStartpos(Protocol::D3DVECTOR vPos)
	{
		m_vStartpos = vPos;
	}

	Protocol::D3DVECTOR  StartPos()
	{
		return m_vStartpos;
	}




private:	//������Ʈ ����Ʈ�� �� or set�� ������?
	
	//������Ʈ�� Ÿ�Կ� ���� ����Ʈ�� �ٸ��� �������°� ������
	USE_LOCK;
	int			m_nZoneID;
	Protocol::D3DVECTOR m_vStartpos;

	int			m_nMaxUserCnt;
	atomic<int>	m_nUserCnt;
	map<ObjectType, map<ObjectID,ObjectRef>> m_nlistObject;

	bool m_bActivate;




};

