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
		cout << "��ü �ʴ� ��Ŷ ó����:" << g_nPacketCount<<endl;


		g_nPacketCount.store(0);
	}
}

ConsoleMapViewer::ConsoleMapViewer()
{
	// Ŀ�� ������ ����
	CONSOLE_CURSOR_INFO cursorInfo = { 1, FALSE };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

	// �ʱ� ȭ�� ����
	clearScreen();
	drawZoneBorders();
	//pendingUpdates.clear();
	//currentDisplay.clear();
}