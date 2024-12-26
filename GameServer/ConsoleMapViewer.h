#pragma once
#include <concurrent_queue.h>
using namespace Concurrency;
using namespace Zone;
extern atomic<int>	g_nPacketCount;
class ConsoleMapViewer
{
private:
	//static const int ZONE_WIDTH = 30;  // 각 존의 가로 크기
	//static const int ZONE_HEIGHT = 15; // 각 존의 세로 크기
	//static const int ZONES_PER_ROW = 5;
	//static const int ZONES_PER_COL = 3;
	bool needRedrawBorders = true;  // 테두리 다시 그려야 하는지 플래그

	struct Update {
		int playerId;
		int zoneId;
		int x, y;
		bool bAlive;
	};
	struct ObjectPos {
		int zoneId;
		int x, y;
		bool bAlive;

	};

	std::mutex mtx;
	std::map<int, std::pair<int, int>> playerPositions;  // playerId -> (x, y)	

	//lock-free 큐, cpu사용량 많이 먹음
	concurrent_queue<Update> updateQueue;  // lock-free 큐

	//더블 버퍼링+주기적 업데이트
	std::map<int/*Objectid*/, ObjectPos> pendingUpdates;  // 업데이트 버퍼
	std::map<int, std::pair<int, int>> currentDisplay;  // 현재 화면


	

	void clearScreen() {
		system("cls");
	}
public:
	void gotoxy(int x, int y) {
		COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
	}

	void displayPacketCnt();


	void drawSectorBorders(int startX, int startY) {
		int sectorWidth = (ZONE_WIDTH ) / 4;
		int sectorHeight = (ZONE_HEIGHT ) / 4;

		// 섹터 세로 구분선
		for (int i = 1; i < SECTORS_PER_SIDE; i++) {
			int x = startX + i * sectorWidth;
			for (int y = startY + 1; y < startY + ZONE_HEIGHT ; y++) {
				gotoxy(x, y);
				std::cout << ":";
			}
		}

		// 섹터 가로 구분선
		for (int i = 1; i < SECTORS_PER_SIDE; i++) {
			int y = startY + i * sectorHeight;
			for (int x = startX + 1; x < startX + ZONE_WIDTH ; x++) {
				gotoxy(x, y);
				std::cout << ".";
			}
		}

		// 섹터 번호 표시
		for (int i = 0; i < SECTORS_PER_SIDE; i++) {
			for (int j = 0; j < SECTORS_PER_SIDE; j++) {
				int x = startX + j * sectorWidth + sectorWidth / 2;
				int y = startY + i * sectorHeight + sectorHeight / 2;
				gotoxy(x, y);
				int sectorNum = i * SECTORS_PER_SIDE + j;
				std::cout << sectorNum;
			}
		}
	}

