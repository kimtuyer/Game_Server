#include "pch.h"
#include "CZone.h"
#include "Player.h" //�������ϸ� static_cast��ȯ ����
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
		��/ ���� ����

		���� �ʿ�����
		���� id 5001~
		���� ���� �ʱ� ��ġ ���� -����
		���� �� id

	*/
	//�ӽ� Ȱ��ȭ,�� �׽�Ʈ
	m_bActivate = true;
#ifdef __DOP__
	m_vecSector.resize(SECTORS_PER_SIDE * SECTORS_PER_SIDE);
#endif
	//int nSectorid = 1;
	m_nBeginSecID = g_nSectorID;
	for (int HEIGHT = 1; HEIGHT <= SECTORS_PER_SIDE; HEIGHT++)	//�ܼ� ���� 1�� ����
		for (int WIDTH = 1; WIDTH <= SECTORS_PER_SIDE; WIDTH++) //�ܼ� ���� 1�� ����
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
	//������ �� ����Ʈ ����
	Set_Neighbor_list();

#else

//	// ���� ����  ����Ʈ ���ϱ�
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


	//	//��
	//	x = currnet_x;
	//	y = currnet_y - Zone::Sector_HEIGHT;
	//	Set_AdjSector(x, y, Sector);
	//
	//	//�Ʒ�
	//	x = currnet_x;
	//	y = currnet_y + Zone::Sector_HEIGHT;
	//
	//	Set_AdjSector(x, y, Sector);
	//
	//	//��
	//	x = currnet_x - Zone::Sector_WIDTH;
	//	y = currnet_y;
	//
	//	Set_AdjSector(x, y, Sector);
	//
	//	//������
	//	x = currnet_x + Zone::Sector_WIDTH;
	//	y = currnet_y;
	//	Set_AdjSector(x, y, Sector);
	//
	//	//�����밢
	//	x = currnet_x - Zone::Sector_WIDTH;
	//	y = currnet_y - Zone::Sector_HEIGHT;
	//	Set_AdjSector(x, y, Sector);
	//
	//	//�������밢
	//	x = currnet_x + Zone::Sector_WIDTH;
	//	y = currnet_y - Zone::Sector_HEIGHT;
	//	Set_AdjSector(x, y, Sector);
	//
	//	//�޾Ʒ��밢
	//	x = currnet_x - Zone::Sector_WIDTH;
	//	y = currnet_y + Zone::Sector_HEIGHT;
	//	Set_AdjSector(x, y, Sector);
	//
	//	//�����Ʒ��밢
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
		// 1~max ����id�� �������� �ҷ��� �ش� ������ ��ǥ�� ���͸� ��ġ.
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
		Sector->Insert(Object::Monster, pObject); //���� ObjectRef �־���ϴµ�..
#endif // __DOP__

	}
	//���� ���� id �������� �־��ٷ���  1~16�� ������ �������� ������ �� ������ ��ǥ�� ������ ����!
}

CZone::~CZone()
{
	m_nlistObject.clear(); //shared_ptr�� �ٷ� Ŭ���� ȣ���ص� �ǳ�?
}

