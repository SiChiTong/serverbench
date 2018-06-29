#include <string.h>
#include <iostream>
using namespace std;

#include "../utils/log/log.h"
#include "cshmcommu.h"

#define C_SAFE_AREA_VAL  0x58505053 //"SPPX"
#define C_SAFE_AREA_LEN  4

CShmMQ::CShmMQ(): shmkey(0), shmsize(0), shmid(0), shmaddr(NULL), head(NULL), tail(NULL), block(NULL), blocksize(0)
{
}

CShmMQ::~CShmMQ()
{
	destroyShm();
}

int CShmMQ::createShm(int _shmkey, int _shmsize)
{
	//destroyShm();

	shmkey = _shmkey;
	shmsize = _shmsize;
	
	// ���shmkeyָ��Ĺ����ڴ��Ѿ�����
	if((shmid = shmget(shmkey, shmsize, 0)) != -1)
	{
		shmaddr = shmat(shmid, NULL, 0);
		if(shmaddr == NULL)
		{
			return -1;
		}
	}
	else// ���shmkeyָ��Ĺ����ڴ治����
	{
		shmid = shmget(shmkey, shmsize, IPC_CREAT | 0666);
		if(shmid != -1)
		{
			shmaddr = shmat(shmid, NULL, 0);
			if(shmaddr == NULL)
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}

	char *tmp = (char *)shmaddr;
	pstat = (ShmState *)tmp;
	pstat->currMsgNum = (unsigned int)0;
	pstat->historyMsgNum = (unsigned int)0;

	tmp += sizeof(ShmState);
	head = (unsigned int *)tmp;
	tmp += sizeof(unsigned int);
	tail = (unsigned int *)tmp;
	tmp += sizeof(unsigned int);

	// ��ʼ������
	*head = *tail = 0;
	block = (char *)tmp;
	blocksize = shmsize - sizeof(ShmState) - 2*sizeof(unsigned int *);
}

void CShmMQ::destroyShm()
{
	if(shmaddr)
	{
		shmdt((const void *)shmaddr);
	}
}

CSHM_Q_RESULT CShmMQ::enqueue(const void *data, unsigned int datalen, int seq)
{
	unsigned int _head = *head;
	unsigned int _tail = *tail;

	unsigned int freespace = (_head >= _tail) ? (blocksize - (_head - _tail) - 1) : (_tail - _head - 1); // ����ʣ��ռ��С
	unsigned int leftlinespace = (_head >= _tail) ? ((_tail>0)?(blocksize-_head):(blocksize-_head-1)) : (_tail - _head - 1); // ʣ��������ռ��С
	unsigned int totallen = C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + C_SAFE_AREA_LEN; // ������Ϣ��С

	char header[C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen)] = {0};
	*(unsigned int *)header = C_SAFE_AREA_VAL;
	memcpy(header + C_SAFE_AREA_LEN, &seq, sizeof(seq));
	memcpy(header + C_SAFE_AREA_LEN + sizeof(seq), &datalen, sizeof(datalen));

	// case1: û���㹻�Ŀռ�����Ϣʱ
	if ( totallen > freespace )
	{
		return CSHM_ENQUEUE_NOSPACE;
	}
	// case2: ʣ�������ռ��㹻�����Ϣ
	else if(leftlinespace >= totallen)
	{
		// append header
		memcpy(block + _head, header, C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen));
		// append data
		memcpy(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen), data, datalen);
		// append tailer
		*(unsigned int *)(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen) = C_SAFE_AREA_VAL;
	}
	// case3: ʣ�������ռ��㹻���header
	else if(leftlinespace >= C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen))
	{
		// case3.1: ʣ�������ռ�ֻ�ܴ�Ų���ĩβ��SPPX
		if(leftlinespace >= C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen)
		{
			// append header
			memcpy(block + _head, header, C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen));
			// append data
			memcpy(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen), data, datalen);
			// append tailer
			unsigned int secondlen = totallen - leftlinespace;
			switch(secondlen)
			{
				case 4:
				{
					*(unsigned int *)block = C_SAFE_AREA_VAL;
					break;
				}
				case 3:
				{
					*(char *)(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen) = 'S';
					*(char *)block = 'P';
					*(char *)(block + 1) = 'P';
					*(char *)(block + 2) = 'X';
					break;
				}
				case 2:
				{
					*(char *)(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen) = 'S';
					*(char *)(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + 1) = 'P';
					*(char *)block = 'P';
					*(char *)(block + 1) = 'X';
					break;
				}
				case 1:
				{
					*(char *)(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen) = 'S';
					*(char *)(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + 1) = 'P';
					*(char *)(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + 2) = 'P';
					*(char *)block = 'X';
					break;
				}
				case 0:
				{
					*(char *)(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen) = 'S';
					*(char *)(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + 1) = 'P';
					*(char *)(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + 2) = 'P';
					*(char *)(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + 3) = 'X';
					break;
				}
				default:
				{
					log_debug("shmmsq error!!");
					return CSHM_ENQUEUE_SPACEERR;
				}
			}
		}
		// case3.2: ʣ�������ռ䲻���Դ�Ų���ĩβ��SPPX
		else
		{
			// append header
			memcpy(block + _head, header, C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen));
			// append data
			unsigned int firstlen = leftlinespace - (C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen)); //���ж����ֽڿ���д�뵽leftlinespace
			memcpy(block + _head + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen), data, firstlen);
			memcpy(block, data + firstlen, datalen - firstlen);
			// append tailer
			*(unsigned int *)(block + datalen - firstlen) = C_SAFE_AREA_VAL;
		}
	}
	// case4: ʣ�������ռ䲻���Դ��header
	else
	{
		unsigned int firstlen = leftlinespace;  //���ж����ֽڿ���д�뵽leftlinespace
		// append header
		memcpy(block + _head, header, firstlen);
		memcpy(block, header + firstlen, C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) - firstlen);
		
		// append data
		memcpy(block + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) - firstlen, data, datalen);
		// append tailer
		*(unsigned int *)(block + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) - firstlen + datalen) = C_SAFE_AREA_VAL;
	}

	// �޸�дָ��
	_head = (_head + totallen)%blocksize;
	*head = _head;

	// ���ݰ�����
	pstat->currMsgNum++;

	return CSHM_ENQUEUE_SUCCESS;
}

