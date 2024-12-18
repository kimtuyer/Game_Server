#include "pch.h"
#include "CZone_Manager.h"
#include "CZone.h"
//IMPLEMENT_SIGNLETON(CZone_Manager);
shared_ptr<CZone_Manager> GZoneManager = make_shared<CZone_Manager>();

CZone_Manager::CZone_Manager():m_nZoneCount(0)
{
	m_listZone.clear();
}

void CZone_Manager::Init(int nZoneCount)
{
	m_nZoneCount = nZoneCount;
	/*
	
	
	*/
	for (int i = 0; i < m_nZoneCount; i++)
	{
		//CZoneRef Zone = MakeShared<CZone>(200);
		m_listZone.insert({ i,MakeShared<CZone>(200)});
		//Zone->DoTimer(Tick::AI_TICK, &CZone::Update);

	}

	DoTimer(Tick::AI_TICK, &CZone_Manager::Update);

}

bool CZone_Manager::Enter(int& nZoneID, ObjectRef object)
{
	auto zone_iter = m_listZone.find(nZoneID);
	if (zone_iter == m_listZone.end())
		return false;

	if ((*zone_iter).second->_Enter(object->ObjectType(), object))
	{
		//Zone _Enter 내부에서 활성화
		//(*zone_iter).second->SetActivate(true);
		return true;
	}

	for (int zoneid = nZoneID+1 ; zoneid != m_listZone.size(); zoneid++)
	{
		auto zone= m_listZone[zoneid];
		if (zone->_Enter(object->ObjectType(), object))
		{
			nZoneID = zoneid;
			return true;
		}

	}

	return false;
}

bool CZone_Manager::Insert(int nZoneID, const CZoneRef Zoneptr)
{
	WRITE_LOCK;
	if (m_listZone.contains(nZoneID))
	{
		return false;
	}
	
	m_listZone.insert({ nZoneID,Zoneptr });

	return true;
}

void CZone_Manager::Remove(int nZoneID, CZoneRef)
{


}


void CZone_Manager::Update()
{
	for (auto& [zoneID, zoneRef] : m_listZone)
	{	
		if (zoneRef->GetActivate() == false)
			continue;

		zoneRef->Update();

	}


	DoTimer(Tick::AI_TICK, &CZone_Manager::Update);

}

int CZone_Manager::IssueZoneID()
{
	for (auto& [zoneID, Zone]: m_listZone)
	{
		if (Zone->Enter())
			return zoneID;
	}

	return -1;
}

CZoneRef CZone_Manager::GetZone(int nZoneID)
{
	READ_LOCK;
	if (m_listZone.contains(nZoneID))
	{
		return m_listZone[nZoneID];
	}
	else
		return nullptr;

	//return CZoneRef();
}
