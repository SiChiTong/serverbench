#include "../utils/log/log.h"
#include "pollerunit.h"

CPollerUnit::CPollerUnit(int pollnum):maxpollers(pollnum)
{
	events = (epoll_event *)calloc(maxpollers, sizeof(epoll_event));
	init();
}

CPollerUnit::~CPollerUnit()
{
	if(events)
	{
		delete events;
	}
}

void CPollerUnit::init()
{
	createEpoll();
}

/**
����epoll
��1��LT��Ĭ��ģʽ���¼�ע�Ტ�Ҵ���֮��ɾ����IN�¼�ֱ�����ջ�����Ϊ�գ�OUT�¼�ֱ�����ͻ�����Ϊ����
��IN�¼������󣬰����ݽ��յ�Ӧ�ò㻺���У����ж��Ƿ�����������
����Ҫ��������ʱ��ע��EPOLLOUT����������Ӧ�����ݷ�����Ϻ󣬷�ע��OUT�¼��������ظ�����Ҫ��OUT�¼�������
��2��ETģʽ�µ��¼�����ʱ����һ����Ӧ�ò�֪ͨ�¼����ڶ���ȡ���¼���ע�ᡣ
��IN�¼���������Ҫ�����н��ջ����������ݽ��գ����ݽ�����Ϻ���Ҫ����ע��IN�¼���
����Ҫ��������ʱ�����Ӧ������û�з�����ȫ����Ҫ����ע��OUT�¼���
**/
void CPollerUnit::createEpoll()
{
	if((epfd = epoll_create(maxpollers)) < 0)
	{
		log_debug("epoll create error. errno=%d", errno);
		exit(0);
	}
}

/**
�ȴ�I/O�¼�����
**/
EpollResult CPollerUnit::waitPollerEvents(int timeout)
{
	errno = 0;

	int pollnum = epoll_wait(epfd, events, maxpollers, timeout);
	if(pollnum < 0)
	{
		if(errno != EINTR)
		{
			log_error("epoll_wait error. errno=%d", errno);
			exit(0);
		}
		pollnum = 0;
	}

	if(pollnum > 0)
	{
		log_debug("proxy waitPollerEvents...pollnum=%d", pollnum);
	}
	
	return EpollResult(events, pollnum);
}

/**
struct epoll_event {
    unsigned long events;
    union {
        void *ptr;
        int fd;
        unsigned long u32;
        unsigned long long u64;
    } data;
};
����epoll_event�¼�
**/
void CPollerUnit::processPollerEvents(EpollResult &result)
{
	//log_debug("proxy processPollerEvents...");

	for(EpollResult::iterator itor = result.begin(); itor != result.end(); itor++)
	{
		epoll_event &event = (epoll_event &)*itor;
		unsigned int seq = event.data.u32;
		log_debug("event.data.u32=%d", seq);
		shared_ptr<CPollerObject> pollerobj = getPollerObj(seq);
		if(pollerobj)
		{
			if(!(event.events & EPOLLOUT) && !(event.events & EPOLLIN))
			{
				pollerobj->onConnOver();
				continue;
			}

			if(event.events & EPOLLOUT)
			{
				if(!pollerobj->outputNotify())
				{
					continue;
				}
			}

			if(event.events & EPOLLIN)
			{
				if(pollerobj != NULL)
				{
				  pollerobj->inputNotify();
				}
			}

		}
	}
	
}

/**
�����ӽ����󣬽������ӣ�����ΪCPollerObject������slot
**/
int CPollerUnit::addPollerObj(shared_ptr<CPollerObject> pollerobj)
{
	if(pollerobj)
	{
		SlotElement<CPollerObject> * slot = pollerSlot.allocNewSlot();
		if(slot)
		{
			epoll_event event;
			event.events = EPOLLIN | EPOLLET;
			event.data.u32 = slot->getSeq();
			log_debug("event.data.u32=%d", event.data.u32);
			epoll_ctl(epfd, EPOLL_CTL_ADD, pollerobj->fd, &event);

			slot->setElement(pollerobj);
			pollerobj->setSeq(slot->getSeq());
			return 0;
		}
	}

	return -1;
}

/**
���ӶϿ��󣬽����ӣ�����ΪCPollerObject���Ƴ�slot
**/
void CPollerUnit::delPollerObj(unsigned int seq)
{
	SlotElement<CPollerObject> * slot = pollerSlot.getSlotElement(seq);
	if(slot)
	{
		shared_ptr<CPollerObject> pollerobj = slot->getElement();

		if(pollerobj)
		{
			epoll_event event;
			event.data.u32 = slot->getSeq();
			epoll_ctl(epfd, EPOLL_CTL_DEL, pollerobj->fd, &event);

			pollerSlot.freeOldSlot(slot);
		}
		else
		{
			log_error("no pollobj find when remove.");
		}
	}
}

/**
��������I/O�¼�����ʱ����ȡ���ӣ�����ΪCPollerObject��
**/
shared_ptr<CPollerObject> CPollerUnit::getPollerObj(unsigned int seq)
{
	SlotElement<CPollerObject> * slot = pollerSlot.getSlotElement(seq);
	if(slot)
	{
		return slot->getElement();
	}

	return NULL;
}