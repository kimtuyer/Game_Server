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

}