void CZone::SetAdjSector()
{

	// ���� ����  ����Ʈ ���ϱ�
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

	//������ �ִ� ���� Ȱ��ȭ
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

	//������ �ִ� ���� Ȱ��ȭ
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
		�ش� ��, ���� ��ġ�� �ٸ� �����鿡�� ��ε�ĳ����
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
	//�ӽ÷� �ּ�,�� �׽�Ʈ
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
	//�ٸ� ������ �Ѱ��� �̿��� ������ ������Ʈ ����Ʈ �������.
	/*
		�� ���͵�� �̿��� ���Ϳ� ��ġ�� �����鿡�� �˷������!
	*/

	Send_AdjSector_ObjList();

#endif // __SEAMLESS__

	//���� ���� ����Ʈ �� ���Ϳ� ��ġ�� �÷��̾�鿡�� ����

	Send_SectorInsertObject();

	Send_SectorRemoveObject();

#ifdef __ZONE_THREAD__
	Send_SectorInsertPlayer();

	Send_SectorRemovePlayer();
#endif
	//Send_MonsterUpdateList();

/*
 �� �������� �ʿ��� ���� ��������, ��� ���������� �ʿ����� ����
 �˾ƾ���.

 1.�� ���� ���� �ʿ��� �̿� ������ �ش� ���ͳ� ������Ʈ ����
 �� ���� �޽��� ť�� ����

 2. �� ���޽���ť ��  ���ο� �����̳� ����


 3. ������ ���� �����̳ʿ� ���ϸ鼭 Remove, Insert ��ü ����Ʈ ����

*/
#ifdef __SEAMLESS__

	for (auto& [sectorID, Sector] : m_listSector)
	{
		auto adjsectorlist = Sector->GetAdjSectorlist();
		int nSendZoneID = 0;
		//�ش� ���� ���� �ʿ��� ����?
		for (auto [SecID, Data] : adjsectorlist)
		{
			int nZoneID = Data.first;
			//���� ���� ���ʹ� continue;
			if (nZoneID == m_nZoneID)
				continue;

			//�ش� ���������� �̹� ���� ���� Pass
			if (nSendZoneID == nZoneID)
				continue;

			nSendZoneID = nZoneID;

			//�̿� ������ �ʿ��� �� ���� ��ü���� �Ѱ��ֱ�
			/*

			*/
			//�̶� ������Ʈ�ʿ��� ������ �ĺ� ����?
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
		�̿��� �ٸ� ������ ������ �޾� ���� �����ؾ���.	
	*/
	/*int lock = lock::Object;
	WRITE_LOCK_IDX(lock);*/

	int nThreadID = LThreadId;

	auto Objlist = AdjObjectList[nSectorID];
	for (auto [ObjectID, Objinfo] : Objlist)
	{
		if (adjSectorInfoList.contains(ObjectID) == false)
		{
			//��� ������Ʈ����Ʈ ������ ����!
			AdjObjectList[nSectorID].erase(ObjectID);
		}
	

	}
	//��輽�� ���� ���� �� �߰� �� ����
	for (auto [ObjectID, Objinfo] : adjSectorInfoList)
	{
		

		if (AdjObjectList[nSectorID].contains(ObjectID))
		{
			AdjObjectList[nSectorID][ObjectID] = Objinfo;
		}
		else
			AdjObjectList[nSectorID].insert({ ObjectID,Objinfo });
	}

	////������ �����ִ� ��� ������ �������� ���� ���, ��������Ʈ �־� ����!



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
	//���͸���Ʈ �����ؿͼ� lock �ٿ����ϳ�?
	map<SectorID, vector<Sector::ObjectInfo>> InsertList;
	{
		//swap�� ���� �����̳ʴ� clear����, ���Ŀ� ���� �����ʹ� ����tick�� ó��!
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
				///���� �Ҹ�, �ش� �÷��̾� ��ü��
			}
		}

	}
}

void CZone::Send_SectorRemoveObject(int beginSectorID, int endSectorID)
{
	map<SectorID, vector<Sector::ObjectInfo>> RemoveList;
	{
		//swap�� ���� �����̳ʴ� clear����, ���Ŀ� ���� �����ʹ� ����tick�� ó��!
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
				///���� �Ҹ�, �ش� �÷��̾� ��ü��
			}
		}


	}
}

void CZone::Send_SectorInsertPlayer(int beginSectorID, int endSectorID)
{
	//���͸���Ʈ �����ؿͼ� lock �ٿ����ϳ�?
	map<SectorID, vector<Sector::ObjectInfo>> InsertList;
	{
		//swap�� ���� �����̳ʴ� clear����, ���Ŀ� ���� �����ʹ� ����tick�� ó��!
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
				///���� �Ҹ�, �ش� �÷��̾� ��ü��
			}
		}
	}
}

