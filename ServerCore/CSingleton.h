#pragma once
static class CSingleton
{
	//template<typename T>
	static CSingleton* GetInstance(void)
	{
		static CSingleton pInstance;
		/*
		  static ��ü ��ȯ�ϴ� �Լ�

		  ���� �ȵǾ������� ������ ��ȯ

		  ������ �״�� ��ȯ
		
		*/

		return &pInstance;

	}

	
};

