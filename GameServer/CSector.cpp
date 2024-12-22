#include "pch.h"
#include "CSector.h"
#include "CMonster.h"
#include "Player.h" //�������ϸ� static_cast��ȯ ����

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
        ����,�÷��̾�  ������Ʈ ��, ����  or ���� �߰��� ���� ����Ʈ�� ���� ���


    
    */


        /*
         �ٸ� ���ͷ� �Űܾ��Ѵٸ� ��𿡼� �Űܾ��ϴ°�? 
         ���⼭ ���Ŵ����� ȣ���ؼ�,�ٸ� ���ͷ� �߰��ϵ��� �ϳ�?

         �̹� ���Ŵ������� ������ ���� ���Ϳ� �߰��� �ʿ��ϴٸ�?

         ���� Ŭ�������� �� ��ġ 

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
