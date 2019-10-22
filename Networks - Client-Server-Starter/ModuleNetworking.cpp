#include "Networks.h"
#include "ModuleNetworking.h"
#include <list>

static uint8 NumModulesUsingWinsock = 0;



void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}

void ModuleNetworking::disconnect()
{
	for (SOCKET socket : sockets)
	{
		shutdown(socket, 2);
		closesocket(socket);
	}

	sockets.clear();
}

bool ModuleNetworking::init()
{
	if (NumModulesUsingWinsock == 0)
	{
		NumModulesUsingWinsock++;

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(version, &data) != 0)
		{
			reportError("ModuleNetworking::init() - WSAStartup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::preUpdate()
{
	if (sockets.empty()) return true;

	// NOTE(jesus): You can use this temporary buffer to store data from recv()
	const uint32 incomingDataBufferSize = Kilobytes(1);
	byte incomingDataBuffer[incomingDataBufferSize];
	
	// New socket set
	fd_set readfds;
	FD_ZERO(&readfds);
	// Fill the set
	for (auto s : sockets) {
		FD_SET(s, &readfds);
	}
	// Timeout (return immediately)
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// TODO(jesus): select those sockets that have a read operation available
	int error = select(0, &readfds, nullptr, nullptr, &timeout);
	if (error == SOCKET_ERROR) 
		reportError("Socket selection error");
	std::vector<SOCKET> disconnected_sockets;


	for (auto s : sockets)
	{
		if (FD_ISSET(s, &readfds)) {
			if (isListenSocket(s)) { // Is the server socket

				SOCKET Socket = INVALID_SOCKET;
				sockaddr_in addr;
				int len = sizeof(addr);
				Socket = accept(s, (sockaddr*)&addr, &len);

				if (Socket == INVALID_SOCKET)
					reportError("Socket accept error");
				else {
					onSocketConnected(Socket, addr);
					addSocket(Socket);
				}

			}
			else { 
					
				error = recv(s, (char*)incomingDataBuffer, incomingDataBufferSize, 0);
				if (error == SOCKET_ERROR) {
					reportError("Socket receive info error");
					disconnected_sockets.push_back(s);
				}
				else {
					if (error == 0 || error == ECONNRESET)
						disconnected_sockets.push_back(s);
					else {
						incomingDataBuffer[error] = '\0';
						onSocketReceivedData(s, incomingDataBuffer);
					}
				}
			}
		}
	}

	for (std::vector<SOCKET>::iterator it = disconnected_sockets.begin();it!= disconnected_sockets.end();it++) {
		for (int i = 0; i < sockets.size(); i++) {
			if (*it == sockets[i]) {
				onSocketDisconnected(sockets[i]);
				sockets.erase(sockets.begin() + i);
				
			}
		}
		
	
	}

	return true;
}

bool ModuleNetworking::cleanUp()
{
	disconnect();

	NumModulesUsingWinsock--;
	if (NumModulesUsingWinsock == 0)
	{

		if (WSACleanup() != 0)
		{
			reportError("ModuleNetworking::cleanUp() - WSACleanup");
			return false;
		}
	}

	return true;
}

void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}