void CZone::Send_SectorRemovePlayer(int beginSectorID, int endSectorID)
{
	map<SectorID, vector<Sector::ObjectInfo>> RemoveList;
	{
		//swap�� ���� �����̳ʴ� clear����, ���Ŀ� ���� �����ʹ� ����tick�� ó��!
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
					�ش� ���� ���̵���, �ռ� ������ �̹� ����, ���Ϳ����� ������ ���
					�Ʒ����� ���� ��ȸ�� false���ð�.
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
				///���� �Ҹ�, �ش� �÷��̾� ��ü��
			}
		}
	}
}

CObject* CZone::SearchEnemy(CObject* pMonster)
{
	ObjectList Playerlist = PlayerList();
	if (Playerlist.empty())
		return nullptr;

	//lock�� �־��, �ش� ������Ʈ�� ������ ���ܼ� �����Ϸ��Ҷ�
	//���� ������ ����.
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
			���� ��ġ�� �ٸ� �÷��̾ ������Ʈ.
			������ �ϴ� ���  ������Ʈ.

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

	//���� ���Ϳ� ������ �ʴ� ��ġ���, ���ο� ���� ���� �����ؾ�
	/*
		���� �ִ� ���� �������� �ѷ��� 8���� ���͸� ���� ���� ���ϴ��� Ȯ��
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

		// 1~max ����id�� �������� �ҷ��� �ش� ������ ��ǥ�� ���͸� ��ġ.
		 
		int nzone=m_nZoneID;
		std::uniform_int_distribution<int> SectorList(m_nBeginSecID, m_nEndSecID);   // 0 ~ MAX_STEP


		//�ɸ��� ���� �ϱ����� ���� ��ȣ ������ ��� ���� 1~Sector_Count �� ������.
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
		 �̶��� �����ϰԴ� �� ���� ���� ���Ϳ����� �������Ը� ������
		*/
		//��Ŷ ũ�� ����� �ϴ� �ѹ��� 30��� ����
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
		 �̶��� �����ϰԴ� �� ���� ���� ���Ϳ����� �������Ը� ������
		*/

		//��Ŷ ũ�� ����� �ϴ� �ѹ��� 30��� ����
		if (nCnt > 30)
			break;

		if (Object->GetActivate() == false)
			continue;

		if (Object->IsUpdatePos() == false)
			continue;

		//���� ��ġ�� �ٸ��� �����ʿ��� �������� Ȯ�ε� �ʿ�

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
			///���� �Ҹ�, �ش� �÷��̾� ��ü��
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
			///���� �Ҹ�, �ش� �÷��̾� ��ü��
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
			//�̿��� ���� ���͸���Ʈ
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
	//���͸���Ʈ �����ؿͼ� lock �ٿ����ϳ�?
	map<SectorID, vector<Sector::ObjectInfo>> InsertList;
	{
		//swap�� ���� �����̳ʴ� clear����, ���Ŀ� ���� �����ʹ� ����tick�� ó��!
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
					//���� ���� ���Ͱ�ü �߰� �Ϸ�� �Ʒ����� Ż������!
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
				///���� �Ҹ�, �ش� �÷��̾� ��ü��
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
				///���� �Ҹ�, �ش� �÷��̾� ��ü��
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
		//swap�� ���� �����̳ʴ� clear����, ���Ŀ� ���� �����ʹ� ����tick�� ó��!
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
			//������ ���µ�, �ش� ���Ϳ��� ���� �ٸ� ���ͷ� �̵��� �����ؾ� �� ���
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
				///���� �Ҹ�, �ش� �÷��̾� ��ü��
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
				///���� �Ҹ�, �ش� �÷��̾� ��ü��
			}
		}
#endif
	}
}

