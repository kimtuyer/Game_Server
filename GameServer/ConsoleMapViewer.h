#pragma once
#include <concurrent_queue.h>
using namespace Concurrency;
using namespace Zone;
extern atomic<int>	g_nPacketCount;
class ConsoleMapViewer
{
private:
	//static const int ZONE_WIDTH = 30;  // �� ���� ���� ũ��
	//static const int ZONE_HEIGHT = 15; // �� ���� ���� ũ��
	//static const int ZONES_PER_ROW = 5;
	//static const int ZONES_PER_COL = 3;
	bool needRedrawBorders = true;  // �׵θ� �ٽ� �׷��� �ϴ��� �÷���

	struct Update {
		int playerId;
		int zoneId;
		int x, y;
	};
	struct ObjectPos {
		int zoneId;
		int x, y;

	};

	std::mutex mtx;
	std::map<int, std::pair<int, int>> playerPositions;  // playerId -> (x, y)	

	//lock-free ť, cpu��뷮 ���� ����
	concurrent_queue<Update> updateQueue;  // lock-free ť

	//���� ���۸�+�ֱ��� ������Ʈ
	std::map<int/*Objectid*/, ObjectPos> pendingUpdates;  // ������Ʈ ����
	std::map<int, std::pair<int, int>> currentDisplay;  // ���� ȭ��


	

	void clearScreen() {
		system("cls");
	}
public:
	void gotoxy(int x, int y) {
		COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
	}

	void displayPacketCnt();


	void drawZoneBorders() {

		if (needRedrawBorders == false)
			return;

		//cout << "��ü �ʴ� ��Ŷ ó����:" << g_nPacketCount << endl;
		for (int col = 0; col < ZONES_PER_COL; col++) {
			for (int row = 0; row < ZONES_PER_ROW; row++) {
				int startX = row * (ZONE_WIDTH + 1);
				int startY = col * (ZONE_HEIGHT + 1);

				// ��� �׵θ�
				gotoxy(startX, startY);
				std::cout << "+";
				for (int i = 0; i < ZONE_WIDTH; i++) std::cout << "-";
				std::cout << "+";

				// �¿� �׵θ�
				for (int i = 1; i < ZONE_HEIGHT; i++) {
					gotoxy(startX, startY + i);
					std::cout << "|";
					gotoxy(startX + ZONE_WIDTH + 1, startY + i);
					std::cout << "|";
				}

				// �ϴ� �׵θ�
				gotoxy(startX, startY + ZONE_HEIGHT);
				std::cout << "+";
				for (int i = 0; i < ZONE_WIDTH; i++) std::cout << "-";
				std::cout << "+";

				// �� ��ȣ ǥ��
				gotoxy(startX + ZONE_WIDTH / 2, startY + ZONE_HEIGHT / 2);
				std::cout << col * ZONES_PER_ROW + row + 1;
			}
		}
		needRedrawBorders = false;
		return;
	}

	std::pair<int, int> getScreenPosition(int zoneId, int localX, int localY) {
		int zoneRow = (zoneId - 1) % ZONES_PER_ROW;
		int zoneCol = (zoneId - 1) % ZONES_PER_COL;

		//int zoneCol = (zoneId - 1) / ZONES_PER_ROW;

		int screenX =localX + 1;
		int screenY = localY + 1;

		//int screenX = zoneRow * (ZONE_WIDTH + 1) + localX + 1;
		//int screenY = zoneCol * (ZONE_HEIGHT + 1) + localY + 1;

		return { screenX, screenY };
	}

public:
	ConsoleMapViewer();

	void updatePlayerPosition(int playerId, int zoneId, int x, int y) {
		std::lock_guard<std::mutex> lock(mtx);

		// ���� ��ġ �����
		if (playerPositions.find(playerId) != playerPositions.end()) {
			auto prevPos = playerPositions[playerId];
			gotoxy(prevPos.first, prevPos.second);
			std::cout << " ";
		}

		// �� ��ġ ��� �� ����
		auto screenPos = getScreenPosition(zoneId, x, y);
		playerPositions[playerId] = screenPos;

		// �� ��ġ�� �÷��̾� ǥ��
		gotoxy(screenPos.first, screenPos.second);
		std::cout << ".";

		//std::cout << playerId;
	}

	/*			�� ���� ť ���									*/
	void Concurrent_queueUpdate(int playerId, int zoneId, int x, int y) {
		//updateQueue.push()
		updateQueue.push({ playerId, zoneId, x, y });
	}
	void processUpdates() {
		Update update;
		while (updateQueue.try_pop(update)) {
			// ȭ�� ���� ó��

			// ���� ��ġ �����
			if (playerPositions.find(update.playerId) != playerPositions.end()) {
				auto prevPos = playerPositions[update.playerId];
				gotoxy(prevPos.first, prevPos.second);
				std::cout << " ";
			}

			// �� ��ġ ��� �� ����
			auto screenPos = getScreenPosition(update.zoneId, update.x, update.y);
			playerPositions[update.playerId] = screenPos;

			// �� ��ġ�� �÷��̾� ǥ��
			gotoxy(screenPos.first, screenPos.second);
			std::cout << ".";
		}
	}
/*																		*/

/* ���� ���۸�+	�ֱ��� ������Ʈ							*/	

	// ��Ŷ ���� �� ȣ�� (lock �ּ�ȭ)
	void queuePlayerUpdate(int objectid, int zoneId, int x, int y) {
		std::lock_guard<std::mutex> lock(mtx);
		//gotoxy(1,0);
		//cout << "x ,y : " << x << "," << y << endl;
		pendingUpdates[objectid]={ zoneId, x, y };
	}

	// �ֱ������� ȣ�� (��: 16ms ����)
	void renderFrame() {

		displayPacketCnt();

		std::map<int, ObjectPos> updates;
		{
			std::lock_guard<std::mutex> lock(mtx);
			updates.swap(pendingUpdates);  // ���� �������� lock �ð� �ּ�ȭ
		}

		// lock ���� ȭ�� ����
		for (const auto& [objectid, pos] : updates) {
			// ���� ��ġ �����
			if (currentDisplay.count(objectid)) {
				auto& prevPos = currentDisplay[objectid];
				gotoxy(prevPos.first, prevPos.second);
				std::cout << " ";
			}

			// �� ��ġ �׸���
			auto screenPos = getScreenPosition(pos.zoneId, pos.x, pos.y);
			currentDisplay[objectid] = screenPos;
			gotoxy(screenPos.first, screenPos.second);
			if(objectid<= g_nZoneCount*g_nZoneUserMax)
				std::cout << "p";
			else
				std::cout << "m";

		}
	}

	/*																	*/



	void removePlayer(int playerId) {
		std::lock_guard<std::mutex> lock(mtx);
		if (currentDisplay.find(playerId) != currentDisplay.end()) {
			auto pos = currentDisplay[playerId];
			gotoxy(pos.first, pos.second);
			std::cout << " ";
			currentDisplay.erase(playerId);
		}
	}

	void refreshDisplay() {
		std::lock_guard<std::mutex> lock(mtx);
		clearScreen();
		needRedrawBorders = true;
		drawZoneBorders();

		// ��� �÷��̾� ��ǥ��
		for (const auto& player : currentDisplay) {
			gotoxy(player.second.first, player.second.second);
			std::cout << player.first;
		}
	}
};
extern shared_ptr<ConsoleMapViewer> GConsoleViewer;

