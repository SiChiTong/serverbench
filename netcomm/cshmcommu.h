#ifndef _CSHM_COMMU_H_
#define _CSHM_COMMU_H_

#include <sys/ipc.h>
#include <sys/shm.h>
#include <atomic>
using namespace std;

class ShmState
{
public:
	atomic<unsigned int> currMsgNum; //��ǰ��Ϣ����
	atomic<unsigned int> historyMsgNum; //�Ѵ�����Ϣ����

	// ����atomicģ�治֧�ֿ������캯���������������ʽ���忽�����캯����������ʽ�������캯���е���atomic�Ŀ������죬���±��뱨��
	ShmState(const ShmState &r):currMsgNum(0),historyMsgNum(0)
	{
	}

	// ����atomicģ�治֧�ֿ������캯���������������ʽ���帳ֵ������������ʽ��ֵ�����е���atomic�Ŀ������죬���±��뱨��
	ShmState& operator=(const ShmState& r)
	{
		currMsgNum = r.currMsgNum.load();
		historyMsgNum = r.currMsgNum.load();
	}
};

typedef enum
{
	CSHM_ENQUEUE_SUCCESS = 0,
	CSHM_ENQUEUE_NOSPACE,
	CSHM_ENQUEUE_SPACEERR,
	
	CSHM_DEQUEUE_SUCCESS,
	CSHM_DEQUEUE_EMPTY,
	CSHM_DEQUEUE_SPACEERR,
	CSHM_DEQUEUE_BADSPPX_1,
	CSHM_DEQUEUE_BADSPPX_2
}CSHM_Q_RESULT;

/**
ʹ�ù����ڴ湹��ѭ����Ϣ���С�
�����ڴ�ṹ��[״̬��Ϣ|head|tail|ѭ����Ϣ�ռ�]
״̬��Ϣ�ṹ��[��ǰ��Ϣ����|�Ѵ�����Ϣ����]
��Ϣ�ṹ��[SPPX|seq|total_len|body|SPPX] , ����SPPXΪħ����������Ϣ������ʱ�����SPPX���ж���Ϣ�������ԡ�
**/
class CShmMQ
{
public:
	CShmMQ();
	virtual ~CShmMQ();
	int createShm(int shmkey, int shmsize);
	void destroyShm();
	CSHM_Q_RESULT enqueue(const void *data, unsigned int datalen, int seq);
	CSHM_Q_RESULT dequeue(void *buff, unsigned int buffsize, unsigned int &datalen, int &seq);

private:
	int shmkey;
	int shmsize; 
	int shmid;
	void *shmaddr; //�����ڴ���ʼ��ַ

	ShmState *pstat;
	atomic<unsigned int *> head;//дָ��
	atomic<unsigned int *> tail;//��ָ��
	char *block;   // ��Ϣ��ſռ���ʼ��ַ
	unsigned int blocksize; // ��Ϣ�ռ��С

	void printBlock();
};

#endif