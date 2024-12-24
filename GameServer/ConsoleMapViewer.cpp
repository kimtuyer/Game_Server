#include "pch.h"
#include "ConsoleMapViewer.h"
shared_ptr<ConsoleMapViewer> GConsoleViewer = make_shared<ConsoleMapViewer>();
atomic<int>	g_nPacketCount = 0;
void ConsoleMapViewer::displayPacketCnt()
{
	int TotalUser = g_nConnectedUser;
	if (LSecondTickCount < GetTickCount64())
	{
		LSecondTickCount = GetTickCount64() + Tick::SECOND_TICK;

		gotoxy(0, 0);
		cout << "전체 초당 패킷 처리량:" << g_nPacketCount<<endl;


		g_nPacketCount.store(0);
	}
}

ConsoleMapViewer::ConsoleMapViewer()
{
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);
	MoveWindow(console, r.left, r.top, 1300, 1000, TRUE); // 창 크기 조정

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	COORD bufferSize = { 200, 100 }; // 버퍼 크기 확장
	SetConsoleScreenBufferSize(hConsole, bufferSize);


	// 커서 깜박임 제거
	CONSOLE_CURSOR_INFO cursorInfo = { 1, FALSE };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

	// 초기 화면 설정
	clearScreen();
	drawZoneBorders();
	//pendingUpdates.clear();
	//currentDisplay.clear();
}