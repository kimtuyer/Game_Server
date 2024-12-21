#pragma once

namespace Tick
{
	enum
	{
		WORKER_TICK = 64,
		SECOND_TICK = 1000,
		MINUTE_TICK = SECOND_TICK * 60,

		AI_TICK=200,
		RENDERING_TICK=100,

	};
}

namespace Zone
{
	static const	int	g_nZoneCount = 15;
	static	const	int g_nZoneUserMax = 200;
	//class CZone_Manager;

	static const int ZONE_WIDTH = 20;  // 각 존의 가로 크기
	static const int ZONE_HEIGHT = 10; // 각 존의 세로 크기
	static const int ZONES_PER_ROW = 5;
	static const int ZONES_PER_COL = 3;

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

namespace Monster
{
	static const int MonsterMaxCount = 200;



}
