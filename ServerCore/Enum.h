#pragma once

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
}
static const	int	g_nClientMaxCount = 1000;
static 	int	g_nConnectedUser = 0;

namespace Zone
{
	static const	int	g_nZoneCount = 15;
	static	const	int g_nZoneUserMax = 200;
	static const	int MonsterMaxCount = 200;

	//class CZone_Manager;

	static const int ZONE_WIDTH = 30;  // 각 존의 가로 크기
	static const int ZONE_HEIGHT = 15; // 각 존의 세로 크기
	static const int ZONES_PER_ROW = 5;
	static const int ZONES_PER_COL = 3;

	static const int  Sector_Count = 16;
	static const float  Sector_WIDTH = ZONE_WIDTH/4;  // 각 섹터의 가로 크기
	static const float  Sector_HEIGHT = (float)ZONE_HEIGHT/4; // 각 섹터의 세로 크기

	static const int  Sector_PER_ROW = 4;
	static const int  Sector_PER_COL = 4;

	static const int  BroadCast_Cnt = 7;
	static const float  BroadCast_Distance = 3.5;






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


