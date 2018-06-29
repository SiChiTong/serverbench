#include "msg.pb.h"
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char *argv[]){
    demo::msg msg_test;
    msg_test.set_msgtype(1);
    msg_test.set_msginfo("I am hahaya");
    msg_test.set_msgfrom("127.0.0.1");

    //����Ϣ��ʽ�����ַ����У�д������
    string in_data;
    msg_test.SerializeToString(&in_data);
    cout << "format:" << in_data << endl;

    //����Ϣ���ַ����з���ʽ����������������
    demo::msg msg_encoding;
    msg_encoding.ParseFromString(in_data);
    cout << "msg type:" << msg_encoding.msgtype() << endl;
    cout << "msg info:" << msg_encoding.msginfo() << endl;
    cout << "msg from:" << msg_encoding.msgfrom() << endl;
    return 0;
}