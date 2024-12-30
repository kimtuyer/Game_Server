#include "pch.h"
#include "Player.h"
#include "CZone.h"
#include "CZone_Manager.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "CMonster.h"
#include "CouchbaseClient.h"
CPlayer::CPlayer()
{
}

CPlayer::CPlayer(int playerid, int zoneid, int sectorid):m_nKillcount(0),nLevel(0)
{
}

void CPlayer::Update()
{
}

void CPlayer::LeaveZone()
{
	/*
		해당 유저의 접속이 끊기는 와중에 다른 유저 및 몹과 상호작용 중이었다면?

	*/

	m_bActivate = false;

	CZoneRef Zone = GZoneManager->GetZone(m_nZoneID);
	Zone->Remove(Object::Player, playerId);
}

bool CPlayer::Attack(Protocol::C_ATTACK& pkt)
{
	Protocol::S_ATTACK_ACK ackpkt;
	ackpkt.set_sendtime(GetTickCount64());
	int nKill = 0;
	
	if(pkt.playerid() != m_nObjectID)
		ackpkt.set_success(false);


	CZoneRef Zone = GZoneManager->GetZone(m_nZoneID);
	if (Zone == nullptr)
		ackpkt.set_success(false);

	CSectorRef Sector = Zone->GetSector(m_nSectorID);
	ObjectRef Monster = Sector->GetMonster(pkt.targetid());
	if (Monster == nullptr)
		ackpkt.set_success(false);
	else
	{
		/**/
		float dist = Util::distance(m_vPos.x(), m_vPos.y(), Monster->GetPos().x(), Monster->GetPos().y());

		if (dist > Zone::BroadCast_Distance)
			ackpkt.set_success(false);

		if (Monster->Attacked(m_nAttack, nKill) == false)
			ackpkt.set_success(false);

		if (nKill > 0)
		{
			//static_cast<CMonster*>(Monster.get())->GetGold();
			int nGainGold=(Monster.get())->GetGold();
			int nGainExp =(Monster.get())->GetGold();

			m_nGold += nGainGold;
			m_nExp += nGainExp;

			m_nKillcount++;
			CouchbaseClient* pDBConnect = g_CouchbaseManager->GetConnection(LThreadId);
			document doc;
			doc.threadID = LThreadId;
			doc.cas = nCas;
			doc.key = playerId;

			if (LevelUp(m_nLevel, m_nExp))
			{
				m_nLevel++;
				doc.type = DB::PLAYER_LEVEL_UPDATE;
				json j = { {"Level",m_nLevel},{"Exp",m_nExp},{"Gold",m_nGold},{"Kill",m_nKillcount} };
				doc.value = j.dump();
					//"SELECT EXISTS(SELECT 1 FROM `default` USE KEYS [\"" + doc.key + "\"]);";
				//std::string query = "SELECT EXISTS(SELECT 1 FROM `default` USE KEYS [\"" + key_to_check + "\"]);";
				pDBConnect->upsert(doc);


			}
			else
			{
				doc.type = DB::PLAYER_EXP_MONEY_UPDATE;
				json j = { {"Exp",m_nExp},{"Gold",m_nGold},{"Kill",m_nKillcount} };
				doc.value = j.dump();
				//"SELECT EXISTS(SELECT 1 FROM `default` USE KEYS [\"" + doc.key + "\"]);";
			//std::string query = "SELECT EXISTS(SELECT 1 FROM `default` USE KEYS [\"" + key_to_check + "\"]);";
				pDBConnect->upsert(doc);
			}

			/*
			
			
			*/

			ackpkt.set_targetalive(false);

		}
		else
			ackpkt.set_targetalive(true);

		ackpkt.set_success(true);
	}
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(ackpkt);
	ownerSession.lock()->Send(sendBuffer);

	
	
	//공격 성공
	/*
	상대 몹 체력 깎기

	>>한 몹이 여러 유저의 타깃이 되어,동시에 공격을 받았을 경우
	>>공격패킷을 받고, 공격가능 판정을 받은 유저중에 죽기전 가장 마지막에
	타격을 준 유저, hp 0으로 만든 유저에게 킬 카운트 체크 !


	몹 체력 없으면, 죽음 판정
	죽음시,해당 몹 상태 변경->죽음 상태.데드 리스트 추가.
	Sector->update에서 같이 검사->리스트 돌면서 죽음쿨타임 끝나면 부활
	타이머 추가 3초뒤 해당 섹터 다시 부활

	*/
	
	return true;
}

bool CPlayer::LevelUp(int nlevel, int& nExp)
{
	int nWeight = 1000;
	int nNextExp = nWeight * nLevel;

	if (nNextExp <= nExp)
	{
		//레벨업, 경험치 초기화
		nExp = 0;	
		return true;
	}



	return false;
}
