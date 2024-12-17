#include "pch.h"
#include "CZone_Manager.h"
#include "CZone.h"
//IMPLEMENT_SIGNLETON(CZone_Manager);
shared_ptr<CZone_Manager> GZoneManager = make_shared<CZone_Manager>();

void CZone_Manager::Init()
{
	/*
	
	
	*/
	for (int i = 0; i < 50; i++)
	{
		//CZoneRef Zone = MakeShared<CZone>();
		//Zone.get()->Insert()

		m_listZone.insert({ i,MakeShared<CZone>()});
	}

	
	DoTimer(Tick::AI_TICK, &CZone_Manager::Update);

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
