#pragma once
#include "CObject.h"
#include "Struct.pb.h"
#include "Protocol.pb.h"
#include "CSector.h"
class CObject;
using ObjectType = int;
using ObjectID = int;
using SectorID = int;
using Zone_ID = int;
typedef  map<ObjectID, ObjectRef> ObjectList;

class CZone : public JobQueue
{

public:

	CZone(int nMaxUserCnt, int m_nZoneID, Protocol::D3DVECTOR vPos);
	~CZone();

	void SetAdjSector();

	int		ZoneID()
	{
		return m_nZoneID;
	}
	//오브젝트 리스트에 객체 삽입
	bool _Enter(ObjectType, PlayerRef&);

	//오브젝트 리스트에 객체 삽입
	bool _EnterMonster(ObjectType, ObjectRef);

	//오브젝트 리스트 객체 삭제
	void Remove(ObjectType, int objectID);

	ObjectRef Object(ObjectType, int objectID);

	//리스트 순회, 객체 타이머 
	void Update();
	void Update_AdjSector(int nSectorID,int nZone, map<ObjectID, Sector::ObjectInfo>);
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



	void	SetActivate(bool bFlag);

	bool	GetActivate()
	{
		return m_bActivate;
	}

	
	bool	Enter();

	
	bool	UpdateSectorID(OUT int& nSectorID,OUT int& nMoveZone,int nObjectType,Protocol::D3DVECTOR vPos);

	int			GetInitSectorID();
#ifndef __DOP__
	CSectorRef	GetSectorRef(int SectorID) {
		if (m_listSector.contains(SectorID) == false)
			return  nullptr;
		return m_listSector[SectorID];
	}
#endif // __DOP__
	bool	IsSector(int sectorID)
	{
		if (m_listSector.contains(sectorID) == false)
			return  false;
		return true;
	}


	const map<SectorID, CSectorRef> GetSectorList()
	{
		return m_listSector;
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

	void BroadCast_Player(int nSectorID,Protocol::S_ATTACK_REACT_ACK& reactPkt);

	void BroadCast(vector<ObjectRef> list, SendBufferRef sendBuffer);

	void BroadCast_Req(vector<SectorID>list, Protocol::S_ATTACK_REACT_ACK reactPkt);


	void	SetStartpos(Protocol::D3DVECTOR vPos)
	{
		m_vStartpos = vPos;
	}

	Protocol::D3DVECTOR  StartPos()
	{
		return m_vStartpos;
	}

#ifdef __DOP__			
	void Set_AdjSector(float x, float y , CSector&);
#else
	void _Set_AdjSector(float x, float y, CSectorRef);

#endif


	void Insert_ObjecttoSector(Sector::ObjectInfo object);
	void Remove_ObjecttoSector(Sector::ObjectInfo object);

	void Insert_PlayertoSector(Sector::ObjectInfo object);
	void Remove_PlayertoSector(Sector::ObjectInfo object);

	void Send_SectorInsertObject();
	void Send_SectorRemoveObject();

	void Send_SectorInsertPlayer();
	void Send_SectorRemovePlayer();


	void Send_AdjSector_ObjList();
	void Send_AdjSector_InsertObj();
	void Send_AdjSector_RemoveObj();


	void SectorInsertPlayerJob(map<SectorID, vector<Sector::ObjectInfo>>);
	void SectorRemovePlayerJob(map<SectorID, vector<Sector::ObjectInfo>>);

	Sector::ObjectInfo GetAdjObjectInfo(int nSectorID,int ObjectID);
	Sector::ObjectInfo GetMyObjectInfo(int nSectorID, int nObjectType,int ObjectID);

	void Update_ObjectInfo(Sector::ObjectInfo);
	
	//CSectorRef GetSectorID(int nSectorID)
	//{
	//
	//	
	//}
#ifdef __DOP__

	CSector* GetSector(int Index);
#endif
private:	//오브젝트 리스트도 맵 or set이 나은가?
	
	//오브젝트는 타입에 따라 리스트를 다르게 가져가는게 나은가
	USE_MANY_LOCKS(lock::End);

	int			m_nZoneID;
	Protocol::D3DVECTOR m_vStartpos;

	int			m_nMaxUserCnt;
	atomic<int>	m_nUserCnt;
	atomic<int> m_nPacketCnt;

	int m_nBeginSecID;
	int m_nEndSecID;

#ifdef __DOP__
	vector<CSector>m_vecSector;
	map<SectorID, int>m_mapSector;
#else
	map<SectorID, CSectorRef> m_listSector;
	//set<Zone_ID> m_setAdjZone;
#ifdef __SEAMLESS__
	std::set<Zone_ID> m_setAdjZone;
	std::map<SectorID, map<ObjectID, Sector::ObjectInfo>>AdjObjectList;

	std::map<SectorID, map<ObjectID, Sector::ObjectInfo>>Remove_AdjObjectList;

#endif // __SEAMLESS__


	map<SectorID, vector<Sector::ObjectInfo>> m_listBorderSector;
	//map<SectorID, CSectorRef> m_listBorderSector;

#endif // __DOP__


	map<SectorID,vector<Sector::ObjectInfo>> m_InsertList;
	map<SectorID, vector<Sector::ObjectInfo>> m_RemoveList;


	map<SectorID, vector<Sector::ObjectInfo>> m_PlayerInsertList;
	map<SectorID, vector<Sector::ObjectInfo>> m_PlayerRemoveList;

//	vector<Sector::ObjectInfo> m_RemoveList;


	//map<ObjectID, ObjectRef>m_nMonsterList;
	//set<int>m_nPlayerList;
	//map<ObjectID, ObjectRef>m_nlistObject;

	map<ObjectType, map<ObjectID, ObjectRef>> m_nlistObject;

	bool m_bActivate;

	//8방향 좌표 리스트
	vector<pair<float, float>> directions = {
		   {0, -Zone::Sector_HEIGHT}, {0, Zone::Sector_HEIGHT }, {-Zone::Sector_WIDTH, 0}, {Zone::Sector_WIDTH, 0},
		   {-Zone::Sector_WIDTH, -Zone::Sector_HEIGHT}, {Zone::Sector_WIDTH, -Zone::Sector_HEIGHT}, {-Zone::Sector_WIDTH, Zone::Sector_HEIGHT},
		{Zone::Sector_WIDTH, Zone::Sector_HEIGHT}
	};

	// 1차원 존 ID를 2차원 그리드 좌표 (0-based)로 변환하는 함수
	std::pair<int, int> get_coords(int zone_id);

	// 2차원 그리드 좌표 (0-based)를 1차원 존 ID로 변환하는 함수
	int get_zone_id(int row, int col);

	// 모든 존의 이웃 리스트를 생성하는 함수
	void Set_Neighbor_list();
};

