#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <iostream>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_ADDRESS "127.0.0.1"

#define SERVER_PORT 8888

#define PAUSE_AND_EXIT() system("pause"); exit(-1)

void printWSErrorAndExit(const char *msg)
{
	wchar_t *s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);
	PAUSE_AND_EXIT();
}

void client(const char *serverAddrStr, int port)
{
	// TODO-1: Winsock init
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		printWSErrorAndExit("WSAStartup");
	}
	std::cout << "WSAStartup done" << std::endl;
	// TODO-2: Create socket (IPv4, datagrams, UDP)
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == INVALID_SOCKET) {
		printWSErrorAndExit("socket");
	}
	std::cout << "socket done" << std::endl;

	std::string pingString("Ping");

	// TODO-3: Create an address object with the server address
	struct sockaddr_in serverAddr;
	const int serverAddrLen = sizeof(serverAddr);
	serverAddr.sin_family = AF_INET; // IPv4
	inet_pton(AF_INET, serverAddrStr, &serverAddr.sin_addr);
	serverAddr.sin_port = htons(port); // Port

	struct sockaddr fromAddr;

	const int inBufferLen = 1300;
	char inBuffer[inBufferLen];

	while (true)
	{
		// TODO-4:
		// - Send a 'ping' packet to the server
		// - Receive 'pong' packet from the server
		// - Control errors in both cases
		int bytes = sendto(s, pingString.c_str(), (int)pingString.size() + 1, 0, (sockaddr*)&serverAddr, serverAddrLen);
		if (bytes >= 0)
		{
			std::cout << "Sent: " << pingString.c_str() << std::endl;

			std::cout << "Waiting for server data... " << std::flush;

			int fromAddrLen = sizeof(fromAddr);
			bytes = recvfrom(s, inBuffer, inBufferLen, 0, &fromAddr, &fromAddrLen);
			if (bytes == SOCKET_ERROR) {
				printWSErrorAndExit("recvfrom");
			}

			std::cout << "Received: " << inBuffer << std::endl;

			Sleep(1000);
		}
		else
		{
			printWSErrorAndExit("sendto");
		}
	}

	// TODO-5: Close socket
	closesocket(s);
	// TODO-6: Winsock shutdown
	WSACleanup();
}

int main(int argc, char **argv)
{
	client(SERVER_ADDRESS, SERVER_PORT);

	PAUSE_AND_EXIT();
}
