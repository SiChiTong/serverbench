#ifndef _PROC_MONITOR_H_
#define _PROC_MONITOR_H_

#include <memory>
#include <list>
#include <vector>
#include <string>
#include "../netcomm/cmqcommu.h"
#include "../comm/cprocmon.h"

using namespace std;

#define MAX_PROC_GROUP_NUM 256

/**
������Ϣ
**/
class CProc
{
public:
	CProc();
	CProc(int procid, time_t timestamp);

	bool operator==(const CProc &r);

	int procid;
	time_t timestamp; //��һ������ʱ���
};

/**
��������Ϣ
**/
class CGroupInfo
{
public:
	CGroupInfo();
	CGroupInfo(int groupid, string groupname, string execfile, unsigned int maxprocnum, unsigned int minprocnum);
	CGroupInfo(const CGroupInfo &right);
	CGroupInfo& operator=(const CGroupInfo &right);

	int    groupid;
	string groupname;
	string execfile;
	unsigned int maxprocnum;
	unsigned int minprocnum;
};

/**
�����Ľ�����
**/
class CProcGroup
{
	friend class CMonitorProcSrv;
public:
	CProcGroup(const CGroupInfo &groupinfo);

	void addproc(CProc &proc);

private:
	CGroupInfo groupinfo;
	list<CProc> proclist;
};

class ProcEvent
{
	friend class CMonitorProcSrv;
public:
	ProcEvent(int eventid, int groupid, int procid);
private:
	int eventid;
	int groupid;
	int procid;
};

/**
���̼����
����ʱ����Ԥ����Ľ�����Ϣ��
ͨ������������,�������Ƿ�ֹͣ������
ͨ�����ʵ�������Ľ�����Ϣ��Ԥ����Ľ�����Ϣ��������ɱ����ؽ��̡�
**/
class CMonitorProcSrv
{
public:
	CMonitorProcSrv();
	void initconf();
	void run();
	void killallproc();

private:
	vector<CProcGroup> procgroup;
	shared_ptr<CMQCommu> mqcomm;
	ProcMonMsg promsg[2]; // 0 -- ��, 1 -- ��
	list<ProcEvent> events;
	
	void doRecv(int msgtype);
	void doCheck();

	CProc& findProc(int groupid, int procid);
	void   addProc(ProcMsg &promsg);
	void   delProc(int groupid, int procid);
	void   showProc();
	void   doEvent();
};

#endif