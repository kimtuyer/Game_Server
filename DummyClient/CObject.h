#pragma once
class CObject : public JobQueue
{

public:
	CObject();

	virtual void Update();

	virtual ~CObject()
	{

	}

	float	GetSearchRange()
	{
		return m_fSearchRange;
	}
	float	GetAttackRange()
	{
		return m_fAttackRange;
	}
	void	SetActivate(bool bFlag)
	{
		m_bActivate = bFlag;
	}
	bool	GetActivate()
	{
		return m_bActivate;
	}
	void	SetZoneid(int zoneid)
	{
		m_nZoneID = zoneid;
	}
	int		GetZoneID()
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

		m_nZoneID;

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



	virtual void	SetSectorID(int nSectorID)
	{
		m_nSectorID = nSectorID;
	}

	virtual int		GetSectorID()
	{
		return m_nSectorID;
	}


protected:
	int	m_nZoneID;
	int	m_nSectorID;
	bool	m_bActivate;
	Object::ObjectType eObjectType;
	int		m_nObjectID;
	atomic<bool>	m_bPos = false;		//이전 위치 다를시 체크용
	Protocol::D3DVECTOR m_vPos;
	int	m_nHP;
	int	m_nAttack;

private:
	float	m_fSearchRange;
	float	m_fAttackRange;

};

