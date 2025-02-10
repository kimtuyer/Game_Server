#include "pch.h"
#include "CPlayerManager.h"
#include"Player.h"
shared_ptr<CPlayerManager> GPlayerManager = make_shared<CPlayerManager>();
CPlayerManager::CPlayerManager()
{
    m_mapPlayerlist.clear();
}

void CPlayerManager::Insert(int nPlayerid, GameSessionRef& gamesession)
{
    WRITE_LOCK; 
    m_mapPlayerlist.insert({ nPlayerid ,MakeShared<CPlayer>(gamesession)});

}

bool CPlayerManager::Find(int nPlayerid)
{
    READ_LOCK;
    return m_mapPlayerlist.contains(nPlayerid);
}

CPlayer* CPlayerManager::GetPlayer(int nPlayerid)
{
    READ_LOCK;
    if (m_mapPlayerlist.contains(nPlayerid))
    {
        return m_mapPlayerlist[nPlayerid].get();

    }
    else
        return nullptr;
}

void CPlayerManager::Remove(int nPlayerid)
{
    WRITE_LOCK;

    m_mapPlayerlist[nPlayerid]->LeaveZone();
    m_mapPlayerlist.erase(nPlayerid);
}

PlayerRef CPlayerManager::Player(int nPlayerID)
{

    READ_LOCK;
    if (m_mapPlayerlist.contains(nPlayerID))
    {
        return m_mapPlayerlist[nPlayerID];

    }
    else
        return nullptr;

}
