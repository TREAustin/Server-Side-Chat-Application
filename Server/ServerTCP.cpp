#include "ServerTCP.h"

int ServerTCP::runServer()
{
	memset(&serverAddr, 0, sizeof(serverAddr));
	memset(&clientAddr, 0, sizeof(clientAddr));
	int result = 0;
	timeVal.tv_sec = 1;
	buffer = new char[BUFFERSIZE];
	for (int i = 0; i < MAXCLIENTS; i++) {
		clients[i] = new ClientRegister(0, "");
	}

	result = setupUDP();

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET)
	{
		return Helper::ResultType(5);
	}

	FD_SET(listenSocket, &masterSet);

	result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		return Helper::ResultType(3);
	}

	result = listen(listenSocket, 5);
	if (result == SOCKET_ERROR) {
		return Helper::ResultType(5);
	}

	logEvent("Server Initialized\n", 0);
	return result;
}

int ServerTCP::setupUDP() {
	int result = 0;
	broadcastSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (broadcastSocket == INVALID_SOCKET)
	{
		return Helper::ResultType(5);
	}	
	
	result = setsockopt(broadcastSocket, SOL_SOCKET, SO_BROADCAST, &broadVal, sizeof(broadVal));
	result = ioctlsocket(broadcastSocket, FIONBIO, &value);

	char hostName[255];
	gethostname(hostName, 255);
	host = gethostbyname(hostName);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = INADDR_BROADCAST;
	clientAddr.sin_port = htons(port);

	if (result == SOCKET_ERROR)
	{
		return Helper::ResultType(3);
	}

	FD_SET(broadcastSocket, &masterSet);

	return result;
}

void ServerTCP::sendBroadcastMsg() {
	int result = 0;	
	ip = inet_ntoa(*(in_addr*)*host->h_addr_list);
	oss << ip << "," << port;
	oss << '\0';
	result = sendto(broadcastSocket, oss.str().c_str(), oss.str().length(), 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
	oss.str("");
	//STILL NEED ERROR CHECKING///
}

int ServerTCP::monitorActivity() {

	int result = SUCCESS;
	FD_ZERO(&readSet);
	readSet = masterSet;
	result = select(0, &readSet, NULL, NULL, &timeVal);
	sendBroadcastMsg();

	if (FD_ISSET(listenSocket, &readSet))
	{
		result = newConnection();
		if (result != SUCCESS) {
			return Helper::ResultType(WSAGetLastError());
		}
	}

	for (int i = 0; i < MAXCLIENTS; i++) {
		if (FD_ISSET(clients[i]->socket, &readSet)) {
			result = determineActivity(clients[i]->socket, (char *)clients[i]->username);
			if (result == SHUTDOWN) {
				FD_CLR(clients[i]->socket, &masterSet);
				std::cout << clients[i]->username << " has signed out." << std::endl;
				clients[i]->socket = 0;
				clients[i]->username = "";
				result = 0;
			}
			if (result != SUCCESS) {
				return Helper::ResultType(WSAGetLastError());
			}
		}
	}

	return result;
}

int ServerTCP::newConnection() {
	int result = SUCCESS;
	commSocket = accept(listenSocket, (SOCKADDR*)&serverAddr, NULL);
	if (result != SUCCESS) {
		return Helper::ResultType(WSAGetLastError());
	}
	result = readMessage(commSocket, buffer, BUFFERSIZE);
	if (result == 0 || result == SOCKET_ERROR) {
		return Helper::ResultType(9);
	}
	char* temp = Helper::CopyMessage(buffer, result);
	for (int i = 0; i < MAXCLIENTS; i++)
	{
		if (clients[i]->socket == 0)
		{
			FD_SET(commSocket, &masterSet);
			clients[i]->socket = commSocket;
			clients[i]->username = temp;
			result = sendMessage(clients[i]->socket, (char*)"SV_SUCCESS", 11);
			oss << "New User Registered, adding user " << clients[i]->username << " to the list.\n";
			std::cout << oss.str();
			logEvent(oss.str().c_str(), 1);
			break;
		}
		if (i == (MAXCLIENTS - 1) && clients[i]->socket != 0) {
			result = sendMessage(commSocket, (char*)"SV_FULL", 8);
			if (result == 0 || result == SOCKET_ERROR) {
				return Helper::ResultType(WSAGetLastError());
			}
			oss << temp << " tried to register to the server but it was full.\n";
			std::cout << oss.str().c_str() << std::endl;
			logEvent(oss.str().c_str(), 1);
		}
	}
	return result;
}

int ServerTCP::determineActivity(SOCKET s, char* username) {
	int result = readMessage(s, buffer, BUFFERSIZE);
	if (result == 0 || result == SOCKET_ERROR) {
		return Helper::ResultType(9);
	}
	char* message = Helper::CopyMessage(buffer, result);
	if (WSAGetLastError() == 10054) {
		oss << username << " logged out of the server.  Connection has been removed.\n\0";
		std::cout << oss.str();
		logEvent(oss.str().c_str(), 1);
		FD_CLR(s, &masterSet);
		for (int i = 0; i < MAXCLIENTS; i++) {
			if (clients[i]->socket == s) {
				clients[i]->socket = 0;
				clients[i]->username = "";
			}
		}
		shutdown(s, SD_BOTH);
		closesocket(s);
	}
	//Send the list of active users to the user requesting it.
	else if (Helper::CompareUserInput("$getlist", message)) {
		oss << "Current Users: ";
		for (auto i = 0; i < MAXCLIENTS; i++) {
			if (clients[i]->socket != 0 && (i != MAXCLIENTS - 1)) {
				oss << clients[i]->username << ", ";
			}
			else if (clients[i]->socket != 0 && (i == MAXCLIENTS - 1)) {
				oss << clients[i]->username << ".";
			}
		}
		oss << "\n";
		sendMessage(s, (char*)oss.str().c_str(), Helper::MessageSize((char *)oss.str().c_str()));
			//If a WSA Error was thrown, this will handle the appropriate message.
	}
	//Send the log to the user requesting it.
	else if (Helper::CompareUserInput("$getlog", message)){
		result = sendFileLog(s);
	}
	//Send exit to client.
	else if (Helper::CompareUserInput("$exit", message)) {
		return Helper::ResultType(1);
	}
	//Forward the message sent to the other users connected.
	else {
		FD_CLR(s, &readSet);
		oss << username << ": " << message << "\n";
		for (int i = 0; i < MAXCLIENTS; i++) {
			if (clients[i]->socket != listenSocket && clients[i]->socket != s) {
				result = sendMessage(clients[i]->socket, (char*)oss.str().c_str(), oss.str().length());
			}

		}
		std::cout << oss.str();
		logEvent(oss.str().c_str(), 1);
	}

	return 0;
}

int ServerTCP::sendFileLog(SOCKET s) {
	int result = 0;
	const char* fileName = "event_log.txt";
	std::ifstream infile(fileName);
	long long bytesToSend = fileSize(fileName);
	result = sendMessage(s, (char*)"$getlog", Helper::MessageSize((char*)"$getlog"));
	oss << bytesToSend;
	result = sendMessage(s, (char*)oss.str().c_str(), oss.str().length());
	oss.str("");
	if (infile.is_open()) {
		std::cout << std::endl << "---- Server Log ----" << std::endl;
		std::string line;
		while (std::getline(infile, line)) {
			printf("%s\n", line.c_str());
			line += "\n";
			result = sendMessage(s, (char*)line.c_str(), line.length());

		}
		infile.close();
	}
	return result;

}

int ServerTCP::readMessage(SOCKET s, char* buffer, int32_t size)
{
	int result = tcp_recv_whole(s, (char*)&size, 1);
	size = size - BUFFERSIZE;
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		return MESSAGE_ERROR;
	}
	else if (WSAGetLastError() != 0) {
		return Helper::ResultType(WSAGetLastError());
	}
	result = tcp_recv_whole(s, buffer, size);

	if ((result == SOCKET_ERROR) || (result == 0))
	{
		return MESSAGE_ERROR;
	}
	else if (result != size) {
		return Helper::ResultType(8);
	}

	return result;
}

