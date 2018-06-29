#include <sys/types.h>
#include <sys/socket.h>

#include <iostream>
using namespace std;

#include "../utils/log/log.h"
#include "csocket.h"

/**
����������socket���ࣨtcp��udp��unix�����׽��֣�
���ܣ�����socket
����ֵ���ɹ����ش�����socket�����ʧ�ܷ���-1
**/
int CSocket::create(int sock_type) 
{
	errno = 0;
	int fd = 0;

	switch(sock_type)
	{
	case TCP_SOCKET:
		fd = socket(AF_INET, SOCK_STREAM, 0);
		break;
	case UDP_SOCKET:
		fd = socket(AF_INET, SOCK_DGRAM, 0);
		break;
	case UNIX_SOCKET:
		fd = socket(AF_INET, SOCK_STREAM, 0);
		break;
	default:
		break;
	}

	if(fd < 0)
	{
		log_debug("cannot create socket. sock_type=%d, errno=%d", sock_type, errno);
		return -1;
	}

	return fd;
}

/**
���ܣ���ָ��IP�Ͷ˿�
����ֵ���ɹ�����0��ʧ�ܷ���-1
**/
int CSocket::bind(int fd, const string &serv_addr, uint16_t port)
{
	CSocketAddr saddr;
	saddr.setFamily(AF_INET);
	saddr.setPort(port);
	saddr.setIpv4Str(serv_addr);

	errno = 0;
	int ret = ::bind(fd, saddr.getCommAddr(), saddr.getSockLen());
	if(ret < 0)
	{
		log_debug("bind error. errno=%d", errno);
		return -1;
	}

	return 0;
}

/**
���ܣ�������IP�Ͷ˿ڣ�������ʱ���κ������յ��յ����ݶ����Ҵ���
����ֵ���ɹ�����0��ʧ�ܷ���-1
**/
int CSocket::bind(int fd, uint16_t port)
{
	CSocketAddr saddr;
	saddr.setFamily(AF_INET);
	saddr.setPort(port);
	saddr.setIpv4Numeric(htonl(INADDR_ANY));

	errno = 0;
	int ret = ::bind(fd, saddr.getCommAddr(), saddr.getSockLen());
	if(ret < 0)
	{
		log_debug("bind error. errno=%d", errno);
		return -1;
	}

	return 0;
}

/**
���ܣ�����socket
����ֵ���ɹ�����0��ʧ�ܷ���-1
**/
int CSocket::listen(int fd, int backlog)
{
	errno = 0;

	int ret = ::listen(fd, backlog);
	if(ret < 0)
	{
		log_debug("listen error. errno=%d", errno);
		return -1;
	}

	return 0;
}

/**
���ܣ���������
����ֵ���ɹ�����0��ʧ�ܷ���-1
**/
int CSocket::accept(int fd, const string &ip, uint16_t port)
{
	CSocketAddr saddr;
	saddr.setFamily(PF_INET);
	saddr.setPort(port);

	if(ip == "any")
	{
		saddr.setIpv4Numeric(htonl(INADDR_ANY));
	}
	else
	{
		saddr.setIpv4Str(ip);
	}

	errno = 0;

	socklen_t &addlen = saddr.getSockLen();
	int ret = ::accept(fd, saddr.getCommAddr(), &addlen);
	if(ret < 0)
	{
		log_debug("accept error. errno=%d", errno);
		return -1;
	}

	return 0;
}

/**
���ܣ�����socket������
����ֵ���ɹ�����0��ʧ�ܷ���-1
**/
int CSocket::set_nonblock(int fd)
{
	int flag = fcntl(fd, F_GETFL);
	if(flag == -1)
	{
		log_debug("fcntl error. errno=%d", errno);
		return -1;
	}

	if(flag & O_NONBLOCK)
	{
		return 0;
	}

	int ret = fcntl(fd, F_SETFL, flag | O_NONBLOCK);
	if(ret < 0)
	{
		log_debug("fcntl error. errno=%d", errno);
		return -1;
	}

	return 0;
}

/**
���ܣ�����˿�æ����TCP״̬ΪTIME_WAITʱ�������ö˿ڡ�
����ֵ���ɹ�����0��ʧ�ܷ���-1
**/
int CSocket::set_reuseaddr(int fd)
{
	int reuse = 1;
	unsigned int optlen = sizeof(reuse);
	int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, optlen);
	if(ret < 0)
	{
		log_debug("reuseaddr error. errno=%d", errno);
		return -1;
	}

	return 0;
}