#pragma once
//#include "CZone.h"
class CZone;
using ZoneID = int;
using ThreadID = int;
using namespace Protocol;
class CZone_Manager : public JobQueue
{
	//DECLARE_SIGNLETON(CZone_Manager);
public:

	//처음 서버 시작시 초기화때 각 존 생성 , 몹 정보 미리 생성 삽입 
	CZone_Manager();
	~CZone_Manager()
	{
		m_listZone.clear();
	}


	//유저 입장시 해당 존 활성화 및 추가 및 업데이트

	//리스트 순회하면서 업데이트
	void Init(const int nZoneCount,  int nZoneUserMax);

	//오브젝트 리스트에 객체 삽입
	bool PlayerEnter(int& nZoneID, PlayerRef object);

	bool Enter(OUT int& nZoneID, PlayerRef& object);

	bool Insert(int nZoneID, const CZoneRef);

	//오브젝트 리스트 객체 삭제
	void Remove(int nZoneID, CZoneRef);

	//리스트 순회, 객체 타이머 
	void Update();


	int	IssueZoneID();


	bool		IsZone(int nZoneID)
	{

		return m_listZone.contains(nZoneID);
	}
	CZoneRef	GetZone(int nZoneID);

	bool GetStartPos(int nZoneID, D3DVECTOR*);


	bool ReassignThreadtoZone(int threadid,OUT vector<pair<int,int>>& zonelist)
	{
		WRITE_LOCK;
		if (m_ThreadToZoneList.contains(threadid))
		{
			zonelist = m_ThreadToZoneList[threadid];
			return true;
		}
		else
			return false;
	}

	void DistributeThreads(vector<pair<int, int>>);

	void PushThreadToZoneList(int threadid, pair<int, int> value);

private:
	USE_LOCK;
	int	m_nZoneCount;
	
	map<ZoneID, CZoneRef> m_listZone;


	int64 LmeasureTime = 0;

	vector<pair<int, int>> m_zoneOrderlist;
	map<ZoneID, int> m_zoneUserCount;
	map<ZoneID, int> m_zonePacketCount;

	vector<ZoneID> m_RemainZonelist;
	//map<ZoneID, vector<int>> m_zoneToThreadList;
	map<ThreadID, vector<pair<int,int>>> m_ThreadToZoneList;

};
extern shared_ptr<CZone_Manager> GZoneManager;
//CREATE_FUNCTION(CZone_Manager, ZoneManager);
