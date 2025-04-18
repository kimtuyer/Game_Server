#pragma once

#include <random>


struct StMonsterAction
{
	Object::eObject_State eState;
	int64	nLastTickCount = 0;
};
class CMonster : public CObject
{
public:
	CMonster(int nObjectID,int nZoneID, int nSectorID,Protocol::D3DVECTOR vStartPos, bool bActivate);
	CMonster(Sector::MonsterData, bool bActivate);

	CMonster(const CMonster& other);
	CMonster& operator=(const CMonster& other);
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
	
	
	int64	nLastTickCount;
	CObject* pTarget;

	//int m_nStateTime[Object::End];

	std::uniform_int_distribution<> m_ndistribute;
	std::random_device rd;
	std::mt19937 gen;
	   // 0 ~ MAX_STEP






};

