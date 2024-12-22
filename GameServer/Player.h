#pragma once

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

	uint64					playerId = 0;
	string					name;
	Protocol::PlayerType	type = Protocol::PLAYER_TYPE_NONE;
	weak_ptr<class GameSession>	ownerSession; // Cycle

	Protocol::D3DVECTOR		Pos;
private:
	//m_nf

};

