#include <libgen.h>

#include "../utils/log/log.h"
#include "../utils/comm/crc32.h"
#include "processcomm.h"

using namespace platform::commlib;

ShmProducer::ShmProducer(int shmkey, int shmsize)
{
	mq = shared_ptr<CShmMQ>(new CShmMQ());
	mq->createShm(shmkey, shmsize);
}

void ShmProducer::setNotifyer(int key)
{
	fifo = shared_ptr<FifoComm>(new FifoComm(key));
	fifo->createFifo();
	notify_fd = fifo->getFd();
}

/*
�ɹ����뷵��0�� ʧ�ܷ���-1
*/
int ShmProducer::produce(const void *data, unsigned int datalen, int seq)
{
	CSHM_Q_RESULT ret = mq->enqueue(data, datalen, seq);

	if(ret == CSHM_ENQUEUE_SUCCESS)
	{
		return 0;
	}

	return -1;
}

void ShmProducer::sendNotify()
{
	if(notify_fd > 0)
	{
		if(write(notify_fd, "!", 1) < 0)
		{
			log_debug("producer send notify error.");
		}
	}
}

ShmConsumer::ShmConsumer(int shmkey, int shmsize)
{
	mq = shared_ptr<CShmMQ>(new CShmMQ());
	mq->createShm(shmkey, shmsize);
}

void ShmConsumer::setNotifyer(int key)
{
	fifo = shared_ptr<FifoComm>(new FifoComm(key));
	fifo->createFifo();
	notify_fd = fifo->getFd();
}

int ShmConsumer::getNofiyFd()
{
	return notify_fd;
}

/*
�ɹ�ȡ������0�� ʧ�ܷ���-1
*/
int ShmConsumer::consume(void *buff, unsigned int buffsize, unsigned int &datalen, int &seq)
{
	CSHM_Q_RESULT ret = mq->dequeue(buff, buffsize, datalen, seq);

	if(ret == CSHM_DEQUEUE_SUCCESS)
	{
		return 0;
	}

	return -1;
}

void ShmConsumer::readNofiy()
{
	if(notify_fd > 0)
	{
		int ret;
		char c;
		while((ret=read(notify_fd, &c, 1)) > 0);
	}
}



ProcessCommItem::ProcessCommItem()
{
	monitFd = -1;
}

ProcessCommItem::~ProcessCommItem()
{
	if(consumeBuff)
	{
		free(consumeBuff);
	}
}

void ProcessCommItem::init(string creator, int groupid, unsigned int shmsize)
{
	if(creator == "proxy")
	{
		int shmkey_producer = pwdtokey(groupid * 2);
		int shmkey_consumer = pwdtokey(groupid * 2 + 1);
		producer = shared_ptr<ShmProducer>(new ShmProducer(shmkey_producer, shmsize));
		producer->setNotifyer(groupid * 2);
		consumer = shared_ptr<ShmConsumer>(new ShmConsumer(shmkey_consumer, shmsize));
		consumer->setNotifyer(groupid * 2 + 1);
	}

	if(creator == "worker")
	{
		int shmkey_producer = pwdtokey(groupid * 2 + 1);
		int shmkey_consumer = pwdtokey(groupid * 2);
		producer = shared_ptr<ShmProducer>(new ShmProducer(shmkey_producer, shmsize));
		producer->setNotifyer(groupid * 2 + 1);
		consumer = shared_ptr<ShmConsumer>(new ShmConsumer(shmkey_consumer, shmsize));
		consumer->setNotifyer(groupid * 2);
	}

	monitFd = consumer->getNofiyFd();

	comsumeBuffSize = shmsize;
	consumeBuff = (char *)malloc(comsumeBuffSize);
}

int ProcessCommItem::getNofiyFd()
{
	return monitFd;
}

/**
��ϵ�ǰ����ִ��Ŀ¼��idֵ�������crc32 code����Ϊ���̼�ͨ�ŵ���Ϣ����key��
��֤һ������������õ�ǰΨһ��key_t��Դ��
**/
key_t ProcessCommItem::pwdtokey(int id)
{
	char seed[256];
	readlink("/proc/self/exe", seed, 256);
	dirname(seed);

	int seed_len = strlen(seed) + sizeof(id);
	memcpy(seed + strlen(seed), &id, sizeof(id));

	CCrc32 generator;
	return generator.Crc32((unsigned char *)seed, seed_len);
}

shared_ptr<ShmProducer> ProcessCommItem::getProducer()
{
	return producer;
}

shared_ptr<ShmConsumer> ProcessCommItem::getConsumer()
{
	return consumer;
}

ProcessComm *ProcessComm::instance = NULL;

ProcessComm *ProcessComm::getInstance()
{
	if(instance == NULL)
	{
		instance = new ProcessComm();
	}

	return instance;
}

ProcessComm::ProcessComm()
{
	ctor.clear();
	ator.clear();
}

void ProcessComm::addRtor(int groupid, shared_ptr<ProcessCommItem> &r)
{
	ctor[groupid] = r;
}

void ProcessComm::addNtor(int notifyfd, shared_ptr<ProcessCommItem> &r)
{
	ator[notifyfd] = r;
}

shared_ptr<ProcessCommItem> ProcessComm::getRtor(int groupid)
{
	return ctor[groupid];
}

shared_ptr<ProcessCommItem> ProcessComm::getNtor(int notifyid)
{
	return ator[notifyid];
}