#include <iostream>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
        std::cout << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }

	
	
 	return 0;
}
