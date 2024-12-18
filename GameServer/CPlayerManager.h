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

