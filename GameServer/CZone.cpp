#include "pch.h"
#include "CZone.h"
#include "Player.h" //참조안하면 static_cast변환 실패
#include "CMonster.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "CSector.h"
#include "CZone_Manager.h"
#include "CPlayerManager.h"

using namespace Zone;
static int g_nSectorID = 1;
CZone::CZone(int nMaxUserCnt, int nZoneID, Protocol::D3DVECTOR vPos)
	:m_bActivate(false), m_nMaxUserCnt(nMaxUserCnt), m_nZoneID(nZoneID), m_nBeginSecID(0), m_nEndSecID(0)
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
#ifdef __DOP__
	m_vecSector.resize(SECTORS_PER_SIDE * SECTORS_PER_SIDE);
#endif
	//int nSectorid = 1;
	m_nBeginSecID = g_nSectorID;
	for (int HEIGHT = 1; HEIGHT <= SECTORS_PER_SIDE; HEIGHT++)	//콘솔 세로 1줄 개수
		for (int WIDTH = 1; WIDTH <= SECTORS_PER_SIDE; WIDTH++) //콘솔 가로 1줄 개수
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
#ifdef __DOP__
			//m_vecSector.emplace_back(nSectorid, m_nZoneID, startpos);
			m_vecSector[nSectorid-1].m_nSectorID = nSectorid;
			m_vecSector[nSectorid-1].m_nZoneID = m_nZoneID;
			m_vecSector[nSectorid-1].SetStartpos(startpos);

			int index = (int)m_vecSector.size() - 1;
			m_mapSector.insert({ nSectorid,index });
#else
			m_listSector.insert({ g_nSectorID,MakeShared<CSector>(g_nSectorID,m_nZoneID,startpos) });

#endif // __DOP__

			g_nSectorID++;
		}

	m_nEndSecID = g_nSectorID - 1;
#ifdef __SEAMLESS__
	//인접한 존 리스트 생성
	Set_Neighbor_list();

#else

//	// 인접 섹터  리스트 구하기
#ifdef __DOP__
	for (int i=0; i<m_vecSector.size(); i++)
#else
	for (auto& [sectorID, Sector] : m_listSector)
#endif	
	{
#ifdef __DOP__
		float currnet_x = m_vecSector[i].StartPos().x();
		float currnet_y = m_vecSector[i].StartPos().y();
#else
		float currnet_x = Sector->StartPos().x();
		float currnet_y = Sector->StartPos().y();
#endif	
		int SectorID = 0;
#ifdef __DOP__
		CSectorRef pSector = MakeShared<CSector>(Sector);
#endif
#ifdef __DOP__
		auto Dir = [&](CSector& pSector)
#else
		auto Dir = [&]()
#endif
			{
				for (auto& [x, y] : directions)
				{
					float new_x = x + currnet_x;
					float new_y = y + currnet_y;
#ifdef __DOP__
					Set_AdjSector(new_x, new_y, pSector);

#else	
					_Set_AdjSector(new_x, new_y, Sector);
#endif
				}
			};
#ifdef __DOP__
		Dir(m_vecSector[i]);
#else
		Dir();

#endif
	}
#endif //__SEAMLESS__


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
		std::uniform_int_distribution<int> SectorList(g_nSectorID- Sector_Count, g_nSectorID-1);   // 0 ~ MAX_STEP
		int sectorID = SectorList(gen);
#ifdef __DOP__
		auto Sector = GetSector(sectorID);//&m_vecSector[sectorID];
		if (Sector == nullptr)
			continue;
#else
		auto Sector = m_listSector[sectorID];
#endif
#ifdef __DOP__
		Sector::MonsterData sData;
		sData.nObjectID = startID;
		sData.nSectorID = sectorID;
		sData.nZoneID = m_nZoneID;
		sData.vPos.x = Sector->StartPos().x();
		sData.vPos.y = Sector->StartPos().y();
		sData.vPos.z = Sector->StartPos().z();

		Sector->Insert_Monster(sData, true);
#else
		MonsterRef Monster = MakeShared<CMonster>(startID, m_nZoneID, sectorID, Sector->StartPos(), true);
		ObjectRef pObject = Monster;

		m_nlistObject[Object::Monster].insert({ startID,Monster });
		Sector->Insert(Object::Monster, pObject); //원래 ObjectRef 넣어야하는데..
#endif // __DOP__

	}
	//몬스터 색터 id 랜덤으로 넣어줄려면  1~16번 섹터중 랜덤으로 가져와 그 섹터의 좌표를 가져와 삽입!
}

CZone::~CZone()
{
	m_nlistObject.clear(); //shared_ptr도 바로 클리어 호출해도 되나?
}

void CZone::SetAdjSector()
{

	// 인접 섹터  리스트 구하기
#ifdef __DOP__
	for (int i = 0; i < m_vecSector.size(); i++)
#else
	for (auto& [sectorID, Sector] : m_listSector)
#endif	
	{
#ifdef __DOP__
		float currnet_x = m_vecSector[i].StartPos().x();
		float currnet_y = m_vecSector[i].StartPos().y();
#else
		float currnet_x = Sector->StartPos().x();
		float currnet_y = Sector->StartPos().y();
#endif	
		int SectorID = sectorID;
#ifdef __DOP__
		CSectorRef pSector = MakeShared<CSector>(Sector);
#endif
#ifdef __DOP__
		auto Dir = [&](CSector& pSector)
#else
		auto Dir = [&]()
#endif
			{
				for (auto& [x, y] : directions)
				{
					float new_x = x + currnet_x;
					float new_y = y + currnet_y;
#ifdef __DOP__
					Set_AdjSector(new_x, new_y, pSector);

#else	
					_Set_AdjSector(new_x, new_y, Sector);
#endif
				}
			};
#ifdef __DOP__
		Dir(m_vecSector[i]);
#else
		Dir();

#endif
	}
}