int ServerTCP::sendMessage(SOCKET s, char* data, int32_t length)
{
	int result;
	result = tcp_send_whole(s, (char*)&length, 1);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		return MESSAGE_ERROR;
	}
	else if (WSAGetLastError() != 0) {
		return Helper::ResultType(WSAGetLastError());
	}

	result = tcp_send_whole(s, data, length);

	if ((result == SOCKET_ERROR) || (result == 0))
	{
		return Helper::ResultType(9);
	}
	else if (length < 0 || length > 1024) {
		return Helper::ResultType(8);
	}

	return result;
}

void ServerTCP::stop()
{
	shutdown(listenSocket, SD_BOTH);
	closesocket(listenSocket);

	shutdown(commSocket, SD_BOTH);
	closesocket(commSocket);
	
	delete[] buffer, clients;
} 

int ServerTCP::tcp_send_whole(SOCKET s, const char* data, uint16_t length) {
	int result;
	int bytesSent = 0;

	while (bytesSent < length)
	{
		result = send(s, (const char*)data + bytesSent, length - bytesSent, 0);

		if (result <= 0)
			return result;

		bytesSent += result;
	}

	return bytesSent;
}

int ServerTCP::tcp_recv_whole(SOCKET s, char* buf, int len)
{
	int total = 0;
	do
	{
		int ret = recv(s, buf + total, len - total, 0);
		if (ret < 1)
			return ret;
		else
			total += ret;

	} while (total < len);

	return total;
}

void ServerTCP::logEvent(const char* _event, int type) {
	FILE* events;
	//Type 0 used for server being initialized.
	if (type == 0) {
		events = fopen("event_log.txt", "w");
	}
	//Type 1 used for appending.
	else {
		events = fopen("event_log.txt", "a");
	}
	if (events != NULL) {
		fputs(_event, events);
		fclose(events);
	}
	oss.str("");
}

long long ServerTCP::fileSize(const char* _fileName) {
	std::ifstream infile(_fileName, std::ifstream::ate | std::ifstream::binary);
	return infile.tellg();
}