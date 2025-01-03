#include "pch.h"
#include "CSector.h"
#include "CMonster.h"
#include "Player.h" //�������ϸ� static_cast��ȯ ����
#include "GameSession.h"

#include "ClientPacketHandler.h"
using namespace LOCK;
CSector::CSector(int nSectorID, int nZoneID, Protocol::D3DVECTOR vPos)
	: m_nSectorID(nSectorID), m_vStartpos(vPos), m_bActivate(true), m_nZoneID(nZoneID)
{
}

void CSector::Update()
{
	vector<CMonster*> vecMonsterlist;
	{
		int lock = lock::Monster;
		READ_LOCK_IDX(lock);
		for (auto& [objectid, Object] : m_nlistObject[Object::Monster])
		{
			CMonster* pMonster = static_cast<CMonster*>(Object.get());
			vecMonsterlist.push_back(pMonster);
			// ���Ϳ� �ִ� ���� ������ ���� �������ϳ�?
			// Update �� ������, ����,��������Ʈ ó���Ҷ� ���� ����.
			 //m_nlistObject[Object::Monster].erase(objectid);
		}
	}

	for (auto& pMonster : vecMonsterlist)
	{
		pMonster->Update();
	}

	int nowtime = GetTickCount64();
	////�Ź� ������Ʈ �������� �����ָ� ����?
	{
		if (nowtime > m_lBroadTime)
		{
			m_lBroadTime = nowtime + Tick::SECOND_TICK * 3;
			//BroadCast_MonsterList();
		}
	}

	{
		int lock = lock::Die;
		READ_LOCK_IDX(lock);
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
			//�ش� ���� ������
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
	if (m_nlistObject[Object::Monster].contains(objectID))
	{
		return (m_nlistObject[Object::Monster][objectID]);
	}
	else
		nullptr;
}

bool CSector::Insert(int nObjectType, ObjectRef Object)
{
	int lock = lock::Object;
	WRITE_LOCK_IDX(lock);
	m_nlistObject[nObjectType].insert({ Object->ObjectID(),Object });
	return true;
}

bool CSector::Delete(int nObjectType, ObjectRef Object)
{
	int lock = lock::Object;
	WRITE_LOCK_IDX(lock);
	m_nlistObject[nObjectType].insert({ Object->ObjectID(),Object });
	return true;
}

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
			///���� �Ҹ�, �ش� �÷��̾� ��ü��
		}
	}
}

void CSector::BroadCast_Player(Sector::UpdateType eType, Sector::ObjectInfo ObjectInfo)
{
	ObjectList Playerlist = m_nlistObject[Object::Player];

	Protocol::S_PLAYER_LIST objpkt; //���� ���� ������ ��ó �����鿡�� �˸�.

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
			///���� �Ҹ�, �ش� �÷��̾� ��ü��
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

	// TODO: ���⿡ return ���� �����մϴ�.
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
	//���� �� 3�ʵ� ������
	int nowTime = GetTickCount64() + Tick::SECOND_TICK * 3;

	if (m_DeadMonsterList.contains(ObjectID))
		return;
	m_DeadMonsterList.insert({ ObjectID,nowTime });
}