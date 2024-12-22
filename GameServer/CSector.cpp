#include "pch.h"
#include "CSector.h"
#include "CMonster.h"
#include "Player.h" //�������ϸ� static_cast��ȯ ����

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
           // ���Ϳ� �ִ� ���� ������ ���� �������ϳ�?
           // Update �� ������, ����,��������Ʈ ó���Ҷ� ���� ����.
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

void CSector::SendPlayer()
{
}

ObjectList& CSector::PlayerList()
{
    {
        READ_LOCK;
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
