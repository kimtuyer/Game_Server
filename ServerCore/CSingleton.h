#pragma once
static class CSingleton
{
	//template<typename T>
	static CSingleton* GetInstance(void)
	{
		static CSingleton pInstance;
		/*
		  static 객체 반환하는 함수

		  생성 안되어있으면 생성후 반환

		  있으면 그대로 반환
		
		*/

		return &pInstance;

	}

	
};

