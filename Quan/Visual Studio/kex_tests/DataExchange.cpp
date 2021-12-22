#pragma once

#include <string>
#include <iostream>
#include <conio.h>
#include <WS2tcpip.h>
#include <winsock2.h>
#pragma comment(lib,"wsock32.lib")
#pragma comment (lib,"Ws2_32.lib")

#include "User.h"
#include "TorCode/socks.h"

// Стандартная длина буфера для отправки/приема сообщений
#define BUFFER_SIZE 2048

void print(std::string text, WORD style, bool newStr = true);
void print(std::wstring text, WORD style, bool newStr = true);
int decryptMsg(std::string &msg);

// И снова extern объект структуры User
extern struct User user;

extern bool fileA;

HWND consolehwnd = GetConsoleWindow();
int unreaddies(0);


extern std::string msg;
extern bool insertDenial;
void insertStr(std::string userName, std::string str, WORD color = 15, bool auth = false)
{
	while (insertDenial) {};

	if (auth)
		print("\r[A]", FOREGROUND_BLUE, false);
	else
		std::cout << "\r";

	print(userName, 14, false);
	print(str, color, false);
	for (int i(0); i < userName.size() + msg.size() + 5; ++i)
		std::cout << " ";

	if (user.authenticated)
		print("\n\r[A]", FOREGROUND_BLUE, false);
	else
		std::cout << "\n\r";

	print(user.userName + ": ", 14, false);
	std::cout << msg;
}


FILE* out;
__int64 filesize;
int accept_g(-1);
void fileInit(std::string& file_string)
{
	decryptMsg(file_string);

	std::string username;
	std::string filename;

	int i(-1);
	while (file_string[++i] != '|')
		username.append(1, file_string[i]);

	while (file_string[++i] != '|')
		filename.append(1, file_string[i]);

	file_string.erase(0, ++i);
	filesize = atoi(file_string.c_str());

	insertStr("", username + " sending a " + std::to_string(filesize / 1024 / 1024.0) + "MB file \"" + filename + "\". Type \"/a\" to download it or \"/d\" to deny.", 14);

	if (out) fclose(out);

	while (accept_g == -1) {}

	if (accept_g == 1)
	{
		fopen_s(&out, ("Downloads\\" + filename).c_str(), "wb");

		if (out)
			insertStr("", "[NOTICE] Downloading a " + std::to_string(filesize / 1024 / 1024.0) + "MB file \"" + filename + "\" from " + username, 14);
		else
			insertStr("", "[ERRORR] A " + std::to_string(filesize / 1024 / 1024.0) + "MB file \"" + filename + "\" from " + username + " cant be saved(", FOREGROUND_RED);
	}
	else
		insertStr("", "\r[NOTICE] File \"" + filename + "\" from " + username + " rejected", FOREGROUND_RED);
}

__int64 wrote(0);
void fileRecv(int sock, std::string& file_msg)
{
	char OK[]  { "FA" };
	char FAIL[]{ "FD" };
	if (out)
	{
		decryptMsg(file_msg);
		wrote += fwrite(file_msg.c_str(), 1, file_msg.size(), out);
		send(sock, OK, 2, 0);

		if (wrote >= filesize)
		{
			wrote = 0;
			fclose(out);
			out = nullptr;
			insertStr("", "[NOTICE] Download finished", FOREGROUND_GREEN);
		}
	}
	else send(sock, FAIL, 2, 0);
}


