#pragma once
#include "CObject.h"

class CObject;
using ObjectType = int;
typedef set<ObjectRef> ObjectList;
class CZone : public JobQueue
{

public:

	CZone();
	~CZone();

	
	//������Ʈ ����Ʈ�� ��ü ����
	bool Insert(ObjectType, ObjectRef);

	//������Ʈ ����Ʈ ��ü ����
	void Remove(ObjectType, ObjectRef);

	//����Ʈ ��ȸ, ��ü Ÿ�̸� 
	void Update();
	
	//���� ��ü �޾Ƽ� �÷��̾� Ž��
	CObject* SearchEnemy(CObject* pMonster);//or MonsterID




	bool	GetActivate()
	{
		return m_bActivate;
	}



	ObjectList& PlayerList()
	{
		//READ_LOCK;

		//if (m_nlistObject.contains(Object::Player))
		{
			return m_nlistObject[Object::Player];
		}
		

	}

	ObjectList& MonsterList()
	{
		//READ_LOCK;

		//if (m_nlistObject.contains(Object::Monster))
		{
			return m_nlistObject[Object::Monster];
		}
	}
	



private:	//������Ʈ ����Ʈ�� �� or set�� ������?

	//������Ʈ�� Ÿ�Կ� ���� ����Ʈ�� �ٸ��� �������°� ������

	map<ObjectType, set<ObjectRef>> m_nlistObject;

	bool m_bActivate;




};

