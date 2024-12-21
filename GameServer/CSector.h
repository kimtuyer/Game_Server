#pragma once
using ObjectType = int;
using ObjectID = int;
class CSector
{
public:
	CSector();

	void UpdateSector();

	int GetSectorID(Protocol::D3DVECTOR vPos);

	bool FindObject(int objectID);


	bool Insert(int sectorID, int objectID);

	bool DeletObject(int objectID);

	void SendObjectlist();

	void SendPlayer();




private:
	USE_LOCK;
	int m_nZoneID;
	int	m_nSectorID;

	map<ObjectType, map<ObjectID, ObjectRef>> m_nlistObject;

	//map<int, ObjectRef> m_listObject;

};

