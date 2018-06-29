#ifndef _INCOMING_H_
#define _INCOMING_H_

#include <memory>
using namespace std;

#include "../netcomm/pollerunit.h"
#include "../comm/cprocmon.h"
#include "../comm/serverbase.h"

typedef struct{
	int socket_type;
	int port;
}socket_conf;

/**
���ܣ�
�������ӣ�������Ϣ��
��1������epoll
��2������listener
��3����listener����epoll slot
**/

class CIncoming
{
public:
	CIncoming();
	void init();
	int processIncoming();
	void addnotify(int fd);

private:
	CPollerUnit pollerUnit;
};

#endif