#pragma once
//#include "CZone.h"
class CZone;
using ZoneID = int;
class CZone_Manager : public JobQueue
{
	//DECLARE_SIGNLETON(CZone_Manager);
public:

	//ó�� ���� ���۽� �ʱ�ȭ�� �� �� ���� , �� ���� �̸� ���� ���� 
	
	~CZone_Manager()
	{
		m_listZone.clear();
	}


	//���� ����� �ش� �� Ȱ��ȭ �� �߰� �� ������Ʈ

	//����Ʈ ��ȸ�ϸ鼭 ������Ʈ
	void Init();

	//������Ʈ ����Ʈ�� ��ü ����
	bool Insert(int nZoneID, const CZoneRef);

	//������Ʈ ����Ʈ ��ü ����
	void Remove(int nZoneID, CZoneRef);

	//����Ʈ ��ȸ, ��ü Ÿ�̸� 
	void Update();




	CZoneRef	GetZone(int nZoneID);



private:
	USE_LOCK;
	map<ZoneID, CZoneRef> m_listZone;


};
extern shared_ptr<CZone_Manager> GZoneManager;
//CREATE_FUNCTION(CZone_Manager, ZoneManager);
