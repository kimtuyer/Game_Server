#pragma once
#include <concurrent_queue.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct PacketInfo
{
	PacketInfo()
	{
		PacketSessionRef m_session = nullptr;
		BYTE* m_buffer = nullptr;
		int32 m_len = 0;
	}

	PacketInfo(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		m_session = session;
		m_buffer = buffer;
		m_len = len;
	}

	PacketSessionRef m_session=nullptr;
	BYTE* m_buffer=nullptr;
	int32 m_len=0;
};
struct ZoneQueue {
	concurrency::concurrent_queue<PacketInfo> jobs;
	mutex zoneMutex;
	// 존 관련 데이터
	//vector<User*> users;
	// ... 기타 존 데이터
};

struct PlayerInfo {
    int id;
    int type;
    int level;
	int Exp;
	int Gold;
	int nKillcount;
    //float hp;
};
// from_json과 to_json 함수 정의
inline void from_json(const json& j, PlayerInfo& p) {
	j.at("playerid").get_to(p.id);
	j.at("type").get_to(p.type);
	j.at("Level").get_to(p.level);
	j.at("Exp").get_to(p.Exp);
	j.at("Gold").get_to(p.Gold);
	j.at("Kill").get_to(p.nKillcount);

	// j.at("hp").get_to(p.hp);
}

inline void to_json(json& j, const PlayerInfo& p) {
	j = json{
		{"playerid", p.id},
		{"type", p.type},
		{"Level", p.level},
		{"Exp", p.Exp},
		{"Gold", p.Gold},
		{"Kill", p.nKillcount},

		//{"hp", p.hp}
	};
}



struct Postion
{
    
    float x = 0;
    float y = 0;
    float z = 0;
    

};
typedef struct {
	DB::Type type;
	std::string value;
	size_t nvalue;
} document_content;

struct document {
	DB::Type type;
	int threadID;
	string key;
	uint64_t cas;
	std::string value;
	int64 sendTime;
	//std::string error_message;
};


namespace Sector
{
	struct ObjectInfo
	{
		int nSectorID=0;
		int nObjectID=0;
		int nObjectType=0;
		Postion vPos;

	};


}

namespace Item
{
	enum Type
	{
		Weapon=1,
		Armor,
		Potion,
		End
	};

	struct ItemInfo
	{
		int nItemid = 0;
		int nItemType = 0;
		int nValue = 0;

	};


}

//struct D3DVECTOR
//{
//    
//    float x = 0;
//    float y = 0;
//    float z = 0;
//    
//
//};