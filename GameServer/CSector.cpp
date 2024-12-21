#include "pch.h"
#include "CSector.h"
#include "CMonster.h"
#include "Player.h" //참조안하면 static_cast변환 실패

CSector::CSector()
{
}

void CSector::UpdateSector()
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
       
            int nowSectorID = GetSectorID(Object->GetPos());
            if (m_nSectorID == nowSectorID)
                continue;

            m_nlistObject[Object::Monster].erase(objectid);
        
        }

    }
    for (auto& pMonster : vecMonsterlist)
    {
        pMonster->Update();
    }

    /*
        몬스터,플레이어  업데이트 후, 삭제  or 새로 추가한 몬스터 리스트가 있을 경우


    
    */


        /*
         다른 섹터로 옮겨야한다면 어디에서 옮겨야하는가? 
         여기서 존매니저를 호출해서,다른 섹터로 추가하도록 하나?

         이미 존매니저에서 루프를 지난 섹터에 추가가 필요하다면?

         몬스터 클래스에서 내 위치 

        */
        




    }

}

int CSector::GetSectorID(Protocol::D3DVECTOR vPos)
{
    return 0;
}

bool CSector::FindObject(int objectID)
{
    return false;
}

bool CSector::Insert(int sectorID, int objectID)
{
    return false;
}

bool CSector::DeletObject(int objectID)
{
    return false;
}

void CSector::SendObjectlist()
{
}

void CSector::SendPlayer()
{
}
