#pragma once

namespace Tick
{
	enum
	{
		WORKER_TICK = 64,
		SECOND_TICK = 1000,
		MINUTE_TICK = SECOND_TICK * 60,

		AI_TICK=200,

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
		Attack
	};
}
