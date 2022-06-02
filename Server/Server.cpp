#define _CRT_SECURE_NO_WARNINGS                 // turns of deprecated warnings
#define _WINSOCK_DEPRECATED_NO_WARNINGS         // turns of deprecated warnings for winsock
#include <winsock2.h>
#include <iostream>
#include "Helper.h"
#include "ServerTCP.h"
//#include <ws2tcpip.h>                         // only need if you use inet_pton
#pragma comment(lib,"Ws2_32.lib")

uint16_t EnterPort();

int main() {
	WSADATA wsadata;
	int wsa = WSAStartup(WINSOCK_VERSION, &wsadata);
	int serverRunning = 0;
	char* buffer = new char[1024];
	uint16_t port = 0;
	ServerTCP* server = new ServerTCP();
	bool serverActive = true;

	port = EnterPort();
	server->savePortNumber(port);
	server->runServer();
	
	while (true) {
		serverRunning = server->monitorActivity();
	}

	server->stop();
	return WSACleanup();
}

uint16_t EnterPort() {
	uint16_t temp = 0;
	temp = Helper::GetValidatedPortNumber();
	return temp;
}