bool CZone::_Enter(ObjectType eObjectType, PlayerRef& object)
{
	int lock = lock::Player;
	if (eObjectType == Object::Monster)
	{
		lock = lock::Monster;
	}
	else if (eObjectType == Object::Player)
	{
		lock = lock::Player;
	}
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

bool CZone::_EnterMonster(ObjectType eObjectType , ObjectRef object)
{
	int lock = lock::Object;
	WRITE_LOCK_IDX(lock);


	if (object == nullptr)
	{
		//cout << "_EnterMonster" << endl;
	}

	if (eObjectType <  Object::Player || eObjectType > Object::Monster)
		return false;

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
			auto listObject = (*it).second;
			auto iter = listObject.find(objectID);
			if (iter != listObject.end())
			{
#ifdef __DOP__
				auto Sector = GetSector((*iter).second->GetSectorID());
#else
				auto Sector = GetSectorRef((*iter).second->GetSectorID());
#endif // __DOP__
				listObject.erase(iter);
				if (Sector == nullptr)
					return;
				Sector->Delete(eObjectType, objectID);
				//listObject.erase(iter);
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
			//if (m_nlistObject[Object::Player].empty())
			//	m_bActivate = false;
		}
	}
	/*
		해당 존, 섹터 위치한 다른 유저들에게 브로드캐스팅
		S_OBJ_REMOVE_ACK

	*/
}

ObjectRef CZone::Object(ObjectType type, int objectID)
{
	int lock = 0;
	switch (type)
	{
		case Object::Player:
		{
			 lock = lock::Player;			
		}
		break;
		case Object::Monster:
		{
			 lock = lock::Monster;		
		}
		break;
		default:
		return nullptr;
		break;
	}

	{
		READ_LOCK_IDX(lock);

		auto pObjectRef = m_nlistObject[type].find(objectID);
		if (pObjectRef == m_nlistObject[type].end())
			return nullptr;
		return ((*pObjectRef).second);
	}

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
	int zone = m_nZoneID;
#ifdef __DOP__
	for (auto& Sector : m_vecSector)
#else	
	for (auto& [sectorID, Sector] : m_listSector)
#endif	
	{
#ifdef __DOP__
		Sector.Update();
#else
		if (Sector == nullptr)
			continue;
		Sector->Update();		

#endif	
	}

#ifdef __SEAMLESS__
	//다른 존에서 넘겨준 이웃한 섹터의 오브젝트 리스트 들어있음.
	/*
		이 섹터들과 이웃한 섹터에 위치한 유저들에게 알려줘야함!
	*/

	Send_AdjSector_ObjList();

#endif // __SEAMLESS__

	//삽입 삭제 리스트 각 섹터에 위치한 플레이어들에게 전송

	Send_SectorInsertObject();

	Send_SectorRemoveObject();

#ifdef __ZONE_THREAD__
	Send_SectorInsertPlayer();

	Send_SectorRemovePlayer();
#endif
	//Send_MonsterUpdateList();

/*
 내 섹터정보 필요한 존이 누구인지, 몇번 섹터정보가 필요한지 여부
 알아야함.

 1.내 섹터 정보 필요한 이웃 존에게 해당 섹터내 오브젝트 정보
 각 존의 메시지 큐로 전송

 2. 내 존메시지큐 로  새로운 컨테이너 구성


 3. 이전에 가진 컨테이너와 비교하면서 Remove, Insert 객체 리스트 구성

*/
#ifdef __SEAMLESS__

	for (auto& [sectorID, Sector] : m_listSector)
	{
		auto adjsectorlist = Sector->GetAdjSectorlist();
		int nSendZoneID = 0;
		//해당 섹터 정보 필요한 존은?
		for (auto [SecID, Data] : adjsectorlist)
		{
			int nZoneID = Data.first;
			//같은 존의 섹터는 continue;
			if (nZoneID == m_nZoneID)
				continue;

			//해당 섹터정보를 이미 보낸 존은 Pass
			if (nSendZoneID == nZoneID)
				continue;

			nSendZoneID = nZoneID;

			//이웃 존에게 필요한 내 섹터 객체정보 넘겨주기
			/*

			*/
			//이때 업데이트필요한 유저만 식별 가능?
			CZoneRef AdjZone = GZoneManager->GetZone(nZoneID);
			if (AdjZone == nullptr)
				continue;

			auto Playerlist = Sector->PlayerInfoList();
			if(Playerlist.empty()== false)
				AdjZone->DoZoneJobTimer(1000, nZoneID, &CZone::Update_AdjSector, sectorID, nZoneID, Playerlist);
				//AdjZone->DoLogicJob(nZoneID, &CZone::Update_AdjSector, sectorID, nZoneID, Playerlist);

			auto Monsetlist = Sector->MonsterInfoList();
			if (Monsetlist.empty() == false)
				AdjZone->DoZoneJobTimer(1000, nZoneID, &CZone::Update_AdjSector, sectorID, nZoneID, Monsetlist);
				//AdjZone->DoLogicJob(nZoneID, &CZone::Update_AdjSector, sectorID, nZoneID, Monsetlist);

		}
	}
#endif // __SEAMLESS__

	///////////////////////////////////////////////////////////////////////////
#ifdef __ZONE_THREAD__
#else
	DoTimer(Tick::AI_TICK, &CZone::Update);
#endif // __ZONE_THREAD__

}
#ifdef __SEAMLESS__
void CZone::Update_AdjSector(int nSectorID, int nZone, map<ObjectID, Sector::ObjectInfo>adjSectorInfoList)
{
	/*
		이웃한 다른 섹터의 정보를 받아 새로 업뎃해야함.	
	*/
	/*int lock = lock::Object;
	WRITE_LOCK_IDX(lock);*/

	int nThreadID = LThreadId;

	auto Objlist = AdjObjectList[nSectorID];
	for (auto [ObjectID, Objinfo] : Objlist)
	{
		if (adjSectorInfoList.contains(ObjectID) == false)
		{
			//경계 오브젝트리스트 에서도 삭제!
			AdjObjectList[nSectorID].erase(ObjectID);
		}
	

	}
	//경계섹터 새로 들어온 몹 추가 및 갱신
	for (auto [ObjectID, Objinfo] : adjSectorInfoList)
	{
		

		if (AdjObjectList[nSectorID].contains(ObjectID))
		{
			AdjObjectList[nSectorID][ObjectID] = Objinfo;
		}
		else
			AdjObjectList[nSectorID].insert({ ObjectID,Objinfo });
	}

	////이전에 갖고있던 경계 섹터의 몹정보가 없을 경우, 삭제리스트 넣어 갱신!



}
#endif // __SEAMLESS__

