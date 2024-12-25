#include "pch.h"
#include "Player.h"
#include "CZone.h"
#include "CZone_Manager.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
CPlayer::CPlayer()
{
}

CPlayer::CPlayer(int playerid, int zoneid, int sectorid):m_nKillcount(0)
{
}

void CPlayer::Update()
{
}

void CPlayer::LeaveZone()
{
	/*
		�ش� ������ ������ ����� ���߿� �ٸ� ���� �� ���� ��ȣ�ۿ� ���̾��ٸ�?

	*/

	m_bActivate = false;

	CZoneRef Zone = GZoneManager->GetZone(m_nZoneID);
	Zone->Remove(Object::Player, playerId);
}

bool CPlayer::Attack(Protocol::C_ATTACK& pkt)
{
	int nKill = 0;
	Protocol::S_ATTACK_ACK ackpkt;

	CZoneRef Zone = GZoneManager->GetZone(m_nZoneID);
	if (Zone == nullptr)
		return false;

	CSectorRef Sector = Zone->GetSector(m_nSectorID);
	ObjectRef Monster = Sector->GetMonster(pkt.targetid());
	if (Monster == nullptr)
		return false;

	
	float dist = Util::distance(m_vPos.x(), m_vPos.y(), Monster->GetPos().x(), Monster->GetPos().y());

	if (dist > Zone::BroadCast_Distance)
		return false;

	if (Monster->Attacked(m_nAttack, nKill) == false)
		return false;

	if (nKill > 0)
	{

		m_nKillcount++;
		ackpkt.set_targetalive(false);
	}
	else
		ackpkt.set_targetalive(true);

	ackpkt.set_success(true);

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(ackpkt);
	ownerSession.lock()->Send(sendBuffer);

	
	
	//���� ����
	/*
	��� �� ü�� ���

	>>�� ���� ���� ������ Ÿ���� �Ǿ�,���ÿ� ������ �޾��� ���
	>>������Ŷ�� �ް�, ���ݰ��� ������ ���� �����߿� �ױ��� ���� ��������
	Ÿ���� �� ����, hp 0���� ���� �������� ų ī��Ʈ üũ !


	�� ü�� ������, ���� ����
	������,�ش� �� ���� ����->���� ����.���� ����Ʈ �߰�.
	Sector->update���� ���� �˻�->����Ʈ ���鼭 ������Ÿ�� ������ ��Ȱ
	Ÿ�̸� �߰� 3�ʵ� �ش� ���� �ٽ� ��Ȱ

	*/
	
	return true;
}