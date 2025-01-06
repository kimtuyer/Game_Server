#include "pch.h"
#include "CZone.h"
#include "Player.h" //참조안하면 static_cast변환 실패
#include "CMonster.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
using namespace Zone;
CZone::CZone(int nMaxUserCnt, int nZoneID, Protocol::D3DVECTOR vPos)
	:m_bActivate(false), m_nMaxUserCnt(nMaxUserCnt), m_nZoneID(nZoneID)
	, m_vStartpos(vPos)
{
	m_nUserCnt.store(0);

	/*
		맵/ 몬스터 세팅

		몬스터 필요정보
		몬스터 id 5001~
		존에 따른 초기 위치 정보 -랜덤
		속한 존 id

	*/
	//임시 활성화,몹 테스트
	m_bActivate = true;

	int nSectorid = 1;
	for (int HEIGHT = 1; HEIGHT <= 4; HEIGHT++)	//콘솔 세로 1줄 개수
		for (int WIDTH = 1; WIDTH <= 4; WIDTH++) //콘솔 가로 1줄 개수
		{
			int col = 0;

			if (m_nZoneID <= Zone::ZONES_PER_ROW)
				col = 0;
			else if (m_nZoneID <= Zone::ZONES_PER_ROW * 2)
				col = 1;
			else if (m_nZoneID <= Zone::ZONES_PER_ROW * 3)
				col = 2;
			else if (m_nZoneID <= Zone::ZONES_PER_ROW * 4)
				col = 3;
			else
				col = 4;
			float x = ((m_nZoneID - 1) % Zone::ZONES_PER_ROW) * ZONE_WIDTH + (WIDTH - 1) * Zone::Sector_WIDTH + Sector_WIDTH / 2;
			float y = (col * ZONE_HEIGHT) + ((HEIGHT - 1) * Zone::Sector_HEIGHT) + Sector_HEIGHT / 2;

			Protocol::D3DVECTOR startpos;
			startpos.set_x(x);
			startpos.set_y(y);

			m_listSector.insert({ nSectorid,MakeShared<CSector>(nSectorid,m_nZoneID,startpos) });

			nSectorid++;
		}

	// 인접 섹터  리스트 구하기
	for (auto& [sectorID, Sector] : m_listSector)
	{
		float currnet_x = Sector->StartPos().x();
		float currnet_y = Sector->StartPos().y();
		int SectorID = 0;

		auto Dir = [&]()
			{
				for (auto& [x, y] : directions)
				{
					float new_x = x + currnet_x;
					float new_y = y + currnet_y;
					Set_AdjSector(new_x, new_y, Sector);
				}
			};
		Dir();
	}
	//	//위
	//	x = currnet_x;
	//	y = currnet_y - Zone::Sector_HEIGHT;
	//	Set_AdjSector(x, y, Sector);
	//
	//	//아래
	//	x = currnet_x;
	//	y = currnet_y + Zone::Sector_HEIGHT;
	//
	//	Set_AdjSector(x, y, Sector);
	//
	//	//좌
	//	x = currnet_x - Zone::Sector_WIDTH;
	//	y = currnet_y;
	//
	//	Set_AdjSector(x, y, Sector);
	//
	//	//오른쪽
	//	x = currnet_x + Zone::Sector_WIDTH;
	//	y = currnet_y;
	//	Set_AdjSector(x, y, Sector);
	//
	//	//왼위대각
	//	x = currnet_x - Zone::Sector_WIDTH;
	//	y = currnet_y - Zone::Sector_HEIGHT;
	//	Set_AdjSector(x, y, Sector);
	//
	//	//오른위대각
	//	x = currnet_x + Zone::Sector_WIDTH;
	//	y = currnet_y - Zone::Sector_HEIGHT;
	//	Set_AdjSector(x, y, Sector);
	//
	//	//왼아래대각
	//	x = currnet_x - Zone::Sector_WIDTH;
	//	y = currnet_y + Zone::Sector_HEIGHT;
	//	Set_AdjSector(x, y, Sector);
	//
	//	//오른아래대각
	//	x = currnet_x + Zone::Sector_WIDTH;
	//	y = currnet_y + Zone::Sector_HEIGHT;
	//	Set_AdjSector(x, y, Sector);

	////////////////////////////////////////////////////////
	if (MonsterMaxCount == 0)
		return;

	std::random_device rd;
	std::mt19937 gen(rd());

	int zoneid = m_nZoneID;
	int startID = (g_nZoneCount * g_nZoneUserMax) + (MonsterMaxCount * (m_nZoneID - 1)) + 1;
	int EndMaxID = (startID - 1) + MonsterMaxCount;

	for (; startID < EndMaxID; startID++)
	{
		// 1~max 섹터id를 랜덤으로 불러와 해당 섹터의 좌표로 몬스터를 배치.
		std::uniform_int_distribution<int> SectorList(1, Sector_Count);   // 0 ~ MAX_STEP
		int sectorID = SectorList(gen);
		auto Sector = m_listSector[sectorID];

		MonsterRef Monster = MakeShared<CMonster>(startID, m_nZoneID, sectorID, Sector->StartPos(), true);

		m_nlistObject[Object::Monster].insert({ startID,Monster });
		Sector->Insert(Object::Monster, Monster); //원래 ObjectRef 넣어야하는데..
	}
	//몬스터 색터 id 랜덤으로 넣어줄려면  1~16번 섹터중 랜덤으로 가져와 그 섹터의 좌표를 가져와 삽입!
}

