#pragma once



struct StMonsterAction
{
	Object::eObject_State eState;
	int64	nLastTickCount = 0;
};
class CMonster : public CObject
{
public:
	CMonster();

	virtual void Update();
	virtual void AI_Idle();
	virtual void AI_Move();
	virtual void AI_Attack();

	void SetTarget(CObject* pObject)
	{
		if (pObject == nullptr)
			return;

		pTarget = pObject;
	}


private:
	Object::eObject_State	m_eState;
	int		m_nHP;
	int		m_nAttack;

	int64	nLastTickCount;
	CObject* pTarget;

	





};

