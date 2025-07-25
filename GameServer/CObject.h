#pragma once
class CObject : public JobQueue
{
public:
	CObject();
	CObject(const CObject& other);
	CObject& operator=(const CObject& other);

	virtual ~CObject()
	{
	}

	virtual void Update();
	virtual void	LeaveZone();

	virtual void AI_Idle();
	virtual void AI_Move();
	virtual void AI_Attack();

	void	Update(Sector::ObjectInfo info);

	float	GetSearchRange()
	{
		return m_fSearchRange;
	}
	float	GetAttackRange()
	{
		return m_fAttackRange;
	}
	virtual void	SetActivate(bool bFlag)
	{
		m_bActivate = bFlag;

		/*
		HP는 랜덤으로 다시 배정
		
		*/





	}
	virtual bool	GetActivate()
	{
		return m_bActivate;
	}

	virtual void	SetAlive(bool bFlag)
	{
		m_bAlive = bFlag;
	}
	virtual bool	GetAlive()
	{
		return m_bAlive;
	}


	virtual void	SetZoneID(int nZoneid)
	{
		m_nZoneID = nZoneid;
	}

	virtual int		GetZoneID()
	{
		return m_nZoneID;
	}

	virtual void	SetSectorID(int nZoneid)
	{
		m_nSectorID = nZoneid;
	}

	virtual int		GetSectorID()
	{
		return m_nSectorID;
	}

	virtual int		ObjectType()
	{
		return (int)m_nObjectType;
	}

	virtual	void	SetObjectType(Object::ObjectType eType)
	{
		m_nObjectType = eType;
	}

	virtual int		ObjectID()
	{
		return m_nObjectID;
	}

	virtual	void	SetObjectID(int nObjectID)
	{
		m_nObjectID = nObjectID;
	}

	void	SetPos(const Protocol::D3DVECTOR& Pos)
	{
		m_vPos = Pos;
		m_bPos = true;
	}

	Protocol::D3DVECTOR& GetPos()
	{
		return m_vPos;
	}

	virtual void	UpdatePos(bool bFlag)
	{
		m_bPos = bFlag;
	}
	virtual bool	IsUpdatePos()
	{
		return m_bPos;
	}


	void	SetExp(int exp)
	{
		m_nExp = exp;
	}
	void	SetGold(int gold)
	{
		m_nGold = gold;
	}

	int GetExp()
	{
		return m_nExp;
	}
	int64 GetGold()
	{
		return m_nGold;
	}

	bool Attacked(int nAttack, OUT int& nKillcount);
	
	int m_nStateTime[Object::End];
	Object::eObject_State	m_eState;

	Sector::ObjectInfo GetObjectInfo()
	{
		Sector::ObjectInfo info;
		info.nAttack = m_nAttack;
		info.nHP = m_nHP;
		info.nObjectID = m_nObjectID;
		info.nObjectType = m_nObjectType;
		info.nSectorID = m_nSectorID;
		info.vPos.x = m_vPos.x();
		info.vPos.y = m_vPos.y();
		info.vPos.z = m_vPos.z();
		info.nZoneID = m_nZoneID;
		info.nExp = m_nExp;
		info.nGold = m_nGold;

			
		return info;
	}

protected:
	//USE_LOCK;
	USE_LOCK;
	//Lock _battleock;
	int	m_nZoneID;
	int m_nSectorID;
	int		m_nObjectID;
	bool	m_bActivate;
	int  m_nObjectType;
	//int m_nStateTime[Object::End];

	Protocol::D3DVECTOR m_vPos;
	atomic<bool>	m_bPos = false;		//이전 위치 다를시 체크용

	bool m_bAlive = true;
	atomic<int>	m_nHP;
	int	m_nAttack=1;

	int		m_nLevel;
	int		m_nExp;
	int64	m_nGold;
private:
	float	m_fSearchRange;
	float	m_fAttackRange;
};
