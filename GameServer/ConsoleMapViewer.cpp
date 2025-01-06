#include "pch.h"
#include "ConsoleMapViewer.h"
shared_ptr<ConsoleMapViewer> GConsoleViewer = make_shared<ConsoleMapViewer>();
atomic<int>	g_nPacketCount = 0;
atomic<int>	g_nDBPacketCount = 0;

void ConsoleMapViewer::displayPacketCnt()
{
	int startX = (ZONE_WIDTH + 1) * ZONES_PER_ROW + 5;  // �� �����ʿ��� 5ĭ ������ ��ġ
	int startY = 1;  // ��ܿ��� ����

	gotoxy(startX, startY);
	if (LSecondTickCount < GetTickCount64())
	{
		LSecondTickCount = GetTickCount64() + Tick::SECOND_TICK;

		//gotoxy(totalWidth * 11, 20);
		cout << "�ʴ� ��Ŷ ó����:" << g_nPacketCount << endl;

		g_nPacketCount.store(0);
	}



}

ConsoleMapViewer::ConsoleMapViewer()
{

	int totalWidth = (ZONE_WIDTH + 1) * ZONES_PER_ROW + 30;  // Zone Info ���� ����
	int totalHeight = (ZONE_HEIGHT + 1) * ZONES_PER_COL + 15;

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	COORD bufferSize = {
		static_cast<SHORT>(totalWidth + 10),  // ���� ���� �߰�
		static_cast<SHORT>(totalHeight + 30)
	};
	SetConsoleScreenBufferSize(hConsole, bufferSize);

	//totalHeight += 10;
	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);
	MoveWindow(console, r.left, r.top,
		totalWidth * 10,  // ���� �ʺ� �ȼ��� ��ȯ (�� 10�ȼ�)
		totalHeight * 20, // ���� ���̸� �ȼ��� ��ȯ (�� 20�ȼ�)
		TRUE);

	


	// Ŀ�� ������ ����
	CONSOLE_CURSOR_INFO cursorInfo = { 1, FALSE };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

	// �ʱ� ȭ�� ����
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

	// �� ���� �÷��̾�/���� �� ���
	std::map<int, std::pair<int, int>> zoneCounts;  // zoneId -> {players, monsters}
	for (const auto& [objectId, pos] : currentDisplay) {
		int zoneId = getZoneIdFromScreenPos(pos.first, pos.second);
		if (objectId <= g_nZoneCount * g_nZoneUserMax) {
			// �÷��̾�
			zoneCounts[zoneId].first++;
			totalPlayers++;
		}
		else if (pos.second != -1) {  // ��ȿ�� ��ġ�� �ִ� ��츸
			// ����
			zoneCounts[zoneId].second++;
			totalMonsters++;
		}
	}
	// ��ü �÷��̾� �� ���
	gotoxy(startX + 22, startY);  // "== Zone Information ==" ���� ���
	std::cout << "��ü ����: " << totalPlayers;

	if (LSecondTickCount < GetTickCount64())
	{
		LSecondTickCount = GetTickCount64() + Tick::SECOND_TICK;

		gotoxy(startX + 35 , startY);
		cout << "   �ʴ� ��Ŷ ó����:" << g_nPacketCount << endl;

		g_nPacketCount.store(0);
	}
	std::vector<int>Rttlist;
	{
		std::lock_guard<std::mutex> lock(mtx);
		Rttlist.swap(DBRTT);  // ���� �������� lock �ð� �ּ�ȭ
	}
	int nCnt = 0;
	for (auto time : DBRTT)
	{
		if (nCnt == 10)
			break;

		gotoxy(startX + 22, startY+1);

		nCnt++;
		cout << "DB RTT:" << time << endl;

	}
	


	// �� ���� ���
	for (int zoneId = 1; zoneId <= g_nZoneCount; zoneId++) {
		gotoxy(startX, startY + zoneId + 1);
		std::cout << "Zone " << zoneId << ": ";
		auto& counts = zoneCounts[zoneId];
		std::cout << "P(" << counts.first << ") ";
		std::cout << "M(" << counts.second << ")   ";  // �������� ���� �ؽ�Ʈ �����
	}
	

}