void CZone::Send_SectorInsertPlayer()
{
	//���͸���Ʈ �����ؿͼ� lock �ٿ����ϳ�?
	map<SectorID, vector<Sector::ObjectInfo>> InsertList;

	{
		//swap�� ���� �����̳ʴ� clear����, ���Ŀ� ���� �����ʹ� ����tick�� ó��!
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
			//�ش� ���Ϳ� ���� ���� ������, ���� ���� ������ �߰�
			for (auto& ObjectInfo : sData)
			{
				if (m_nlistObject[ObjectInfo.nObjectType].contains(ObjectInfo.nObjectID) == false)
				{
					PlayerRef pPlayer = GPlayerManager->Player(ObjectInfo.nObjectID);
					if (pPlayer == nullptr)
						continue;

					pPlayer->SetZoneID(ObjectInfo.nZoneID);
					pPlayer->SetSectorID(ObjectInfo.nSectorID);

					//�ٸ� ������ �Ѿ�� ������ ���� �߰�
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

					//�ٸ� ������ �Ѿ�� ������ ���� �߰�
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
				///���� �Ҹ�, �ش� �÷��̾� ��ü��
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
		//swap�� ���� �����̳ʴ� clear����, ���Ŀ� ���� �����ʹ� ����tick�� ó��!
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
				///���� �Ҹ�, �ش� �÷��̾� ��ü��
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
		�Ʒ� ���Ϳ� �̿��� �� ���� ���ϱ�
		�� ���� ��ġ�� �����鿡�� �Ʒ� ���� ���ϱ�!
	*/
	for (auto& [nSecID, ObjList] : AdjObjectList)
	{
		
		for (auto& [nMySecID, Sector] : m_listSector)
		{
			auto AdjSeclist= Sector->GetAdjSectorlist();
			if (AdjSeclist.contains(nSecID) == false)
				continue;

			//�̿��� �� ���� ã��!
			
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

					//�̿��� ����� �ٸ� �� ������ ���̹Ƿ� �߰� �ʿ�x
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
					///���� �Ҹ�, �ش� �÷��̾� ��ü��
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
				///���� �Ҹ�, �ش� �÷��̾� ��ü��
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
				///���� �Ҹ�, �ش� �÷��̾� ��ü��
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

	//������Ʈ Ÿ��=0 ���� ��ã�� �ϴ� ���� ����
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
	// TODO: ���⿡ return ���� �����մϴ�.
}
#endif
std::pair<int, int> CZone::get_coords(int zone_id)
{
	// �� ID�� 1���� �����ϹǷ�, 0-based index�� ��ȯ
	int index = zone_id - 1;
	int row = index / ZONES_PER_ROW;
	int col = index % ZONES_PER_ROW;
	return { row, col };
}

int CZone::get_zone_id(int row, int col)
{

	// ��ǥ�� ��ȿ���� ���� Ȯ��
	if (row < 0 || row >= ZONES_PER_COL || col < 0 || col >= ZONES_PER_ROW) {
		return -1; // ��ȿ���� ���� ��ǥ
	}
	return row * ZONES_PER_ROW + col + 1; // 1-based ID�� ��ȯ
}
#ifdef __SEAMLESS__

void CZone::Set_Neighbor_list()
{
	// 8���� ���� (��, ��, ��, ��, �»�, ���, ����, ����)
	const int dr[] = { -1, 1, 0, 0, -1, -1, 1, 1 };
	const int dc[] = { 0, 0, -1, 1, -1, 1, -1, 1 };

	for (int current_zone = m_nZoneID; current_zone <= g_nZoneCount; ++current_zone) {
		std::vector<int> neighbors;
		std::pair<int, int> coords = get_coords(current_zone);
		int row = coords.first;
		int col = coords.second;

		// 8�������� �̿� �� Ž��
		for (int i = 0; i < 8; ++i) {
			int new_row = row + dr[i];
			int new_col = col + dc[i];

			int neighbor_id = get_zone_id(new_row, new_col);

			// ��ȿ�� �̿� ���� ���
			if (neighbor_id != -1) {
				neighbors.push_back(neighbor_id);
			}
		}

		// �̿� ����Ʈ�� ������������ ����
		std::sort(neighbors.begin(), neighbors.end());

		// �ʿ� ���� ���� �̿� ����Ʈ�� ����
		for (auto zoneid : neighbors)
		{
			m_setAdjZone.insert(zoneid);
		}

		return;
		//m_adjZonelist[current_zone] = neighbors;
	}


}
#endif
