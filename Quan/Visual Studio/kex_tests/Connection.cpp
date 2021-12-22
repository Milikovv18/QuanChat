#include <string>
#include <iostream>
#include <conio.h>
#include <thread>
#include <fstream>
#include <WS2tcpip.h>
#include <winsock2.h>
// Эти штуки нужны, чтоб избавиться от предупреждений
#pragma comment(lib,"wsock32.lib")
#pragma comment (lib,"Ws2_32.lib")

#include "TorCode/socks.h"
#include "User.h"
#include <ctime>

// Порт, через который мы подключаемся к серверу
// (выглядит красиво, оставим его)
#define DEFAULT_PORT "7007"

using namespace std;

// Как выяснить так сделать гораздо легче
// чем инклудить файлы, оно хотя бы компилируется
void IPrecv(SOCKET socket);
void TORrecv(socks5cpp::SocksClient sockCli, SOCKET sock);
void print(std::string text, WORD style, bool newStr = true);
void getPswd(char pswd[256]);
void insertStr(string userName, string str, WORD color, bool auth = false);
bool connectTor(string address, string name);
bool connectIP(string address, string name);
int encryptMsg(string &msg);
void RC5init(unsigned char sharedKey[188]);

// Снова сишные функции
extern "C"
{
	void generateKey(unsigned char key[334]);
	void agreeKeys(unsigned char shrKey[188], unsigned char serverKey[334]);
}

// И снова extern объект структуры User
extern struct User user;
extern int unreaddies;
extern int accept_g;


// Тип посылаемого/получаемого пакета
enum packet_type
{
	GREETING,
	PLAIN,
	REGISTER,
};


// Функция для определения к чему надо подключаться
// Tor hidden service или обычный сервер с известным IP
void connectTo(string address, string name)
{
	print("WELCOME TO CHAT ZONE!\n", 11);
	cout << "Resolving host address... " << endl;
	
	struct sockaddr_in sa;  // Эта штука нужна для валидации IP
	// Проверка на tor hidden service
	if (address.size() >= 6 && address.substr(address.length() - 6, address.length()) == ".onion")
	{
		cout << "Detected .onion hidden service" << endl;
		connectTor(address, name);
	}
	// Проверка на валидный IP адрес
	else if (inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) == 1)
	{
		cout << "Detected valid IP address" << endl;
		connectIP(address, name);
	}
	// Иначе никуда не подключаемся и выдаем ошибку
	else
		print("[ERROR] Network address type unrecognized", FOREGROUND_RED);

	cout << "\nPress any key to return to main menu...";

	// Аналог "Нажмите на любую клавишу для выхода"
	if (_getch()) {}
}


bool busy(false), fileA(false);
__int64 sent(0); struct _stat64 buf;
void sendFile(SOCKET sock, string msg)
{
	if (msg.size() < 4)
	{
		insertStr("", "[ERRORR] Empty parameter", FOREGROUND_RED);
		busy = false;
		return;
	}

	if (msg[3] == '"')
	{
		msg.erase(3, 1);
		msg.erase(msg.size() - 1);
	}

	// Пробуем открыть файл
	FILE* in;
	fopen_s(&in, msg.substr(3, msg.size()).c_str(), "rb");

	if (in)
	{
		_stat64(msg.substr(3, msg.size()).c_str(), &buf);
		string filename = msg.substr(msg.find_last_of("/\\") + 1) + ".quan|" + to_string(buf.st_size);

		int size = encryptMsg(filename);
		send(sock, ("FI" + filename).c_str(), size + 2, 0);

		insertStr("", "[NOTICE] Sending a " + to_string(buf.st_size) + " byte file \"" + msg.substr(msg.find_last_of("/\\") + 1) + ".quan\"                                   ", 14);

		unsigned char buff[2000];

		// Измеряем скорость доставки файла
		std::clock_t start = clock();

		while (sent < buf.st_size)
		{
			fileA = false;

			int s = fread(buff, 1, sizeof buff, in);
			string filepart((char*)buff, s);

			size = encryptMsg(filepart);
			send(sock, ("F" + filepart).c_str(), size + 1, 0);

			sent += s;

			clock_t timeout = clock();
			while (!fileA && clock() <= timeout + 5000) {}

			if (clock() - timeout > 5000)
			{
				insertStr("", "[ERRORR] File was not delivered. Time is out", FOREGROUND_RED);
				if (fclose(in)) perror("close input file");
				in = nullptr;
				busy = false;
				return;
			}
		}

		if (fclose(in)) perror("close input file");
		in = nullptr;

		insertStr("", "[NOTICE] File \"" + msg.substr(msg.find_last_of("/\\") + 1) + ".quan\" delivered in " + to_string((clock() - start) / (double)CLOCKS_PER_SEC) + "s", FOREGROUND_GREEN);
	}
	else
		insertStr("", "[ERRORR] File \"" + msg.substr(3, msg.size()) + "\" not found or its already busy", FOREGROUND_RED);

	busy = false;
}


