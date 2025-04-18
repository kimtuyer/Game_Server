#pragma once


#define	__SECTOR_UPDATE__ //섹터 단위에서 업데이트
//#define	__NOT_SECTOR_OBJLIST_PLAYER__ //섹터 삽입.삭제리스트에 플레이어는 제외. 
//플레이어는 섹터 변경시 해당 섹터에 직접 삽입,삭제 
#define __CONSOLE_UI__
#define __BROADCAST_DISTANCE__ //거리에 따라 브로드캐스팅범위 조절후 2~300ms -> 150ms이내로 응답시간 감소
#define __COUCHBASE_DB__
#define	__ZONE_THREAD__
//#define __ZONE_THREAD_VER1__	//I/o ,존 스레드 분리, 존 마다 concurrent_queue 사용
//#define __ZONE_THREAD_VER2__	//존 마다 큐 사용X, 각자 할당한 존 업데이트 후,남은시간에 I/O 입출력 같이 처리.
#define __ZONE_THREAD_VER3__	//존 마다 큐 사용X, 각자 할당한 존 업데이트 후,남은시간에 I/O 입출력 같이 처리.
#define __DOP__	//데이터중심 설계
#define	__5000_USER_ZONE__