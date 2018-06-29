#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include <algorithm>

#include "Utils.h"

using namespace std;

int Utils::mypopen(char *cmd, char type)
{
	int pipefd[2];           //�ܵ�������
	int pid_t;               //����������

	if(type !='r' && type != 'w')
	{
		printf("myopen() flag error/n");
		return NULL;
	}

	if(pipe(pipefd)<0)        //�����ܵ�
	{
		printf("myopen() pipe create error/n");
		return NULL;
	}
	
	pid_t=fork();             //�����ӽ���

	if(pid_t < 0) 
		return NULL;

	if(0 == pid_t)            //�ӽ�����......
	{
		if(type == 'r')
		{
			close(pipefd[0]);               //�˺��������ļ�ָ�������ڶ�����ˣ����ӽ�����Ӧ�ý����д��ܵ������������̷��ص��ļ�ָ����ܶ����������ｫ���˹ر�
			dup2(pipefd[1],STDOUT_FILENO);  //exec������ִ�н������ͨ����׼���д������̨�ϣ����������ǲ���Ҫ�ڿ���̨�����������Ҫ��������أ���˽���׼����ض��򵽹ܵ�д��
			close(pipefd[1]);		

		}
		else{
			close(pipefd[1]);
			dup2(pipefd[0],STDIN_FILENO);
			close(pipefd[0]);
		}
		char *argv[] = {cmd,NULL};	
		if(execl("/bin/bash", "/bin/bash", "-c", cmd, NULL)<0)          //��exec�庯��ִ������
			return NULL;
	}
	
	wait(0);                                //�ȴ��ӽ��̷���

	if(type=='r'){
		close(pipefd[1]);
		return pipefd[0];	//���ڳ�����Ҫ���صĲ������ļ�ָ�룬�����Ҫ��fdopen�������������򿪣��䷵��ֵΪ��Ӧ���ļ�ָ��	
	}else{
		close(pipefd[0]);
		return pipefd[1];
	}
}

void Utils::delrn(char *str)
{
	if(str != NULL)
	{
		int len = strlen(str);
		int r_index = 0;
		int n_index = 0;
		int index;

		r_index = (str[len - 1] == '\r')?len -1:0;
		n_index = (str[len - 1] == '\n')?len -1:0;

		if(r_index == 0 && len >= 2) 
		{
			r_index = (str[len - 2] == '\r')?len -2:0;
		}

		if(n_index == 0 && len >= 2) 
		{
			n_index = (str[len - 2] == '\n')?len -2:0;
		}

		if(r_index != 0 && n_index != 0)
		{
			index = min(r_index, n_index);
		}
		else if(r_index == 0 && n_index != 0)
		{
			index = n_index;
		}
		else if(r_index != 0 && n_index == 0)
		{
			index = r_index;
		}
		else
		{
			index = 0;
		}

		if(index != 0)
		{
			str[index] = '\0';
		}
	}
}