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

	
	//오브젝트 리스트에 객체 삽입
	bool Insert(ObjectType, ObjectRef);

	//오브젝트 리스트 객체 삭제
	void Remove(ObjectType, ObjectRef);

	//리스트 순회, 객체 타이머 
	void Update();
	
	//몬스터 객체 받아서 플레이어 탐색
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
	



private:	//오브젝트 리스트도 맵 or set이 나은가?

	//오브젝트는 타입에 따라 리스트를 다르게 가져가는게 나은가

	map<ObjectType, set<ObjectRef>> m_nlistObject;

	bool m_bActivate;




};

