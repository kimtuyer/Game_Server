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

	//ó�� ���� ���۽� �ʱ�ȭ�� �� �� ���� , �� ���� �̸� ���� ���� 
	CZone_Manager();
	~CZone_Manager()
	{
		m_listZone.clear();
	}


	//���� ����� �ش� �� Ȱ��ȭ �� �߰� �� ������Ʈ

	//����Ʈ ��ȸ�ϸ鼭 ������Ʈ
	void Init(const int nZoneCount,  int nZoneUserMax);

	//������Ʈ ����Ʈ�� ��ü ����
	bool PlayerEnter(int& nZoneID, PlayerRef object);

	bool Enter(OUT int& nZoneID,ObjectRef object);

	bool Insert(int nZoneID, const CZoneRef);

	//������Ʈ ����Ʈ ��ü ����
	void Remove(int nZoneID, CZoneRef);

	//����Ʈ ��ȸ, ��ü Ÿ�̸� 
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
			if (zonelist.empty())
			{
				int b = 12;
			}
			return true;
		}
		else
			return false;
	}

	void DistributeThreads(vector<pair<int, int>>);

	void PushThreadToZoneList(int threadid, pair<int, int> value)
	{
		m_ThreadToZoneList[threadid].push_back(value);
	}

private:
	USE_LOCK;
	int	m_nZoneCount;
	
	map<ZoneID, CZoneRef> m_listZone;


	int64 LmeasureTime = 0;

	vector<pair<int, int>> m_zoneOrderlist;
	map<ZoneID, int> m_zoneUserCount;
	map<ZoneID, int> m_zonePacketCount;

	map<ZoneID, vector<int>> m_zoneToThreadList;
	map<ThreadID, vector<pair<int,int>>> m_ThreadToZoneList;

};
extern shared_ptr<CZone_Manager> GZoneManager;
//CREATE_FUNCTION(CZone_Manager, ZoneManager);
