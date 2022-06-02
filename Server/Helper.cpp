#include "Helper.h"

uint16_t Helper::GetValidatedPortNumber()
{
	uint16_t userInput;
	std::cout << "Enter the server's port number: ";
	while (true)
	{
		std::cin >> userInput;
		if (std::cin.fail() || (userInput < 0 || userInput > 65536))
		{
			std::cin.clear();
			std::cin.ignore(UINT16_MAX, '\n');
			std::cout << std::endl << "You did not enter a valid port number, please try again: " << std::endl;
			continue;
		}
		std::cin.ignore(UINT16_MAX, '\n');
		break;
	}
	return userInput;
}

int Helper::MessageSize(char* message) {
	int total;
	for (total = 0; message[total] != '\0'; total++);
	return total;
}

char* Helper::CopyMessage(char* bufferToCopy, int size) {
	char* toReturn = new char[size + 1];
	for (int i = 0; i < size; i++) {
		toReturn[i] = bufferToCopy[i];
	}
	toReturn[size] = '\0';
	return toReturn;
}

bool Helper::CompareUserInput(const char* _string1, char* _string2) {
	int size1;
	int size2;

	for (size1 = 0; _string1[size1] != '\0'; size1++);
	for (size2 = 0; _string2[size2] != '\0'; size2++);

	if (size1 != size2) {
		return false;
	}

	for (int i = 0; i < size1; i++) {
		if (_string1[i] != _string2[i]) {
			return false;
		}
	}

	return true;
}

void Helper::MessageToBeLogged(const char* message, char* _username, int _type) {
	std::string temp = _username;
	if (_type == 0) {
		temp += " registered to the server and is ready for chat.\n";
	}
	*&message = temp.c_str();
}

int Helper::ResultType(int _error) {
	switch (_error) {
	case 0:
		std::cout << "There was an issue with the message.  It didn't return anytihng.  Please check the message and try again." << std::endl;
		return MESSAGE_ERROR;
	case 1:
		return SHUTDOWN;
	case 2:
		std::cout << "The connection has been disconnected." << std::endl;
		return DISCONNECT;
	case 3:
		std::cout << "There was an issue binding the socket." << std::endl;
		return BIND_ERROR;
	case 4:
		std::cout << "There was an issue connecting.  Please try again later." << std::endl;
		return CONNECT_ERROR;
	case 5:
		std::cout << "There was an setting up the connection.  Please try again later." << std::endl;
		return SETUP_ERROR;
	case 6:
		std::cout << "There was an issue starting up.  Please try  again later." << std::endl;
		return STARTUP_ERROR;
	case 7:
		std::cout << "The was an issue with the address. Please make sure it is valid and in the quadratic format." << std::endl;
	case 8:
		std::cout << "There was an issue with the message.  It was either too long or not long enough." << std::endl;
		return PARAMETER_ERROR;
	case 9:
		std::cout << "There was an issue with the message.  Please check the message and try again." << std::endl;
		return MESSAGE_ERROR;
	case 10004:
		std::cout << "There was an issue with the connection and has been shutdown" << std::endl;
		return SHUTDOWN;
	case 10054:
		std::cout << "There has been an issue with the connection and has been disconnected" << std::endl;
		return DISCONNECT;
	case 10058:
		std::cout << "The connection has been shutdown." << std::endl;
		return SHUTDOWN;
	case 10061:
		std::cout << "There was an issue with connecting to the sever.  It may not be active.  Please try again later." << std::endl;
		return CONNECT_ERROR;
	default:
		return SUCCESS;
	}
}