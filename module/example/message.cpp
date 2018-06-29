#include "message.h"

datapackage::datapackage():mData(NULL)
{
	
}

datapackage::~datapackage()
{
	if(mData)
	{
		delete mData;
	}
}

const char * datapackage::getData() const
{
	return mData;
}

/**
������EPOLLIN���յ����ػ���������
����ֵ��
����������ݰ����򷵻�ҵ�����ݰ�����;
���û�����ݰ����򷵻�0;

===>  �� proxy�� bench_handle_input() �е���
�ͻ��˰��� datapackage �ĸ�ʽ����
**/
int PackageProcess::haspackage(CClientData *dataCache)
{
	int pkglen = 0;
	int datalen = 0;
	char *data = dataCache->getReadbleData(datalen);

	if(datalen < sizeof(dataheader))
	{
		return 0;
	}
	else
	{
		dataheader *header = (dataheader *)data;
		if(header->mDataLength > datalen - sizeof(dataheader))
		{
			return 0;
		}
		else
		{
			pkglen = sizeof(dataheader) + header->mDataLength;
			return pkglen;
		}
	}
}


/**
������worker��share mem �ж�ȡ�ͻ����������ݣ���װ��CClientData����
����ֵ��
datapackage ��ָ��

===>  �� worker �� bench_handle_process() �е���
�ͻ��˰��� datapackage �ĸ�ʽ�����������Ҳ���� datapackage �ĸ�ʽ���� 
**/
shared_ptr<datapackage> PackageProcess::getPackage(CClientData *dataCache, int &seq)
{
	// ��ȡ�ͻ�������
	int datalen = 0;
	char *data = dataCache->getReadbleData(datalen);

	seq = *(int *)data;
	data += sizeof(int);
	datalen -= sizeof(int);

	// ���췵�ذ�
	shared_ptr<datapackage> resp(new datapackage());
	resp->header = *(dataheader *)data;
	resp->mData = (char *)malloc(resp->header.mDataLength + 2); // ���Ǻܲ��õ�������ÿ������������malloc����ռ䣬�����ڴ���Ƭ��
	memcpy(resp->mData, data + sizeof(dataheader), resp->header.mDataLength);

	// ����
	return resp;
}