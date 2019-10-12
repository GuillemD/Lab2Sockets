#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <iostream>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

#define LISTEN_PORT 8888

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

void server(int port)
{
	// TODO-1: Winsock init
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		printWSErrorAndExit("WSAStartup");
	}
	std::cout << "WSAStartup done" << std::endl;

	// TODO-2: Create socket (IPv4, stream, TCP)
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		printWSErrorAndExit("socket");
	}
	std::cout << "socket done" << std::endl;
	// TODO-3: Configure socket for address reuse
	int enable = 1;
	int result = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	if (result == SOCKET_ERROR) {
		printWSErrorAndExit("setsockopt");
	}
	std::cout << "setsockopt SO_REUSEADDR done" << std::endl;
	// TODO-4: Create an address object with any local address
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET; // IPv4
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any address
	serverAddr.sin_port = htons(port); // Port
	// TODO-5: Bind socket to the local address
	int bindRes = bind(s, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if (bindRes == SOCKET_ERROR) {
		printWSErrorAndExit("bind");
	}
	std::cout << "bind done on port " << port << std::endl;
	// TODO-6: Make the socket enter into listen mode
	int listenRes = listen(s, 1);
	if (listenRes == SOCKET_ERROR) {
		printWSErrorAndExit("listen");
	}
	std::cout << "listen done" << std::endl;
	// TODO-7: Accept a new incoming connection from a remote host
	// Note that once a new connection is accepted, we will have
	// a new socket directly connected to the remote host.
	struct sockaddr clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	SOCKET clientSocket = accept(s, (struct sockaddr*)&clientAddr, &clientAddrLen);
	if (clientSocket == INVALID_SOCKET) {
		printWSErrorAndExit("accept");
	}
	std::cout << "accept done" << std::endl;

	std::string pongString("Pong");

	const int inBufferLen = 1300;
	char inBuffer[inBufferLen];
	while (true)
	{
		// TODO-8:
		// - Wait a 'ping' packet from the client
		// - Send a 'pong' packet to the client
		// - Control errors in both cases
		std::cout << "Waiting for client data... " << std::flush;

		int bytes = recv(clientSocket, inBuffer, inBufferLen, 0);
		if (bytes > 0)
		{
			std::cout << "Received: " << inBuffer << std::endl;

			Sleep(1000);

			bytes = send(clientSocket, pongString.c_str(), (int)pongString.size() + 1, 0);
			if (bytes == SOCKET_ERROR) {
				printWSErrorAndExit("send");
			}

			std::cout << "Sent: '" << pongString.c_str() << "' sent" << std::endl;
		}
		else if (bytes == 0)
		{
			std::cout << "connection closed by client" << std::endl;
			break;
		}
		else
		{
			printWSErrorAndExit("recv");
		}
	}

	// TODO-9: Close socket
	closesocket(s);
	// TODO-10: Winsock shutdown
	WSACleanup();
}

int main(int argc, char **argv)
{
	server(LISTEN_PORT);

	PAUSE_AND_EXIT();
}
