#define WIN32_LEAN_AND_MEAN   // ��ֹ�������ڵ�������
#include <stdio.h>
#include <windows.h>
#include <WinSock2.h>
/*
#pragma comment(lib, "ws2_32.lib")
**/

int main(int argc, char *argv[])
{
	/** ����winsock����������*/
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);


	/** ����socket��İ󶨣� �ͷ�ϵͳ��Դ*/
	WSACleanup();

	printf("hello");
	return 0;
}