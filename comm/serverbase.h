#ifndef _SPP_COMM_SERVERBASE_H_
#define _SPP_COMM_SERVERBASE_H_

#include "cprocmon.h"


#define SIG_FLAG_QUIT 0x01

/**
�̳й�ϵ�ĳ�ʼ��˳��
1. �������ݳ�Ա��ʼ����
2. ���๹�캯����
3. �������ݳ�Ա��ʼ����
4. ���๹�캯����
**/

class CServerBase
{
public:
	CServerBase();
	~CServerBase();
 
	void run();
	virtual void startup();
	virtual void realrun();

	static void sigusr1_handle(int signo);
	static void sigusr2_handle(int signo);
	static bool isquit();
	static int sigflag;

protected:
	CProcMonitorCli procmonCli;
};

#endif