// Получаем сообщения с .onion сервера
bool aborted(false);
void TORrecv(socks5cpp::SocksClient sockCli, SOCKET sock)
{
	// Сюда будет приходить сообщение
	char data[BUFFER_SIZE]{};
	int iResult;

	// Читаем пока не будет разорвано соединение
	do
	{
		// Очищаем буфер для приема новых данных
		RtlSecureZeroMemory(data, BUFFER_SIZE);
		// Получаем само сообщение с .onion сервера
		iResult = sockCli.recvPacket(sock, data, BUFFER_SIZE);
		// Если получено больше 0 байт
		if (iResult > 0)
		{
			// Создаем строку и помещаем туда данные из буфера
			std::string msg_s(iResult, '\0');
			memcpy(&msg_s[0], data, iResult);

			if (msg_s[0] == 'F')
			{
				// File acceptance
				if (msg_s[1] == 'A')
					fileA = true;

				// File initialization
				else if (msg_s[1] == 'I')
					fileInit(msg_s.erase(0, 1));

				// File piece of file
				else
					fileRecv(int(sock), msg_s.erase(0));

				continue;
			}

			// Пробуем расшифровать сообщение
			// иначе выводим ошибку
			int size = decryptMsg(msg_s);
			if (!(size) && user.decryptMsg)
			{
				insertStr("", "[ERRORR] Can't decrypt message, wrong key", FOREGROUND_RED);
				continue;
			}

			// Если пользователь зарегистрирован, выделим это цветом
			int i(0);
			if (msg_s[0] == '[')
			{
				print("[A]", FOREGROUND_BLUE, false);
				i = 3;
			}

			// Выделяем имя из сообщения
			std::string userName = "";
			for (; i < size && msg_s[i] != ' '; ++i)
				userName += msg_s[i];
			msg_s.erase(0, i);

			// Выводим сообщение в консоль
			insertStr(userName, msg_s);

			// Звуквое уведомление, если quan вне фокуса
			HWND currenthwnd = GetForegroundWindow();
			if (currenthwnd != consolehwnd)
			{
				if (user.soundNotify)
				{
					Beep(300, 80);
					Beep(400, 80);
				}
				++unreaddies;
				SetConsoleTitleW((L"Quan | " + std::to_wstring(unreaddies) + L" unreaddies").c_str());
			}
			else
			{
				unreaddies = 0;
				SetConsoleTitleW(L"Quan [Tor]");
			}
		}
		// Иначе, если новых сообщений нет (iResult == -1),
		// проверяем на наличие этой ошибки и продолжаем читать дальше
		else if (WSAGetLastError() == 10035)
			continue;

	} while (iResult && !aborted);  // Здесь iResult < 0 говорит что все хорошо

	aborted = false;
}


// Получаем сообщения с сервера
void IPrecv(SOCKET socket)
{
	// Сюда будет приходить сообщение
	char data[BUFFER_SIZE]{};
	int iResult;

	// Читаем пока не будет разорвано соединение
	do
	{
		// Очищаем буфер для приема новых данных
		RtlSecureZeroMemory(data, sizeof(data));
		// Получаем само сообщение с IP адреса сервера
		iResult = recv(socket, data, BUFFER_SIZE, 0);
		// Если получено больше 0 байт
		if (iResult > 0)
		{
			// Создаем строку и помещаем туда данные из буфера
			std::string msg_s(iResult, '\0');
			memcpy(&msg_s[0], data, iResult);

			if (msg_s[0] == 'F')
			{
				// File acceptance
				if (msg_s[1] == 'A')
					fileA = true;

				// File initialization
				else if (msg_s[1] == 'I')
					fileInit(msg_s.erase(0, 1));

				// File piece of file
				else
					fileRecv(int(socket), msg_s.erase(0));

				continue;
			}

			// Пробуем расшифровать сообщение
			// иначе выводим ошибку
			int size = decryptMsg(msg_s);
			if (!(size) && user.decryptMsg)
			{
				insertStr("", "[ERRORR] Can't decrypt message, wrong key", FOREGROUND_RED);
				continue;
			}

			// Если пользователь зарегистрирован, выделим это цветом
			int i(0);
			bool auth(false);
			if (msg_s[0] == '[')
			{
				auth = true;
				i = 3;
			}

			// Выделяем имя из сообщения
			std::string userName = "";
			for (; i < size && msg_s[i] != ' '; ++i)
				userName += msg_s[i];
			msg_s.erase(0, i);

			// Выводим сообщение в консоль
			insertStr(userName, msg_s, 15, auth);

			// Звуквое уведомление, если quan вне фокуса
			HWND currenthwnd = GetForegroundWindow();
			if (currenthwnd != consolehwnd)
			{
				if (user.soundNotify)
				{
					Beep(300, 80);
					Beep(400, 80);
				}
				++unreaddies;
				SetConsoleTitleW((L"Quan | " + std::to_wstring(unreaddies) + L" unreaddies").c_str());
			}
			else
			{
				unreaddies = 0;
				SetConsoleTitleW(L"Quan [IPv4]");
			}

		}
		else if (iResult == 0)
			insertStr("", "[NOTICE] Connection closed", FOREGROUND_RED);

	} while (iResult > 0);  // Здесь iResult < 0 говорит о разорванном соединении
}