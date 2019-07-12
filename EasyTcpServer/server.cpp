#define WIN32_LEAN_AND_MEAN   // 防止引入早期的依赖库
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <WinSock2.h>
#include <vector>
using namespace std;
/*
#pragma comment(lib, "ws2_32.lib")
**/

enum CMD
{
	CMD_LOGIN,
	CMD_LOGOUT,
	CMD_ERROR,
	CMD_RESULT,
	CMD_NEW_USER_JOIN
};
enum RESULT
{
	LOGIM_SUCCESS,
	LOGIN_ERROR,
	LOGOUT_SUCESS,
	LOGOUT_ERROR

};
// 注意字节序
struct DataHeader {
	short dataLength;
	short cmd;
};
struct Login : public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};
struct UserInfo
{
	int age;
	char name[32];
};

struct Logout : public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct Result : public DataHeader
{
	Result() {
		dataLength = sizeof(Result);
		cmd = CMD_RESULT;
	}
	int result;
};
struct NewUser : public DataHeader
{
	NewUser() {
		dataLength = sizeof(NewUser);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};

std::vector<SOCKET> g_clients;

// 数据处理
int processor(SOCKET active_sock) {
	char msgBuf[1024] = { };
	int nlen = recv(active_sock, msgBuf, sizeof(DataHeader), 0);
	if (nlen <= 0) {
		printf("error...... service over\r\n");
		return -1;
	}
	DataHeader *dheader = NULL;
	dheader = (DataHeader *)msgBuf;
	printf("recv cmd: %d, data length: %d\r\n", dheader->cmd, dheader->dataLength);
	// if (nlen >= sizeof(DataHeader)) {}
	switch (dheader->cmd) {
	case CMD_LOGIN: {
		Login *login = NULL;
		int nlen = recv(active_sock, msgBuf + sizeof(DataHeader), dheader->dataLength - sizeof(DataHeader), 0);
		if (nlen <= 0) {
			printf("error...... service over\r\n");
			return -1;
		}
		login = (Login *)msgBuf;
		printf("user: %s, passwd: %s; datalength: %d\r\n", login->userName, login->passWord, login->dataLength);
		// todo校验
		Result loginR;
		loginR.result = LOGIM_SUCCESS;
		send(active_sock, (char *)&loginR, sizeof(Result), 0);
	}break;
	case CMD_LOGOUT: {
		Logout *logout = NULL;
		int nlen = recv(active_sock, msgBuf + sizeof(DataHeader), dheader->dataLength - sizeof(DataHeader), 0);
		if (nlen <= 0) {
			printf("error...... service over\r\n");
			return -1;
		}
		logout = (Logout *)msgBuf;
		printf("user: %s; datalength: %d \r\n", logout->userName, logout->dataLength);
		Result loginR;
		loginR.result = LOGIM_SUCCESS;
		send(active_sock, (char *)&loginR, sizeof(Result), 0);

	}break;
	default:
		DataHeader dh = { 0, CMD_ERROR };
		send(active_sock, (char *)&dh, sizeof(DataHeader), 0);
		break;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	/** 启动winsock的启动环境*/
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);


	// 建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// bind
	sockaddr_in _sin = { };
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(6666);  // host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;// inet_addr("127.0.0.1"); // 主机有好几个IP地址， 可以指定ip。
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in))) {
		printf("bind error\r\n");
	}
	// listen
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("listen error\r\n");
	}
	printf("Listening at port 6666\r\n");

	fd_set readAbleSet;
	fd_set writAableSet;
	fd_set fdExp;
	while (true)
	{
		// 伯克利socket UNIX
		// 在window上第一个参数没有意义

		FD_ZERO(&readAbleSet);
		FD_ZERO(&writAableSet);
		FD_ZERO(&fdExp);
		FD_SET(_sock, &readAbleSet);
		FD_SET(_sock, &writAableSet);
		FD_SET(_sock, &fdExp);

		for (int i = (int)(g_clients.size() - 1); i >= 0; i--)
		{
			// 将新加入的客户端加入到关心
			FD_SET(g_clients[i], &readAbleSet);
		}

		// 超时时间
		timeval timer = {0, 0};
		// 在unix 以及类unix nfds 是一个整数，是指fd_set集合中所有描述符的范围， 描述符的最大值
		int retFds = select(_sock + 1, &readAbleSet, &writAableSet, &fdExp, &timer);
		if (retFds < 0) {
			printf("select error!\r\n");
			break; 
		}


		// 有没有可读socket
		if (FD_ISSET(_sock, &readAbleSet)) 
		{
			// 有新客户端来了
			FD_CLR(_sock, &readAbleSet); // 从可读套接字清除
			// accept
			sockaddr_in _client_sin = { };
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _activ_sock = INVALID_SOCKET;
			_activ_sock = accept(_sock, (sockaddr*)&_client_sin, &nAddrLen);
			if (INVALID_SOCKET == _activ_sock) {
				printf("accept error!/r/n"); 
			}
			else 
			{
				for (int i = (int)(g_clients.size() - 1); i >= 0; i--)
				{
					NewUser userJoin;
					userJoin.sock = (int)g_clients[i];
					send(g_clients[i], (const char *)&userJoin, sizeof(NewUser), 0);
				}
				g_clients.push_back(_activ_sock);
				printf("Welcom a new client [IP = %s] coming!\r\n", inet_ntoa(_client_sin.sin_addr));
			}
		}

		// 处理可读套接字
		for (size_t n = 0; n < readAbleSet.fd_count; n++)
		{
			if (-1 == processor(readAbleSet.fd_array[n])) 
			{
				auto iter = find(g_clients.begin(), g_clients.end(), readAbleSet.fd_array[n]);
				if (iter != g_clients.end()) 
				{
					g_clients.erase(iter);
				}
			}
		}

		printf("Free time! I can do the other task!\r\n");

	}
	for (size_t i = g_clients.size() - 1; i >= 0; i--)
	{
		// 清理套接字
		closesocket(g_clients[i]);
	}
	closesocket(_sock);
	
	/** 结束socket库的绑定， 释放系统资源*/
	WSACleanup();

	printf("hello");
	return 0;
}