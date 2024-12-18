#pragma once
#include "CObject.h"

class CObject;
using ObjectType = int;
using ObjectID = int;
typedef  map<ObjectID, ObjectRef> ObjectList;
class CZone : public JobQueue
{

public:

	CZone(int nMaxUserCnt);
	~CZone();

	
	//������Ʈ ����Ʈ�� ��ü ����
	bool _Enter(ObjectType, ObjectRef);

	//������Ʈ ����Ʈ ��ü ����
	void Remove(ObjectType, int objectID);

	//����Ʈ ��ȸ, ��ü Ÿ�̸� 
	void Update();
	
	//���� ��ü �޾Ƽ� �÷��̾� Ž��
	CObject* SearchEnemy(CObject* pMonster);//or MonsterID




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
	



private:	//������Ʈ ����Ʈ�� �� or set�� ������?
	
	//������Ʈ�� Ÿ�Կ� ���� ����Ʈ�� �ٸ��� �������°� ������
	USE_LOCK;
	int			m_nMaxUserCnt;
	atomic<int>	m_nUserCnt;
	map<ObjectType, map<ObjectID,ObjectRef>> m_nlistObject;

	bool m_bActivate;




};

