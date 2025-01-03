#include "pch.h"
#include "ClientSession.h"
#include "ServerPacketHandler.h"

void ClientSession::OnConnected()
{
	Protocol::C_LOGIN pkt;
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt,0);
	Send(sendBuffer);
}

void ClientSession::OnDisconnected()
{
}

void ClientSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	PacketSessionRef session = GetPacketSessionRef();
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	// TODO : packetId 대역 체크
	ServerPacketHandler::HandlePacket(session, buffer, len);
}

void ClientSession::OnSend(int32 len)
{
}


