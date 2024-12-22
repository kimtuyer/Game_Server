#pragma once
using ObjectType = int;
using ObjectID = int;
typedef  map<ObjectID, ObjectRef> ObjectList;

class CSector : public JobQueue
{
public:
	CSector(int nSectorID, int nZoneID, Protocol::D3DVECTOR vPos);

	void Update();

	bool BelongtoSector(Protocol::D3DVECTOR vPos);

	bool FindObject(int objectID);


	bool Insert(int nObjectType, ObjectRef Object);

	bool Delete(int nObjectType, ObjectRef Object);

	void SendObjectlist();

	void SendPlayer();

	bool Empty()
	{
		WRITE_LOCK;
		return m_nlistObject.empty();
	}

	void	SetActivate(bool bFlag)
	{
		m_bActivate = bFlag;
	}
	bool	GetActivate()
	{
		return m_bActivate;
	}

	ObjectList& PlayerList();
	bool	Empty_Player()
	{
		WRITE_LOCK;
		return m_nlistObject[Object::Player].empty();
	}


	void	SetStartpos(Protocol::D3DVECTOR vPos)
	{
		m_vStartpos = vPos;
	}

	Protocol::D3DVECTOR  StartPos()
	{
		return m_vStartpos;
	}

	void Insert_adjSector(int sectorID, float x, float y);


	map<int, Protocol::D3DVECTOR> m_adjSectorList;
private:
	USE_LOCK;
	int m_nZoneID;
	int	m_nSectorID;
	bool m_bActivate;
	Protocol::D3DVECTOR m_vStartpos;



	//map<ObjectID, ObjectRef > m_nlistObject;

	map<ObjectType, map<ObjectID, ObjectRef>> m_nlistObject;


	//map<int, ObjectRef> m_listObject;

};

