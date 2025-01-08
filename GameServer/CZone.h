#pragma once
#include "CObject.h"
#include "Struct.pb.h"
#include "Protocol.pb.h"
#include "CSector.h"
class CObject;
using ObjectType = int;
using ObjectID = int;
using SectorID = int;
typedef  map<ObjectID, ObjectRef> ObjectList;

class CZone : public JobQueue
{

public:

	CZone(int nMaxUserCnt, int m_nZoneID, Protocol::D3DVECTOR vPos);
	~CZone();


	int		ZoneID()
	{
		return m_nZoneID;
	}
	//오브젝트 리스트에 객체 삽입
	bool _Enter(ObjectType, PlayerRef&);

	//오브젝트 리스트 객체 삭제
	void Remove(ObjectType, int objectID);

	//리스트 순회, 객체 타이머 
	void Update();
	void Update_Player();
	
	void Update_Partial(int beginSectorID, int endSectorID);

	void Send_SectorInsertObject(int beginSectorID, int endSectorID);
	void Send_SectorRemoveObject(int beginSectorID, int endSectorID);

	void Send_SectorInsertPlayer(int beginSectorID, int endSectorID);
	void Send_SectorRemovePlayer(int beginSectorID, int endSectorID);

	void AddPacketCount()
	{
		m_nPacketCnt.fetch_add(1);
	}
	void ResetPacketCount()
	{
		m_nPacketCnt.store(0);
	}
	atomic<int>& GetPacketCnt()
	{
		return m_nPacketCnt;
	}
	
	//몬스터 객체 받아서 플레이어 탐색
	CObject* SearchEnemy(CObject* pMonster);//or MonsterID

	int GetUserCount()
	{
		return m_nUserCnt;
	}

	void	Update_Pos(Object::ObjectType,int nObjectID, const Protocol::D3DVECTOR& vPos);



	void	SetActivate(bool bFlag)
	{
		m_bActivate = bFlag;
	}
	bool	GetActivate()
	{
		return m_bActivate;
	}

	
	bool	Enter();

	
	bool	UpdateSectorID(OUT int& nSectorID,Protocol::D3DVECTOR vPos);

	int			GetInitSectorID();
	CSectorRef	GetSector(int SectorID) {
		
		if (m_listSector.contains(SectorID) == false)
			return  nullptr;

		return m_listSector[SectorID];
	}

	ObjectList& PlayerList()
	{
		int lock = lock::Player;
		READ_LOCK_IDX(lock);
		//READ_LOCK;
		{
			return m_nlistObject[Object::Player];
		}
		

	}

	ObjectList& MonsterList()
	{
		int lock = lock::Monster;
		READ_LOCK_IDX(lock);
		//if (m_nlistObject.contains(Object::Monster))
		{
			return m_nlistObject[Object::Monster];
		}
	}

	void Send_MonsterUpdateList();

	void Send_PlayerUpdateList();


	void BroadCast_Monster(Protocol::S_MOVE_MONSTER& movepkt);

	void BroadCast_Player(Protocol::S_MOVE_PLAYER& movepkt);



	void	SetStartpos(Protocol::D3DVECTOR vPos)
	{
		m_vStartpos = vPos;
	}

	Protocol::D3DVECTOR  StartPos()
	{
		return m_vStartpos;
	}


	void Set_AdjSector(float x, float y , CSectorRef);



	void Insert_ObjecttoSector(Sector::ObjectInfo object);
	void Remove_ObjecttoSector(Sector::ObjectInfo object);

	void Insert_PlayertoSector(Sector::ObjectInfo object);
	void Remove_PlayertoSector(Sector::ObjectInfo object);

	void Send_SectorInsertObject();
	void Send_SectorRemoveObject();

	void Send_SectorInsertPlayer();
	void Send_SectorRemovePlayer();

	//CSectorRef GetSectorID(int nSectorID)
	//{
	//
	//	
	//}



private:	//오브젝트 리스트도 맵 or set이 나은가?
	
	//오브젝트는 타입에 따라 리스트를 다르게 가져가는게 나은가
	USE_MANY_LOCKS(lock::End);

	int			m_nZoneID;
	Protocol::D3DVECTOR m_vStartpos;

	int			m_nMaxUserCnt;
	atomic<int>	m_nUserCnt;
	atomic<int> m_nPacketCnt;

	map<SectorID, CSectorRef> m_listSector;

	map<SectorID,vector<Sector::ObjectInfo>> m_InsertList;
	map<SectorID, vector<Sector::ObjectInfo>> m_RemoveList;


	map<SectorID, vector<Sector::ObjectInfo>> m_PlayerInsertList;
	map<SectorID, vector<Sector::ObjectInfo>> m_PlayerRemoveList;

//	vector<Sector::ObjectInfo> m_RemoveList;


	map<ObjectType, map<ObjectID,ObjectRef>> m_nlistObject;

	bool m_bActivate;

	//8방향 좌표 리스트
	vector<pair<int, int>> directions = {
		   {0, -Zone::Sector_HEIGHT}, {0, Zone::Sector_HEIGHT }, {-Zone::Sector_WIDTH, 0}, {Zone::Sector_WIDTH, 0},
		   {-Zone::Sector_WIDTH, -Zone::Sector_HEIGHT}, {Zone::Sector_WIDTH, -Zone::Sector_HEIGHT}, {-Zone::Sector_WIDTH, Zone::Sector_HEIGHT},
		{Zone::Sector_WIDTH, Zone::Sector_HEIGHT}
	};

};

