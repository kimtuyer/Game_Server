#include "pch.h"
#include "RandomMove.h"
#include <ranges>
#define _USE_MATH_DEFINES // for C++
#include <math.h>

RandomMove::RandomMove(): gen(rd()),
angleDist(0, 2 * M_PI),
stepDist(0, MAX_STEP)
{
  
}

std::pair<float, float> RandomMove::getNextPosition(int nZoneid ,float currentX, float currentY)
{
    float angle = angleDist(gen);  // 랜덤 각도
    float step = stepDist(gen);    // 랜덤 이동 거리

    // 새 위치 계산
    float newX = currentX + step * cos(angle);
    float newY = currentY + step * sin(angle);

    /*
     
     존 uild가  
     1    2    3    4    5
     6    7    8    9    10
     11   12   13   14   15
   


        30,5
    */
    int ZONE_WIDTH = 1;
    int ZONE_HEIGHT = 1;

    if (nZoneid < 6)
    {
        ZONE_WIDTH = nZoneid * Zone::ZONE_WIDTH;
        ZONE_HEIGHT = Zone::ZONE_HEIGHT;
    }
    else if (nZoneid < 11)
    {
        ZONE_WIDTH = (nZoneid-5) * Zone::ZONE_WIDTH;
        ZONE_HEIGHT = 10 +Zone::ZONE_HEIGHT;
    }
    else
    {
        ZONE_WIDTH = (nZoneid - 10) *  Zone::ZONE_WIDTH;
        ZONE_HEIGHT = 20 + Zone::ZONE_HEIGHT;

    }

    // 존 경계 체크 및 보정
    newX = std::clamp(newX, 0.0f, ZONE_WIDTH - 1.0f);
    newY = std::clamp(newY, 0.0f, ZONE_HEIGHT - 1.0f);

    return { newX, newY };
}
