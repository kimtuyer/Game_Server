#include "pch.h"
#include "CZone_Manager.h"
#include "CZone.h"
#include "ConsoleMapViewer.h"
//IMPLEMENT_SIGNLETON(CZone_Manager);
using namespace Zone;
shared_ptr<CZone_Manager> GZoneManager = make_shared<CZone_Manager>();

CZone_Manager::CZone_Manager():m_nZoneCount(0)
{
	m_listZone.clear();
}

void CZone_Manager::Init(const int nZoneCount,const int nZoneUserMax )
{
	m_nZoneCount = nZoneCount;
	/*
	
	
	*/
	int nZoneid = 1;
	for (int HEIGHT = 1; HEIGHT <= 3; HEIGHT++)	//콘솔 세로 1줄 개수
		for (int WIDTH = 1; WIDTH <= 5; WIDTH++) //콘솔 가로 1줄 개수
		{
			float x = (WIDTH - 1) * Zone::ZONE_WIDTH + ZONE_WIDTH / 2;
			float y = (HEIGHT - 1) * Zone::ZONE_HEIGHT + ZONE_HEIGHT / 2;


			Protocol::D3DVECTOR startpos;
			startpos.set_x(x);
			startpos.set_y(y);

			CZoneRef Zone = MakeShared<CZone>(nZoneUserMax, nZoneid, startpos);
			m_listZone.insert({ nZoneid,Zone });

			Zone->DoTimer(Tick::AI_TICK, &CZone::Update);
			Zone->DoTimer(Tick::AI_TICK, &CZone::Update_Player);

			nZoneid++;
		}

	//	for (int i = 1; i <= m_nZoneCount; i++)
	//{
	//
	//	//CZoneRef Zone = MakeShared<CZone>(200);
	//	float x =  (nCnt - 1) * Zone::ZONE_WIDTH + ZONE_WIDTH / 2;
	//	float y=   (nCnt - 1) * Zone::ZONE_HEIGHT + ZONE_HEIGHT / 2;
	//
	//	Protocol::D3DVECTOR startpos;
	//	startpos.set_x(x);
	//	startpos.set_y(y);
	//
	//	m_listZone.insert({ i,MakeShared<CZone>(nZoneUserMax,i,startpos)});
	//	//Zone->DoTimer(Tick::AI_TICK, &CZone::Update);
	//
	//}


	/*
	!!!!!존 매니저단위에서 존을 돌리지말고, 존 단위에서 타이머를 돌려서
	각 멀티스레드들이 모든 존을 동시에 update할수있게!!!
	*/
	DoTimer(Tick::AI_TICK, &CZone_Manager::Update);

}

bool CZone_Manager::PlayerEnter(int& nZoneID, PlayerRef object)
{
	return false;
}

bool CZone_Manager::Enter(OUT int& nZoneID, ObjectRef object)
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

	for (int zoneid = nZoneID+1 ; zoneid != m_listZone.size()+1; zoneid++)
	{
		auto zone= m_listZone[zoneid];
		if (zone->_Enter(object->ObjectType(), object))
		{
			object->SetZoneID(zoneid);
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
	g_nConnectedUser = 0;
	for (auto& [zoneID, zoneRef] : m_listZone)
	{	
		if (zoneRef->GetActivate() == false)
			continue;

		g_nConnectedUser += zoneRef->GetUserCount();
		//zoneRef->DoAsync(&CZone::Update);
		//zoneRef->DoTimer(10, &CZone::Update);
		//zoneRef->Update();

	}
//#ifdef __CONSOLE_UI__
	GConsoleViewer->gotoxy(30,0);
	cout << "현재 접속중인 유저 수 :" << g_nConnectedUser << endl;
//#endif // __CONSOLE_UI__


	DoTimer(Tick::SECOND_TICK, &CZone_Manager::Update);

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

bool  CZone_Manager::GetStartPos(int nZoneID, D3DVECTOR* vPos)
{
	D3DVECTOR pos;
	READ_LOCK;
	if (m_listZone.contains(nZoneID))
	{
		pos = m_listZone[nZoneID]->StartPos();
		
		vPos->set_x(pos.x());
		vPos->set_y(pos.y());
		vPos->set_z(pos.z());

		return true;


		//return m_listZone[nZoneID];
	}
	else
		return false;
		//return nullptr;
}
