#include "pch.h"
#include "CPlayerManager.h"
shared_ptr<CPlayerManager> GPlayerManager = make_shared<CPlayerManager>();
CPlayerManager::CPlayerManager()
{
    m_mapPlayerlist.clear();
}

void CPlayerManager::Insert(int nPlayerid,PlayerRef& Player)
{
    WRITE_LOCK; 
    m_mapPlayerlist.insert({ nPlayerid ,Player });

}

bool CPlayerManager::Find(int nPlayerid)
{
    return m_mapPlayerlist.contains(nPlayerid);
}

void CPlayerManager::Remove(int nPlayerid)
{
    WRITE_LOCK;
    m_mapPlayerlist.erase(nPlayerid);
}
