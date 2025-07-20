#pragma once
#include <random>

class RandomMove
{
private:
    static constexpr float MAX_STEP = 0.5f;  // �ѹ��� �̵��� �ִ� �Ÿ�
    //static constexpr float ZONE_WIDTH = 20.0f;
  //  static constexpr float ZONE_HEIGHT = 10.0f;

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> angleDist;  // 0 ~ 2��
    std::uniform_real_distribution<float> stepDist;   // 0 ~ MAX_STEP

public:
    RandomMove();

    // ���ο� ��ġ ���
    std::pair<float, float> getNextPosition(int nZoneid, float currentX, float currentY,int nObjectType);

    // Ư�� �� ���� ������ �ʱ� ��ġ ����
    std::pair<float, float> getRandomInitialPosition() {
        std::uniform_real_distribution<float> xDist(0, Zone::ZONE_WIDTH - 1);
        std::uniform_real_distribution<float> yDist(0, Zone::ZONE_HEIGHT - 1);

        return { xDist(gen), yDist(gen) };
    }

};

