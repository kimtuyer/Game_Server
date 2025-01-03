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
void GameSession::OnConnected()
{
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
	GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));

	if (_currentPlayer)
	{
		//레드는 플레이어매니저는 자체적으로 초마다 체크하면서,로그아웃시간이 1시간경과시 삭제함.
		GPlayerManager->Remove(_currentPlayer->playerId);
		_currentPlayer->LeaveZone();
		//_currentPlayer->DoAsync(&CPlayer::LeaveZone);

		if (auto room = _room.lock())
			room->DoAsync(&Room::Leave, _currentPlayer);


	}

	_currentPlayer = nullptr;
	_players.clear();
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

#ifdef __ZONE_THREAD_VER1__
	if (header->id != PKT_C_LOGIN)
	{
		int Zoneid = header->zoneID;
		if (GZoneManager->IsZone(Zoneid)==false)
		{
			return;
		}
		/*
			해당 Zone 큐에 넣고 나옴.
		*/
		zoneQueues[Zoneid]->jobs.push(PacketInfo(session, buffer, len));

	}
	else
		ClientPacketHandler::HandlePacket(session, buffer, len);
#endif
#ifdef __ZONE_THREAD_VER2__
	ClientPacketHandler::HandlePacket(session, buffer, len);
#endif
	// TODO : packetId 대역 체크
}

void GameSession::OnSend(int32 len)
{
}