string msg = "";
bool insertDenial(false);
void readMsg()
{
	insertDenial = true;

	if (user.authenticated)
		print("\r[A]", FOREGROUND_BLUE, false);
	else
		cout << "\r";

	print(user.userName + ": ", 14, false);
	int i(0);
	msg.clear();

	insertDenial = false;
	while (1) {
		msg.append(1, char(_getch()));
		if (msg[i] == 13)
		{
			msg.erase(i);
			return;
		}
		else if (msg[i] == '\b')
		{
			if (i >= 0)
			{
				msg.erase(i);
				if (i)
				{
					msg.erase(--i);
					std::cout << "\b \b";
				}
			}
		}
		else
			std::cout << msg[i++];
	}
}


// Подключение к Tor hidden service по .onion адресу
bool connectTor(string address, string name)
{
	cout << "Starting Tor..." << endl;

	if (!user.manualTor)
	{
		// Определяем параметры для запуска tor (свернутое окно)
		STARTUPINFO info{};
		info.cb = sizeof(STARTUPINFO);
		info.dwFlags = STARTF_USESHOWWINDOW;
		info.wShowWindow = SW_MINIMIZE;

		PROCESS_INFORMATION processInfo;

		// Запускаем tor.exe
		if (CreateProcess("Tor/tor.exe", NULL, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &info, &processInfo))
			cout << "Tor.exe started successfully (check your taskbar)" << endl;

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}
	else
		print("[NOTICE] Please start tor.exe manually (path *Quan path*\\Tor\\tor.exe)", 14);


	cout << "Initializing Winsock (WSAStartup)..." << endl;
	WSADATA wsadata;
	if (WSAStartup((WORD)0x202, &wsadata) == SOCKET_ERROR)
	{
		print(("[ERRORR] Fault at connect: " + WSAGetLastError()), FOREGROUND_RED);
		WSACleanup();
		return 1;
	}

	cout << "Attempt to connect to " << address << "..." << endl;
	socks5cpp::SocksClient sockCli("127.0.0.1:9050", address + ":80");
	SOCKET sock = 0;

	int j(0);
	while (sockCli.connect(sock) < 0 || !sock)
	{
		if (j++ > 10)
		{
			print(("[ERRORR] Fault at connect: " + WSAGetLastError()), FOREGROUND_RED);
			WSACleanup();
			return 1;
		}
		Sleep(500);
	}

	if (sockCli.getState() == sockCli.Closed) {
		print(("[ERRORR] Fault at connect: " + WSAGetLastError()), FOREGROUND_RED);
		WSACleanup();
		return 1;
	}

	// Собираем сообщение для приветсвия (с именем)
	string greeting = to_string(GREETING) + name;

	// И отсылаем его
	int iRes = sockCli.sendPacket(sock, greeting.c_str(), 1 + name.size());

	// Ждем когда сервер разрешит подключение
	char stat;
	do {
		iRes = sockCli.recvPacket(sock, &stat, 1);
	} while (iRes == -1);

	if (stat != '1')
	{
		print("[ERRORR] User " + name + " already connected to " + address, FOREGROUND_RED);
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	cout << "Connected succesfully" << endl;

	cout << "Generating SIDH key..." << endl;
	unsigned char key[334]{};
	generateKey(key);

	cout << "Key exchange with server..." << endl;

	// Теперь ждем когда сервер поприветствует нас в ответ
	char r[334]{};
	do {
		iRes = sockCli.recvPacket(sock, r, 334);
	} while (iRes == -1);

	// Получаем общий с сервером ключ
	unsigned char shrKey[188]{};
	agreeKeys(shrKey, (unsigned char*)r);
	greeting.clear();

	// И отсылаем его
	iRes = sockCli.sendPacket(sock, (char*)key, 334);

	// Обязательно очищаем все больше не использующиеся ключи
	RtlSecureZeroMemory(key, 334);

	cout << "Your key is: ";
	for (int k(0); k < 188; ++k) printf("%.2X", shrKey[k]);
	cout << endl << endl;

	cout << "Initialising RC5..." << endl;
	RC5init(shrKey);

	// Ждем требования пароля от сервера
	do {
		iRes = sockCli.recvPacket(sock, &stat, 1);
	} while (iRes == -1);

	// Если пароль требуется
	if (stat == '1')
	{
		cout << "This server requires password for " << name << endl;
		print("Enter your password: ", 14, false);
		char pswd[256]{};
		getPswd(pswd);
		string pswd_s(pswd);
		int size = encryptMsg(pswd_s);

		// Сразу очищаем пароль, так как дальше нигде его не используем
		RtlSecureZeroMemory(pswd, 256);

		iRes = sockCli.sendPacket(sock, pswd_s.c_str(), size);

		do {
			iRes = sockCli.recvPacket(sock, &stat, 1);
		} while (iRes == -1);

		if (stat == '1')
		{
			print("\rPswd is right                                                                                                                                                                                                                                                 ", FOREGROUND_GREEN);
			user.authenticated = true;
		}
		else
		{
			print("\rPswd is wrong                                                                                                                                                                                                                                                 ", FOREGROUND_RED);
			WSACleanup();
			return 1;
		}
	}
	else
		cout << "This server does not require password for " << name << endl;


	// Все, можно начинать чаттиться
	SetConsoleTitleW(L"Quan [Tor]");
	cout << "\n\n-------------------------------------------------------------------------------" << endl;

	print("START CHATTING (write your message or type \"/\" to disconnect)\n\n", 11);

	// Создаем отдельный поток для приема сообщений (пока сервер не отключится)
	thread recv = thread(TORrecv, sockCli, sock);
	recv.detach();

	// Запускаем бесконечный цикл для считывания и отправки сообщений
	while (true)
	{
		readMsg();
		cout << endl;
		// Проверяем если сообщение это команда для выхода
		if (msg == "/") break;
		if (msg.substr(0, 2) == "/f")
		{
			if (busy)
			{
				insertStr("", "[ERROR] File thread is busy!", FOREGROUND_RED);
				continue;
			}

			busy = true;
			thread file(sendFile, sock, msg);
			file.detach();
			continue;
		}
		else if (msg.substr(0, 2) == "/a")
		{
			msg.clear();
			accept_g = true;
			Sleep(100);
			accept_g = -1;
			continue;
		}
		else if (msg.substr(0, 2) == "/d")
		{
			msg.clear();
			accept_g = false;
			Sleep(100);
			accept_g = -1;
			continue;
		}
		else if (msg == "/status")
		{
			msg.clear();
			if (busy)
				print("File uploading " + to_string(float(sent) / buf.st_size * 100) + "% -- " + to_string(sent) + "/" + to_string(buf.st_size) + "B", 14);
			else
				print("No files downloading", 14);
		}

		// Шифруем полученную строку
		int size = encryptMsg(msg);

		// Посылаем шифрованное сообщение на .onion адрес
		iRes = sockCli.sendPacket(sock, msg.c_str(), size);
		if (iRes != msg.length()) {
			WSACleanup();
			print("[ERRORR] Fault at send", FOREGROUND_RED);
			break;
		}

		unreaddies = 0;
		SetConsoleTitleW(L"Quan [Tor]");
	}

	// Самостоятельно отключаемся от сервера
	extern bool aborted;
	aborted = true;

	// Затираем ключи именной через эту функцию (не memset)!!!
	RtlSecureZeroMemory(key, 188);
	WSACleanup();
	closesocket(sock);
	return 0;
}


// Подключение к серверу по IP адресу
bool connectIP(string address, string name)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	int iResult;

	// Initialize Winsock
	cout << "Initializing Winsock (WSAStartup)..." << endl;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		print(("[ERRORR] WSAStartup failed with error: ", iResult, "\n"), FOREGROUND_RED);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	cout << "Setting up TCP connection..." << endl;
	iResult = getaddrinfo(address.c_str(), DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		print(("[ERRORR] Getaddrinfo failed with error: ", iResult, "\n"), FOREGROUND_RED);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	cout << "Attempt to connect to " << address << " until one succeeds..." << endl;
	int i(0);
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		cout << "Attempt " << i++ << endl;
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			print(("[ERRORR] Socket failed with error: ", WSAGetLastError(), "\n"), FOREGROUND_RED);
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		print("[ERRORR] Unable to connect to server!\n", FOREGROUND_RED);
		WSACleanup();
		return 1;
	}

	// Собираем сообщение для приветсвия (с именем)
	string greeting = to_string(GREETING) + name;

	// И отсылаем его
	iResult = send(ConnectSocket, greeting.c_str(), int(1 + name.size()), 0);

	// Ждем когда сервер разрешит подключение
	char stat;
	iResult = recv(ConnectSocket, &stat, 1, 0);
	if (stat != '1')
	{
		print("[ERRORR] User " + name + " already connected to " + address, FOREGROUND_RED);
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	cout << "Connected succesfully" << endl;

	cout << "Generating SIDH key..." << endl;
	unsigned char key[334]{};
	generateKey(key);

	cout << "Key exchange with server..." << endl;

	// Теперь ждем когда сервер поприветствует нас в ответ
	char r[334]{};
	iResult = recv(ConnectSocket, r, 334, 0);

	// Получаем общий с сервером ключ
	unsigned char shrKey[188]{};
	agreeKeys(shrKey, (unsigned char*)r);
	greeting.clear();

	// И отсылаем его
	iResult = send(ConnectSocket, (char*)key, 334, 0);

	// Обязательно очищаем все больше не использующиеся ключи
	RtlSecureZeroMemory(key, 334);

	cout << "Your key is: ";
	for (int j(0); j < 188; ++j) printf("%.2X", shrKey[j]);
	cout << endl << endl;

	cout << "Initialising RC5..." << endl;
	RC5init(shrKey);

	// Ждем требования пароля от сервера
	iResult = recv(ConnectSocket, &stat, 1, 0);

	// Если пароль требуется
	if (stat == '1')
	{
		cout << "This server requires password for " << name << endl;
		print("Enter your password: ", 14, false);
		char pswd[256]{};
		getPswd(pswd);
		string pswd_s(pswd);
		int size = encryptMsg(pswd_s);

		// Сразу очищаем пароль, так как дальше нигде его не используем
		RtlSecureZeroMemory(pswd, 256);

		iResult = send(ConnectSocket, pswd_s.c_str(), size, 0);

		iResult = recv(ConnectSocket, &stat, 1, 0);
		if (stat == '1')
		{
			print("\rPswd is right                                                                                                                                                                                                                                                 ", FOREGROUND_GREEN);
			user.authenticated = true;
		}
		else
		{
			print("\rPswd is wrong                                                                                                                                                                                                                                                 ", FOREGROUND_RED);
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
	}
	else
		cout << "This server does not require password for " << name << endl;


	// Все, можно начинать чаттиться
	SetConsoleTitleW(L"Quan [IPv4]");
	cout << "\n-------------------------------------------------------------------------------" << endl;
	print("START CHATTING (write your message or type \"/\" to disconnect)\n\n", 11);

	// Создаем отдельный поток для приема сообщений (пока сервер не отключится)
	thread recv = thread(IPrecv, ConnectSocket);
	recv.detach();

	// Запускаем бесконечный цикл для считывания и отправки сообщений
	while (true)
	{
		readMsg();
		cout << endl;
		// Проверяем если сообщение это команда для выхода
		if (msg == "/") break;
		// Если отправляем файл
		if (msg.substr(0, 2) == "/f")
		{
			if (busy)
			{
				insertStr("", "[ERROR] File thread is busy!", FOREGROUND_RED);
				continue;
			}

			busy = true;
			thread file(sendFile, ConnectSocket, msg);
			file.detach();
			continue;
		}
		else if (msg.substr(0, 2) == "/a")
		{
			msg.clear();
			accept_g = true;
			Sleep(100);
			accept_g = -1;
			continue;
		}
		else if (msg.substr(0, 2) == "/d")
		{
			msg.clear();
			accept_g = false;
			Sleep(100);
			accept_g = -1;
			continue;
		}
		else if (msg == "/status")
		{
			msg.clear();
			if (busy)
				print("File uploading " + to_string(float(sent) / buf.st_size * 100) + "% -- " + to_string(sent) + "/" + to_string(buf.st_size) + "B", 14);
			else
				print("No files downloading", 14);
		}

		// Шифруем полученную строку
		int size = encryptMsg(msg);

		// Посылаем шифрованное сообщение на сервер
		iResult = send(ConnectSocket, msg.c_str(), size, 0);
		if (iResult == SOCKET_ERROR) {
			print(("[ERROR] Send failed with error: " + to_string(WSAGetLastError())), FOREGROUND_RED);
			closesocket(ConnectSocket);
			WSACleanup();
			break;
		}

		unreaddies = 0;
		SetConsoleTitleW(L"Quan [IPv4]");
	}
	
	// Самостоятельно отключаемся от сервера
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		print(("[ERROR] Shutdown failed with error: " + to_string(WSAGetLastError())), FOREGROUND_RED);
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// Затираем ключи именной через эту функцию (не memset)!!!
	RtlSecureZeroMemory(shrKey, 188);
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}