CSHM_Q_RESULT CShmMQ::dequeue(void *buff, unsigned int buffsize, unsigned int &datalen, int &seq)
{
	unsigned int _head = *head;
	unsigned int _tail = *tail;

	unsigned int readblespace = (_head >= _tail)?(_head - _tail):(blocksize - _tail + _head);// δ�����ܿռ��С
	unsigned int leftlinespace = (_head >= _tail)?(_head - _tail):(blocksize - _tail); // δ���������ռ��С

	char header[C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen)] = {0};

	if(_head == _tail)
	{
		return CSHM_DEQUEUE_EMPTY;
	}

	unsigned int totallen = 0;
	
	// case1: δ���ܿռ䲻�㹻���header
	if(readblespace < C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen))
	{
		log_debug("shmmsq error!!1.1");
		return CSHM_DEQUEUE_SPACEERR;
	}

	// case1: δ�������ռ䲻�㹻���header
	if(leftlinespace < C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen))
	{
		unsigned int firstlen = leftlinespace;
		memcpy(header, block + _tail, firstlen);
		memcpy(header + firstlen, block, C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) - firstlen);

		// check first SPPX
		if(*(unsigned int *)header != C_SAFE_AREA_VAL)
		{
			log_debug("shmmsq error!!2, header=%s", header);
			return CSHM_DEQUEUE_BADSPPX_1;
		}

		seq = *(unsigned int *)(header + C_SAFE_AREA_LEN);
		datalen = *(unsigned int *)(header + C_SAFE_AREA_LEN + sizeof(seq));
		totallen = C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + C_SAFE_AREA_LEN;
		if(readblespace < totallen)
		{
			log_debug("shmmsq error!!3");
			return CSHM_DEQUEUE_SPACEERR;
		}

		if(buffsize < datalen)
		{
			log_debug("shmmsq error!!4");
			return CSHM_DEQUEUE_SPACEERR;
		}
		
		memcpy(buff, block + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) - firstlen, datalen);

		// check last SPPX
		if(*(unsigned int *)(block + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) - firstlen + datalen) != C_SAFE_AREA_VAL)
		{
			log_debug("shmmsq error!!5");
			return CSHM_DEQUEUE_BADSPPX_2;
		}
	}
	// case2: δ�������ռ��㹻���header
	else
	{
		memcpy(header, block + _tail, C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen));

		// check first SPPX
		if(*(unsigned int *)header != C_SAFE_AREA_VAL)
		{
			log_debug("shmmsq error!!6");
			return CSHM_DEQUEUE_BADSPPX_1;
		}

		seq = *(unsigned int *)(header + C_SAFE_AREA_LEN);
		datalen = *(unsigned int *)(header + C_SAFE_AREA_LEN + sizeof(seq));
		totallen = C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + C_SAFE_AREA_LEN;

		if(readblespace < totallen)
		{
			log_debug("shmmsq error!!7");
			return CSHM_DEQUEUE_SPACEERR;
		}

		if(buffsize < datalen)
		{
			log_debug("shmmsq error!!8");
			return CSHM_DEQUEUE_SPACEERR;
		}

		// case2.1: ʣ������δ���ռ��㹻���һ����Ϣ
		if(leftlinespace >= totallen)
		{
			// copy data
			memcpy(buff, block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen), datalen);
			
			// check last SPPX
			if(*(unsigned int *)(block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen) != C_SAFE_AREA_VAL)
			{
				log_debug("shmmsq error!!9");
				return CSHM_DEQUEUE_BADSPPX_2;
			}
		}
		// case2.1: ʣ������δ���ռ�ֻ�ܴ�Ų���ĩβ��SPPX
		else if(leftlinespace >= C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen)
		{
			memcpy(buff, block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen), datalen);
			unsigned int secondlen = totallen - leftlinespace;
			char sppx[4] = {0};
			switch(secondlen)
			{
				case 4:
				{
					memcpy(sppx, block, 4);
					break;
				}
				case 3:
				{
					sppx[0] = *(char *)(block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen);
					sppx[1] = *(char *)block;
					sppx[2] = *(char *)(block + 1);
					sppx[3] = *(char *)(block + 2);
					break;
				}
				case 2:
				{
					sppx[0] = *(char *)(block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen);
					sppx[1] = *(char *)(block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + 1);
					sppx[2] = *(char *)block;
					sppx[3] = *(char *)(block + 1);
					break;
				}
				case 1:
				{
					sppx[0] = *(char *)(block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen);
					sppx[1] = *(char *)(block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + 1);
					sppx[2] = *(char *)(block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + 2);
					sppx[3] = *(char *)block;
					break;
				}
				case 0:
				{
					sppx[0] = *(char *)(block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen);
					sppx[1] = *(char *)(block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + 1);
					sppx[2] = *(char *)(block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + 2);
					sppx[3] = *(char *)(block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen) + datalen + 3);
					break;
				}
				default:
				{
					log_debug("shmmsq error!!10");
					return CSHM_DEQUEUE_SPACEERR;
				}
			}

			if(*(unsigned int *)sppx != C_SAFE_AREA_VAL)
			{
				log_debug("shmmsq error!!11");
				printBlock();
				exit(0);
				return CSHM_DEQUEUE_BADSPPX_2;
			}
		}
		// case2.1: ʣ������δ���ռ䲻���Դ�Ų���ĩβ��SPPX
		else
		{
			unsigned int firstlen = leftlinespace - (C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen));
			unsigned int secondlen =  datalen - firstlen;
			memcpy(buff, block + _tail + C_SAFE_AREA_LEN + sizeof(seq) + sizeof(datalen), firstlen);
			memcpy(buff + firstlen, block, secondlen);

			if(*(unsigned int *)(block + secondlen) != C_SAFE_AREA_VAL)
			{
				log_debug("shmmsq error!!11");
				return CSHM_DEQUEUE_BADSPPX_2;
			}
		}
	}

	// �޸�дָ��
	_tail = (_tail + totallen)%blocksize;
	*tail = _tail;

	// ���ݰ�����
	pstat->currMsgNum--;
	pstat->historyMsgNum++;

	return CSHM_DEQUEUE_SUCCESS;
}

void CShmMQ::printBlock()
{
	char c;

	printf("=================\n");
	for(int i=0; i<blocksize; i++)
	{
		c = *(block + i);
		printf("%d-%c,", i+1,c);
	}
	printf("=================\n");
}