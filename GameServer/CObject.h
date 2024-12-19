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
	}
	virtual bool	GetActivate()
	{
		return m_bActivate;
	}

	virtual void	SetZoneID(int nZoneid)
	{
		m_nZoneID = nZoneid;
	}

	virtual int		GetZoneID()
	{
		return m_nZoneID;
	}

	virtual int		ObjectType()
	{
		return (int)eObjectType;
	}

	virtual	void	SetObjectType(Object::ObjectType eType)
	{
		eObjectType = eType;
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
		vPos = Pos;
		m_bPos = true;
	}

	Protocol::D3DVECTOR& GetPos()
	{
		return vPos;
	}


	virtual void	UpdatePos(bool bFlag)
	{
		m_bPos = bFlag;
	}
	virtual bool	IsUpdatePos()
	{
		return m_bPos;
	}


protected:
	int	m_nZoneID;
	bool	m_bActivate;
	Object::ObjectType eObjectType; 
	int		m_nObjectID;
	atomic<bool>	m_bPos = false;		//이전 위치 다를시 체크용
	Protocol::D3DVECTOR vPos;
	//int	m_nHP;
	//int	m_nAttack;

private:
	float	m_fSearchRange;
	float	m_fAttackRange;
};

