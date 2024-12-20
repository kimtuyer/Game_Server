#pragma once
class RandomMove;
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
	Object::eObject_State	m_eState;



private:
	shared_ptr<RandomMove> m_RandomMovement;
	//RandomMove* m_RandomMovement;
	//m_nf

};