void CZone::Update_Player()
{
	Send_SectorInsertPlayer();

	Send_SectorRemovePlayer();

#ifdef __ZONE_THREAD__
#else
	DoTimer(Tick::AI_TICK, &CZone::Update_Player);
#endif
}

void CZone::Update_Partial(int beginSectorID, int endSectorID)
{
	int zone = m_nZoneID;
	for (int secID = beginSectorID; secID <= endSectorID; secID++)
	{
#ifdef __DOP__
		auto sector = &m_vecSector[secID];

#else
		auto sector = m_listSector[secID];
		if (sector == nullptr)
			continue;
		m_listSector[secID]->Update();
#endif
	}

	Send_SectorInsertObject(beginSectorID, endSectorID);

	Send_SectorRemoveObject(beginSectorID, endSectorID);

	Send_SectorInsertPlayer(beginSectorID, endSectorID);

	Send_SectorRemovePlayer(beginSectorID, endSectorID);

}

void CZone::Send_SectorInsertObject(int beginSectorID, int endSectorID)
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
		if (Util::SectorRange(beginSectorID, endSectorID,SectorID) == false)
			continue;
#ifdef __DOP__
		auto Sector = GetSector(SectorID);
		if (Sector == nullptr)
			continue;
#else
		CSectorRef Sector = m_listSector[SectorID];
#endif // __DOP__

		if (Sector->Empty_Player())
			continue;
		
		auto Playerlist = Sector->PlayerList();
		for (auto& [playerid, Player] : Playerlist)
		{
			Protocol::S_OBJ_LIST objpkt;
			int64 nowtime = GetTickCount64();

			objpkt.set_sendtime(nowtime);

			int nCnt = 0;

			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{
				
				if(m_nlistObject[ObjectInfo.nObjectType].contains(ObjectInfo.nObjectID) == false)
					continue;
				ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];
#ifdef __DOP__
				Sector::MonsterData sData;
				{
					sData.nObjectID = ObjectInfo.nObjectID;
					sData.nSectorID = ObjectInfo.nSectorID;
					sData.vPos = ObjectInfo.vPos;
					sData.nZoneID = m_nZoneID;
				}
				Sector->Insert_Monster(sData, true);
#else
				Sector->Insert(ObjectInfo.nObjectType, Object);
			
#endif // __DOP__

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

				objectPos.set_zoneid(ObjectInfo.nZoneID);
				objectPos.set_secid(ObjectInfo.nSectorID);
			
				objectPos.set_objecttype(ObjectInfo.nObjectType);

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

void CZone::Send_SectorRemoveObject(int beginSectorID, int endSectorID)
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
		if (Util::SectorRange(beginSectorID, endSectorID, SectorID) == false)
			continue;
#ifdef __DOP__
		CSector* Sector = GetSector(SectorID);//m_listSector[SectorID];
		if (Sector == nullptr)
			continue;
#else
		CSectorRef Sector = m_listSector[SectorID];
#endif // __DOP__

		if (Sector->Empty_Player())
			continue;

		
		auto Playerlist = Sector->PlayerList();
		for (auto& [playerid, Player] : Playerlist)
		{
			int64 nowtime = GetTickCount64();

			Protocol::S_OBJ_REMOVE_ACK objpkt;
			objpkt.set_sendtime(nowtime);

			int nCnt = 0;

			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{

				if (m_nlistObject[ObjectInfo.nObjectType].contains(ObjectInfo.nObjectID) == false)
					continue;

				ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];				
#ifdef __DOP__				
				Sector->Delete_Monster(ObjectInfo.nObjectID);
#else
				Sector->Delete(ObjectInfo.nObjectType, Object->ObjectID());
#endif
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


	}
}

void CZone::Send_SectorInsertPlayer(int beginSectorID, int endSectorID)
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
		if (Util::SectorRange(beginSectorID, endSectorID, SectorID) == false)
			continue;
#ifdef __DOP__
		CSector* Sector = GetSector(SectorID);//m_listSector[SectorID];
		if (Sector == nullptr)
			continue;
#else
		CSectorRef Sector = m_listSector[SectorID];
