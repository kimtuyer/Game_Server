#pragma once


struct Postion
{
    
    float x = 0;
    float y = 0;
    float z = 0;
    

};

namespace Sector
{
	struct ObjectInfo
	{
		int nSectorID=0;
		int nObjectID=0;
		int nObjectType=0;
		Postion vPos;

	};


}

//struct D3DVECTOR
//{
//    
//    float x = 0;
//    float y = 0;
//    float z = 0;
//    
//
//};