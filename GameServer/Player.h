#pragma once
#include "Protocol.pb.h"
class CPlayer: public CObject
{
public:
	CPlayer(GameSessionRef&);
	CPlayer(int playerid, int zoneid, int sectorid);
	//void	SetActivate(bool bFlag)
	//{
	//	m_bActivate = bFlag;
	//}
	//bool	GetActivate()
	virtual void Update();

	virtual void	LeaveZone();

	void LoginOff(bool bFlag)
	{
		bLogin = bFlag;
	}
	bool IsLogin()
	{
		return bLogin;
	}


	bool	Attack(Protocol::C_ATTACK& pkt);

	bool	LevelUp(int nlevel, int& nExp);

	int						nCas = 0;
	uint64					playerId = 0;
	string					name;
	int						nLevel;
	Protocol::PlayerType	type = Protocol::PLAYER_TYPE_NONE;
	weak_ptr<class GameSession>	ownerSession; // Cycle
	int						m_nKillcount = 0;

	Protocol::D3DVECTOR		Pos;
private:
	bool	bLogin;
	//m_nf

};