#endif	
		if (Sector->Empty_Player())
			continue;

		
		auto Playerlist = Sector->PlayerList();
		for (auto& [playerid, Player] : Playerlist)
		{
			int64 nowtime = GetTickCount64();
			Protocol::S_PLAYER_LIST objpkt;
			objpkt.set_sendtime(nowtime);
			int nCnt = BroadCast_Cnt;

			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{
				if (nCnt <= 0)
					break;
				ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];
				if (Object == nullptr)
					continue;
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
			
			int size = objpkt.ByteSizeLong();
			if (size >= 6000)
			{
				string data;
				objpkt.SerializeToString(&data);
				int length = data.length();

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
}

void CZone::Send_SectorRemovePlayer(int beginSectorID, int endSectorID)
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
		if (Util::SectorRange(beginSectorID, endSectorID, SectorID) == false)
			continue;
#ifdef __DOP__
		CSector* Sector = GetSector(SectorID);//m_listSector[SectorID];
		if (Sector == nullptr)
			continue;
#else
		CSectorRef Sector = m_listSector[SectorID];
#endif
		if (Sector->Empty_Player())
			continue;

		auto Playerlist = Sector->PlayerList();
		for (auto& [playerid, Player] : Playerlist)
		{
			int64 nowtime = GetTickCount64();
			Protocol::S_PLAYER_REMOVE_ACK objpkt;
			objpkt.set_sendtime(nowtime);

			int nCnt = BroadCast_Cnt;

			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{
				if (nCnt <= 0)
					break;
				/*
					해당 유저 존이동해, 앞서 존에서 이미 삭제, 섹터에서도 삭제할 경우
					아래에서 유저 조회시 false나올것.
				*/
				if (m_nlistObject[ObjectInfo.nObjectType].contains(ObjectInfo.nObjectID) )
				{
					Sector->Delete(ObjectInfo.nObjectType, ObjectInfo.nObjectID);

				}
			/*	ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];
				if (Object == nullptr)
					continue;*/
				//Sector->Delete(ObjectInfo.nObjectType, Object->ObjectID());

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

			int size = objpkt.ByteSizeLong();
			if (size >= 6000)
			{
				string data;
				objpkt.SerializeToString(&data);
				int length = data.length();

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
}

CObject* CZone::SearchEnemy(CObject* pMonster)
{
	ObjectList Playerlist = PlayerList();
	if (Playerlist.empty())
		return nullptr;

	//lock이 있어야, 해당 오브젝트의 접속이 끊겨서 삭제하려할때
	//접근 막을수 있음.
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
		auto Object = m_nlistObject[eObjectType][nObjectID];
		if (Object == nullptr)
		{
			//cout << "" << endl;
		}
		m_nlistObject[eObjectType][nObjectID]->SetPos(vPos);
	}
}

void CZone::SetActivate(bool bFlag)
{
	m_bActivate = bFlag;
}

bool CZone::Enter()
{
	int lock = lock::Player;
	READ_LOCK_IDX(lock);
	return m_nUserCnt < m_nMaxUserCnt;
	//return m_nlistObject[Object::Player].size() < m_nMaxUserCnt;
}

bool CZone::UpdateSectorID(OUT int& nSectorID, OUT int& nMoveZone, int nObjectType, Protocol::D3DVECTOR vPos)
{
#ifdef __DOP__
	CSector* Sector = GetSector(nSectorID);//m_listSector[SectorID];
	if (Sector == nullptr)
		return false;;
#else
	if (m_listSector.contains(nSectorID) == false)
		return false;

	CSectorRef Sector = m_listSector[nSectorID];
#endif //__DOP__
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
	if (nObjectType == Object::Player)
	{
		//cout << "" << endl;
	}
	else
	{
		//cout << "" << endl;

	}
	auto Adjlist = Sector->GetAdjSectorlist();
	for (auto& [SectorID, Data] : Adjlist)
	{
		float Max_x = Data.second.x() + Zone::Sector_WIDTH / 2;
		float Min_x = Data.second.x() - Zone::Sector_WIDTH / 2;

		float Max_y = Data.second.y() + Zone::Sector_HEIGHT / 2;
		float Min_y = Data.second.y() - Zone::Sector_HEIGHT / 2;

		if ((Min_x <= vPos.x() && vPos.x() <= Max_x) &&
			Min_y <= vPos.y() && vPos.y() <= Max_y)
		{
			if (Data.first != m_nZoneID)
				nMoveZone = Data.first;

			nSectorID = SectorID;
			return true;
		}

	/*	if ((Min_x <= Data.second.x() && Data.second.x() <= Max_x) &&
			Min_y <= Data.second.y() && Data.second.y() <= Max_y)
		{
			if (Data.first != m_nZoneID)
				nMoveZone = Data.first;

			nSectorID = SectorID;
			return true;
		}*/
	}

	return false;
}

int CZone::GetInitSectorID()
{
	{
		std::random_device rd;
		std::mt19937 gen(rd());

		// 1~max 섹터id를 랜덤으로 불러와 해당 섹터의 좌표로 몬스터를 배치.
		 
		int nzone=m_nZoneID;
		std::uniform_int_distribution<int> SectorList(m_nBeginSecID, m_nEndSecID);   // 0 ~ MAX_STEP


		//심리스 구현 하기전엔 섹터 번호 구간이 모든 존이 1~Sector_Count 로 같았음.
		//std::uniform_int_distribution<int> SectorList(1, Sector_Count);   // 0 ~ MAX_STEP
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

void CZone::BroadCast(vector<ObjectRef> list, SendBufferRef sendBuffer)
{
	for (auto player : list)
	{
		if (player->GetActivate() == false)
			continue;

		CPlayer* pPlayer = static_cast<CPlayer*>(player.get());
		pPlayer->ownerSession.lock()->Send(sendBuffer);
	}

}

#ifdef __DOP__			
void CZone::Set_AdjSector(float x, float y, CSector& SectorRef)
#else
void CZone::_Set_AdjSector(float x, float y, CSectorRef SectorRef)
#endif
{
	//int nSectorID = 0;
	auto SectorIDExit = [&](float x, float y,OUT int& zoneID) ->int
		{
			int nSectorID = 0;
#ifdef __DOP__			
			for (auto&  Sector : m_vecSector)
#else
			for (auto& [sectorID, Sector] : m_listSector)
#endif
			{
#ifdef __DOP__
				float currnet_x = Sector.StartPos().x();
				float currnet_y = Sector.StartPos().y();

				if (x == currnet_x && currnet_y == y)
					SectorID = Sector.GetSecID();
#else
				int mmSecid = SectorRef->GetSecID();
				if (SectorRef->GetSecID() == sectorID)
					continue;

				float currnet_x = Sector->StartPos().x();
				float currnet_y = Sector->StartPos().y();

				if (x == currnet_x && currnet_y == y)
				{
					nSectorID = sectorID;
					return nSectorID;

				}


#endif	
			}
		
#ifdef __SEAMLESS__
			//이웃한 존의 섹터리스트
			for (auto& ZoneID : m_setAdjZone)
			{
				map<SectorID, CSectorRef> m_AdjSectorlist;
				auto ZoneRef = GZoneManager->GetZone(ZoneID);
				if (ZoneRef == nullptr)
					continue;

				m_AdjSectorlist = ZoneRef->GetSectorList();

				for (auto& [sectorID, Sector] : m_AdjSectorlist)
				{
					float currnet_x = Sector->StartPos().x();
					float currnet_y = Sector->StartPos().y();

					if (x == currnet_x && currnet_y == y)
					{
						zoneID = ZoneID;
						nSectorID = sectorID;
						return nSectorID;
					}


				}
			}
#endif

			return nSectorID;
		};

	int nZoneID = m_nZoneID;
	int nSectorID = SectorIDExit(x, y, nZoneID);
	if (nSectorID != 0)
#ifdef __DOP__
		SectorRef.Insert_adjSector(nSectorID, x, y);
#else
		SectorRef->Insert_adjSector(nSectorID, x, y, nZoneID);

#endif
}

void CZone::Insert_ObjecttoSector(Sector::ObjectInfo object)
{
	/*int lock = lock::Insert_M;
	WRITE_LOCK_IDX(lock);*/
	m_InsertList[object.nSectorID].push_back(object);
}

void CZone::Remove_ObjecttoSector(Sector::ObjectInfo object)
{
	int lock = lock::Insert_M;
	WRITE_LOCK_IDX(lock);
	m_RemoveList[object.nSectorID].push_back(object);
}

void CZone::Insert_PlayertoSector(Sector::ObjectInfo object)
{
	/*int lock = lock::Player;
	WRITE_LOCK_IDX(lock);*/
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
		int lock = lock::Insert_M;
		WRITE_LOCK_IDX(lock);
		InsertList.swap(m_InsertList);
	}
	//WRITE_LOCK;

	for (auto& [SectorID, sData] : InsertList)
	{
#ifdef __DOP__
		CSector* Sector = GetSector(SectorID);//m_listSector[SectorID];
		if (Sector == nullptr)
			continue;
#else
		CSectorRef Sector = m_listSector[SectorID];
#endif
		if (Sector->Empty_Player())
			continue;
#ifdef __BROADCAST_DISTANCE__
		//auto distance = [](float source_x, float source_y, float target_x, float target_y)->float
		//	{
		//		return sqrt(pow(target_x - source_x, 2) + pow(target_y - source_y, 2));
		//
		//	};

		auto Playerlist = Sector->PlayerList();
		for (auto& [playerid, Player] : Playerlist)
		{
			Protocol::S_OBJ_LIST objpkt;
			int64 nowtime = GetTickCount64();
			objpkt.set_sendtime(nowtime);


			int nCnt = 0;

			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{
				if (m_nlistObject[ObjectInfo.nObjectType].contains(ObjectInfo.nObjectID) == false)
				{
					//먼저 존에 몬스터객체 추가 완료시 아래문은 탈수없음!
					Protocol::D3DVECTOR m_vStartpos;
					m_vStartpos.set_x(ObjectInfo.vPos.x);
					m_vStartpos.set_y(ObjectInfo.vPos.y);
					m_vStartpos.set_z(ObjectInfo.vPos.z);

					MonsterRef Monster = MakeShared<CMonster>(ObjectInfo.nObjectID, m_nZoneID, ObjectInfo.nSectorID, m_vStartpos, true);
					//Object = Monster;

					m_nlistObject[Object::Monster].insert({ ObjectInfo.nObjectID,Monster });
					//continue;
				}
				ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];
#ifdef __DOP__
				Sector::MonsterData sData;
				{
					sData.nObjectID = ObjectInfo.nObjectID;
					sData.nSectorID = ObjectInfo.nSectorID;
					sData.vPos = ObjectInfo.vPos;
					sData.nZoneID = m_nZoneID;
				}
				Sector->Insert_Monster(sData, true);
#else
				Sector->Insert(ObjectInfo.nObjectType, Object);
#endif
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

				objectPos.set_secid(ObjectInfo.nSectorID);
				objectPos.set_zoneid(ObjectInfo.nZoneID);
				objectPos.set_objecttype(ObjectInfo.nObjectType);

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
#ifdef __DOP__
		CSector* Sector = GetSector(SectorID);//m_listSector[SectorID];
		if (Sector == nullptr)
			continue;
#else
		CSectorRef Sector = m_listSector[SectorID];
#endif
		if (Sector->Empty_Player())
		{
			//유저가 없는데, 해당 섹터에서 몹이 다른 섹터로 이동해 삭제해야 할 경우
			for (auto& ObjectInfo : sData)
			{
				if (m_nlistObject[ObjectInfo.nObjectType].contains(ObjectInfo.nObjectID) == false)
					continue;

				Sector->Delete(ObjectInfo.nObjectType, ObjectInfo.nObjectID);
			}

			continue;
		}

#ifdef __BROADCAST_DISTANCE__
		//auto distance = [](float source_x, float source_y, float target_x, float target_y)->float
		//	{
		//		return sqrt(pow(target_x - source_x, 2) + pow(target_y - source_y, 2));
		//
		//	};
		auto Playerlist = Sector->PlayerList();
		for (auto& [playerid, Player] : Playerlist)
		{
			int64 nowtime = GetTickCount64();

			Protocol::S_OBJ_REMOVE_ACK objpkt;
			objpkt.set_sendtime(nowtime);

			int nCnt = 0;

			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{
				if(m_nlistObject[ObjectInfo.nObjectType].contains(ObjectInfo.nObjectID)==false)
					continue;
#ifdef __DOP__				
				Sector->Delete_Monster(ObjectInfo.nObjectID);
#else
				Sector->Delete(ObjectInfo.nObjectType, ObjectInfo.nObjectID);
#endif
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
#ifdef __DOP__
		CSector* Sector = GetSector(SectorID);//m_listSector[SectorID];
		if (Sector == nullptr)
			continue;
#else
		CSectorRef Sector = m_listSector[SectorID];

#endif	
		if (Sector->Empty_Player())
		{
			//해당 섹터에 기존 유저 없을시, 새로 들어온 유저만 추가
			for (auto& ObjectInfo : sData)
			{
				if (m_nlistObject[ObjectInfo.nObjectType].contains(ObjectInfo.nObjectID) == false)
				{
					PlayerRef pPlayer = GPlayerManager->Player(ObjectInfo.nObjectID);
					if (pPlayer == nullptr)
						continue;

					pPlayer->SetZoneID(ObjectInfo.nZoneID);
					pPlayer->SetSectorID(ObjectInfo.nSectorID);

					//다른 존에서 넘어온 유저면 새로 추가
					if (_Enter(Object::Player, pPlayer) == false)
						continue;
					ObjectRef Object = pPlayer;
					Sector->Insert(ObjectInfo.nObjectType, Object);

				}
				else
				{
					ObjectRef pObject = Object(Object::Player, ObjectInfo.nObjectID);
					pObject->SetSectorID(ObjectInfo.nSectorID);
	
					Sector->Insert(ObjectInfo.nObjectType, pObject);
				}
			}
			continue;
		}

		//auto distance = [](float source_x, float source_y, float target_x, float target_y)->float
		//	{
		//		return sqrt(pow(target_x - source_x, 2) + pow(target_y - source_y, 2));
		//
		//	};

		auto Playerlist = Sector->PlayerList();

		int nSendCnt = 0;
		int nPlayerCnt = Playerlist.size();
		vector<ObjectRef> BroadCastlist;
		for (auto& [playerid, Player] : Playerlist)
		{
			int64 nowtime = GetTickCount64();
			Protocol::S_PLAYER_LIST objpkt;
			objpkt.set_sendtime(nowtime);

#ifdef __BROADCAST_LOADBALANCE__
#else
			int nCnt = BroadCast_Cnt;
#endif
			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{
#ifdef __BROADCAST_LOADBALANCE__
#else
				if (nCnt <= 0)
					break;
#endif

				if (m_nlistObject[ObjectInfo.nObjectType].contains(ObjectInfo.nObjectID) == false)
				{
					PlayerRef pPlayer = GPlayerManager->Player(ObjectInfo.nObjectID);
					if (pPlayer == nullptr)
						continue;

					pPlayer->SetZoneID(ObjectInfo.nZoneID);
					pPlayer->SetSectorID(ObjectInfo.nSectorID);

					//다른 존에서 넘어온 유저면 새로 추가
					if (_Enter(Object::Player, pPlayer) == false)
						continue;
				}

				ObjectRef pObject = Object(Object::Player, ObjectInfo.nObjectID);
				if (pObject == nullptr)
					continue;
				Sector->Insert(ObjectInfo.nObjectType, pObject);

				float dist = Util::distance(ObjectInfo.vPos.x, ObjectInfo.vPos.y, Player->GetPos().x(), Player->GetPos().y());

				if (dist > BroadCast_Distance)
				{
					continue;
				}

				bSend = true;
				//nCnt--;

				Protocol::Object_Pos objectPos;
				objectPos.set_id(ObjectInfo.nObjectID);
				Protocol::D3DVECTOR* vPos = objectPos.mutable_vpos();
				vPos->set_x(ObjectInfo.vPos.x);
				vPos->set_y(ObjectInfo.vPos.y);

				objpkt.add_pos()->CopyFrom(objectPos);
			}
			if (bSend == false)
				continue;
			int size = objpkt.ByteSizeLong();
			if (size >= 6000)
			{
				string data;
				objpkt.SerializeToString(&data);
				int length = data.length();
			}
#ifdef __ZONE_THREAD__
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#else
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#endif // __ZONE_THREAD__	
			CPlayer* pPlayer = static_cast<CPlayer*>(Player.get());
			if (pPlayer->ownerSession.expired() == false)
			{

#ifdef __BROADCAST_LOADBALANCE__
				if (nPlayerCnt >= BroadCast_Cnt)
					BroadCastlist.push_back(Player);
				else
					pPlayer->ownerSession.lock()->Send(sendBuffer);
#else
				pPlayer->ownerSession.lock()->Send(sendBuffer);

#endif		
			}
			else
			{
				pPlayer->SetActivate(false);
				///세션 소멸, 해당 플레이어 객체도
			}
			nSendCnt++;

#ifdef __BROADCAST_LOADBALANCE__
			if (nSendCnt == nPlayerCnt)
			{
				if (BroadCastlist.empty() == false)
					DoTimer(0,&CZone::BroadCast, BroadCastlist,sendBuffer);
			}
#endif
		}		

	}	
}

void CZone::Send_SectorRemovePlayer()
{
	map<SectorID, vector<Sector::ObjectInfo>> RemoveList;
	map<SectorID, vector<Sector::ObjectInfo>> AdjZone_RemoveList;

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
#ifdef __DOP__
		CSector* Sector = GetSector(SectorID);//m_listSector[SectorID];
		if (Sector == nullptr)
			continue;
#else
		CSectorRef Sector = m_listSector[SectorID];
#endif

		if (Sector->Empty_Player())		
			continue;

		//auto distance = [](float source_x, float source_y, float target_x, float target_y)->float
		//	{
		//		return sqrt(pow(target_x - source_x, 2) + pow(target_y - source_y, 2));
		//
		//	};

		
		auto Playerlist = Sector->PlayerList();
		int nSendCnt = 0;
		int nPlayerCnt = Playerlist.size();
		vector<ObjectRef> BroadCastlist;
		for (auto& [playerid, Player] : Playerlist)
		{
			int64 nowtime = GetTickCount64();

			Protocol::S_PLAYER_REMOVE_ACK objpkt;
			objpkt.set_sendtime(nowtime);
#ifdef __BROADCAST_LOADBALANCE__
#else
			int nCnt = BroadCast_Cnt;
#endif
			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{

#ifdef __BROADCAST_LOADBALANCE__
#else
				if (nCnt <= 0)
				break;
#endif
				if (m_nlistObject[ObjectInfo.nObjectType].contains(ObjectInfo.nObjectID) == false)
					continue;

				/*ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];
				if (Object == nullptr)
					continue;*/
				Sector->Delete(ObjectInfo.nObjectType, ObjectInfo.nObjectID);

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

#ifdef __BROADCAST_LOADBALANCE__
#else
				nCnt--;
#endif
			}
			if (bSend == false)
				continue;
			int size = objpkt.ByteSizeLong();
			if (size >= 6000)
			{
				string data;
				objpkt.SerializeToString(&data);
				int length = data.length();

			}
#ifdef __ZONE_THREAD__
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#else
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#endif // __ZONE_THREAD__	
			CPlayer* pPlayer = static_cast<CPlayer*>(Player.get());
			if (pPlayer->ownerSession.expired() == false)
			{
#ifdef __BROADCAST_LOADBALANCE__
				if (nPlayerCnt >= BroadCast_Cnt)
					BroadCastlist.push_back(Player);
				else
					pPlayer->ownerSession.lock()->Send(sendBuffer);
#else
				pPlayer->ownerSession.lock()->Send(sendBuffer);

#endif // __BROADCAST_LOADBALANCE__
			}
			else
			{
				pPlayer->SetActivate(false);
				///세션 소멸, 해당 플레이어 객체도
			}
			nSendCnt++;

#ifdef __BROADCAST_LOADBALANCE__
			if (nSendCnt == nPlayerCnt)
			{
				if (BroadCastlist.empty() == false)
					DoTimer(0, &CZone::BroadCast, BroadCastlist, sendBuffer);
			}
#endif
		}
	}
}
#ifdef __SEAMLESS__
void CZone::Send_AdjSector_ObjList()
{

	/*int lock = lock::Object;
	WRITE_LOCK_IDX(lock);*/

	/*
		아래 섹터와 이웃한 내 색터 구하기
		그 섹터 위치한 유저들에게 아래 정보 전하기!
	*/
	for (auto& [nSecID, ObjList] : AdjObjectList)
	{
		
		for (auto& [nMySecID, Sector] : m_listSector)
		{
			auto AdjSeclist= Sector->GetAdjSectorlist();
			if (AdjSeclist.contains(nSecID) == false)
				continue;

			//이웃한 내 섹터 찾음!
			
			auto Playerlist = Sector->PlayerList();

			int nSendCnt = 0;
			int nPlayerCnt = Playerlist.size();
			vector<ObjectRef> BroadCastlist;
			for (auto& [playerid, Player] : Playerlist)
			{
				int64 nowtime = GetTickCount64();
				Protocol::S_ALL_OBJ_LIST objpkt;
				objpkt.set_sendtime(nowtime);

#ifdef __BROADCAST_LOADBALANCE__
#else
				int nCnt = BroadCast_Cnt;
#endif
				bool bSend = false;
				for (auto [ObjID,ObjectInfo] : ObjList)
				{
#ifdef __BROADCAST_LOADBALANCE__
#else
					if (nCnt <= 0)
						break;
#endif

					//이웃한 경계의 다른 존 섹터의 몹이므로 추가 필요x
					/*ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];
					if (Object == nullptr)
						continue;
					Sector->Insert(ObjectInfo.nObjectType, Object);*/

					float dist = Util::distance(ObjectInfo.vPos.x, ObjectInfo.vPos.y, Player->GetPos().x(), Player->GetPos().y());

					if (dist > BroadCast_Distance)
					{
						continue;
					}

					bSend = true;
					//nCnt--;

					Protocol::Object_Pos objectPos;
					objectPos.set_id(ObjectInfo.nObjectID);
					Protocol::D3DVECTOR* vPos = objectPos.mutable_vpos();
					vPos->set_x(ObjectInfo.vPos.x);
					vPos->set_y(ObjectInfo.vPos.y);
					
					objectPos.set_secid(ObjectInfo.nSectorID);
					objectPos.set_zoneid(ObjectInfo.nZoneID);					
					objectPos.set_objecttype(ObjectInfo.nObjectType);

					objpkt.add_pos()->CopyFrom(objectPos);
				}
				if (bSend == false)
					continue;
				int size = objpkt.ByteSizeLong();
				if (size >= 6000)
				{
					string data;
					objpkt.SerializeToString(&data);
					int length = data.length();
				}
#ifdef __ZONE_THREAD__
				auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#else
				auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#endif // __ZONE_THREAD__	
				CPlayer* pPlayer = static_cast<CPlayer*>(Player.get());
				if (pPlayer->ownerSession.expired() == false)
				{

#ifdef __BROADCAST_LOADBALANCE__
					if (nPlayerCnt >= BroadCast_Cnt)
						BroadCastlist.push_back(Player);
					else
						pPlayer->ownerSession.lock()->Send(sendBuffer);
#else
					pPlayer->ownerSession.lock()->Send(sendBuffer);

#endif		
				}
				else
				{
					pPlayer->SetActivate(false);
					///세션 소멸, 해당 플레이어 객체도
				}
				nSendCnt++;

#ifdef __BROADCAST_LOADBALANCE__
				if (nSendCnt == nPlayerCnt)
				{
					if (BroadCastlist.empty() == false)
						DoTimer(0, &CZone::BroadCast, BroadCastlist, sendBuffer);
				}
#endif
			}

		}


	}
}
void CZone::Send_AdjSector_InsertObj()
{
}
void CZone::Send_AdjSector_RemoveObj()
{
}
void CZone::SectorInsertPlayerJob(map<SectorID, vector<Sector::ObjectInfo>>Insertlist)
{
	for (auto& [SectorID, sData] : Insertlist)
	{
		CSectorRef Sector = m_listSector[SectorID];
		if (Sector == nullptr)
		{
			continue;
		}


		auto Playerlist = Sector->PlayerList();
		int nSendCnt = 0;
		int nPlayerCnt = Playerlist.size();
		vector<ObjectRef> BroadCastlist;
		for (auto& [playerid, Player] : Playerlist)
		{
			int64 nowtime = GetTickCount64();
			Protocol::S_PLAYER_LIST objpkt;
			objpkt.set_sendtime(nowtime);

#ifdef __BROADCAST_LOADBALANCE__
#else
			int nCnt = BroadCast_Cnt;
#endif
			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{
#ifdef __BROADCAST_LOADBALANCE__
#else
				if (nCnt <= 0)
					break;
#endif
				ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];
				if (Object == nullptr)
					continue;
				Sector->Insert(ObjectInfo.nObjectType, Object);

				float dist = Util::distance(ObjectInfo.vPos.x, ObjectInfo.vPos.y, Player->GetPos().x(), Player->GetPos().y());

				if (dist > BroadCast_Distance)
				{
					continue;
				}

				bSend = true;
				//nCnt--;

				Protocol::Object_Pos objectPos;
				objectPos.set_id(ObjectInfo.nObjectID);
				Protocol::D3DVECTOR* vPos = objectPos.mutable_vpos();
				vPos->set_x(ObjectInfo.vPos.x);
				vPos->set_y(ObjectInfo.vPos.y);

				objpkt.add_pos()->CopyFrom(objectPos);
			}
			if (bSend == false)
				continue;
			int size = objpkt.ByteSizeLong();
			if (size >= 6000)
			{
				string data;
				objpkt.SerializeToString(&data);
				int length = data.length();
			}
#ifdef __ZONE_THREAD__
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#else
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#endif // __ZONE_THREAD__	
			CPlayer* pPlayer = static_cast<CPlayer*>(Player.get());
			if (pPlayer->ownerSession.expired() == false)
			{

#ifdef __BROADCAST_LOADBALANCE__
				if (nPlayerCnt >= BroadCast_Cnt)
					BroadCastlist.push_back(Player);
				else
					pPlayer->ownerSession.lock()->Send(sendBuffer);
#else
				pPlayer->ownerSession.lock()->Send(sendBuffer);

#endif		
			}
			else
			{
				pPlayer->SetActivate(false);
				///세션 소멸, 해당 플레이어 객체도
			}
			nSendCnt++;

#ifdef __BROADCAST_LOADBALANCE__
			if (nSendCnt == nPlayerCnt)
			{
				if (BroadCastlist.empty() == false)
					DoTimer(0, &CZone::BroadCast, BroadCastlist, sendBuffer);
			}
#endif
		}

	}


}
void CZone::SectorRemovePlayerJob(map<SectorID, vector<Sector::ObjectInfo>>RemoveList)
{
	for (auto& [SectorID, sData] : RemoveList)
	{
		CSectorRef Sector = m_listSector[SectorID];
		if (Sector == nullptr)
			continue;

		auto Playerlist = Sector->PlayerList();
		int nSendCnt = 0;
		int nPlayerCnt = Playerlist.size();
		vector<ObjectRef> BroadCastlist;
		for (auto& [playerid, Player] : Playerlist)
		{
			int64 nowtime = GetTickCount64();

			Protocol::S_PLAYER_REMOVE_ACK objpkt;
			objpkt.set_sendtime(nowtime);
#ifdef __BROADCAST_LOADBALANCE__
#else
			int nCnt = BroadCast_Cnt;
#endif
			bool bSend = false;
			for (auto& ObjectInfo : sData)
			{

#ifdef __BROADCAST_LOADBALANCE__
#else
				if (nCnt <= 0)
					break;
#endif
				ObjectRef Object = m_nlistObject[ObjectInfo.nObjectType][ObjectInfo.nObjectID];
				if (Object == nullptr)
					continue;
				Sector->Delete(ObjectInfo.nObjectType, Object->ObjectID());

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

#ifdef __BROADCAST_LOADBALANCE__
#else
				nCnt--;
#endif
			}
			if (bSend == false)
				continue;
			int size = objpkt.ByteSizeLong();
			if (size >= 6000)
			{
				string data;
				objpkt.SerializeToString(&data);
				int length = data.length();

			}
#ifdef __ZONE_THREAD__
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#else
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(objpkt, m_nZoneID);
#endif // __ZONE_THREAD__	
			CPlayer* pPlayer = static_cast<CPlayer*>(Player.get());
			if (pPlayer->ownerSession.expired() == false)
			{
#ifdef __BROADCAST_LOADBALANCE__
				if (nPlayerCnt >= BroadCast_Cnt)
					BroadCastlist.push_back(Player);
				else
					pPlayer->ownerSession.lock()->Send(sendBuffer);
#else
				pPlayer->ownerSession.lock()->Send(sendBuffer);

#endif // __BROADCAST_LOADBALANCE__
			}
			else
			{
				pPlayer->SetActivate(false);
				///세션 소멸, 해당 플레이어 객체도
			}
			nSendCnt++;

#ifdef __BROADCAST_LOADBALANCE__
			if (nSendCnt == nPlayerCnt)
			{
				if (BroadCastlist.empty() == false)
					DoTimer(0, &CZone::BroadCast, BroadCastlist, sendBuffer);
			}
#endif
		}



	}


}
Sector::ObjectInfo CZone::GetAdjObjectInfo(int nSectorID, int nObjectID)
{
	Sector::ObjectInfo info;
	if (AdjObjectList.contains(nSectorID))
	{
		if (AdjObjectList[nSectorID].contains(nObjectID))
			info = AdjObjectList[nSectorID][nObjectID];
	}
	else
	{
		//cout << "" << endl;

	}
	return info;
}
Sector::ObjectInfo CZone::GetMyObjectInfo(int nSectorID, int nObjectType,int ObjectID)
{
	Sector::ObjectInfo info;
	auto pObject = Object(Object::Monster, ObjectID);
	if (pObject)
	{
		info= pObject->GetObjectInfo();
		return info;
	}

	//오브젝트 타입=0 원인 못찾아 일단 몬스터 고정
		return info;
}
#endif __SEAMLESS__

void CZone::Update_ObjectInfo(Sector::ObjectInfo info)
{
	m_nlistObject[info.nObjectType][info.nObjectID]->Update(info);

}
#ifdef __DOP__
CSector* CZone::GetSector(int Index)
{
	if (Index < 0 || Index >= m_vecSector.size())
		return nullptr;
	return &m_vecSector[Index];
	// TODO: 여기에 return 문을 삽입합니다.
}
#endif
std::pair<int, int> CZone::get_coords(int zone_id)
{
	// 존 ID는 1부터 시작하므로, 0-based index로 변환
	int index = zone_id - 1;
	int row = index / ZONES_PER_ROW;
	int col = index % ZONES_PER_ROW;
	return { row, col };
}

int CZone::get_zone_id(int row, int col)
{

	// 좌표가 유효한지 먼저 확인
	if (row < 0 || row >= ZONES_PER_COL || col < 0 || col >= ZONES_PER_ROW) {
		return -1; // 유효하지 않은 좌표
	}
	return row * ZONES_PER_ROW + col + 1; // 1-based ID로 변환
}
#ifdef __SEAMLESS__

void CZone::Set_Neighbor_list()
{
	// 8방향 벡터 (상, 하, 좌, 우, 좌상, 우상, 좌하, 우하)
	const int dr[] = { -1, 1, 0, 0, -1, -1, 1, 1 };
	const int dc[] = { 0, 0, -1, 1, -1, 1, -1, 1 };

	for (int current_zone = m_nZoneID; current_zone <= g_nZoneCount; ++current_zone) {
		std::vector<int> neighbors;
		std::pair<int, int> coords = get_coords(current_zone);
		int row = coords.first;
		int col = coords.second;

		// 8방향으로 이웃 존 탐색
		for (int i = 0; i < 8; ++i) {
			int new_row = row + dr[i];
			int new_col = col + dc[i];

			int neighbor_id = get_zone_id(new_row, new_col);

			// 유효한 이웃 존인 경우
			if (neighbor_id != -1) {
				neighbors.push_back(neighbor_id);
			}
		}

		// 이웃 리스트를 오름차순으로 정렬
		std::sort(neighbors.begin(), neighbors.end());

		// 맵에 현재 존과 이웃 리스트를 저장
		for (auto zoneid : neighbors)
		{
			m_setAdjZone.insert(zoneid);
		}

		return;
		//m_adjZonelist[current_zone] = neighbors;
	}


}
#endif
