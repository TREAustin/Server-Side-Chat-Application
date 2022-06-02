#include <iostream>
#include <string>
#include "Definitions.h"
#pragma once

namespace Helper
{
	uint16_t GetValidatedPortNumber();
	int MessageSize(char* message);
	char* CopyMessage(char* bufferToCopy, int size);
	void MessageToBeLogged(const char* message, char* _username, int _type);
	bool CompareUserInput(const char* _string1, char* _string2);
	int ResultType(int _error);
};

   