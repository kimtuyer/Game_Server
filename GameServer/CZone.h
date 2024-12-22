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
	bool _Enter(ObjectType, ObjectRef);

	//오브젝트 리스트 객체 삭제
	void Remove(ObjectType, int objectID);

	//리스트 순회, 객체 타이머 
	void Update();
	
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
		READ_LOCK;
		{
			return m_nlistObject[Object::Player];
		}
		

	}

	ObjectList& MonsterList()
	{
		READ_LOCK;
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

	void Insert_PlayertoSector(int sectorID, PlayerRef);
	void Remove_PlayertoSector(int sectorID, PlayerRef);

	void Send_SectorInsertObject();
	void Send_SectorRemoveObject();

	//CSectorRef GetSectorID(int nSectorID)
	//{
	//
	//	
	//}



private:	//오브젝트 리스트도 맵 or set이 나은가?
	
	//오브젝트는 타입에 따라 리스트를 다르게 가져가는게 나은가
	USE_LOCK;
	int			m_nZoneID;
	Protocol::D3DVECTOR m_vStartpos;

	int			m_nMaxUserCnt;
	atomic<int>	m_nUserCnt;

	map<SectorID, CSectorRef> m_listSector;

	map<SectorID,vector<Sector::ObjectInfo>> m_InsertList;
	map<SectorID, vector<Sector::ObjectInfo>> m_RemoveList;

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

