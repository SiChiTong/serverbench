#ifndef _NOCOPYABLE_H_
#define _NOCOPYABLE_H_

#include <iostream>
using namespace std;

/**
��ֹ���󱻿�������򱻸�ֵ��
�����ڼ䷢�ִ���
**/  

class NoCopyable
{
public:
	NoCopyable() 
	{
		//cout << "NoCopyable() " << endl;
	}
  
	virtual ~NoCopyable()
	{
		//cout << "NoCopyable() " << endl;
	}

private:
	NoCopyable(const NoCopyable &r);

	NoCopyable &operator=(const NoCopyable &r);
};

#endif