#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct PlayerInfo {
    int id;
    int type;
    int level;
    //float hp;
};
// from_json과 to_json 함수 정의
inline void from_json(const json& j, PlayerInfo& p) {
	j.at("id").get_to(p.id);
	j.at("type").get_to(p.type);
	j.at("level").get_to(p.level);
	// j.at("hp").get_to(p.hp);
}

inline void to_json(json& j, const PlayerInfo& p) {
	j = json{
		{"id", p.id},
		{"type", p.type},
		{"level", p.level},
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

//struct D3DVECTOR
//{
//    
//    float x = 0;
//    float y = 0;
//    float z = 0;
//    
//
//};