CZone::~CZone()
{
	m_nlistObject.clear(); //shared_ptr도 바로 클리어 호출해도 되나?
}

bool CZone::_Enter(ObjectType eObjectType, ObjectRef object)
{
	int lock = lock::Object;
	WRITE_LOCK_IDX(lock);

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
		int lock = lock::Object;
		WRITE_LOCK_IDX(lock);

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
			int lock = lock::Player;
			WRITE_LOCK_IDX(lock);

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
	//임시로 주석,몹 테스트
	//if (m_nlistObject[Object::Player].empty())
	//{
	//	m_bActivate = false;
	//	return;
	//}
///////////////////////////////////////////////////////////////
#ifdef __SECTOR_UPDATE__
	for (auto& [sectorID, Sector] : m_listSector)
	{
		//if (Object->GetActivate() == false)
		//	continue;
		//if (Sector->Empty())
		//	continue;

		Sector->Update();
	}

	//삽입 삭제 리스트 각 섹터에 위치한 플레이어들에게 전송
	Send_SectorInsertObject();

	Send_SectorRemoveObject();

#ifdef __ZONE_THREAD__
	Send_SectorInsertPlayer();

	Send_SectorRemovePlayer();
#endif
	//Send_MonsterUpdateList();

#endif
	///////////////////////////////////////////////////////////////////////////
#ifdef __SECTOR_UPDATE__
#else

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

	 현재 몹 개수는 존당 200개, 다만 해당 몹 데이터 전부를 다 보낼필요는 없음.
	 유저입장에선 자신과같은 영역(카메라보이는)에 있는 몹정보만 받아서 출력하면됨

	 일단 섹터적용했단 가정하에 30개의 몹정보만 전송

	*/
	//Send_MonsterUpdateList();
	Send_PlayerUpdateList();
#endif

	//ZoneManager에서 모든 zone update 호출중
#ifdef __ZONE_THREAD__
#else
	DoTimer(Tick::AI_TICK, &CZone::Update);
#endif // __ZONE_THREAD__

}

void CZone::Update_Player()
{
	Send_SectorInsertPlayer();

	Send_SectorRemovePlayer();

#ifdef __ZONE_THREAD__
#else
	DoTimer(Tick::AI_TICK, &CZone::Update_Player);
#endif
}

CObject* CZone::SearchEnemy(CObject* pMonster)
{
	ObjectList Playerlist = PlayerList();
	if (Playerlist.empty())
		return nullptr;

	for (auto& [playerid, ObjectRef] : Playerlist)
	{
		float targetRange = ObjectRef->GetSearchRange();

		float dist = Util::distance(pMonster->GetPos().x(), pMonster->GetPos().y(), ObjectRef->GetPos().x(), ObjectRef->GetPos().y());

		if (dist > BroadCast_Distance)
			continue;
		return ObjectRef.get();

		// Player* pPlayer = (Player*)(ObjectRef.get());

		//Player* pPlayer = static_cast<Player*>(ObjectRef.get());
	}

	return nullptr;
}

