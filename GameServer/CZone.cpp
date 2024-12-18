#include "pch.h"
#include "CZone.h"
#include "Player.h" //참조안하면 static_cast변환 실패
#include "CMonster.h"
CZone::CZone(int nMaxUserCnt) :m_bActivate(false), m_nMaxUserCnt(nMaxUserCnt)
{
	m_nUserCnt.store(0);

	/*
		맵/ 몬스터 세팅

	*/
}

CZone::~CZone()
{
	m_nlistObject.clear(); //shared_ptr도 바로 클리어 호출해도 되나?
}

bool CZone::_Enter(ObjectType eObjectType, ObjectRef object)
{
	WRITE_LOCK;

	if (eObjectType <  Object::Player || eObjectType > Object::Monster)
		return false;

	if (eObjectType == Object::Player)
	{
		if (m_nMaxUserCnt <=m_nlistObject[Object::Player].size())
			return false;
	}

	auto it = m_nlistObject.find(eObjectType);
	if (it == m_nlistObject.end())
	{
		ObjectList list;
		list.insert({ object->ObjectType(), object });
		m_nlistObject.insert({ eObjectType, list });
	}
	else
	{
		m_nlistObject[eObjectType].insert({ object->ObjectType(), object });
	}

	//유저가 있는 존만 활성화
	if (eObjectType == Object::ObjectType::Player)
	{
		m_nUserCnt.fetch_add(1);
		m_bActivate = true;
	}

	return true;
}

void CZone::Remove(ObjectType eObjectType, int objectID)
{
	if (eObjectType <  Object::ObjectType::Player || eObjectType > Object::ObjectType::Monster)
		return;

	{
		WRITE_LOCK;
		auto it = m_nlistObject.find(eObjectType);
		if (it != m_nlistObject.end())
		{
			auto& listObject = (*it).second;
			auto iter = listObject.find(objectID);
			if (iter != listObject.end())
			{
				listObject.erase(iter);
			}
			//for (auto iter = listObject.begin(); iter != listObject.end();)
			//{
			//	if ((*iter) == object)
			//		listObject.erase(iter++);
			//}
		}
	}

	//유저가 있는 존만 활성화
	if (eObjectType == Object::ObjectType::Player)
	{
		{
			WRITE_LOCK;
			m_nUserCnt.fetch_sub(1);
			if (m_nlistObject[Object::Player].empty())
				m_bActivate = false;
		}


	}
	/*
		해당 존, 섹터 위치한 다른 유저들에게 브로드캐스팅
		S_OBJ_REMOVE_ACK
		
	*/

}

void CZone::Update()
{
	if (m_nlistObject[Object::Player].empty())
	{
		m_bActivate = false;
		return;
	}

	vector<CMonster*> vecMonsterlist;
	{
		READ_LOCK;
		for (auto& [objectid, Object] : m_nlistObject[Object::Monster])
		{
			//if (ObjectType != (int)Object::Monster)
				//continue;
			if (Object->GetActivate() == false)
				continue;
			CMonster* pMonster = static_cast<CMonster*>(Object.get());
			vecMonsterlist.push_back(pMonster);
			//pMonster->Update();
		}
	}

	for (auto& pMonster : vecMonsterlist)
	{
		pMonster->Update();
	}

	//for (auto& Object : m_nlistObject[Object::Player])
	//{
	//	if (Object->GetActivate() == false)
	//		continue;
	//
	//	//if (ObjectType != (int)Object::Monster)
	//		//continue;
	//	CPlayer* pPlayer = static_cast<CPlayer*>(Object.get());
	//	pPlayer->Update();
	//}

	//ZoneManager에서 모든 zone update 호출중
	//DoTimer(Tick::AI_TICK, &CZone::Update);
}

CObject* CZone::SearchEnemy(CObject* pMonster)
{
	ObjectList Playerlist = PlayerList();
	if (Playerlist.empty())
		return nullptr;

	for (auto& [playerid, ObjectRef] : Playerlist)
	{
		float targetRange = ObjectRef->GetSearchRange();

		if (pMonster->GetSearchRange() < targetRange)
			continue;

		return ObjectRef.get();

		// Player* pPlayer = (Player*)(ObjectRef.get());

		//Player* pPlayer = static_cast<Player*>(ObjectRef.get());
	}

	return nullptr;
}

bool CZone::Enter()
{
	READ_LOCK;
	return m_nlistObject[Object::Player].size() < m_nMaxUserCnt;
}