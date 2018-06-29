#ifndef _C_MSGQUEUE_COMMU_H_
#define _C_MSGQUEUE_COMMU_H_

/**
System V ��Ϣ���У����ڽ��̼���Ϣͬ����
���ԭ��
���ں��б�����Ϣ�������û�����ָ����Ϣ���ͺ����ݣ���Щ��Ϣ�����õ�Ԥ���ڵ���Ϣ�ṹ�У����뵽��Ϣ�����С�
�û����̿��Ը�����Ϣ���ͣ���ѡ��شӶ����а���FIFOԭ���ȡ�ض�������Ϣ��
��Ϣ�������ݽṹ��
struct msqid_ds {
	struct ipc_perm msg_perm; // ���̶���Ϣ���е�Ȩ��
	struct msg_message *msg_first; //��һ����Ϣ
	struct msg_message *msg_last;  //���һ����Ϣ
	...
};

struct msg_message {
	struct msg *msg; //������Ϣ��
	int m_ts; //������Ϣ��С
	long m_type; //������Ϣ����
	struct msg_message *next;
	...
};

struct msgbuf {
   long mtype;       // message type, must be > 0
   char mtext[1];    // message data 
};
�򿪻򴴽���Ϣ����:
int msgget(key_t key, int msgflag);
* key: һ���� ftok(const char * fname, int id) ���ɡ�
* msgflag�� IPC_CREAT:�����µ���Ϣ���С� IPC_EXCL:��IPC_CREATһͬʹ�ã���ʾ���Ҫ��������Ϣ�����Ѿ����ڣ��򷵻ش��� IPC_NOWAIT:��д��Ϣ����Ҫ���޷�����ʱ����������

��Ϣ���ͣ�
int msgsnd(int msqid, struct msgbuf *msgp, size_t msgsz, int msgflg); 
* msgsz ���Ĵ�Сָ��Ϊ mtext �Ĵ�С��
* msgflg ��ָ��ΪIPC_NOWAITʱ�����ռ���ʱ����-1,errno=EAGAIN

��Ϣ���գ�
ssize_t msgrcv(int msqid, struct msgbuf *msgp, size_t msgsz, long msgtyp, int msgflg);
* msgsz ������Ϊ mtext�Ĵ�С��
* msgtyp �� ����Ϊ0��ʾ���ն����е�һ����Ϣ��>0��ʾ���ն����е�һ��msgtyp���͵���Ϣ��<0��ʾ���ն�����msgtyp������С���Ǹ���Ϣ��
* msgflg ��ָ��ΪIPC_NOWAITʱ�����ռ��ʱ����-1,errno=ENOMSG.
**/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

class CMQCommu
{
public:
	CMQCommu(int id);
	virtual ~CMQCommu();

	static key_t pwdtokey(int id);
	int send(struct msgbuf *msg, int msgsize);
	int recv(struct msgbuf *msg, int msgsize, long msgtype = 0);
private:
	int init(int id);
	void fini();

	int mqid;
};

#endif