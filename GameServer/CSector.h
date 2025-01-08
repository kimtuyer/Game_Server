#pragma once
using ObjectType = int;
using ObjectID = int;
typedef  map<ObjectID, ObjectRef> ObjectList;
using namespace LOCK;

class ClientPacketHandler;
class CSector : public JobQueue
{
public:
	CSector(int nSectorID, int nZoneID, Protocol::D3DVECTOR vPos);

	void Update();

	bool BelongtoSector(Protocol::D3DVECTOR vPos);

	bool FindObject(int objectID);
	ObjectRef GetMonster(int objectID);


	bool Insert(int nObjectType, ObjectRef& Object);

	bool Delete(int nObjectType, int objectID);

	void SendObjectlist();

	void BroadCast_MonsterList();

	void BroadCast_Player(Sector::UpdateType eType, Sector::ObjectInfo object);

	template <typename T>
	void BroadCast_Player(T& pkt, Sector::ObjectInfo object);



	bool Empty()
	{
		int lock = lock::Object;
		WRITE_LOCK_IDX(lock);

		return m_nlistObject.empty();
	}

	void	SetActivate(bool bFlag)
	{
		m_bActivate = bFlag;
	}
	bool	GetActivate()
	{
		return m_bActivate;
	}

	ObjectList& PlayerList();
	bool	Empty_Player()
	{
		int lock = lock::Player;
		WRITE_LOCK_IDX(lock);

		return m_nlistObject[Object::Player].empty();
	}


	void	SetStartpos(Protocol::D3DVECTOR vPos)
	{
		m_vStartpos = vPos;
	}

	Protocol::D3DVECTOR  StartPos()
	{
		return m_vStartpos;
	}

	CObject* SearchEnemy(CObject* pMonster);

	void Insert_adjSector(int sectorID, float x, float y);

	void Insert_ObjecttoSector(Sector::ObjectInfo object);
	void Remove_ObjecttoSector(Sector::ObjectInfo object);

	void Insert_DeadList(int ObjectID);


	map<int, Protocol::D3DVECTOR> m_adjSectorList;
private:
	USE_MANY_LOCKS(lock::End);
	
	int64 m_lBroadTime = 0;
	int m_nZoneID;
	int	m_nSectorID;
	bool m_bActivate;
	Protocol::D3DVECTOR m_vStartpos;


	map<ObjectID, int64>		m_DeadMonsterList;

	//map<ObjectID, ObjectRef > m_nlistObject;

	map<ObjectType, map<ObjectID, ObjectRef>> m_nlistObject;


	//map<int, ObjectRef> m_listObject;

};

template<typename T>
inline void CSector::BroadCast_Player(T& objpkt, Sector::ObjectInfo ObjectInfo)
{

	objpkt.set_sendtime(GetTickCount64());
	ObjectList Playerlist = m_nlistObject[Object::Player];

	Protocol::Object_Pos objectPos;
	objectPos.set_id(ObjectInfo.nObjectID);
	Protocol::D3DVECTOR* vPos = objectPos.mutable_vpos();
	vPos->set_x(ObjectInfo.vPos.x);
	vPos->set_y(ObjectInfo.vPos.y);

	objpkt.add_pos()->CopyFrom(objectPos);


	//auto distance = [](float source_x, float source_y, float target_x, float target_y)->float
	//	{
	//		return sqrt(pow(target_x - source_x, 2) + pow(target_y - source_y, 2));
	//
	//	};

	for (auto [playerid, Player] : Playerlist)
	{
		if (ObjectInfo.nObjectID == playerid)
			continue;

		float dist = distance(ObjectInfo.vPos.x, ObjectInfo.vPos.y, Player->GetPos().x(), Player->GetPos().y());


		if (dist > Zone::BroadCast_Distance)
		{
			continue;

		}

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt);
		CPlayer* pPlayer = static_cast<CPlayer*>(Player.get());
		if (pPlayer->ownerSession.expired() == false)
		{
			pPlayer->ownerSession.lock()->Send(sendBuffer);
		}
		else
		{
			pPlayer->SetActivate(false);
			///세션 소멸, 해당 플레이어 객체도
		}

	}

}
