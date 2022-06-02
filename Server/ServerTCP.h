#define _WINSOCK_DEPRECATED_NO_WARNINGS        // turns of deprecated warnings for winsock
#define _CRT_SECURE_NO_WARNINGS
#define MAXCLIENTS 3
#define BUFFERSIZE 1024

#pragma once
#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "Helper.h"
#include "Definitions.h"

#pragma comment(lib,"Ws2_32.lib")

struct ClientRegister {
	SOCKET socket;
	const char* username;
	ClientRegister(SOCKET _socket, const char* _username) {
		socket = _socket;
		username = _username;
	}
};

class ServerTCP
{

private:
	uint16_t port = 0;
	char* buffer = nullptr;
	const char* logMessage = nullptr;
	hostent* host;
	char* ip;
	std::string broadcastMsg;
	int addrLength = sizeof(serverAddr);
	u_long value = 1;
	char broadVal = '1';
	std::ostringstream oss;
	fd_set masterSet, readSet;
	timeval timeVal;
	SOCKET listenSocket;
	SOCKET broadcastSocket;
	SOCKET commSocket;
	sockaddr_in serverAddr, clientAddr;
	ClientRegister* clients[MAXCLIENTS];
public:
	void savePortNumber(uint16_t _port) { port = _port; }
	int runServer();
	int setupUDP();
	int monitorActivity();
	void sendBroadcastMsg();
	int newConnection();
	int determineActivity(SOCKET s, char* username);
	int sendFileLog(SOCKET s);
	int readMessage(SOCKET s, char* buffer, int32_t size);
	int sendMessage(SOCKET s, char* data, int32_t length);
	int tcp_send_whole(SOCKET s, const char* data, uint16_t length);
	int tcp_recv_whole(SOCKET s, char* buf, int len);
	void stop();
	void logEvent(const char* event, int type);
	long long fileSize(const char* _fileName);
};
