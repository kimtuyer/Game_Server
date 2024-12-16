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


protected:
	int	m_nZoneID;
	//int	m_nHP;
	//int	m_nAttack;

private:
	bool	m_bActivate;
	float	m_fSearchRange;
	float	m_fAttackRange;

};

