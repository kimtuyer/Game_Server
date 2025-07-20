#pragma once
#include <random>

class RandomMove
{
private:
    static constexpr float MAX_STEP = 0.5f;  // 한번에 이동할 최대 거리
    //static constexpr float ZONE_WIDTH = 20.0f;
  //  static constexpr float ZONE_HEIGHT = 10.0f;

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> angleDist;  // 0 ~ 2π
    std::uniform_real_distribution<float> stepDist;   // 0 ~ MAX_STEP

public:
    RandomMove();

    // 새로운 위치 계산
    std::pair<float, float> getNextPosition(int nZoneid, float currentX, float currentY,int nObjectType);

    // 특정 존 내의 랜덤한 초기 위치 생성
    std::pair<float, float> getRandomInitialPosition() {
        std::uniform_real_distribution<float> xDist(0, Zone::ZONE_WIDTH - 1);
        std::uniform_real_distribution<float> yDist(0, Zone::ZONE_HEIGHT - 1);

        return { xDist(gen), yDist(gen) };
    }

};

