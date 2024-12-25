#pragma once
class CObject : public JobQueue
{
public:
	CObject();

	virtual ~CObject()
	{
	}

	virtual void Update();
	virtual void	LeaveZone();

	virtual void AI_Idle();
	virtual void AI_Move();
	virtual void AI_Attack();

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

	bool Attacked(int nAttack, OUT int& nKillcount);
	
	int m_nStateTime[Object::End];
	Object::eObject_State	m_eState;

protected:
	//USE_LOCK;
	Lock _battleock;
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
	int	m_nAttack=0;
private:
	float	m_fSearchRange;
	float	m_fAttackRange;
};
