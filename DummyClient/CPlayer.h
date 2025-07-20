#pragma once
class RandomMove;
class CClientPlayer : public CObject
{
public:
	CClientPlayer();
	~CClientPlayer();

	virtual void Update();
	virtual void AI_Idle();
	virtual void AI_Search();
	virtual void AI_Move();
	virtual void AI_Attack();

	//uint64					m_nplayerId = 0;
	string					name;
	Protocol::PlayerType	type = Protocol::PLAYER_TYPE_NONE;
	ClientSessionRef			ownerSession; // Cycle
	int						nLevel;


	//int		m_nZoneid;
	Object::eObject_State	m_eState;


	void Insert_Target(Protocol::Object_Pos info);
	void Update_TargetList(vector<Protocol::Object_Pos>list);
	void Delete_Target(Protocol::Object_Pos info);
	void Delete_TargetList(vector<Sector::ObjectInfo>list);
	void Clear_TargetList();
	void SetSearchOn(bool bFlag)
	{
		m_bSearch = bFlag;
	}

private:
	USE_LOCK;
	Protocol::Object_Pos		m_targetInfo;
	vector<Sector::ObjectInfo> m_vecTarget;
	map<int, Protocol::Object_Pos> m_listTarget;

	int m_nStateTime[Object::End];
	bool m_bSearch;

	int64	nLastTickCount;



	//shared_ptr<RandomMove> m_RandomMovement;
	//RandomMove* m_RandomMovement;
	//m_nf

};

