#include "pch.h"
#include "CZone_Manager.h"
#include "CZone.h"
#include "ConsoleMapViewer.h"
#include "Player.h"
//IMPLEMENT_SIGNLETON(CZone_Manager);
using namespace Zone;
shared_ptr<CZone_Manager> GZoneManager = make_shared<CZone_Manager>();

CZone_Manager::CZone_Manager():m_nZoneCount(0)
{
	m_listZone.clear();
}

void CZone_Manager::Init(const int nZoneCount, int nZoneUserMax )
{
	m_nZoneCount = nZoneCount;

	int nZoneid = 1;
	for (int HEIGHT = 1; HEIGHT <= ZONES_PER_COL; HEIGHT++)	//콘솔 세로 1줄 개수
		for (int WIDTH = 1; WIDTH <= ZONES_PER_ROW; WIDTH++) //콘솔 가로 1줄 개수
		{
			float x = (WIDTH - 1) * Zone::ZONE_WIDTH + ZONE_WIDTH / 2;
			float y = (HEIGHT - 1) * Zone::ZONE_HEIGHT + ZONE_HEIGHT / 2;

			Protocol::D3DVECTOR startpos;
			startpos.set_x(x);
			startpos.set_y(y);

			if (g_nZoneCount == 15)
			{

#ifdef __ZONE_THREAD_VER3__
			if (nZoneid < 6)
				nZoneUserMax = 200;
			else if(nZoneid==6 || nZoneid == 11)
				nZoneUserMax = 300;
			else if (nZoneid == 7 || nZoneid == 12)
				nZoneUserMax = 250;
			else if (nZoneid == 8 || nZoneid == 13)
				nZoneUserMax = 200;
			else if (nZoneid == 9 || nZoneid == 14)
				nZoneUserMax = 150;
			else if (nZoneid == 10 || nZoneid == 15)
				nZoneUserMax = 100;
#endif
			}
			else if (g_nZoneCount == 20)
			{

#ifdef __ZONE_THREAD_VER3__
#ifdef __5000_USER_ZONE__ //5000
				if (nZoneid == 1 || nZoneid == 6 || nZoneid == 11 || nZoneid == 16)
					nZoneUserMax = 100;
				else if (nZoneid == 2 || nZoneid == 7 || nZoneid == 12 || nZoneid == 17)
					nZoneUserMax = 200;
				else if (nZoneid == 3 || nZoneid == 8 || nZoneid == 13 || nZoneid == 18)
					nZoneUserMax = 250;
				else if (nZoneid == 4 || nZoneid == 9 || nZoneid == 14 || nZoneid == 19)
					nZoneUserMax = 300;	
				else if (nZoneid == 5 || nZoneid == 10 || nZoneid == 15 || nZoneid == 20)
					nZoneUserMax = 400;
#else	//4000
				if (nZoneid < 6)
					nZoneUserMax = 200;
				else if (nZoneid == 6 || nZoneid == 11)
					nZoneUserMax = 300;
				else if (nZoneid == 7 || nZoneid == 12)
					nZoneUserMax = 250;
				else if (nZoneid == 8 || nZoneid == 13)
					nZoneUserMax = 200;
				else if (nZoneid == 9 || nZoneid == 14)
					nZoneUserMax = 150;
				else if (nZoneid == 10 || nZoneid == 15)
					nZoneUserMax = 100;
				else if (nZoneid >= 16 )
					nZoneUserMax = 200;
#endif // 
#endif
			
			
			
			}


			CZoneRef Zone = MakeShared<CZone>(nZoneUserMax, nZoneid, startpos);
			m_listZone.insert({ nZoneid,Zone });

#ifdef __ZONE_THREAD__
#else
			Zone->DoTimer(Tick::AI_TICK, &CZone::Update);
			Zone->DoTimer(Tick::AI_TICK, &CZone::Update_Player);
#endif // __ZONE_THREAD__


			nZoneid++;
		}
	for (auto& Zone : m_listZone)
	{
		Zone.second.get()->SetAdjSector();
	}

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

bool CZone_Manager::Enter(OUT int& nZoneID, PlayerRef& object)
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

	auto Compare = [](const std::pair<int, int>& left, const std::pair<int, int>& right) ->bool {
		return left.second > right.second; // 내림차순
		//return left.second < right.second; // 오름차순

		};


	int64 nowtime = GetTickCount64();
	g_nConnectedUser = 0;
	map<ZoneID, int> zoneUserCount;
	for (auto& [zoneID, zoneRef] : m_listZone)
	{
			if (zoneRef->GetActivate() == false)
				continue;

			g_nConnectedUser += zoneRef->GetUserCount();

			{
			//일정 시간동안 유저수, 해당 존 패킷수 측정
				//m_zoneUserCount[zoneID] = zoneRef->GetUserCount();

				zoneUserCount[zoneID] = zoneRef->GetUserCount();

				m_zonePacketCount[zoneID] = zoneRef->GetPacketCnt();

				zoneRef->ResetPacketCount();

			}

	}
	
	for (auto zoneid : m_RemainZonelist)
	{
		auto Zoneref = GetZone(zoneid);
		Zoneref->DoTimer(Tick::AI_TICK,&CZone::Update);
	}


	if (g_nConnectedUser > 0)
	{
		WRITE_LOCK;
		vector<pair<int, int>> Orderlist(zoneUserCount.begin(), zoneUserCount.end());
		std::sort(Orderlist.begin(), Orderlist.end(), Compare);	
		
		
		if (m_zoneOrderlist.empty())
		{
			DistributeThreads(Orderlist);
			m_zoneOrderlist.swap(Orderlist);

		}
		else
		{
			for (int i = 0; i < Orderlist.size(); i++)
			{
				if (Orderlist[i].first != m_zoneOrderlist[i].first)
				{
					DistributeThreads(Orderlist);
					m_zoneOrderlist.swap(Orderlist);
					break;

				}

			}

		}
	}


	/*
		
	
	
	*/



//#ifdef __CONSOLE_UI__
	//GConsoleViewer->gotoxy(30,0);
	//cout << "현재 접속중인 유저 수 :" << g_nConnectedUser << endl;
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

void CZone_Manager::DistributeThreads(vector<pair<int, int>>list)
{
	int nThreadCnt = g_nThreadCnt;
	/*
		4등까지는 2개 스레드
		고 중 저 로 등수 매김.
		기존에 존 배정받은 스레드가 같은 등급의 존 아이디를 배정받은
		상태시 재할당 제외.
	
	*/
	queue<pair<int,int>> loadRanklist;

	int nCnt = 0;
	for (auto [ZoneID, number] : list)
	{
		if (number == 0)
		{
			loadRanklist.push(pair(ZoneID, 3));
			continue;
		}

		if(number>=300)
			loadRanklist.push(pair(ZoneID, 1));
		else if(number>=200)
			loadRanklist.push(pair(ZoneID, 2));
		else
			loadRanklist.push(pair(ZoneID, 3));

		//nCnt++;
		//if (nCnt < 5) //30$
		//	loadRanklist.push(pair(ZoneID, 1));
		//else if (nCnt < 16) //60%
		//	loadRanklist.push(pair(ZoneID, 2));
		//else //10%
		//	loadRanklist.push(pair(ZoneID, 3));
	}


	int rank_1 = 0;
	{
		//WRITE_LOCK;
		for (auto& [ThreadID, Zonelist] : m_ThreadToZoneList)
		{
			int nCnt = 1;
			m_ThreadToZoneList[ThreadID].clear();

			while (loadRanklist.empty() == false)
			{
				auto ZoneData = loadRanklist.front();

				if (ZoneData.second == 3 && nCnt < 3) //부하가 제일 약한 존, 1개의 스레드가 두개의 존 담당
				{

					m_ThreadToZoneList[ThreadID].push_back(pair(ZoneData.first, 0));
					loadRanklist.pop();
					nCnt++;

					if (nCnt == 3)
					{
						break;
					}

				}
				else if (ZoneData.second == 2) //부하 보통 존은 1개의 스레드만
				{
					m_ThreadToZoneList[ThreadID].push_back(pair(ZoneData.first, 0));
					loadRanklist.pop();

					break;

				}
				else if (ZoneData.second == 1) //부하 심한 존은 두개의 스레드가 담당
				{
					rank_1++;
					m_ThreadToZoneList[ThreadID].push_back(pair(ZoneData.first, rank_1));

					if (rank_1 == 2)
					{
						rank_1 = 0;
						loadRanklist.pop();
						break;

					}
					break;

				}

			}

			threadRebalance[ThreadID] = true;

		}
	}
	m_RemainZonelist.clear();
	while (loadRanklist.empty() == false)
	{
		int zoneid = loadRanklist.front().first;
		m_RemainZonelist.push_back(zoneid);

		loadRanklist.pop();		
	}
	//2~25 threaed id



	//m_ThreadToZoneList[]

	//g_nThreadCnt

		//threadRebalance[];

}

void CZone_Manager::PushThreadToZoneList(int threadid, pair<int, int> value)
{
	m_ThreadToZoneList[threadid].push_back(value);
}