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

void CZone_Manager::Init(const int nZoneCount, int nZoneUserMax )
{
	m_nZoneCount = nZoneCount;

	int nZoneid = 1;
	for (int HEIGHT = 1; HEIGHT <= ZONES_PER_COL; HEIGHT++)	//�ܼ� ���� 1�� ����
		for (int WIDTH = 1; WIDTH <= ZONES_PER_ROW; WIDTH++) //�ܼ� ���� 1�� ����
		{
			float x = (WIDTH - 1) * Zone::ZONE_WIDTH + ZONE_WIDTH / 2;
			float y = (HEIGHT - 1) * Zone::ZONE_HEIGHT + ZONE_HEIGHT / 2;

			Protocol::D3DVECTOR startpos;
			startpos.set_x(x);
			startpos.set_y(y);
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


			CZoneRef Zone = MakeShared<CZone>(nZoneUserMax, nZoneid, startpos);
			m_listZone.insert({ nZoneid,Zone });

#ifdef __ZONE_THREAD__
#else
			Zone->DoTimer(Tick::AI_TICK, &CZone::Update);
			Zone->DoTimer(Tick::AI_TICK, &CZone::Update_Player);
#endif // __ZONE_THREAD__


			nZoneid++;
		}

	/*
	!!!!!�� �Ŵ����������� ���� ����������, �� �������� Ÿ�̸Ӹ� ������
	�� ��Ƽ��������� ��� ���� ���ÿ� update�Ҽ��ְ�!!!
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
		//Zone _Enter ���ο��� Ȱ��ȭ
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
		return left.second > right.second; // ��������
		//return left.second < right.second; // ��������

		};


	int64 nowtime = GetTickCount64();
	g_nConnectedUser = 0;
	//bool bFlag = true;
	//if (LmeasureTime < nowtime)
	//{
	//	LmeasureTime = nowtime + Tick::SECOND_TICK;
	//
	//}
	//else
	//{
	//	bFlag = false;
	//}
	map<ZoneID, int> zoneUserCount;
	for (auto& [zoneID, zoneRef] : m_listZone)
	{
			if (zoneRef->GetActivate() == false)
				continue;

			g_nConnectedUser += zoneRef->GetUserCount();

			{
			//���� �ð����� ������, �ش� �� ��Ŷ�� ����
				//m_zoneUserCount[zoneID] = zoneRef->GetUserCount();

				zoneUserCount[zoneID] = zoneRef->GetUserCount();

				m_zonePacketCount[zoneID] = zoneRef->GetPacketCnt();

				zoneRef->ResetPacketCount();

			}

	}
	//if (m_zoneUserCount.empty())
	//{
	//	m_zoneUserCount.swap(zoneUserCount);
	//}
	//else
	//{
	//
	//
	//
	//
	//}


	if (g_nConnectedUser > 0)
	{
		WRITE_LOCK;
		vector<pair<int, int>> Orderlist(zoneUserCount.begin(), zoneUserCount.end());
		std::sort(Orderlist.begin(), Orderlist.end(), Compare);	
		
		
		if (m_zoneOrderlist.empty())
		{
			DistributeThreads(Orderlist);
			m_zoneOrderlist.swap(Orderlist);

			if (m_ThreadToZoneList.size() > 0)
			{
				int b = 1;
			}

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
	//cout << "���� �������� ���� �� :" << g_nConnectedUser << endl;
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
		4������� 2�� ������

		�� �� �� �� ��� �ű�.

		������ �� �������� �����尡 ���� ����� �� ���̵� ��������
		���½� ���Ҵ� ����.
	
	*/
	queue<pair<int,int>> loadRanklist;

	int nCnt = 0;
	for (auto [ZoneID, number] : list)
	{
		if (number == 0)
		{
			loadRanklist.push(pair(ZoneID, 3));
			//loadRanklist[ZoneID] = 3;
			continue;
		}

		nCnt++;
		if (nCnt < 5) //30$
			loadRanklist.push(pair(ZoneID, 1));
//			loadRanklist[ZoneID] = 1;
		else if (nCnt < 16) //60%
			loadRanklist.push(pair(ZoneID, 2));
			//loadRanklist[ZoneID] = 2;
		else //10%
			loadRanklist.push(pair(ZoneID, 3));
			//loadRanklist[ZoneID] = 3;
	}


	int rank_1 = 0;
	for (auto& [ThreadID, Zonelist] : m_ThreadToZoneList)
	{
		int nCnt = 1;
		//Zonelist.clear();
		m_ThreadToZoneList[ThreadID].clear();
		//for(auto iter=loadRanklist.)
		while(loadRanklist.empty()==false)
		//for (auto ZoneRank : loadRanklist)
		{
			auto ZoneData = loadRanklist.front();

			if (ZoneData.second == 3 && nCnt < 3) //���ϰ� ���� ���� ��
			{
				//for (int i = 0; i < 2; i++)
				{
					m_ThreadToZoneList[ThreadID].push_back(pair(ZoneData.first, 0));
					//Zonelist.push_back(pair(ZoneData.first, 0));
					loadRanklist.pop();
					nCnt++;

				}
				if (nCnt == 3)
				{
					break;
				}
			
			}
			else if (ZoneData.second == 2) //���� ���� ���� 1���� �����常
			{
				m_ThreadToZoneList[ThreadID].push_back(pair(ZoneData.first, 0));
				//Zonelist.push_back(pair(ZoneData.first, 0));
				loadRanklist.pop();

				break;

			}
			else if (ZoneData.second == 1) //���� ���� ���� �ΰ��� �����尡 ���
			{
				rank_1++;
				m_ThreadToZoneList[ThreadID].push_back(pair(ZoneData.first, rank_1));
				//Zonelist.push_back(pair(ZoneData.first, rank_1));
				if (rank_1 == 2)
				{
					rank_1 = 0;
					loadRanklist.pop();
					break;

				}
				break;
				//Zonelist.push_back(pair(ZoneData.first, 0));
				//nCnt++;
				//
				//if (nCnt == 3)
				//{
				//	loadRanklist.pop();
				//	break;
				//}

			}

		}

		threadRebalance[ThreadID] = true;


	}

	//2~25 threaed id



	//m_ThreadToZoneList[]

	//g_nThreadCnt

		//threadRebalance[];

}
