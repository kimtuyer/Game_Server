#include "pch.h"
#include "CSector.h"
#include "CMonster.h"
#include "Player.h" //참조안하면 static_cast변환 실패
#include "GameSession.h"

#include "ClientPacketHandler.h"
using namespace LOCK;
#ifdef __DOP__
CSector::CSector()
{
}
#endif
CSector::CSector(int nSectorID, int nZoneID, Protocol::D3DVECTOR vPos)
	: m_nSectorID(nSectorID), m_vStartpos(vPos), m_bActivate(true), m_nZoneID(nZoneID)
{
}

CSector::CSector(const CSector& other)
	: m_nSectorID(other.m_nSectorID), m_vStartpos(other.m_vStartpos), m_bActivate(true), m_nZoneID(other.m_nZoneID)
{
	if (this == &other) {
		return;
	}

}

void CSector::Update()
{
#ifdef __DOP__
#else
	vector<CMonster*> m_vecMonsters;
	{
		int lock = lock::Monster;
		READ_LOCK_IDX(lock);
		for (auto& [objectid, Object] : m_nlistObject[Object::Monster])
		{
			CMonster* pMonster = static_cast<CMonster*>(Object.get());
			m_vecMonsters.push_back(pMonster);
			// 섹터에 있는 몬스터 정보는 언제 지워야하나?
			// Update 다 끝난후, 삽입,삭제리스트 처리할때 같이 지움.
			 //m_nlistObject[Object::Monster].erase(objectid);
		}
	}

#endif // __DOP__
	for (auto& pMonster : m_vecMonsters)
	{
#ifdef __DOP__
		pMonster.Update();
#else
		pMonster->Update();

#endif	
	}
	
	int nowtime = GetTickCount64();
	////매번 업데이트 돌때마다 보내주면 부하?
	{
		if (nowtime > m_lBroadTime)
		{
			m_lBroadTime = nowtime + Tick::SECOND_TICK * 3;
			//BroadCast_MonsterList();
		}
	}

	{
		int lock = lock::Die;
		WRITE_LOCK_IDX(lock);
		for (auto it = m_DeadMonsterList.begin(); it != m_DeadMonsterList.end();)
			// for (auto& [objectID, respawntime] : m_DeadMonsterList)
		{
			auto objectID = (*it).first;
			auto respawntime = (*it).second;

			if (nowtime < respawntime)
			{
				it++;
				continue;
			}
			//해당 몬스터 리스폰
			if (m_nlistObject[Object::Monster].contains(objectID))
			{
				m_nlistObject[Object::Monster][objectID]->SetActivate(true);
				m_nlistObject[Object::Monster][objectID]->m_eState = Object::Idle;
			}
			m_DeadMonsterList.erase(it++);
		}
	}
}

bool CSector::BelongtoSector(Protocol::D3DVECTOR vPos)
{
	float StartX = m_vStartpos.x() - Zone::Sector_WIDTH / 2;
	float StartY = m_vStartpos.y() - Zone::Sector_HEIGHT / 2;

	float EndX = m_vStartpos.x() + Zone::Sector_WIDTH / 2;
	float EndY = m_vStartpos.y() + Zone::Sector_HEIGHT / 2;

	if ((StartX <= vPos.x() && vPos.x() <= EndX) &&
		StartY <= vPos.y() && vPos.y() <= EndY)
		return true;

	return false;
}

bool CSector::FindObject(int objectID)
{
	return false;
}

ObjectRef CSector::GetMonster(int objectID)
{
	int lock = lock::Monster;
	READ_LOCK_IDX(lock);

	if (m_nlistObject[Object::Monster].contains(objectID))
	{
		return (m_nlistObject[Object::Monster][objectID]);
	}
	else
		return nullptr;
}

bool CSector::Insert(int nObjectType, ObjectRef& Object)
{
	int lock = lock::Object;
	WRITE_LOCK_IDX(lock);
	m_nlistObject[nObjectType].insert({ Object->ObjectID(),Object });
	
	return true;
}
#ifdef __DOP__

bool CSector::Insert_Monster(Sector::MonsterData& sData, bool bActivate)
{
	int lock = lock::Object;
	WRITE_LOCK_IDX(lock);
	m_vecMonsters.emplace_back(sData, true);

	return true;
}
#endif
bool CSector::Delete(int nObjectType, int objectID)
{
	int lock = lock::Object;
	WRITE_LOCK_IDX(lock);
	m_nlistObject[nObjectType].erase(objectID);
	return true;
}
#ifdef __DOP__
bool CSector::Delete_Monster(int nObjectID)
{
	int lock = lock::Object;
	WRITE_LOCK_IDX(lock);

	if(m_mapMonster.contains(nObjectID)==false)
		return false;
	m_vecMonsters[m_mapMonster[nObjectID]].SetActivate(false);

}
#endif
void CSector::SendObjectlist()
{
}

