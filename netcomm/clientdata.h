#ifndef _CLIENT_DATA_H_
#define _CLIENT_DATA_H_

#include <memory>
using namespace std;

#include "../comm/nocopyable.h"

#define CLIENT_DATA_BUFF_SIZE 1024



/**
���ܣ�
����I0�ɶ�ʱд��ͻ������ݣ���ΪӦ�ò�ͻ������ݻ��档
**/
class CClient;
class CClientData
{
public:
	friend class datapackage;
	friend class CClient;

	CClientData();
	virtual ~CClientData();
	int write(char *buff, int size);
/*
	shared_ptr<datapackage> getPackage();
	bool haspackage();
	void fillpackage(shared_ptr<datapackage> package);
*/
	
	void alignPosition();
	void alignPosition(int prolen);
	char *getReadbleData(int &len);
	
public:
	char *mData;
	int  mWritePos;    //д��λ��
	int  mReadPos;     //��ȡλ��
	int  mCapacity;    //�ܹ���С

	int expandeSpace(int newsize);
	int revertSpace();
};

#endif