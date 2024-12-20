#include "pch.h"
#include "CZone.h"
#include "Player.h" //참조안하면 static_cast변환 실패
#include "CMonster.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"

CZone::CZone(int nMaxUserCnt,int nZoneID, Protocol::D3DVECTOR vPos) :m_bActivate(false), m_nMaxUserCnt(nMaxUserCnt), m_nZoneID(nZoneID)
, m_vStartpos(vPos)
{
	m_nUserCnt.store(0);

	/*
		맵/ 몬스터 세팅

	*/
}

CZone::~CZone()
{
	m_nlistObject.clear(); //shared_ptr도 바로 클리어 호출해도 되나?
}

bool CZone::_Enter(ObjectType eObjectType, ObjectRef object)
{
	WRITE_LOCK;

	if (eObjectType <  Object::Player || eObjectType > Object::Monster)
		return false;

	if (eObjectType == Object::Player)
	{
		if (m_nMaxUserCnt <= m_nUserCnt)
			return false;
	}

	auto it = m_nlistObject.find(eObjectType);
	if (it == m_nlistObject.end())
	{
		ObjectList list;
		list.insert({ object->ObjectID(), object });
		m_nlistObject.insert({ eObjectType, list });
	}
	else
	{
		m_nlistObject[eObjectType].insert({ object->ObjectID(), object });
	}

	//유저가 있는 존만 활성화
	if (eObjectType == Object::ObjectType::Player)
	{
		m_nUserCnt.fetch_add(1);
		m_bActivate = true;
	}

	return true;
}

void CZone::Remove(ObjectType eObjectType, int objectID)
{
	if (eObjectType <  Object::ObjectType::Player || eObjectType > Object::ObjectType::Monster)
		return;

	{
		WRITE_LOCK;
		auto it = m_nlistObject.find(eObjectType);
		if (it != m_nlistObject.end())
		{
			auto& listObject = (*it).second;
			auto iter = listObject.find(objectID);
			if (iter != listObject.end())
			{
				listObject.erase(iter);
			}
			//for (auto iter = listObject.begin(); iter != listObject.end();)
			//{
			//	if ((*iter) == object)
			//		listObject.erase(iter++);
			//}
		}
	}

	//유저가 있는 존만 활성화
	if (eObjectType == Object::ObjectType::Player)
	{
		{
			WRITE_LOCK;
			m_nUserCnt.fetch_sub(1);
			if (m_nlistObject[Object::Player].empty())
				m_bActivate = false;
		}
	}
	/*
		해당 존, 섹터 위치한 다른 유저들에게 브로드캐스팅
		S_OBJ_REMOVE_ACK

	*/
}

void CZone::Update()
{
	if (m_nlistObject[Object::Player].empty())
	{
		m_bActivate = false;
		return;
	}

	vector<CMonster*> vecMonsterlist;
	{
		READ_LOCK;
		for (auto& [objectid, Object] : m_nlistObject[Object::Monster])
		{
			//if (ObjectType != (int)Object::Monster)
				//continue;
			if (Object->GetActivate() == false)
				continue;
			CMonster* pMonster = static_cast<CMonster*>(Object.get());
			vecMonsterlist.push_back(pMonster);
			//pMonster->Update();
		}
	}

	for (auto& pMonster : vecMonsterlist)
	{
		pMonster->Update();
	}

	/*
	 몬스터 업뎃후 실시간 좌표 주변 유저에게 동기화

	*/
	Protocol::S_MOVE_PLAYER pkt;
	pkt.set_sendtime(GetTickCount64());
	map<ObjectID, ObjectRef> PlayerList = m_nlistObject[Object::Player];

	int nCnt = 0;
	for (auto& [objectid, Object] : PlayerList)
	{
		/*
		 이때도 엄밀하게는 존 안의 같은 섹터영역에 유저에게만 보내야
		*/

		//패킷 크기 고려해 일단 한번에 30명분 제한
		if (nCnt > 30)
			break;

		if (Object->GetActivate() == false)
			continue;

		if (Object->IsUpdatePos() == false)
			continue;

		//이전 위치와 다른지 업뎃필요한 유저인지 확인도 필요

		Protocol::Player_Pos sData;
		sData.set_id(objectid);
		Protocol::D3DVECTOR* vPos = sData.mutable_vpos();
		Protocol::D3DVECTOR targetPos = Object->GetPos();
		vPos->set_x(targetPos.x());
		vPos->set_y(targetPos.y());
		vPos->set_z(targetPos.z());

		pkt.add_pos()->CopyFrom(sData);

		nCnt++;
		//CPlayer* pPlayer = static_cast<CPlayer*>(Object.get());
		//pPlayer->Update();
	}

	BroadCasting(pkt);
	//ZoneManager에서 모든 zone update 호출중
	//DoTimer(Tick::AI_TICK, &CZone::Update);
}

CObject* CZone::SearchEnemy(CObject* pMonster)
{
	ObjectList Playerlist = PlayerList();
	if (Playerlist.empty())
		return nullptr;

	for (auto& [playerid, ObjectRef] : Playerlist)
	{
		float targetRange = ObjectRef->GetSearchRange();

		if (pMonster->GetSearchRange() < targetRange)
			continue;

		return ObjectRef.get();

		// Player* pPlayer = (Player*)(ObjectRef.get());

		//Player* pPlayer = static_cast<Player*>(ObjectRef.get());
	}

	return nullptr;
}

void CZone::Update_Pos(int nObjectID, const Protocol::D3DVECTOR& vPos)
{
	if (m_nlistObject[Object::Player].contains(nObjectID))
	{
		/*
			이전 위치와 다른 플레이어만 업데이트.
			지금은 일단 모두  업데이트.

		*/
		m_nlistObject[Object::Player][nObjectID]->SetPos(vPos);
	}
}

bool CZone::Enter()
{
	READ_LOCK;
	return m_nlistObject[Object::Player].size() < m_nMaxUserCnt;
}

void CZone::BroadCasting(Protocol::S_MOVE_PLAYER& movepkt)
{
	//int exceptID = movepkt.playerid();
	//Protocol::S_MOVE_PLAYER movepkt;
	//movepkt.set_playerid(exceptID);
	////movepkt.set_allocated_pos(vPos);
	//Protocol::D3DVECTOR* Pos = movepkt.mutable_pos();
	//Pos->set_x(vPos.x());
	//Pos->set_y(vPos.y());
	//Pos->set_z(vPos.z());
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(movepkt);
	int nCnt = 0;
	for (auto& [playerid, ObjectRef] : m_nlistObject[Object::Player])
	{
		//if (nCnt > 10)
		//	break;

		if(ObjectRef->GetActivate() == false)
			continue;

		//if (playerid == exceptID)
		//	continue;

		CPlayer* pPlayer = static_cast<CPlayer*>(ObjectRef.get());
		if (pPlayer->ownerSession.expired()==false)
		{
			pPlayer->ownerSession.lock()->Send(sendBuffer);

		}
		else
		{
			pPlayer->SetActivate(false);
			///세션 소멸, 해당 플레이어 객체도 

		}


		nCnt++;
	}
}