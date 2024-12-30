#pragma once
//#include <nlohmann/json.hpp>
using json = nlohmann::json;

//enum Type
//{
//	PLAYER_KEY_REQ = 0,
//	PLAYER_DATA_LOAD = 1,
//	PLAYER_DATA_CREATE,
//	PLAYER_EXP_MONEY_UPDATE,
//	PLAYER_LEVEL_UPDATE,
//	PLAYER_ITEM_ADD,
//	PLAYER_ITEM_REMOVE,
//	PLAYER_EQUIP_ITEM,
//	PLAYER_UNEQUIP_ITEM
//
//
//};
class CouchbaseHandler
{

public:
	void	HandleDBJob(const document doc,const  json j);

	void	Handle_PLAYER_KEY_REQ_ACK(const document& doc,const json& j);
	void	Handle_PLAYER_DATA_CREATE_ACK(const document& doc);
	void	Handle_PLAYER_DATA_LOAD_ACK(const document& doc, const json& j);

};
extern shared_ptr<CouchbaseHandler> GCouchbaseHandler;

