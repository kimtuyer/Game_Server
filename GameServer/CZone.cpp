#include "pch.h"
#include "CZone.h"
#include "Player.h" //참조안하면 static_cast변환 실패
#include "CMonster.h"
CZone::CZone()
{
}

CZone::~CZone()
{
	m_nlistObject.clear(); //shared_ptr도 바로 클리어 호출해도 되나?
}

bool CZone::Insert(ObjectType eObjectType, ObjectRef object)
{
	if (eObjectType <  Object::ObjectType::Player || eObjectType > Object::ObjectType::Monster)
		return false;

	auto it = m_nlistObject.find(eObjectType);
	if (it == m_nlistObject.end())
	{
		set<ObjectRef> list;
		list.insert(object);
		m_nlistObject.insert({ eObjectType, list});
	}
	else
	{
		m_nlistObject[eObjectType].insert(object);

	}

	//유저가 있는 존만 활성화
	if (eObjectType == Object::ObjectType::Player)
		m_bActivate = true; 


	return true;
}

void CZone::Remove(ObjectType eObjectType , ObjectRef object)
{
	if (eObjectType <  Object::ObjectType::Player || eObjectType > Object::ObjectType::Monster)
		return;

	auto it = m_nlistObject.find(eObjectType);
	if (it != m_nlistObject.end())
	{
		auto& listObject = (*it).second;
		auto iter = listObject.find(object);
		if(iter != listObject.end())
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

void CZone::Update()
{

	if (m_nlistObject[Object::Player].empty())
	{
		m_bActivate = false;
		return;
	}

	for (auto& Object : m_nlistObject[Object::Monster])
	{
		//if (ObjectType != (int)Object::Monster)
			//continue;
		if (Object->GetActivate() == false)
			continue;

		CMonster* pMonster = static_cast<CMonster*>(Object.get());
		
		pMonster->Update();

	}

	for (auto& Object : m_nlistObject[Object::Player])
	{
		if (Object->GetActivate() == false)
			continue;

		//if (ObjectType != (int)Object::Monster)
			//continue;
		CPlayer* pPlayer = static_cast<CPlayer*>(Object.get());
		pPlayer->Update();

	}

}

CObject* CZone::SearchEnemy(CObject* pMonster)
{

	ObjectList Playerlist =PlayerList();
	if (Playerlist.empty())
		return nullptr;

	for (auto& ObjectRef : Playerlist)
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

