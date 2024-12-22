#include "pch.h"
#include "CZone.h"
#include "Player.h" //�������ϸ� static_cast��ȯ ����
#include "CMonster.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
using namespace Zone;
CZone::CZone(int nMaxUserCnt,int nZoneID, Protocol::D3DVECTOR vPos) :m_bActivate(false), m_nMaxUserCnt(nMaxUserCnt), m_nZoneID(nZoneID)
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

	int startID =( g_nZoneCount * g_nZoneUserMax) + (MonsterMaxCount* (m_nZoneID-1)) +1;
	int EndMaxID = (startID-1) + MonsterMaxCount;

	for (;startID < EndMaxID; startID++)
	{
		MonsterRef Monster = MakeShared<CMonster>(startID, m_nZoneID, m_vStartpos,true);
	
		m_nlistObject[Object::Monster].insert({startID,Monster});
	}


	






}

CZone::~CZone()
{
	m_nlistObject.clear(); //shared_ptr�� �ٷ� Ŭ���� ȣ���ص� �ǳ�?
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

	//������ �ִ� ���� Ȱ��ȭ
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

	//������ �ִ� ���� Ȱ��ȭ
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
		�ش� ��, ���� ��ġ�� �ٸ� �����鿡�� ��ε�ĳ����
		S_OBJ_REMOVE_ACK

	*/
}

void CZone::Update()
{
	//�ӽ÷� �ּ�,�� �׽�Ʈ
	//if (m_nlistObject[Object::Player].empty())
	//{
	//	m_bActivate = false;
	//	return;
	//}

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
	 ���� ������ �ǽð� ��ǥ �ֺ� �������� ����ȭ

	 ���� �� ������ ���� 200��, �ٸ� �ش� �� ������ ���θ� �� �����ʿ�� ����.
	 �������忡�� �ڽŰ����� ����(ī�޶��̴�)�� �ִ� �������� �޾Ƽ� ����ϸ��

	 �ϴ� ���������ߴ� �����Ͽ� 30���� �������� ���� 

	*/
	Send_MonsterUpdateList();

	Send_PlayerUpdateList();

	//ZoneManager���� ��� zone update ȣ����
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
			���� ��ġ�� �ٸ� �÷��̾ ������Ʈ.
			������ �ϴ� ���  ������Ʈ.

		*/
		m_nlistObject[Object::Player][nObjectID]->SetPos(vPos);
	}
}

bool CZone::Enter()
{
	READ_LOCK;
	return m_nlistObject[Object::Player].size() < m_nMaxUserCnt;
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

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(movepkt);
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
			///���� �Ҹ�, �ش� �÷��̾� ��ü�� 

		}


		nCnt++;
	}
}