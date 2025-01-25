#pragma once

using playerID = int;
typedef std::map<playerID, PlayerRef> PlayerList;
class CPlayerManager
{
public:
	CPlayerManager();
	

	void Insert(int nPlayerid, GameSessionRef&);
	bool Find(int nPlayerid);
	CPlayer* GetPlayer(int nPlayerid);
	void Remove(int nPlayerid);



	PlayerRef Player(int nPlayerID);
	
		
	
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

