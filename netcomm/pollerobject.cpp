#include <iostream>
#include <sys/epoll.h>

using namespace std;

#include "pollerobject.h"
#include "../utils/log/log.h"

CPollerObject::CPollerObject() : fd(-1), seq(0), pollunit(NULL)
{}

CPollerObject::~CPollerObject()
{
	log_debug("~CPollerObject()");
}

void CPollerObject::setFd(int _fd)
{
	fd = _fd;
	CSocket::set_nonblock(fd);
}

int CPollerObject::getFd()
{
	return fd;
}

void CPollerObject::setSeq(unsigned int _seq)
{
	seq = _seq;
}

int CPollerObject::getSeq()
{
	return seq;
}

/**
���ܣ�������׼�����������������
**/
void CPollerObject::onConnReady(CPollerUnit &pollerUnit)
{
	pollunit = &pollerUnit;

	if(attachPoller(pollerUnit) == -1)
	{
		log_debug("attach poller failed.");

		string resp = "attach failed.";
		::write(fd, resp.c_str(), resp.size());
		closeFd();
	}
}

/**
���ܣ���������ʧʱ���Ƴ����������ر����ӣ���󽫱�������ͷš�
code tips:
����ĳ�Ա�������ܲ��ܵ���delete this�����ǿ϶���.
����release֮���ܵ��������ķ����������и�ǰ�᣺�����õķ������漰�����������ݳ�Ա���麯����
��Ϊ�����������ڴ�ռ��У�ֻ�����ݳ�Ա���麯����ָ�룬���������������ݣ���ĳ�Ա�����������ڴ�����С���delete this֮����е������κκ������ã�ֻҪ���漰��thisָ������ݣ����ܹ��������С�
**/
void CPollerObject::onConnOver()
{
	closeFd();

	if(pollunit)
	{
		detachPoller(*pollunit);
	}
}

/**
���ܣ���EPOLLIN�¼�����EPOLLETģʽ�£�EPOLLIN����֮��ᱻɾ�����ʴ�������Ҫ����ע�ᡣ
EPOLLETģʽ�£�û�б�Ҫ��ע��EPOLLIN�¼���
**/
void CPollerObject::enableInput()
{
	epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.u32 = seq;
	epoll_ctl(pollunit->epfd, EPOLL_CTL_MOD, fd, &event);
}

/**
���ܣ���EPOLLOUT�¼��������������ݷ��͵��ͻ��ˡ�
EPOLLETģʽ�£�EPOLLOUT����֮��ᱻɾ�������û�����û�з������֮ǰ����Ҫ����ע��EPOLLOUT.
EPOLLETģʽ�£�û�б�Ҫ��ע��EPOLLOUT�¼���
**/
void CPollerObject::enableOutput()
{
	epoll_event event;
	event.events = EPOLLIN | EPOLLOUT | EPOLLET;
	event.data.u32 = seq;
	epoll_ctl(pollunit->epfd, EPOLL_CTL_MOD, fd, &event);
}

/**
���ܣ���EPOLLOUT�¼��������������ݷ��͵��ͻ��ˡ�
EPOLLETģʽ�£�û�б�Ҫ��ע��EPOLLOUT�¼���
**/
void CPollerObject::disableOutput()
{
	epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.u32 = seq;
	epoll_ctl(pollunit->epfd, EPOLL_CTL_MOD, fd, &event);
}

/**
���ܣ����Լ����������
����ֵ��
�ɹ�0��ʧ��-1
**/
int CPollerObject::attachPoller(CPollerUnit &pollerUnit)
{
	shared_ptr<CPollerObject> shared_this = shared_from_this();

	int ret = pollerUnit.addPollerObj(shared_this);

	return ret;
}

/**
���ܣ���������ӹرգ��ͷ����Ӷ���
����ֵ��
�ɹ�0��ʧ��-1
**/
void CPollerObject::detachPoller(CPollerUnit &pollerUnit)
{
	pollerUnit.delPollerObj(seq);
}

void CPollerObject::closeFd()
{
	::close(fd);
}