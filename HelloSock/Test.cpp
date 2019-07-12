#define WIN32_LEAN_AND_MEAN   // 防止引入早期的依赖库
#include <stdio.h>
#include <windows.h>
#include <WinSock2.h>
/*
#pragma comment(lib, "ws2_32.lib")
**/

int main(int argc, char *argv[])
{
	/** 启动winsock的启动环境*/
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);


	/** 结束socket库的绑定， 释放系统资源*/
	WSACleanup();

	printf("hello");
	return 0;
}