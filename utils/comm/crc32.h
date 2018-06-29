#ifndef _CIPHER_CRC32_H_
#define _CIPHER_CRC32_H_

#include <stdint.h>

namespace platform
{
namespace commlib
{
class CCrc32
{
public:
    CCrc32() : crc(0) {}

    /*
     *@brief ��������crc32��ֵ
     *@ptr �����������
     *@len ����������ݳ���
     *@return  = 0  �ɹ�
               < 0  ʧ��
     */
    int Update(unsigned char *ptr, uint32_t len);
    /*
     *@brief ��Update����������������
     *@return  crc32�Ľ��
     */
    uint32_t Final();

    /*
     *@brief һ���Լ���crc32ֵ
     *@ptr �����������
     *@len ����������ݳ���
     *@return  crc32�Ľ��
     */
    uint32_t Crc32(unsigned char *ptr, uint32_t len);

private:
    uint32_t crc;
};
}
}

#endif /* _CIPHER_CRC32_H_ */
