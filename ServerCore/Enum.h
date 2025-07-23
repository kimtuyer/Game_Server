#pragma once
#include "Dev_define.h"
namespace Tick
{
	enum
	{
		WORKER_TICK = 64,
		SECOND_TICK = 1000,
		MINUTE_TICK = SECOND_TICK * 60,

		AI_TICK=200,
		BROADCAST_TICK=100,
		RENDERING_TICK=100,

	};

	static const int g_nRTT = 125;
}

namespace JobType
{
	enum
	{
		GLOBAL_JOB=1,
		DB_JOB=2,
		Zone_Job=3,

	};


}
#ifdef __10000_USER_ZONE__
static const	int	g_nServerMaxUser = 10000;
#else
static const	int	g_nServerMaxUser = 5000;
#endif
static const	int	g_nClientMaxCount = 1000;
static 	int	g_nConnectedUser = 0;
static	int	g_nThreadCnt = 0;
namespace Thread {

	static const int IOCP_THREADS = 16;//6  cpu코어 개수 따라 맞춰서 설정
	static	int	ZONE_THREADS = 0;

}

namespace Zone
{
	static const	int	g_nZoneCount = 20;
#ifdef __10000_USER_ZONE__
	static	const	int g_nZoneUserMax = 300; //5000 테스트시 존당 최소 250이상 설정필요
#else
	static	const	int g_nZoneUserMax = 250; //5000 테스트시 존당 최소 250이상 설정필요
#endif // __10000_USER_ZONE__

	static const	int MonsterMaxCount = 200;

	//class CZone_Manager;

	static const int ZONE_WIDTH = 30;  // 각 존의 가로 크기
	static const int ZONE_HEIGHT = 16; // 각 존의 세로 크기
	static const int ZONES_PER_ROW = 5;
	static const int ZONES_PER_COL = g_nZoneCount/5;  //3
	static const int SECTORS_PER_SIDE = 4;



	static const int  Sector_Count = 16;
	static const float  Sector_WIDTH = (float)ZONE_WIDTH/4;  // 각 섹터의 가로 크기
	static const float  Sector_HEIGHT = (float)ZONE_HEIGHT/4; // 각 섹터의 세로 크기

	static const int  Sector_PER_ROW = 4;
	static const int  Sector_PER_COL = 4;

	static const int  BroadCast_Cnt = 20; //7  //20 기본값 ->존큐버전 5000성공
	static const float  BroadCast_Distance = 2; //3.5;


	enum ZoneJob
	{
		None=0,
		Insert=1,
		Delete=2,
		Update=3,

	};




}
namespace Sector
{
	enum UpdateType
	{
		Insert=1,
		Delete=2
	};
}


namespace Object
{

	enum ObjectType
	{
		Player=1,
		Monster=2
	};

	enum eObject_State
	{
		Idle = 1,
		Move = 2,
		Attack=3,
		End=4
	};
}


namespace LOCK
{
	enum lock
	{
		Object = 0,
		Player = 1,
		Monster = 2,
		Die = 3,
		Insert_M=4,
		End,

	};
}

namespace DB
{

	enum Type
	{
		PLAYER_KEY_REQ=0,
		PLAYER_DATA_LOAD=1,
		PLAYER_DATA_CREATE,
		PLAYER_EXP_MONEY_UPDATE,
		PLAYER_LEVEL_UPDATE,
		PLAYER_ITEM_ADD,
		PLAYER_ITEM_REMOVE,
		PLAYER_EQUIP_ITEM,
		PLAYER_UNEQUIP_ITEM


	};


}
