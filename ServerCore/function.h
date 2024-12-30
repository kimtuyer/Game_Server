#pragma once
#include <math.h>
#include <random>
namespace Util
{
	float static distance(float source_x, float source_y, float target_x, float target_y)
	{
		return sqrt(pow(target_x - source_x, 2) + pow(target_y - source_y, 2));
	}

	int	static Random_HP()
	{
		random_device rd;
		mt19937 gen(rd());
		uniform_int_distribution<int> HPlist(1,5);

		return HPlist(gen);
	}

	int	static Random_ClassType()
	{
		//Protocol::PlayerType::PLAYER_TYPE_END

		random_device rd;
		mt19937 gen(rd());
		uniform_int_distribution<int> classlist(1, 4);

		return classlist(gen);
	}
	int	static Random_Level()
	{
		//Protocol::PlayerType::PLAYER_TYPE_END

		random_device rd;
		mt19937 gen(rd());
		uniform_int_distribution<int> Level(1, 5);

		return Level(gen);
	}

	int	static Random_ExpGold(int level)
	{
		//Protocol::PlayerType::PLAYER_TYPE_END
		int exp = 0;
		if (level <= 1)
			exp = 100;
		else if (level <= 2)
			exp = 200;
		else if (level <= 3)
			exp = 300;
		else if (level <= 4)
			exp = 400;
		else
			exp = 500;
	
		return exp;
	}



}