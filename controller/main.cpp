#include <unistd.h>

#include "defaultctl.h"

int main()
{
	//������̨����
	if(daemon(1,1))
	{
		perror("daemon");
		return -1;
	}

	CDdefaultControl control;
	control.run();
}