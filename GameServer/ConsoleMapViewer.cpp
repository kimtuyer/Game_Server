#include "pch.h"
#include "ConsoleMapViewer.h"
shared_ptr<ConsoleMapViewer> GConsoleViewer = make_shared<ConsoleMapViewer>();
atomic<int>	g_nPacketCount = 0;
atomic<int>	g_nDBPacketCount = 0;

void ConsoleMapViewer::displayPacketCnt()
{
	int startX = (ZONE_WIDTH + 1) * ZONES_PER_ROW + 5;  // 맵 오른쪽에서 5칸 떨어진 위치
	int startY = 1;  // 상단에서 시작

	gotoxy(startX, startY);
	if (LSecondTickCount < GetTickCount64())
	{
		LSecondTickCount = GetTickCount64() + Tick::SECOND_TICK;

		//gotoxy(totalWidth * 11, 20);
		cout << "초당 패킷 처리량:" << g_nPacketCount << endl;

		g_nPacketCount.store(0);
	}



}

ConsoleMapViewer::ConsoleMapViewer()
{

	int totalWidth = (ZONE_WIDTH + 1) * ZONES_PER_ROW + 30;  // Zone Info 영역 포함
	int totalHeight = (ZONE_HEIGHT + 1) * ZONES_PER_COL + 15;

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	COORD bufferSize = {
		static_cast<SHORT>(totalWidth + 10),  // 여유 공간 추가
		static_cast<SHORT>(totalHeight + 30)
	};
	SetConsoleScreenBufferSize(hConsole, bufferSize);

	//totalHeight += 10;
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);
	MoveWindow(console, r.left, r.top,
		totalWidth * 10,  // 문자 너비를 픽셀로 변환 (약 10픽셀)
		totalHeight * 20, // 문자 높이를 픽셀로 변환 (약 20픽셀)
		TRUE);

	


	// 커서 깜박임 제거
	CONSOLE_CURSOR_INFO cursorInfo = { 1, FALSE };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

	// 초기 화면 설정
	clearScreen();
	drawZoneBorders();
	drawZoneInfo();
	//pendingUpdates.clear();
	//currentDisplay.clear();
}

void ConsoleMapViewer::drawZoneInfo()
{
	int startX = (ZONE_WIDTH + 1) * ZONES_PER_ROW + 5;
	int startY = 1;

	int totalPlayers = 0;
	int totalMonsters = 0;

	gotoxy(startX, startY);
	std::cout << "== Zone Information==";

	// 각 존별 플레이어/몬스터 수 계산
	std::map<int, std::pair<int, int>> zoneCounts;  // zoneId -> {players, monsters}
	for (const auto& [objectId, pos] : currentDisplay) {
		int zoneId = getZoneIdFromScreenPos(pos.first, pos.second);
		if (objectId <= g_nZoneCount * g_nZoneUserMax) {
			// 플레이어
			zoneCounts[zoneId].first++;
			totalPlayers++;
		}
		else if (pos.second != -1) {  // 유효한 위치에 있는 경우만
			// 몬스터
			zoneCounts[zoneId].second++;
			totalMonsters++;
		}
	}
	// 전체 플레이어 수 출력
	gotoxy(startX + 22, startY);  // "== Zone Information ==" 옆에 출력
	std::cout << "전체 유저:" << totalPlayers;

	if (LSecondTickCount < GetTickCount64())
	{
		LSecondTickCount = GetTickCount64() + Tick::SECOND_TICK;

		gotoxy(startX + 22, startY+2);
		cout  << "  초당 패킷 처리량:"<< g_nPacketCount <<" " << endl;

		g_nPacketCount.store(0);
	}

	

	// 존 정보 출력
	for (int zoneId = 1; zoneId <= g_nZoneCount; zoneId++) {
		gotoxy(startX, startY + zoneId + 1);
		std::cout << "Zone " << zoneId << ": ";
		auto& counts = zoneCounts[zoneId];
		std::cout << "P(" << counts.first << ") ";
		std::cout << "M(" << counts.second << ")   ";  // 공백으로 이전 텍스트 덮어쓰기
	}
	

	concurrent_unordered_map<int, int64>threadLatency;
	map<int, vector<pair<int, int>>> tempThreadtoZonelist;

	{
		std::lock_guard<mutex>lock(threadlock);

		threadLatency.swap(threadLatencyTime);
		tempThreadtoZonelist.swap(ThreadtoZonelist);

	}


	int nCnt = 0;
	
	//for (auto [threadid, Latency] : threadLatency)
	//{
	//	gotoxy(startX, startY + g_nZoneCount + 3+ nCnt);
	//
	//	if(tempThreadtoZonelist[threadid].empty())
	//	std::cout << "Thread :" << threadid << " UpdateTime: " << Latency <<" ZoneID1: empty" << "     " << endl;
	//	else if (tempThreadtoZonelist[threadid].size()==1)
	//		std::cout << "Thread :" << threadid << " UpdateTime: " << Latency << " ZoneID1:" << tempThreadtoZonelist[threadid][0].first << "     " << endl;
	//	else if (tempThreadtoZonelist[threadid].size() == 2)
	//		std::cout << "Thread :" << threadid << " UpdateTime: " << Latency << " ZoneID1:" << tempThreadtoZonelist[threadid][0].first << " ZoneID2:" << tempThreadtoZonelist[threadid][1].first << "     " << endl;
	//
	//	nCnt++;
	//}

	//for (int threadid = 8; threadid < 23; threadid++)
	//{
	//	gotoxy(startX, startY + g_nZoneCount + 3 + nCnt);
	//
	//	if (tempThreadtoZonelist[threadid].empty())
	//		std::cout << "Thread :" << threadid << " UpdateTime: " <<"(" << threadLatency[threadid] <<")" << " ZoneID1:" <<"("<< " " << ")" <<  " ZoneID2:" << "(" << "0" <<")"<<"     " << endl;
	//	else if (tempThreadtoZonelist[threadid].size() == 1)
	//		std::cout << "Thread :" << threadid << " UpdateTime: " <<"(" << threadLatency[threadid] <<")" << " ZoneID1:" <<"("<< tempThreadtoZonelist[threadid][0].first << ")" << " ZoneID2:" << "(" << "0" << ")" << "     " << endl;
	//	else if (tempThreadtoZonelist[threadid].size() == 2)
	//		std::cout << "Thread :" << threadid << " UpdateTime: " <<"(" << threadLatency[threadid] <<")" << " ZoneID1:" <<"("<< tempThreadtoZonelist[threadid][0].first << ")" << " ZoneID2:" << "(" << tempThreadtoZonelist[threadid][1].first << ")" << "     " << endl;
	//
	//	nCnt++;
	//
	//
	//}


}
