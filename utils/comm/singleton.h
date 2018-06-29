#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include <stdlib.h>

using namespace std;

/**
����ָ���ǽ����ڴ�������Ҫ���ߡ����Ǿ�������ָ�����ʵ��ָ����з�װ���Ѵﵽ�ͷŶ��ڴ�����á�
https://blog.csdn.net/lanchunhui/article/details/50878505
**/
template <class T>
class Singleton
{
public:
	static shared_ptr<T> getInstance()
	{
		if(instance == NULL)
		{
			instance = shared_ptr<T>(new T());
		}

		return instance;
	}

private:
	static shared_ptr<T> instance;
};

template <class T>
shared_ptr<T> Singleton<T>::instance = NULL;
#endif