void CSector::BroadCast_MonsterList()
{
	map<ObjectID, ObjectRef> Monsterlist;
	{
		int lock = lock::Monster;
		READ_LOCK_IDX(lock);
		Monsterlist = m_nlistObject[Object::Monster];
	}
	map<ObjectID, ObjectRef> Playerlist;
	{
		int lock = lock::Player;
		READ_LOCK_IDX(lock);
		Playerlist = m_nlistObject[Object::Player];
	}

	for (auto& [playerid, Player] : Playerlist)
	{
		Protocol::S_OBJ_LIST objpkt;
		int64 nowtime = GetTickCount64();
		objpkt.set_sendtime(GetTickCount64());

		int nCnt = 0;
		bool bSend = false;
		for (auto& [monsterid, Monster] : Monsterlist)
		{
			if (Monster->m_eState == Object::Idle)
				continue;

			auto MonsterPos = Monster->GetPos();

			float dist = Util::distance(MonsterPos.x(), MonsterPos.y(), Player->GetPos().x(), Player->GetPos().y());

			if (dist > Zone::BroadCast_Distance)
			{
				continue;
			}
			if (nCnt + 2 > Zone::BroadCast_Cnt)
			{
				continue;
			}

			bSend = true;
			nCnt++;

			Protocol::Object_Pos objectPos;
			objectPos.set_id(monsterid);
			Protocol::D3DVECTOR* vPos = objectPos.mutable_vpos();
			vPos->set_x(MonsterPos.x());
			vPos->set_y(MonsterPos.y());

			objpkt.add_pos()->CopyFrom(objectPos);
		}
		if (bSend == false)
			continue;

#ifdef __ZONE_THREAD__
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#else
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#endif // __ZONE_THREAD__		
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

void CSector::BroadCast_Player(Sector::UpdateType eType, Sector::ObjectInfo ObjectInfo)
{
	ObjectList Playerlist = m_nlistObject[Object::Player];

	Protocol::S_PLAYER_LIST objpkt; //새로 들어온 섹터의 근처 유저들에게 알림.

	Protocol::Object_Pos objectPos;
	objectPos.set_id(ObjectInfo.nObjectID);
	Protocol::D3DVECTOR* vPos = objectPos.mutable_vpos();
	vPos->set_x(ObjectInfo.vPos.x);
	vPos->set_y(ObjectInfo.vPos.y);

	objpkt.add_pos()->CopyFrom(objectPos);

	for (auto [playerid, Player] : Playerlist)
	{
		if (ObjectInfo.nObjectID == playerid)
			continue;

		float dist = Util::distance(ObjectInfo.vPos.x, ObjectInfo.vPos.y, Player->GetPos().x(), Player->GetPos().y());

		if (dist > Zone::BroadCast_Distance)
		{
			continue;
		}

#ifdef __ZONE_THREAD__
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#else
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#endif // __ZONE_THREAD__			
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

ObjectList& CSector::PlayerList()
{
	{
		int lock = lock::Player;
		READ_LOCK_IDX(lock);
		{
			return m_nlistObject[Object::Player];
		}
	}

	// TODO: 여기에 return 문을 삽입합니다.
}

CObject* CSector::SearchEnemy(CObject* pMonster)
{
	ObjectList Playerlist = PlayerList();
	if (Playerlist.empty())
		return nullptr;

	//lock이 있어야, 해당 오브젝트의 접속이 끊겨서 삭제하려할때
	//접근 막을수 있음.
	int lock = lock::Player;
	READ_LOCK_IDX(lock);
	for (auto& [playerid, ObjectRef] : Playerlist)
	{
		float targetRange = ObjectRef->GetSearchRange();

		float dist = Util::distance(pMonster->GetPos().x(), pMonster->GetPos().y(), ObjectRef->GetPos().x(), ObjectRef->GetPos().y());

		if (dist > Zone::BroadCast_Distance)
			continue;
		return ObjectRef.get();

		// Player* pPlayer = (Player*)(ObjectRef.get());

		//Player* pPlayer = static_cast<Player*>(ObjectRef.get());
	}

	return nullptr;
}

void CSector::Insert_adjSector(int sectorID, float x, float y)
{
	Protocol::D3DVECTOR vPos;
	vPos.set_x(x);
	vPos.set_y(y);

	m_adjSectorList.insert({ sectorID,vPos });
}

void CSector::Insert_DeadList(int ObjectID)
{
	int lock = lock::Die;
	WRITE_LOCK_IDX(lock);
	//죽은 후 3초뒤 리스폰
	int nowTime = GetTickCount64() + Tick::SECOND_TICK * 3;

	if (m_DeadMonsterList.contains(ObjectID))
		return;
	m_DeadMonsterList.insert({ ObjectID,nowTime });
}