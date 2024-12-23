#include "pch.h"
#include "CSector.h"
#include "CMonster.h"
#include "Player.h" //참조안하면 static_cast변환 실패
#include "GameSession.h"

#include "ClientPacketHandler.h"

CSector::CSector(int nSectorID, int nZoneID, Protocol::D3DVECTOR vPos)
:    m_nSectorID(nSectorID),m_vStartpos(vPos),m_bActivate(true) ,m_nZoneID(nZoneID)
{
}

void CSector::Update()
{

    vector<CMonster*> vecMonsterlist;
    {
        READ_LOCK;
        for (auto& [objectid, Object] : m_nlistObject[Object::Monster])
        {
            if (Object->GetActivate() == false)
                continue;

            CMonster* pMonster = static_cast<CMonster*>(Object.get());
            vecMonsterlist.push_back(pMonster);
           // 섹터에 있는 몬스터 정보는 언제 지워야하나?
           // Update 다 끝난후, 삽입,삭제리스트 처리할때 같이 지움.
            //m_nlistObject[Object::Monster].erase(objectid);
        
        }

    }

    for (auto& pMonster : vecMonsterlist)
    {
        pMonster->Update();
    }      

}

bool CSector::BelongtoSector(Protocol::D3DVECTOR vPos)
{

    float StartX = m_vStartpos.x() - Zone::Sector_WIDTH / 2;
    float StartY = m_vStartpos.y() - Zone::Sector_HEIGHT / 2;

    float EndX= m_vStartpos.x() + Zone::Sector_WIDTH / 2;
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

bool CSector::Insert(int nObjectType, ObjectRef Object)
{
    WRITE_LOCK;
    m_nlistObject[nObjectType].insert({Object->ObjectID(),Object});
    return true;
}

bool CSector::Delete(int nObjectType, ObjectRef Object)
{
    WRITE_LOCK;
    m_nlistObject[nObjectType].insert({ Object->ObjectID(),Object });
    return true;
}

void CSector::SendObjectlist()
{
}

void CSector::BroadCast_Player(Sector::UpdateType eType,Sector::ObjectInfo ObjectInfo)
{
    ObjectList Playerlist=m_nlistObject[Object::Player];

        Protocol::S_PLAYER_LIST objpkt; //새로 들어온 섹터의 근처 유저들에게 알림.
  

    Protocol::Object_Pos objectPos;
    objectPos.set_id(ObjectInfo.nObjectID);
    Protocol::D3DVECTOR* vPos = objectPos.mutable_vpos();
    vPos->set_x(ObjectInfo.vPos.x);
    vPos->set_y(ObjectInfo.vPos.y);

    objpkt.add_pos()->CopyFrom(objectPos);


    auto distance = [](float source_x, float source_y, float target_x, float target_y)->float
        {
            return sqrt(pow(target_x - source_x, 2) + pow(target_y - source_y, 2));

        };

    for (auto [playerid , Player] : Playerlist)
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


ObjectList& CSector::PlayerList()
{
    {
        READ_LOCK;
        {
            return m_nlistObject[Object::Player];
        }


    }

    // TODO: 여기에 return 문을 삽입합니다.
}

void CSector::Insert_adjSector(int sectorID, float x, float y)
{

    Protocol::D3DVECTOR vPos;
    vPos.set_x(x);
    vPos.set_y(y);

    m_adjSectorList.insert({ sectorID,vPos });
}
