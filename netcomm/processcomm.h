/**
����proxy��worker֮���ͨ�š�
proxy����������ݰ���·�����ݰ���worker��
worker���������ݰ�ҵ���߼���
һ��proxy��Ӧ���worker����ÿ��proxy��worker��ͨ��ʹ��һ�鹲���ڴ棬ÿһ�鹲���ڴ�ʵ������produer��consumer���������ڴ�飬����proxy��worker֮���໥�������ݵĻ��档
�����߼�ȫ����װ��ProcessComm���У��ö�����proxy��worker�㴴�������Ե�����ʽ����network����á�
���磺proxy������Ϣ��workerʱ����network��ֱ�ӵ��� ProcessComm::getInstance()->sendto();
**/

#ifndef _PROCESS_COMM_H_
#define _PROCESS_COMM_H_

#include <map>
#include <memory>
using namespace std;

#include "cshmcommu.h"
#include "fifocomm.h"

class ShmProducer
{
public:
	ShmProducer(int shmkey, int shmsize);
	void setNotifyer(int key);
	int produce(const void *data, unsigned int datalen, int seq);
	void sendNotify();
private:
	shared_ptr<CShmMQ> mq;
	shared_ptr<FifoComm> fifo;
	int notify_fd;
};

class ShmConsumer
{
public:
	ShmConsumer(int shmkey, int shmsize);
	void setNotifyer(int key);
	int consume(void *buff, unsigned int buffsize, unsigned int &datalen, int &seq);
	void readNofiy();
	int getNofiyFd();
private:
	shared_ptr<CShmMQ> mq;
	shared_ptr<FifoComm> fifo;
	int notify_fd;
};

class ProcessCommItem
{
public:
	ProcessCommItem();
	virtual ~ProcessCommItem();
	void init(string creator, int groupid, unsigned int shmsize);
	int getNofiyFd();  // ����ǰ�Ļ���fifo
	shared_ptr<ShmProducer> getProducer();
	shared_ptr<ShmConsumer> getConsumer();
private:
	key_t pwdtokey(int id);

public:
	char *consumeBuff;  //��consumer��ȡ��Ϣ��ʱ��ŵ�
	unsigned int comsumeBuffSize;
private:
	shared_ptr<ShmProducer> producer;
	shared_ptr<ShmConsumer> consumer;
	int monitFd; // ����ǰ�Ļ���fifo
};

class ProcessComm
{
public:
	static ProcessComm *instance;
	static ProcessComm *getInstance();
	
	void addRtor(int groupid, shared_ptr<ProcessCommItem> &r);
	void addNtor(int notifyfd, shared_ptr<ProcessCommItem> &r);
	shared_ptr<ProcessCommItem> getRtor(int groupid);
	shared_ptr<ProcessCommItem> getNtor(int notifyid);

private:
	ProcessComm();

private:
	map<int, shared_ptr<ProcessCommItem> > ctor; // ���� groupid - ProcessCommItem �Ķ�Ӧ��ϵ,��proxy��·�ɷַ���Ϣʱʹ�ã�
	map<int, shared_ptr<ProcessCommItem> > ator; // ���� monitFd - ProcessCommItem �Ķ�Ӧ��ϵ,��monitFd��������Ϣ��������ʱ����������������
};

#endif