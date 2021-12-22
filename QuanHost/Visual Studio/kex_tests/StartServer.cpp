#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <fstream>
#include <string>
#include <iostream>
#include <direct.h>

#include <thread>

#include <WinSock2.h>
#include <WS2tcpip.h>

#include "Client.h"
#include "CommandHandler.h"

#pragma comment(lib, "Ws2_32.lib")

#define CHECK_ERROR(edge) if(i < edge - 1) {  \
	clients[client_id].name.clear();          \
	clients[client_id].socket = 0;            \
	ZeroMemory(clients[client_id].key, 188);  \
	clients[client_id].authenticated = false; \
	clients[client_id].ip.clear();			  \
	clients[client_id].thread = thread();	  \
	clients[client_id].fileThread = thread(); \
	clients[client_id].fileA = false;		  \
	continue;                                 \
}

using namespace std;

wstring utf8_decode(const string &text);
void clientRecv(int id);
void RC5init(unsigned char sharedKey[188]);
int decryptMsg(unsigned char key[188], std::string& msg);

// ����� ������ �������
extern "C"
{
	void generateKey(unsigned char key[334]);
	void agreeKeys(unsigned char shrKey[188], unsigned char serverKey[334]);
}


// ��� �����������/����������� ������
enum packet_type
{
	GREETING,
	PLAIN,
	REGISTER,
};


ofstream mesFile;
fstream base;
Client clients[MAX_CLIENTS]{};
bool connected[MAX_CLIENTS + 1]{}; int connected_int(0);


bool findUser(string name, string &pass, bool &auth)
{
	std::string line;
	base.seekg(0, std::ios::beg);
	while (std::getline(base, line))
	{
		std::string user; int i(0);

		while (line[i] != '|')
			user += line[i++];

		if (user == name)
		{
			if (++i != line.size())
			{
				auth = true;
				while (i != line.size())
					pass += line[i++];
			}
			else
				auth = false;

			return 1;
		}
	}
	auth = false;
	return 0;
}


