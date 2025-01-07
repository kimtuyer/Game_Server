#pragma once

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#ifdef _DEBUG
#pragma comment(lib, "ServerCore\\Debug\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib, "ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#endif

#include "CorePch.h"
#include "Enum.pb.h"
#include "Struct.pb.h"
#include "CObject.h"
//#include "ConsoleMapViewer.h"
using GameSessionRef = shared_ptr<class GameSession>;
using ObjectRef = shared_ptr<class CObject>;
using PlayerRef = shared_ptr<class CPlayer>;
using MonsterRef = shared_ptr<class CMonster>;
using CouchbaseRef = shared_ptr<class CouchbaseClient>;
extern array<shared_ptr<ZoneQueue>, Zone::g_nZoneCount + 1> zoneQueues;
extern map<int, bool>	threadRebalance;