void CZone::Update_Pos(Object::ObjectType eObjectType, int nObjectID, const Protocol::D3DVECTOR& vPos)
{
	if (eObjectType <  Object::Player || eObjectType > Object::Monster)
		return;

	if (m_nlistObject[eObjectType].contains(nObjectID))
	{
		/*
			이전 위치와 다른 플레이어만 업데이트.
			지금은 일단 모두  업데이트.

		*/
		m_nlistObject[eObjectType][nObjectID]->SetPos(vPos);
	}
}

bool CZone::Enter()
{
	int lock = lock::Player;
	READ_LOCK_IDX(lock);
	return m_nlistObject[Object::Player].size() < m_nMaxUserCnt;
}

bool CZone::UpdateSectorID(OUT int& nSectorID, Protocol::D3DVECTOR vPos)
{
	if (m_listSector.contains(nSectorID) == false)
		return false;

	CSectorRef Sector = m_listSector[nSectorID];

	if (Sector->BelongtoSector(vPos))
		return false;

	Protocol::D3DVECTOR prev_pos = Sector->StartPos();
	float prev_x = prev_pos.x();
	float prev_y = prev_pos.y();

	//기존 섹터에 속하지 않는 위치라면, 새로운 섹터 구해 삽입해야
	/*
		현재 있는 섹터 기준으로 둘러싼 8분위 섹터만 구해 그중 속하는지 확인
	*/
	//for(int i=)
	map<int, Protocol::D3DVECTOR> Adjlist = Sector->m_adjSectorList;
	for (auto& [SectorID, vPos] : Adjlist)
	{
		float Max_x = vPos.x() + Zone::Sector_WIDTH / 2;
		float Min_x = vPos.x() - Zone::Sector_WIDTH / 2;

		float Max_y = vPos.y() + Zone::Sector_HEIGHT / 2;
		float Min_y = vPos.y() - Zone::Sector_HEIGHT / 2;

		if ((Min_x <= vPos.x() && vPos.x() <= Max_x) &&
			Min_y <= vPos.y() && vPos.y() <= Max_y)
		{
			nSectorID = SectorID;
			return true;
		}
	}

	return false;
}

int CZone::GetInitSectorID()
{
	{
		std::random_device rd;
		std::mt19937 gen(rd());

		// 1~max 섹터id를 랜덤으로 불러와 해당 섹터의 좌표로 몬스터를 배치.
		std::uniform_int_distribution<int> SectorList(1, Sector_Count);   // 0 ~ MAX_STEP
		return  SectorList(gen);
		//auto Sector = m_listSector[sectorID];
	}
}

void CZone::Send_MonsterUpdateList()
{
	Protocol::S_MOVE_MONSTER objectpkt;
	objectpkt.set_sendtime(GetTickCount64());
	map<ObjectID, ObjectRef> MonsterList = m_nlistObject[Object::Monster];

	int nCnt = 0;
	for (auto& [objectid, Object] : MonsterList)
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

		Protocol::Object_Pos sData;
		sData.set_id(objectid);
		Protocol::D3DVECTOR* vPos = sData.mutable_vpos();
		Protocol::D3DVECTOR targetPos = Object->GetPos();
		vPos->set_x(targetPos.x());
		vPos->set_y(targetPos.y());
		vPos->set_z(targetPos.z());

		objectpkt.add_pos()->CopyFrom(sData);

		nCnt++;
		//CPlayer* pPlayer = static_cast<CPlayer*>(Object.get());
		//pPlayer->Update();
	}

	if (MonsterList.empty())
		return;

	BroadCast_Monster(objectpkt);
}

void CZone::Send_PlayerUpdateList()
{
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

		Protocol::Object_Pos sData;
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

	if (PlayerList.empty())
		return;

	BroadCast_Player(pkt);
}

