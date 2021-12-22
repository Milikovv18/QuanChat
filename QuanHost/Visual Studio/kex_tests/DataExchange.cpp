#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <WS2tcpip.h>
#include <winsock2.h>
#pragma comment(lib,"wsock32.lib")
#pragma comment (lib,"Ws2_32.lib")

#include "Client.h"
#include "TorCode/socks.h"

// Стандартная длина буфера для отправки/приема сообщений
#define BUFFER_SIZE 2048

void print(std::wstring text, WORD style, bool newStr = true);
int decryptMsg(unsigned char key[188], std::string &msg);
int encryptMsg(unsigned char key[188], std::string& msg);

extern Client clients[10];
extern bool connected[MAX_CLIENTS + 1];
extern int connected_int;
extern std::ofstream mesFile;


// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}
// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string& str) {
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}


int ban(SOCKET socket)
{
	int iResult = shutdown(socket, 2);
	closesocket(socket);
	if (iResult == SOCKET_ERROR) {
		print(L"shutdown failed with error: " + std::to_wstring(WSAGetLastError()), FOREGROUND_RED);
		return 1;
	}

	return 0;
}

// Переменные для работы с файлами
__int64 filesize;
std::string filename;

void fileSend(int id, std::string author)
{
	std::string filename_toSend = author + "|" + filename + "|" + std::to_string(filesize);

	int size = encryptMsg(clients[id].key, filename_toSend);
	send(clients[id].socket, ("FI" + filename_toSend).c_str(), size + 2, 0);

	FILE* in = fopen(("Downloads\\" + filename).c_str(), "rb");

	if (in)
	{
		unsigned char buff[2000]; __int64 sent(0);

		while (sent < filesize)
		{
			clients[id].fileA = -1;

			int s = fread(buff, 1, sizeof buff, in);
			std::string filepart((char*)buff, s);

			size = encryptMsg(clients[id].key, filepart);
			send(clients[id].socket, ("F" + filepart).c_str(), size + 1, 0);

			sent += s;

			clock_t timeout = clock();
			while (clock() <= timeout + 5 && clients[id].fileA == -1) {}

			if (clock() <= timeout + 5)
			{
				clients[id].fileA = -1;
				if (fclose(in)) perror("close input file");
				in = nullptr;
				return;
			}

			if (!clients[id].fileA) break;
		}

		clients[id].fileA = -1;
		if (fclose(in)) perror("close input file");
		in = nullptr;
	}
}


FILE* out;
clock_t timeout(0);
void fileInit(int id, std::string &file_string)
{
	if (clock() <= timeout + 5)
		return;
	else
		timeout = 0;

	filename.clear();
	decryptMsg(clients[id].key, file_string);

	int i(-1);
	while (file_string[++i] != '|')
		filename.append(1, file_string[i]);
	
	file_string.erase(0, ++i);
	filesize = atoi(file_string.c_str());

	out = fopen(("Downloads\\" + filename).c_str(), "wb");

	if (out)
		mesFile << clients[id].name << " sending a " << filesize / 1024 / 1024.0 << " megabyte file " << filename << std::endl;
	else
		mesFile << clients[id].name << " tried to send a " << filesize / 1024 / 1024.0 << " megabyte file " << filename << " but it cant be opened(" << std::endl;
}

// Строка накопления данных, если пакет порвался
std::string funded_str = "";
__int64 wrote(0);
void fileRecv(int id,  std::string& file_msg)
{
	if (out)
	{
		funded_str += file_msg;
		if (funded_str.size() != 2040 && filesize - wrote >= 2000)
			return;

		decryptMsg(clients[id].key, funded_str);
		wrote += fwrite(funded_str.c_str(), 1, funded_str.size(), out);
		funded_str.clear();
		
		timeout = clock();
		char OK[]{ "FA" };
		send(clients[id].socket, OK, 2, 0);

		if (wrote >= filesize)
		{
			wrote = 0;
			fclose(out);
			out = nullptr;

			for (int i(0); i < MAX_CLIENTS; ++i)
				if (connected[i] && clients[i].name != clients[id].name)
				{
					clients[i].fileThread = std::thread(fileSend, i, clients[id].name);
					clients[i].fileThread.detach();
				}
		}
	}
}


void broadcast(int id, std::string msg)
{
	if (id != -1)
	{
		msg = clients[id].name + ": " + msg;

		if (clients[id].authenticated)
			msg = "[A]" + msg;
	}

	for (int i(0); i < MAX_CLIENTS; ++i)
	{
		std::string encrMsg = msg;

		if (connected[i] && i != id)
		{
			int size = encryptMsg(clients[i].key, encrMsg);
			send(clients[i].socket, encrMsg.c_str(), size, 0);
		}
	}
}


// Получаем сообщения с сервера
void clientRecv(int id)
{
	// Сюда будет приходить сообщение
	char data[BUFFER_SIZE]{};
	int iResult;

	// Выводим сообщение о подключении пользователя в файл
	mesFile << "----------" << clients[id].name << " connected----------" << std::endl;
	mesFile.flush();
	broadcast(-1, " --- " + clients[id].name + " connected ---");

	// Читаем пока не будет разорвано соединение
	do
	{
		// Очищаем буфер для приема новых данных
		RtlSecureZeroMemory(data, sizeof(data));
		// Получаем само сообщение с IP адреса сервера
		iResult = recv(clients[id].socket, data, BUFFER_SIZE, 0);
		// Если получено больше 0 байт
		if (iResult > 0)
		{
			// Создаем строку и помещаем туда данные из буфера
			std::string msg_s(iResult, '\0');
			memcpy(&msg_s[0], data, iResult);

			// Проверяем сообщение на пресылку файла
			if (msg_s[0] == 'F')
			{
				// File acceptance
				if (msg_s[1] == 'A')
					clients[id].fileA = true;

				// File denial
				else if (msg_s[1] == 'D')
					clients[id].fileA = false;

				// Если это инициализция пересылки нового файла
				else if (msg_s[1] == 'I')
					fileInit(id, msg_s.substr(2, msg_s.size()));

				// Если это продолжение пересылки старого файла
				else
					fileRecv(id, msg_s.substr(1, msg_s.size()));

				continue;
			}
			else if (msg_s[0] != 'M')
				fileRecv(id, msg_s);

			// Пробуем расшифровать сообщение
			// иначе выводим ошибку
			if (!decryptMsg(clients[id].key, msg_s))
			{
				mesFile << clients[id].name << ": Can't decrypt message, wrong key" << std::endl;
				mesFile.flush();
				continue;
			}

			// Рассылаем сообщение остальным клиетам
			broadcast(id, msg_s);

			// Подготавливаем сообщение для вывода в файл
			OemToCharBuffA(msg_s.c_str(), &msg_s[0], msg_s.size());

			// Выводим сообщение в файл
			mesFile << clients[id].name << ": " << msg_s << std::endl;
			mesFile.flush();
		}
		else if (iResult <= 0)
		{
			--connected_int;
			connected[id] = false;

			mesFile << "----------" << clients[id].name << " disconnected----------" << std::endl;
			mesFile.flush();
			broadcast(-1, " --- " + clients[id].name + " disconnected ---");

			// Очищаем всю инфу о пользователе
			clients[id].name.clear();
			clients[id].socket = 0;
			ZeroMemory(clients[id].key, 188);
			clients[id].authenticated = false;
			clients[id].ip.clear();
			clients[id].thread = std::thread();
			clients[id].fileThread = std::thread();
			clients[id].fileA = false;
		}

	} while (iResult > 0);  // Здесь iResult < 0 говорит о разорванном соединении
}