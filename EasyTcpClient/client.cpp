#define WIN32_LEAN_AND_MEAN   // 防止引入早期的依赖库
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <WinSock2.h>
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
	case CMD_NEW_USER_JOIN: {
		NewUser *userJoin = NULL;
		int nlen = recv(active_sock, msgBuf + sizeof(DataHeader), dheader->dataLength - sizeof(DataHeader), 0);
		if (nlen <= 0) {
			printf("error...... service over\r\n");
			return -1;
		}
		userJoin = (NewUser *)msgBuf;
		printf("Recv server msg: CMD: CMD_NEW_USER_JOIN, size=%d\r\n", userJoin->dataLength);
	} break;
	case CMD_RESULT: {
		Result *result = NULL;
		printf("size : %d\r\n", sizeof(Result));
		int nlen = recv(active_sock, msgBuf + sizeof(DataHeader), dheader->dataLength - sizeof(DataHeader), 0);
		if (nlen <= 0) {
			printf("error...... service over\r\n");
			return -1;
		}
		result = (Result *)msgBuf;
		printf("Recv server msg: CMD: CMD_RESULT, size=%d\r\n", result->dataLength);
	}break;
	case CMD_LOGOUT: {
		Logout *logout = NULL;
		int nlen = recv(active_sock, msgBuf + sizeof(DataHeader), dheader->dataLength - sizeof(DataHeader), 0);
		if (nlen <= 0) {
			printf("error...... service over\r\n");
			return -1;
		}
		logout = (Logout *)msgBuf;
		printf("Recv server msg: CMD: CMD_LOGOUT, size=%d\r\n", logout->dataLength);
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
	if (INVALID_SOCKET == _sock) {
		printf("create socket error!\r\n");
		return 0;
	}
	// connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(6666);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (INVALID_SOCKET == connect(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in))) {
		printf("CONNECT ERROR!\r\n");
	}
	printf("connect success!\r\n");

	DataHeader dh = {};
	while (true) 
	{

		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);

		// 超时时间
		timeval timer = { 1, 0 };
		int ret = select(_sock, &fdReads, NULL, NULL, &timer);
		if (ret < 0) {
			printf("server select is over!\r\n");
			break;
		}
		if (FD_ISSET(_sock, &fdReads)) 
		{
			FD_CLR(_sock, &fdReads);
			if (-1 == processor(_sock)) {
				printf("client is over\r\n");
				break;
			}
		}
		printf("Free time! I can do the other task!\r\n");

		Login login = {};
		strcpy(login.userName, "mjcc");
		strcpy(login.passWord, "123");
		send(_sock, (char *)&login, sizeof(Login), 0);
		// Sleep(1000);
	}

	closesocket(_sock);	

	/** 结束socket库的绑定， 释放系统资源*/
	WSACleanup();

	getchar();
	return 0;
}