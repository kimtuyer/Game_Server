#pragma once

enum eMonster_State
{
	Idle=1,
	Move=2,
	Attack
};

struct StMonsterAction
{
	eMonster_State eState;
	int64	nLastTickCount = 0;
};
class CMonster : public CObject
{
public:
	CMonster();

	virtual void Update();
	void AI_Idle();
	void AI_Move();
	void AI_Attack();

	void SetTarget(CObject* pObject)
	{
		if (pObject == nullptr)
			return;

		pTarget = pObject;
	}


private:
	eMonster_State	m_eState;
	int		m_nHP;
	int		m_nAttack;

	int64	nLastTickCount;
	CObject* pTarget;

	





};