int startIP(string IP_ADDRESS = "")
{
	char* PORT = "7007";

	if (IP_ADDRESS.empty())
	{
		// ���� IP, �� ������� ������� ������
		print("Enter IP on which the server will run: ", 14, false);
		cin >> IP_ADDRESS;
		std::cout << std::endl;
	}

	// ��������� ��������� ��� �������� ����������
	// � ���������� Windows Sockets
	WSADATA wsaData;

	cout << "Initializing Winsock (WSAStartup)" << endl;
	// ����� ������������� ���������� ������� ���������
	// (������������ Ws2_32.dll)
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	// ���� ��������� ������ ��������� ����������
	if (result != 0) {
		cerr << "WSAStartup failed: " << result << "\n";
		return result;
	}

	struct addrinfo* addr = NULL; // ���������, �������� ����������
	// �� IP-������  ���������� ������

	// ������ ��� ������������� ��������� ������
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));

	// AF_INET ����������, ��� ������������ ���� ��� ������ � �������
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; // ������ ��������� ��� ������
	hints.ai_protocol = IPPROTO_TCP; // ���������� �������� TCP
	// ����� �������� �� �����, ����� ��������� �������� ����������
	hints.ai_flags = AI_PASSIVE;

	cout << "Setting up TCP connection on port " << PORT << endl;
	// �������������� ���������, �������� ����� ������ - addr.
	// C����� ����� ������ �� 7007-� �����
	result = getaddrinfo(IP_ADDRESS.c_str(), PORT, &hints, &addr);

	// ���� ������������� ��������� ������ ����������� � �������,
	// ������� ���������� �� ���� � �������� ���������� ��������� 
	if (result != 0) {
		cerr << "getaddrinfo failed: " << result << "\n";
		WSACleanup(); // �������� ���������� Ws2_32.dll
		return 1;
	}

	cout << "Creating listening socket" << endl;
	// �������� ������
	int listen_socket = (int)socket(addr->ai_family, addr->ai_socktype,
		addr->ai_protocol);
	// ���� �������� ������ ����������� � �������, ������� ���������,
	// ����������� ������, ���������� ��� ��������� addr,
	// ��������� dll-���������� � ��������� ���������
	if (listen_socket == INVALID_SOCKET) {
		cerr << "Error at socket: " << WSAGetLastError() << "\n";
		freeaddrinfo(addr);
		WSACleanup();
		return 1;
	}

	cout << "Binding socket to IP address " << IP_ADDRESS << endl;
	// ����������� ����� � IP-������
	result = bind(listen_socket, addr->ai_addr, (int)addr->ai_addrlen);

	// ���� ��������� ����� � ������ �� �������, �� ������� ���������
	// �� ������, ����������� ������, ���������� ��� ��������� addr.
	// � ��������� �������� �����.
	// ��������� DLL-���������� �� ������ � ��������� ���������.
	if (result == SOCKET_ERROR) {
		cerr << "bind failed with error: " << WSAGetLastError() << "\n";
		freeaddrinfo(addr);
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}

	cout << "Initialize listening socket" << endl;
	// �������������� ��������� �����
	if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
		cerr << "listen failed with error: " << WSAGetLastError() << "\n";
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}

	// ���������� � ������ ��������
	print("Ready!\n", 2);
	std::thread threads[MAX_CLIENTS];
	char userName[64]{};
	char OK('1'); char FAIL('0');

	// �������� ��������� ����� ��� ��������� ������
	thread commands(mainHandler);
	commands.detach();

	// �������� ���� �������������
	base.open("base.txt", std::fstream::in | std::fstream::app);

	// ��������� ���� ��� ���������� ���������
	mesFile.open("Messages.txt", std::ios::out);

	while (true)
	{
		RtlSecureZeroMemory(userName, sizeof(userName));

		// ��������� �������� ����������
		SOCKADDR_IN nsa;
		int sizeof_nsa = sizeof(nsa);

		int acceptedSocket = (int)accept(listen_socket, (SOCKADDR*)&nsa, &sizeof_nsa);
		if (acceptedSocket == INVALID_SOCKET) {
			cerr << "accept failed: " << WSAGetLastError() << "\n";
			closesocket(listen_socket);
			WSACleanup();
			return 1;
		}

		// �������� ���������� ����� ��� �������
		// � ��������� �������� �� ����� �� �������
		int client_id;
		for (client_id = 0; client_id <= MAX_CLIENTS; ++client_id)  // <= bc MAX_CLIENTS + 1 is always empty
			if (!connected[client_id]) break;

		if (client_id == MAX_CLIENTS)
		{
			std::cout << "Somebody tried to connect\n";
			closesocket(acceptedSocket);
			continue;
		}
		

		result = recv(acceptedSocket, userName, 64, 0);
		if (result == SOCKET_ERROR)
		{
			closesocket(acceptedSocket);
			ZeroMemory(userName, 64);
			continue;
		}

		// ��������� ��� �� ������������ �������
		int i(0);
		for (; i < result; ++i)
			if (!isalnum((unsigned char)userName[i]))
			{
				closesocket(acceptedSocket);
				break;
			}

		CHECK_ERROR(result)

		// ��������� �� ��������� �� ��� ������������ � ����� �����
		for (i = 0; i < MAX_CLIENTS; ++i)
			if ('0' + clients[i].name == userName)
			{
				send(acceptedSocket, &FAIL, 1, 0);
				closesocket(acceptedSocket);
				break;
			}

		CHECK_ERROR(MAX_CLIENTS)

		send(acceptedSocket, &OK, 1, 0);

		// ������� ������ ������������
		clients[client_id].socket = acceptedSocket;
		clients[client_id].ip = inet_ntoa(nsa.sin_addr);
		for (i = 1; i < result; ++i)
			clients[client_id].name.append(1, userName[i]);

		unsigned char key[334]{};
		generateKey(key);

		// �������� ����������� � ������ �������
		result = send(acceptedSocket, (char*)key, 334, 0);

		// ���� ���������� ��������� � ������
		char r[334]{};
		result = recv(acceptedSocket, r, 334, 0);

		// �������� ����� � �������� ����
		unsigned char shrKey[188]{};
		agreeKeys(shrKey, (unsigned char*)r);

		// �������������� RC5
		RC5init(shrKey);

		memcpy(&clients[client_id].key, shrKey, 188);

		// ������� ������������ � ����, ���� ��� ��� ��� ���
		// � ��������� ��������������� �� ������������
		string pass;
		bool auth;
		if (!findUser(clients[client_id].name, pass, auth))
		{
			base.clear();
			base << clients[client_id].name << "|" << std::endl;
			base.flush();
			//print("User added to database", 2);
		}
		
		if (auth)
		{
			char recvPass[256]{};
			send(acceptedSocket, &OK, 1, 0);
			recv(acceptedSocket, recvPass, 256, 0);
			string recvPass_s(recvPass);
			RtlSecureZeroMemory(recvPass, 256);

			decryptMsg(clients[client_id].key, recvPass_s);

			if (recvPass_s == pass)
			{
				clients[client_id].authenticated = true;
				send(acceptedSocket, &OK, 1, 0);
			}
			else
			{
				send(acceptedSocket, &FAIL, 1, 0);
				mesFile << "----------" << clients[client_id].name << " tried to connect, but didnt guess the pasword----------" << endl;
				CHECK_ERROR(INT_MAX)
			}
		}
		else
			send(acceptedSocket, &FAIL, 1, 0);


		// �������� ��������� ����� ��� ������������
		clients[client_id].thread = std::thread(clientRecv, client_id);
		clients[client_id].thread.detach();

		// �������� ����� ��� ������� 
		++connected_int;
		connected[client_id] = true;
	}
}


