#include "pch.h"
#include "Player.h"
#include "CZone.h"
#include "CZone_Manager.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "CMonster.h"
#include "CouchbaseClient.h"
#include "CBattle.h"
CPlayer::CPlayer(GameSessionRef& gamesession) :m_nKillcount(0), nLevel(0), bLogin(false), ownerSession(gamesession)
{

}

CPlayer::CPlayer(int playerid, int zoneid, int sectorid):m_nKillcount(0),nLevel(0),bLogin(false)
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
#ifdef __DOP__
	auto Sector = Zone->GetSector(m_nSectorID);
	if (Sector == nullptr)
		return false;
#else
	bool bRet = true;
#ifdef __SEAMLESS__
	bool bSameZone = (m_nZoneID== pkt.targetzoneid() ? true :false) ;
	Sector::ObjectInfo targetInfo;
	auto TargetZone = GZoneManager->GetZone(pkt.targetzoneid());
	if (TargetZone == nullptr)
	{
			ackpkt.set_success(false);
			bRet = false;
	}
	else
	{
		targetInfo= TargetZone->GetMyObjectInfo(pkt.targetsecid(), pkt.objecttype(), pkt.targetid());
		if (targetInfo.nObjectID == 0)
		{
			int secid = pkt.targetsecid();
			int id = pkt.targetid();
			cout << "" << endl;
		}
	}
	
#else
	ObjectRef ObjectSP = Zone->Object(pkt.objecttype(), pkt.targetid());
	if (ObjectSP.get() == nullptr)
	{
		ackpkt.set_success(false);
		return false;
	}
	else	
		bRet = true;
#endif // __SEAMLESS__

	//CSectorRef Sector = Zone->GetSectorRef(m_nSectorID);
#endif	
	/*switch (pkt.objecttype())
	{
	case Object::Player:
		pObject = Sector->Object(pkt.targetid(), Object::Player);
		if (pObject == nullptr)
			ackpkt.set_success(false);
		break;
	case Object::Monster:
		pObject = Sector->GetMonster(pkt.targetid());
		if (pObject == nullptr)
		{
			ackpkt.set_success(false);
			bRet = false;
		}
		break;
	default:
	{
		bRet = false;
		ackpkt.set_success(false);
		break;

	}
	}*/
	if (bRet == true)
	{
		//if (pkt.objecttype() == Object::Monster)

			//ObjectRef Monster = Sector->GetMonster(pkt.targetid());
			//if (Monster == nullptr)
			//	ackpkt.set_success(false);
			//else
		{
			/**/

#ifdef __SEAMLESS__
			float dist = Util::distance(m_vPos.x(), m_vPos.y(), targetInfo.vPos.x, targetInfo.vPos.y);

			if (dist > Zone::BroadCast_Distance)
				ackpkt.set_success(false);
			
			CBattle Logic;
			if(Logic.Attack(m_nAttack, nKill, targetInfo)==false)
				ackpkt.set_success(false);
			else
			{
				//타겟이 다른 존 일경우, 해당 존으로 상태정보 알림!
				if (bSameZone == false)
				{
					TargetZone->DoZoneJobTimer(0,TargetZone->ZoneID(), &CZone::Update_ObjectInfo, targetInfo);
					//TargetZone->DoLogicJob(TargetZone->ZoneID(), &CZone::Update_ObjectInfo, targetInfo);

				}
				else
				{
					Zone->Update_ObjectInfo(targetInfo);
					
				}

				S_ATTACK_REACT_ACK reactpkt;
				reactpkt.set_attackobjectid(m_nObjectID);
				reactpkt.set_attackobjecttype(Object::Player);
				reactpkt.set_sendtime(GetTickCount64());
				reactpkt.set_skillid(1);
				reactpkt.set_targetobjectid(targetInfo.nObjectID);
				reactpkt.set_targetobjecttype(targetInfo.nObjectType);
				auto vPos = reactpkt.mutable_pos();
				vPos->set_x(m_vPos.x());
				vPos->set_y(m_vPos.y());
				vPos->set_z(m_vPos.z());

				/*
					해당 유저가 위치한 섹터 및 둘러싼 이웃섹터의 유저들에게 전투결과 브로드캐스팅

				*/
				Zone->BroadCast_Player(m_nSectorID, reactpkt);



			}
#else

			float dist = Util::distance(m_vPos.x(), m_vPos.y(), ObjectSP->GetPos().x(), ObjectSP->GetPos().y());

			if (dist > Zone::BroadCast_Distance)
				ackpkt.set_success(false);

			if (ObjectSP->Attacked(m_nAttack, nKill) == false)
				ackpkt.set_success(false);
#endif	//__SEAMLESS__

			if (nKill > 0)
			{
				//static_cast<CMonster*>(Monster.get())->GetGold();
#ifdef __SEAMLESS__
				int nGainGold = targetInfo.nGold;
				int nGainExp = targetInfo.nExp;
#else
				int nGainGold = (ObjectSP)->GetGold();
				int nGainExp = (ObjectSP)->GetGold();
#endif
				m_nGold += nGainGold;
				m_nExp += nGainExp;

				m_nKillcount++;
#ifdef  __COUCHBASE_DB__
				CouchbaseClient* pDBConnect = g_CouchbaseManager->GetConnection(LThreadId);

#ifdef __COUCHBASE_DB_ASYNC__
				auto doc = make_shared<document>();
#else
				document* doc = new document;
#endif // __COUCHBASE_DB_ASYNC__
				doc->threadID = LThreadId;
				doc->cas = nCas;
				doc->key = to_string(playerId);

				if (LevelUp(m_nLevel, m_nExp))
				{
					m_nLevel++;
					doc->type = DB::PLAYER_LEVEL_UPDATE;
					json j = { {"playerid",playerId } ,{"Level",m_nLevel},{"Exp",m_nExp},{"Gold",m_nGold},{"Kill",m_nKillcount} };
					doc->value = j.dump();

#ifdef __COUCHBASE_DB_ASYNC__
					pDBConnect->DoAsyncDB(&CouchbaseClient::upsertSP, doc);
#else
					pDBConnect->upsert(doc);
#endif // __COUCHBASE_DB_ASYNC__



				}
				else
				{
					doc->type = DB::PLAYER_EXP_MONEY_UPDATE;
					json j = { {"playerid",playerId } ,{"Level",m_nLevel} ,{"Exp",m_nExp},{"Gold",m_nGold},{"Kill",m_nKillcount} };
					doc->value = j.dump();
					//"SELECT EXISTS(SELECT 1 FROM `default` USE KEYS [\"" + doc.key + "\"]);";
				//std::string query = "SELECT EXISTS(SELECT 1 FROM `default` USE KEYS [\"" + key_to_check + "\"]);";
#ifdef __COUCHBASE_DB_ASYNC__
					pDBConnect->DoAsyncDB(&CouchbaseClient::upsertSP, doc);
#else
					pDBConnect->upsert(doc);
#endif // __COUCHBASE_DB_ASYNC__


				}
#endif
				/*


				*/

				ackpkt.set_targetalive(false);

			}
			else
				ackpkt.set_targetalive(true);

			ackpkt.set_success(true);
		}
	}
#ifdef __ZONE_THREAD__
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(ackpkt, m_nZoneID);
#else
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(ackpkt, m_nZoneID);
#endif // __ZONE_THREAD__	
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
