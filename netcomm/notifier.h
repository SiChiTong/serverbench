#ifndef _NOTIFIER_H_
#define _NOTIFIER_H_

#include <string>

#include "pollerobject.h"

using namespace std;

class Notifier : public CPollerObject
{
public:
	Notifier(int fd, string _module);
	void inputNotify();
	bool outputNotify();

private:
	string module; // ����proxy��worker�ڹܵ���Ϣ������Ĳ�ͬ����
};

#endif