int startTor()
{
	string line;
	string appdata = getenv("APPDATA");
	fstream torrc;
	char current_work_dir[FILENAME_MAX]{};

	// �������� ����������, �� ������� ������� QHost
	if (!_getcwd(current_work_dir, sizeof(current_work_dir)))
	{
		print("Torrc ���� �� ������", FOREGROUND_RED);
		return 1;
	}

	// ��������� ���������������� ���� ���� ��� ������
	torrc.open(appdata + "\\tor\\torrc", ios::in);

	if (torrc.is_open())
	{
		while (!torrc.eof())
			getline(torrc, line);

		torrc.close();
	}
	else cout << "Unable to open file";

	// ��������� ���� ���� ��� �� �������������� (������������ �� ���������)
	if (line != "#Edited")
	{
		cout << "Tor is not configured" << endl;
		cout << "Configuring " << appdata << "\\tor\\torrc file..." << endl;

		// ��������� ���������������� ���� ���� � ������ ����������
		torrc.open(appdata + "\\tor\\torrc", ios::app);

		cout << "Binding Tor hidden service on port 7007..." << endl;

		// ������ hidden service � IP 127.0.0.1 � ����� 7007
		// (���������������� � �������� ����� 80 �� ��������� 7007)
		if (torrc.is_open())
			torrc << "\nHiddenServiceDir " << current_work_dir << "\\Tor\\HiddenSevice\n	HiddenServicePort 80 127.0.0.1:7007\n#Edited";

		torrc.close();
	}

	cout << "Tor is configured" << endl;

	// ������� ����� �������, �� �������� ������������ ������
	ifstream addr;
	addr.open(string(current_work_dir) + "\\Tor\\HiddenSevice\\hostname", ios::in);
	string address;
	if (addr)
	{
		addr >> address;
		cout << "Your hidden service address: ";
		print(address, FOREGROUND_GREEN);
		addr.close();
	}
	else
	{
		print("Cant find hidden service address", FOREGROUND_RED);
		return 1;
	}


	// ���������� ��������� ��� ������� tor (��������� ����)
	STARTUPINFO info{};
	info.cb = sizeof(STARTUPINFO);
	info.dwFlags = STARTF_USESHOWWINDOW;
	info.wShowWindow = SW_MINIMIZE;

	PROCESS_INFORMATION processInfo;

	// ��������� tor.exe
	if (CreateProcess("Tor/tor.exe", NULL, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &info, &processInfo))
		cout << "Tor.exe started successfully (check your taskbar)" << endl;

	// ��������� ��� ������ �� ��������� IP
	startIP("127.0.0.1");

	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);

	return 0;
}