	void drawZoneBorders() {

		if (needRedrawBorders == false)
			return;

		//cout << "전체 초당 패킷 처리량:" << g_nPacketCount << endl;
		for (int col = 0; col < ZONES_PER_COL; col++) {
			for (int row = 0; row < ZONES_PER_ROW; row++) {
				int startX = row * (ZONE_WIDTH );
				int startY = col * (ZONE_HEIGHT );


				// 상단 테두리
				gotoxy(startX, startY);
				std::cout << "+";
				for (int i = 0; i < ZONE_WIDTH; i++) std::cout << "-";
				std::cout << "+";

				// 좌우 테두리
				for (int i = 1; i < ZONE_HEIGHT; i++) {
					gotoxy(startX, startY + i);
					std::cout << "|";
					gotoxy(startX + ZONE_WIDTH , startY + i);
					std::cout << "|";
				}

				// 하단 테두리
				gotoxy(startX, startY + ZONE_HEIGHT);
				std::cout << "+";
				for (int i = 0; i < ZONE_WIDTH; i++) std::cout << "-";
				std::cout << "+";

				// 존 번호 표시

				gotoxy(startX + 2, startY + 1);
				//gotoxy(startX + ZONE_WIDTH / 2, startY + ZONE_HEIGHT / 2);
				std::cout <<"ZoneID:" << col * ZONES_PER_ROW + row + 1;

				// 섹터 그리기
				drawSectorBorders(startX, startY);

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

		// 이전 위치 지우기
		if (playerPositions.find(playerId) != playerPositions.end()) {
			auto prevPos = playerPositions[playerId];
			gotoxy(prevPos.first, prevPos.second);
			std::cout << " ";
		}

		// 새 위치 계산 및 저장
		auto screenPos = getScreenPosition(zoneId, x, y);
		playerPositions[playerId] = screenPos;

		// 새 위치에 플레이어 표시
		gotoxy(screenPos.first, screenPos.second);
		std::cout << ".";

		//std::cout << playerId;
	}

	/*			락 프리 큐 기법									*/
	void Concurrent_queueUpdate(int playerId, int zoneId, int x, int y) {
		//updateQueue.push()
		updateQueue.push({ playerId, zoneId, x, y });
	}
	void processUpdates() {
		Update update;
		while (updateQueue.try_pop(update)) {
			// 화면 갱신 처리

			// 이전 위치 지우기
			if (playerPositions.find(update.playerId) != playerPositions.end()) {
				auto prevPos = playerPositions[update.playerId];
				gotoxy(prevPos.first, prevPos.second);
				std::cout << " ";
			}

			// 새 위치 계산 및 저장
			auto screenPos = getScreenPosition(update.zoneId, update.x, update.y);
			playerPositions[update.playerId] = screenPos;

			// 새 위치에 플레이어 표시
			gotoxy(screenPos.first, screenPos.second);
			std::cout << ".";
		}
	}
/*																		*/

/* 더블 버퍼링+	주기적 업데이트							*/	

	// 패킷 수신 시 호출 (lock 최소화)
	void queuePlayerUpdate(int objectid, int zoneId, int x, int y,bool bAlive=true) {
		std::lock_guard<std::mutex> lock(mtx);
		//gotoxy(1,0);
		//cout << "x ,y : " << x << "," << y << endl;
		pendingUpdates[objectid]={ zoneId, x, y,bAlive };
	}

	// 주기적으로 호출 (예: 16ms 마다)
	void renderFrame() {

		displayPacketCnt();

		std::map<int, ObjectPos> updates;
		{
			std::lock_guard<std::mutex> lock(mtx);
			updates.swap(pendingUpdates);  // 빠른 스왑으로 lock 시간 최소화
		}

		// lock 없이 화면 갱신
		for (const auto& [objectid, pos] : updates) {
			// 이전 위치 지우기
			if (currentDisplay.count(objectid)) {
				auto& prevPos = currentDisplay[objectid];
				gotoxy(prevPos.first, prevPos.second);
				std::cout << " ";
			}

			// 새 위치 그리기
			auto screenPos = getScreenPosition(pos.zoneId, pos.x, pos.y);
			currentDisplay[objectid] = screenPos;
			gotoxy(screenPos.first, screenPos.second);
			if(objectid<= g_nZoneCount*g_nZoneUserMax)
				std::cout << "p";
			else
			{
				if(pos.bAlive)
					std::cout << "m";
				else
					std::cout << "x";

			}

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

		// 모든 플레이어 재표시
		for (const auto& player : currentDisplay) {
			gotoxy(player.second.first, player.second.second);
			std::cout << player.first;
		}
	}

	void refreshCurrentDisplay() {
		std::lock_guard<std::mutex> lock(mtx);
		clearScreen();
		needRedrawBorders = true;
		drawZoneBorders();

		currentDisplay.clear();
		//// 모든 플레이어 재표시
		//for (const auto& player : currentDisplay) {
		//	gotoxy(player.second.first, player.second.second);
		//	std::cout << player.first;
		//}
	}

};
extern shared_ptr<ConsoleMapViewer> GConsoleViewer;

