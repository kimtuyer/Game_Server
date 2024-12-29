#pragma once

using playerID = int;
typedef std::map<playerID, PlayerRef> PlayerList;
class CPlayerManager
{
public:
	CPlayerManager();
	

	void Insert(int nPlayerid,PlayerRef Player);
	bool Find(int nPlayerid);
	void Remove(int nPlayerid);



	PlayerRef Player(int nPlayerID)
	{
		if (Find(nPlayerID))
		{

			return m_mapPlayerlist[nPlayerID];
		}
		return nullptr;
	}
	PlayerList& GetPlayerList()
	{
		READ_LOCK;
		return m_mapPlayerlist;

	}


private:
	USE_LOCK;
	PlayerList m_mapPlayerlist;


};
extern shared_ptr<CPlayerManager> GPlayerManager;

