#pragma once
#include "Protocol.pb.h"
class CPlayer: public CObject
{
public:
	CPlayer();
	CPlayer(int playerid, int zoneid, int sectorid);
	//void	SetActivate(bool bFlag)
	//{
	//	m_bActivate = bFlag;
	//}
	//bool	GetActivate()
	virtual void Update();

	virtual void	LeaveZone();

	bool	Attack(Protocol::C_ATTACK& pkt);


	uint64					playerId = 0;
	string					name;
	Protocol::PlayerType	type = Protocol::PLAYER_TYPE_NONE;
	weak_ptr<class GameSession>	ownerSession; // Cycle
	int						m_nKillcount = 0;

	Protocol::D3DVECTOR		Pos;
private:
	//m_nf

};

