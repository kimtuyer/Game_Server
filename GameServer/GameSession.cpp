#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "Room.h"
#include "Player.h"
#include "CPlayerManager.h"
void GameSession::OnConnected()
{
	GSessionManager.Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
	GSessionManager.Remove(static_pointer_cast<GameSession>(shared_from_this()));

	if (_currentPlayer)
	{
		//����� �÷��̾�Ŵ����� ��ü������ �ʸ��� üũ�ϸ鼭,�α׾ƿ��ð��� 1�ð������ ������.
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

	// TODO : packetId �뿪 üũ
	ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)
{
}