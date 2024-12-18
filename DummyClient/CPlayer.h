#pragma once

class CClientPlayer : public CObject
{
public:
	CClientPlayer();
	~CClientPlayer();

	virtual void Update();
	virtual void AI_Idle();
	virtual void AI_Move();
	virtual void AI_Attack();

	uint64					playerId = 0;
	string					name;
	Protocol::PlayerType	type = Protocol::PLAYER_TYPE_NONE;
	ClientSessionRef			ownerSession; // Cycle


	//int		m_nZoneid;
	Protocol::D3DVECTOR	m_vPos;
	Object::eObject_State	m_eState;



private:
	//m_nf

};

