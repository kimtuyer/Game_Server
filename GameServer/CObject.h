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


protected:
	int	m_nZoneID;
	bool	m_bActivate;
	Object::ObjectType eObjectType;
	//int	m_nHP;
	//int	m_nAttack;

private:
	float	m_fSearchRange;
	float	m_fAttackRange;
};

