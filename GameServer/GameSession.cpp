#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "Room.h"
#include "Player.h"
#include "CPlayerManager.h"
#include "CZone_Manager.h"
#include "CZone.h"
#include "ThreadManager.h"
#include "ConsoleMapViewer.h"
void GameSession::OnConnected()
{
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
	GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));

	//if (_currentPlayer)
	{
		//����� �÷��̾�Ŵ����� ��ü������ �ʸ��� üũ�ϸ鼭,�α׾ƿ��ð��� 1�ð������ ������.
		GPlayerManager->Remove(nPlayerID);
		GConsoleViewer->queuePlayerUpdate(nPlayerID, 0, 0, 0, false);
		//_currentPlayer->LeaveZone();
		//_currentPlayer->DoAsync(&CPlayer::LeaveZone);

		//if (auto room = _room.lock())
		//	room->DoAsync(&Room::Leave, _currentPlayer);


	}

	//_currentPlayer = nullptr;
	//_players.clear();
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

#ifdef __ZONE_THREAD_VER3__
	if (header->id != PKT_C_LOGIN)
	{
		int Zoneid = header->zoneID;
		auto Zone = GZoneManager->GetZone(Zoneid);
		if (Zone == nullptr)
		{
			return;
		}
		Zone->AddPacketCount();
		/*
			�ش� Zone ť�� �ְ� ����.
		*/
#ifdef __ZONE_THREAD_VER1__
		ZonePacketQueues[Zoneid]->jobs.push(PacketInfo(session, buffer, len));
#endif
	}
		ClientPacketHandler::HandlePacket(session, buffer, len);
#else
#ifdef __ZONE_THREAD_VER1__
	if (header->id != PKT_C_LOGIN)
	{
		int Zoneid = header->zoneID;
		if (GZoneManager->IsZone(Zoneid)==false)
		{
			return;
		}
		/*
			�ش� Zone ť�� �ְ� ����.
		*/
		ZonePacketQueues[Zoneid]->jobs.push(PacketInfo(session, buffer, len));

	}
	else
		ClientPacketHandler::HandlePacket(session, buffer, len);
#endif //__ZONE_THREAD_VER1__

#ifdef __ZONE_THREAD_VER2__
	ClientPacketHandler::HandlePacket(session, buffer, len);
#endif
#endif // __ZONE_THREAD_VER3__
	// TODO : packetId �뿪 üũ
}

void GameSession::OnSend(int32 len)
{
}