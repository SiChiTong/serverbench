#include "csocket.h"
#include "listener.h"
#include "client.h"

CListener::CListener(int port) 
{
	newfds.clear();
	init(port);
}

CListener::~CListener() { }

void CListener::inputNotify()
{
	newfds.clear();

	int count = handleAccept();
	if(count > 0)
	{
		for(vector<int>::iterator itor = newfds.begin(); itor != newfds.end(); itor++)
		{
			int newfd = *itor;
			shared_ptr<CPollerObject> clientPoller(new CClient(newfd, TCP_SOCKET));
			clientPoller->onConnReady(*pollunit);
		}
	}
}

bool CListener::outputNotify()
{
	return true;
}

/**
ET��������������
����ֵ�����ν�������������
**/
int CListener::handleAccept()
{
	int count = 0;

	struct sockaddr_in peer;
	socklen_t peersize = sizeof(struct sockaddr_in);
	while(true)
	{
		int newfd = ::accept(fd, (struct sockaddr *)&peer, &peersize);
		if(newfd <= 0)
		{
			if(errno == EINTR) continue;
			else if(errno == EAGAIN) break;
			else
			{
				log_error("accept error. errno=%d", errno);
				break;
			}
		}
		else
		{
			newfds.push_back(newfd);
			count++;
		}
	}

	return count;
}

/**
���û�������С��
1. ���SO_SENDBUFѡ��ֵ��2���������ֵ����ô������Ϊ���ֵ����̼���֪����������ͻ�����ֵΪ��425984
2. ���SO_SNDBUFѡ��ֵ��2��С���׽ӿ�SO_SNDBUF����Сֵ����ôʵ�ʵ�SO_SNDBUF�������ΪSO_SNDBUF����Сֵ����̼���֪��������С���ͻ�����ֵΪ��4608
3. ���SO_SNDBUFѡ��ֵ��2�������׽ӿ�SO_SNDBUF����Сֵ����ôʵ�ʵ�SO_SNDBUF�������ΪSO_SNDBUF��2����
ͬ��
�����������ջ�����ֵΪ��425984
��������С���ջ�����ֵΪ��2304
**/
int CListener::init(int port)
{
	int lfd = CSocket::create(TCP_SOCKET);
	if(lfd > 0)
	{
		CSocket::set_reuseaddr(lfd);

		if(CSocket::bind(lfd, port) == 0)
		{
			if(CSocket::listen(lfd) == 0)
			{
				fd = lfd;
				CSocket::set_nonblock(fd);
				
				return 0;
			}
		}
	}

	return -1;
}