void CZone::BroadCast_Monster(Protocol::S_MOVE_MONSTER& movepkt)
{
#ifdef __ZONE_THREAD__
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(movepkt, m_nZoneID);
#else
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(movepkt, m_nZoneID);
#endif // __ZONE_THREAD__	
	int nCnt = 0;
	for (auto& [playerid, ObjectRef] : m_nlistObject[Object::Player])
	{
		//if (nCnt > 10)
		//	break;

		if (ObjectRef->GetActivate() == false)
			continue;

		//if (playerid == exceptID)
		//	continue;

		CPlayer* pPlayer = static_cast<CPlayer*>(ObjectRef.get());
		if (pPlayer->ownerSession.expired() == false)
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

void CZone::BroadCast_Player(Protocol::S_MOVE_PLAYER& movepkt)
{
	//int exceptID = movepkt.playerid();
	//Protocol::S_MOVE_PLAYER movepkt;
	//movepkt.set_playerid(exceptID);
	////movepkt.set_allocated_pos(vPos);
	//Protocol::D3DVECTOR* Pos = movepkt.mutable_pos();
	//Pos->set_x(vPos.x());
	//Pos->set_y(vPos.y());
	//Pos->set_z(vPos.z());
#ifdef __ZONE_THREAD__
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(movepkt, m_nZoneID);
#else
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(movepkt, m_nZoneID);
#endif // __ZONE_THREAD__	
	int nCnt = 0;
	for (auto& [playerid, ObjectRef] : m_nlistObject[Object::Player])
	{
		//if (nCnt > 10)
		//	break;

		if (ObjectRef->GetActivate() == false)
			continue;

		//if (playerid == exceptID)
		//	continue;

		CPlayer* pPlayer = static_cast<CPlayer*>(ObjectRef.get());
		if (pPlayer->ownerSession.expired() == false)
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

void CZone::Set_AdjSector(float x, float y, CSectorRef Sector)
{
	int nSectorID = 0;
	auto SectorIDExit = [&](float x, float y) ->int
		{
			int SectorID = 0;
			for (auto& [sectorID, Sector] : m_listSector)
			{
				float currnet_x = Sector->StartPos().x();
				float currnet_y = Sector->StartPos().y();

				if (x == currnet_x && currnet_y == y)
					SectorID = sectorID;
			}

			return SectorID;
		};

	nSectorID = SectorIDExit(x, y);
	if (nSectorID != 0)
		Sector->Insert_adjSector(nSectorID, x, y);
}

void CZone::Insert_ObjecttoSector(Sector::ObjectInfo object)
{
	int lock = lock::Monster;
	WRITE_LOCK_IDX(lock);
	m_InsertList[object.nSectorID].push_back(object);
}

void CZone::Remove_ObjecttoSector(Sector::ObjectInfo object)
{
	int lock = lock::Monster;
	WRITE_LOCK_IDX(lock);
	m_RemoveList[object.nSectorID].push_back(object);
}

void CZone::Insert_PlayertoSector(Sector::ObjectInfo object)
{
	int lock = lock::Player;
	WRITE_LOCK_IDX(lock);
	m_PlayerInsertList[object.nSectorID].push_back(object);
	//m_listSector[sectorID]->Insert(Object::Player, Player);
}

void CZone::Remove_PlayertoSector(Sector::ObjectInfo object)
{
	int lock = lock::Player;
	WRITE_LOCK_IDX(lock);
	m_PlayerRemoveList[object.nSectorID].push_back(object);
	//m_listSector[sectorID]->Delete(Object::Player, Player);
}

void CZone::Send_SectorInsertObject()
{
	//섹터리스트 복사해와서 lock 줄여야하나?
	map<SectorID, vector<Sector::ObjectInfo>> InsertList;
	{
		//swap후 원본 컨테이너는 clear상태, 이후에 들어온 데이터는 다음tick에 처리!
		int lock = lock::Monster;
		WRITE_LOCK_IDX(lock);
		InsertList.swap(m_InsertList);
	}
	//WRITE_LOCK;

	for (auto& [SectorID, sData] : InsertList)
	{
		CSectorRef Sector = m_listSector[SectorID];
		if (Sector->Empty_Player())
			continue;
		Protocol::S_OBJ_LIST objpkt;
#ifdef __BROADCAST_DISTANCE__
		int64 nowtime = GetTickCount64();
		//auto distance = [](float source_x, float source_y, float target_x, float target_y)->float
		//	{
		//		return sqrt(pow(target_x - source_x, 2) + pow(target_y - source_y, 2));
		//
		//	};

		objpkt.set_sendtime(GetTickCount64());
		auto Playerlist = Sector->PlayerList();
		for (auto& [playerid, Player] : Playerlist)
		{
			int nCnt = 0;

			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{
				ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];
				Sector->Insert(ObjectInfo.nObjectType, Object);

				float dist = Util::distance(ObjectInfo.vPos.x, ObjectInfo.vPos.y, Player->GetPos().x(), Player->GetPos().y());

				if (dist > BroadCast_Distance)
				{
					continue;
				}

				bSend = true;
				nCnt++;

				Protocol::Object_Pos objectPos;
				objectPos.set_id(ObjectInfo.nObjectID);
				Protocol::D3DVECTOR* vPos = objectPos.mutable_vpos();
				vPos->set_x(ObjectInfo.vPos.x);
				vPos->set_y(ObjectInfo.vPos.y);

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

#else

		objpkt.set_sendtime(GetTickCount64());
		for (auto& ObjectInfo : sData)
		{
			ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];

			Sector->Insert(ObjectInfo.nObjectType, Object);

			Protocol::Object_Pos objectPos;
			objectPos.set_id(ObjectInfo.nObjectID);
			Protocol::D3DVECTOR* vPos = objectPos.mutable_vpos();
			vPos->set_x(ObjectInfo.vPos.x);
			vPos->set_y(ObjectInfo.vPos.y);

			objpkt.add_pos()->CopyFrom(objectPos);

			//objpkt.set
		}
		//int ncnt = 0;
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt);
		auto Playerlist = Sector->PlayerList();
		for (auto& [playerid, ObjectRef] : Playerlist)
		{
			//if (ncnt >= 10)
			//	return;
			CPlayer* pPlayer = static_cast<CPlayer*>(ObjectRef.get());
			if (pPlayer->ownerSession.expired() == false)
			{
				pPlayer->ownerSession.lock()->Send(sendBuffer);
			}
			else
			{
				pPlayer->SetActivate(false);
				///세션 소멸, 해당 플레이어 객체도
			}
			//ncnt++;
		}
#endif // __BROADCAST_DISTANCE__
	}
}

void CZone::Send_SectorRemoveObject()
{
	map<SectorID, vector<Sector::ObjectInfo>> RemoveList;
	{
		//swap후 원본 컨테이너는 clear상태, 이후에 들어온 데이터는 다음tick에 처리!
		int lock = lock::Monster;
		WRITE_LOCK_IDX(lock);
		RemoveList.swap(m_RemoveList);
	}
	//WRITE_LOCK;
	for (auto& [SectorID, sData] : RemoveList)
	{
		CSectorRef Sector = m_listSector[SectorID];

		if (Sector->Empty_Player())
			continue;

#ifdef __BROADCAST_DISTANCE__
		int64 nowtime = GetTickCount64();
		//auto distance = [](float source_x, float source_y, float target_x, float target_y)->float
		//	{
		//		return sqrt(pow(target_x - source_x, 2) + pow(target_y - source_y, 2));
		//
		//	};

		Protocol::S_OBJ_REMOVE_ACK objpkt;
		objpkt.set_sendtime(GetTickCount64());
		auto Playerlist = Sector->PlayerList();
		for (auto& [playerid, Player] : Playerlist)
		{
			int nCnt = 0;

			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{
				ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];
				Sector->Delete(ObjectInfo.nObjectType, Object);

				float dist = Util::distance(ObjectInfo.vPos.x, ObjectInfo.vPos.y, Player->GetPos().x(), Player->GetPos().y());

				if (dist > BroadCast_Distance)
				{
					continue;
				}

				bSend = true;

				Protocol::Object_Pos objectPos;
				objectPos.set_id(ObjectInfo.nObjectID);
				Protocol::D3DVECTOR* vPos = objectPos.mutable_vpos();
				vPos->set_x(ObjectInfo.vPos.x);
				vPos->set_y(ObjectInfo.vPos.y);

				objpkt.add_pos()->CopyFrom(objectPos);
			}
			if (bSend == false)
				continue;
			nCnt++;

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

#else
		Protocol::S_OBJ_REMOVE_ACK objpkt;
		objpkt.set_sendtime(GetTickCount64());
		for (auto& ObjectInfo : sData)
		{
			ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];

			Sector->Delete(ObjectInfo.nObjectType, Object);

			Protocol::Object_Pos objectPos;
			objectPos.set_id(ObjectInfo.nObjectID);
			Protocol::D3DVECTOR* vPos = objectPos.mutable_vpos();
			vPos->set_x(ObjectInfo.vPos.x);
			vPos->set_y(ObjectInfo.vPos.y);

			objpkt.add_pos()->CopyFrom(objectPos);

			//objpkt.set
		}

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt);
		auto PlayerList = Sector->PlayerList();
		for (auto& [playerid, ObjectRef] : PlayerList)
		{
			CPlayer* pPlayer = static_cast<CPlayer*>(ObjectRef.get());
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
#endif
	}
}

void CZone::Send_SectorInsertPlayer()
{
	//섹터리스트 복사해와서 lock 줄여야하나?
	map<SectorID, vector<Sector::ObjectInfo>> InsertList;
	{
		//swap후 원본 컨테이너는 clear상태, 이후에 들어온 데이터는 다음tick에 처리!
		int lock = lock::Player;
		WRITE_LOCK_IDX(lock);
		InsertList.swap(m_PlayerInsertList);
	}
	//WRITE_LOCK;

	for (auto& [SectorID, sData] : InsertList)
	{
		CSectorRef Sector = m_listSector[SectorID];
		if (Sector->Empty_Player())
			continue;
		Protocol::S_PLAYER_LIST objpkt;
		int64 nowtime = GetTickCount64();
		//auto distance = [](float source_x, float source_y, float target_x, float target_y)->float
		//	{
		//		return sqrt(pow(target_x - source_x, 2) + pow(target_y - source_y, 2));
		//
		//	};

		objpkt.set_sendtime(GetTickCount64());
		auto Playerlist = Sector->PlayerList();
		for (auto& [playerid, Player] : Playerlist)
		{
			int nCnt = BroadCast_Cnt;

			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{
				if (nCnt <= 0)
					break;
				ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];
				Sector->Insert(ObjectInfo.nObjectType, Object);

				float dist = Util::distance(ObjectInfo.vPos.x, ObjectInfo.vPos.y, Player->GetPos().x(), Player->GetPos().y());

				if (dist > BroadCast_Distance)
				{
					continue;
				}

				bSend = true;
				nCnt--;

				Protocol::Object_Pos objectPos;
				objectPos.set_id(ObjectInfo.nObjectID);
				Protocol::D3DVECTOR* vPos = objectPos.mutable_vpos();
				vPos->set_x(ObjectInfo.vPos.x);
				vPos->set_y(ObjectInfo.vPos.y);

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
}

void CZone::Send_SectorRemovePlayer()
{
	map<SectorID, vector<Sector::ObjectInfo>> RemoveList;
	{
		//swap후 원본 컨테이너는 clear상태, 이후에 들어온 데이터는 다음tick에 처리!
		int lock = lock::Player;
		WRITE_LOCK_IDX(lock);
		RemoveList.swap(m_PlayerRemoveList);
	}
	//int nCnt = 0;
	//WRITE_LOCK;
	for (auto& [SectorID, sData] : RemoveList)
	{
		CSectorRef Sector = m_listSector[SectorID];

		if (Sector->Empty_Player())
			continue;

		int64 nowtime = GetTickCount64();
		//auto distance = [](float source_x, float source_y, float target_x, float target_y)->float
		//	{
		//		return sqrt(pow(target_x - source_x, 2) + pow(target_y - source_y, 2));
		//
		//	};

		Protocol::S_PLAYER_REMOVE_ACK objpkt;
		objpkt.set_sendtime(GetTickCount64());
		auto Playerlist = Sector->PlayerList();
		for (auto& [playerid, Player] : Playerlist)
		{
			int nCnt = BroadCast_Cnt;

			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{
				if (nCnt <= 0)
					break;

				ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];
				Sector->Delete(ObjectInfo.nObjectType, Object);

				float dist = Util::distance(ObjectInfo.vPos.x, ObjectInfo.vPos.y, Player->GetPos().x(), Player->GetPos().y());

				if (ObjectInfo.nObjectID == playerid)
					continue;

				if (dist > BroadCast_Distance)
				{
					continue;
				}

				bSend = true;

				Protocol::Object_Pos objectPos;
				objectPos.set_id(ObjectInfo.nObjectID);
				Protocol::D3DVECTOR* vPos = objectPos.mutable_vpos();
				vPos->set_x(ObjectInfo.vPos.x);
				vPos->set_y(ObjectInfo.vPos.y);

				objpkt.add_pos()->CopyFrom(objectPos);

				nCnt--;
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
}
