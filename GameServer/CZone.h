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
	//������Ʈ ����Ʈ�� ��ü ����
	bool _Enter(ObjectType, ObjectRef);

	//������Ʈ ����Ʈ ��ü ����
	void Remove(ObjectType, int objectID);

	//����Ʈ ��ȸ, ��ü Ÿ�̸� 
	void Update();
	
	//���� ��ü �޾Ƽ� �÷��̾� Ž��
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

	void Insert_PlayertoSector(Sector::ObjectInfo object);
	void Remove_PlayertoSector(Sector::ObjectInfo object);

	void Send_SectorInsertObject();
	void Send_SectorRemoveObject();

	void Send_SectorInsertPlayer(Sector::ObjectInfo object);
	void Send_SectorRemovePlayer(Sector::ObjectInfo object);
	//CSectorRef GetSectorID(int nSectorID)
	//{
	//
	//	
	//}



private:	//������Ʈ ����Ʈ�� �� or set�� ������?
	
	//������Ʈ�� Ÿ�Կ� ���� ����Ʈ�� �ٸ��� �������°� ������
	USE_LOCK;
	int			m_nZoneID;
	Protocol::D3DVECTOR m_vStartpos;

	int			m_nMaxUserCnt;
	atomic<int>	m_nUserCnt;

	map<SectorID, CSectorRef> m_listSector;

	map<SectorID,vector<Sector::ObjectInfo>> m_InsertList;
	map<SectorID, vector<Sector::ObjectInfo>> m_RemoveList;


	map<SectorID, vector<Sector::ObjectInfo>> m_PlayerInsertList;
	map<SectorID, vector<Sector::ObjectInfo>> m_PlayerRemoveList;
//	vector<Sector::ObjectInfo> m_RemoveList;


	map<ObjectType, map<ObjectID,ObjectRef>> m_nlistObject;

	bool m_bActivate;

	//8���� ��ǥ ����Ʈ
	vector<pair<int, int>> directions = {
		   {0, -Zone::Sector_HEIGHT}, {0, Zone::Sector_HEIGHT }, {-Zone::Sector_WIDTH, 0}, {Zone::Sector_WIDTH, 0},
		   {-Zone::Sector_WIDTH, -Zone::Sector_HEIGHT}, {Zone::Sector_WIDTH, -Zone::Sector_HEIGHT}, {-Zone::Sector_WIDTH, Zone::Sector_HEIGHT},
		{Zone::Sector_WIDTH, Zone::Sector_HEIGHT}
	};

};

