#include "ModuleNetworkingClient.h"


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;

	// TODO(jesus): TCP connection stuff

	//winsock init done in module networking
	/*WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) == SOCKET_ERROR)
		reportError("WSAStartup error (client)");*/
	// - Create the socket

	c_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (c_socket == INVALID_SOCKET)
		reportError("Socket creation error (client)");

	// - Create the remote address object
	serverAddress.sin_family = AF_INET; //IPv4
	inet_pton(AF_INET, serverAddressStr, &serverAddress.sin_addr);
	serverAddress.sin_port = htons(serverPort);

	// - Connect to the remote address
	const int serveraddlength = sizeof(serverAddress);
	int connectRes = connect(c_socket, (const sockaddr*)&serverAddress, serveraddlength);
	if (connectRes == SOCKET_ERROR)
		reportError("Connection error (client)");
	else
	{
		// - Add the created socket to the managed list of sockets using addSocket()
		addSocket(c_socket);
		// If everything was ok... change the state
		state = ClientState::Start;
	}
	
	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
	if (state == ClientState::Start)
	{
		// TODO(jesus): Send the player name to the server
		char inputBuffer[100];
		strcpy_s(inputBuffer, playerName.c_str());
		if (send(c_socket, inputBuffer, sizeof(inputBuffer), 0) == SOCKET_ERROR)
			reportError("Sending info error (client)");

		state = ClientState::Logging;
	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("%s connected to the server...", playerName.c_str());

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, byte * data)
{
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

