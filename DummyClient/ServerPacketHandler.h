#pragma once
#include "Protocol.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_C_LOGIN = 1000,
	PKT_S_LOGIN = 1001,
	PKT_C_ENTER_ZONE = 1002,
	PKT_S_ENTER_ACK = 1003,
	PKT_C_MOVE = 1004,
	PKT_S_MOVE_ACK = 1005,
	PKT_C_ATTACK = 1006,
	PKT_S_ATTACK_ACK = 1007,
	PKT_S_OBJ_LIST = 1008,
	PKT_S_OBJ_REMOVE_ACK = 1009,
	PKT_C_CHAT = 1010,
	PKT_S_CHAT = 1011,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt);
bool Handle_S_ENTER_ACK(PacketSessionRef& session, Protocol::S_ENTER_ACK& pkt);
bool Handle_S_MOVE_ACK(PacketSessionRef& session, Protocol::S_MOVE_ACK& pkt);
bool Handle_S_ATTACK_ACK(PacketSessionRef& session, Protocol::S_ATTACK_ACK& pkt);
bool Handle_S_OBJ_LIST(PacketSessionRef& session, Protocol::S_OBJ_LIST& pkt);
bool Handle_S_OBJ_REMOVE_ACK(PacketSessionRef& session, Protocol::S_OBJ_REMOVE_ACK& pkt);
bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt);

class ServerPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_S_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_LOGIN>(Handle_S_LOGIN, session, buffer, len); };
		GPacketHandler[PKT_S_ENTER_ACK] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_ENTER_ACK>(Handle_S_ENTER_ACK, session, buffer, len); };
		GPacketHandler[PKT_S_MOVE_ACK] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_MOVE_ACK>(Handle_S_MOVE_ACK, session, buffer, len); };
		GPacketHandler[PKT_S_ATTACK_ACK] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_ATTACK_ACK>(Handle_S_ATTACK_ACK, session, buffer, len); };
		GPacketHandler[PKT_S_OBJ_LIST] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_OBJ_LIST>(Handle_S_OBJ_LIST, session, buffer, len); };
		GPacketHandler[PKT_S_OBJ_REMOVE_ACK] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_OBJ_REMOVE_ACK>(Handle_S_OBJ_REMOVE_ACK, session, buffer, len); };
		GPacketHandler[PKT_S_CHAT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::S_CHAT>(Handle_S_CHAT, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::C_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_C_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ENTER_ZONE& pkt) { return MakeSendBuffer(pkt, PKT_C_ENTER_ZONE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_MOVE& pkt) { return MakeSendBuffer(pkt, PKT_C_MOVE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ATTACK& pkt) { return MakeSendBuffer(pkt, PKT_C_ATTACK); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_C_CHAT); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};