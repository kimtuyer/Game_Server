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
	PKT_S_MOVE_MONSTER = 1006,
	PKT_S_MOVE_PLAYER = 1007,
	PKT_C_ATTACK = 1008,
	PKT_S_ATTACK_ACK = 1009,
	PKT_S_ATTACK_REACT_ACK = 1010,
	PKT_S_OBJ_LIST = 1011,
	PKT_S_OBJ_REMOVE_ACK = 1012,
	PKT_S_PLAYER_LIST = 1013,
	PKT_S_PLAYER_REMOVE_ACK = 1014,
	PKT_S_ALL_OBJ_LIST = 1015,
	PKT_C_CHAT = 1016,
	PKT_S_CHAT = 1017,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt);
bool Handle_C_ENTER_ZONE(PacketSessionRef& session, Protocol::C_ENTER_ZONE& pkt);
bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt);
bool Handle_C_ATTACK(PacketSessionRef& session, Protocol::C_ATTACK& pkt);
bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt);

class ClientPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_C_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_LOGIN>(Handle_C_LOGIN, session, buffer, len); };
		GPacketHandler[PKT_C_ENTER_ZONE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_ENTER_ZONE>(Handle_C_ENTER_ZONE, session, buffer, len); };
		GPacketHandler[PKT_C_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_MOVE>(Handle_C_MOVE, session, buffer, len); };
		GPacketHandler[PKT_C_ATTACK] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_ATTACK>(Handle_C_ATTACK, session, buffer, len); };
		GPacketHandler[PKT_C_CHAT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_CHAT>(Handle_C_CHAT, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
static SendBufferRef MakeSendBuffer(Protocol::S_LOGIN&pkt, uint16 zoneID) { return MakeSendBuffer(pkt, PKT_S_LOGIN, zoneID); }
static SendBufferRef MakeSendBuffer(Protocol::S_ENTER_ACK&pkt, uint16 zoneID) { return MakeSendBuffer(pkt, PKT_S_ENTER_ACK, zoneID); }
static SendBufferRef MakeSendBuffer(Protocol::S_MOVE_ACK&pkt, uint16 zoneID) { return MakeSendBuffer(pkt, PKT_S_MOVE_ACK, zoneID); }
static SendBufferRef MakeSendBuffer(Protocol::S_MOVE_MONSTER&pkt, uint16 zoneID) { return MakeSendBuffer(pkt, PKT_S_MOVE_MONSTER, zoneID); }
static SendBufferRef MakeSendBuffer(Protocol::S_MOVE_PLAYER&pkt, uint16 zoneID) { return MakeSendBuffer(pkt, PKT_S_MOVE_PLAYER, zoneID); }
static SendBufferRef MakeSendBuffer(Protocol::S_ATTACK_ACK&pkt, uint16 zoneID) { return MakeSendBuffer(pkt, PKT_S_ATTACK_ACK, zoneID); }
static SendBufferRef MakeSendBuffer(Protocol::S_ATTACK_REACT_ACK&pkt, uint16 zoneID) { return MakeSendBuffer(pkt, PKT_S_ATTACK_REACT_ACK, zoneID); }
static SendBufferRef MakeSendBuffer(Protocol::S_OBJ_LIST&pkt, uint16 zoneID) { return MakeSendBuffer(pkt, PKT_S_OBJ_LIST, zoneID); }
static SendBufferRef MakeSendBuffer(Protocol::S_OBJ_REMOVE_ACK&pkt, uint16 zoneID) { return MakeSendBuffer(pkt, PKT_S_OBJ_REMOVE_ACK, zoneID); }
static SendBufferRef MakeSendBuffer(Protocol::S_PLAYER_LIST&pkt, uint16 zoneID) { return MakeSendBuffer(pkt, PKT_S_PLAYER_LIST, zoneID); }
static SendBufferRef MakeSendBuffer(Protocol::S_PLAYER_REMOVE_ACK&pkt, uint16 zoneID) { return MakeSendBuffer(pkt, PKT_S_PLAYER_REMOVE_ACK, zoneID); }
static SendBufferRef MakeSendBuffer(Protocol::S_ALL_OBJ_LIST&pkt, uint16 zoneID) { return MakeSendBuffer(pkt, PKT_S_ALL_OBJ_LIST, zoneID); }
static SendBufferRef MakeSendBuffer(Protocol::S_CHAT&pkt, uint16 zoneID) { return MakeSendBuffer(pkt, PKT_S_CHAT, zoneID); }

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

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId,uint16 zoneID)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		header->zoneID = zoneID;
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};