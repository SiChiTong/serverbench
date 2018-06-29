#ifndef _SOCKET_H_
#define _SOCKET_H_

#define TCP_SOCKET 0x1
#define UDP_SOCKET 0x2
#define UNIX_SOCKET 0x3

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "../utils/log/log.h"
 

/**
�๦�ܣ�
	���PF_INETЭ�����װ�󶨵�ַ��
	struct socketaddr_in
	{
		unsigned short int sin_family;
		uint16_t sin_port;
		struct in_addr sin_addr;
		unsigned char sin_zero[8];
	};
	struct in_addr
	{
		uint32_t s_addr;
	};
	sin_family ��Ϊsa_family
	sin_port Ϊʹ�õ�port���
	sin_addr.s_addr ΪIP ��ַ
	sin_zero δʹ�á�
**/
class CSocketAddr
{
public:
	CSocketAddr():_len(sizeof(struct sockaddr_in))
	{
		memset(&_addr, 0, _len);
	}

	struct sockaddr * getCommAddr()
	{
		return (struct sockaddr *)&_addr;
	}

	struct sockaddr_in * getNetAddr()
	{
		return &_addr;
	}

	socklen_t & getSockLen()
	{
		return _len;
	}

	unsigned short int getFamily()
	{
		return _addr.sin_family;
	}

	void setFamily(unsigned short int family)
	{
		_addr.sin_family = family;
	}

	uint16_t getPort()
	{
		return _addr.sin_port;
	}

	void setPort(uint16_t port)
	{
		_addr.sin_port = htons(port);
	}

	uint32_t getIpv4Numeric()
	{
		return _addr.sin_addr.s_addr;
	}

	void setIpv4Numeric(uint32_t ip)
	{
		_addr.sin_addr.s_addr = ip;
	}

	void setIpv4Str(const string &ip)
	{
		uint32_t inaddr;
		int ret = ipToNumeric(ip, inaddr);
		if(ret < 0)
		{
			exit(0);
		}

		_addr.sin_addr.s_addr = inaddr;
	}
	
	//��ַת��
	string ipToString(uint32_t ip)
	{
		char buff[20];
		const char *p = inet_ntop(PF_INET, &ip, buff, sizeof(buff));
		
		if(p == NULL)
		{
			log_error("buff overflow. errno=%d", errno);
			return string();
		}
		else
		{
			return string(p);
		}
	}

	int ipToNumeric(const string &ip, uint32_t &in_addr)
	{
		int ret = inet_pton(PF_INET, ip.c_str(), &in_addr);
		
		if(ret < 0)
		{
			log_error("errno=%d", errno);
			return ret;
		}
		else if(ret == 0)
		{
			log_error("wrong af or ip format ,errno=%d", errno);
			return -1;
		}
		else 
		{
			return 0;
		}
	}

	/**
	���Ժ�����
	const��������ֵĬ����const���ԣ���˷���ֵ���붨���const��
	const��Ա�����У�Ҳ��thisĬ��ת������const���͡�
	const CSocketAddr *obj = new CSocketAddr();
	**/
	const socklen_t & geta() const
	{
		return _len; 
	}
private:
	struct sockaddr_in _addr;
	socklen_t _len;
};
 
class CSocket
{
public:
	static int create(int sock_type = TCP_SOCKET);
	static int bind(int fd, const string &serv_addr, uint16_t port);
	static int bind(int fd, uint16_t port);
	static int listen(int fd, int backlog = 1024);
	static int accept(int fd, const string &ip, uint16_t port);

	static int set_nonblock(int fd);
	static int set_reuseaddr(int fd); 
};

#endif