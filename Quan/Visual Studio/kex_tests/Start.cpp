
#include <iostream>
using namespace std;

#include <Windows.h>
#include <Wininet.h>
#pragma comment(lib, "wininet.lib")

#include <sstream> // ��� getline

#include "Something.h"
#include "CommandHandler.h"


// ����� ������ Quan
int main()
{
	SetConsoleTextAttribute(hConsole, 15);

	// ���� ����� �����������, ������ ��� ������ ���������)
	cout << "Libraries loaded" << endl;
	cout << "Checking internet connection... ";

	// ��������� ������� ����� � ������ (��������� �� ����)
	// ���� �� ���, �������� �����, ��� ��� ���������� �� ������������� ������������
	if (InternetCheckConnection("http://google.com", FLAG_ICC_FORCE_CONNECTION, 0))
		print("OK", FOREGROUND_GREEN);
	else
	{
		print("ERROR", FOREGROUND_RED);
		return 1;
	}

	cout << "Obtaining your IP address... ";

	// �������� IP ����� ������������ �����
	if (getIp())
		print("OK", FOREGROUND_GREEN);
	else
	{
		print("ERROR", FOREGROUND_RED);
		return 1;
	}

	cout << "Starting..." << endl << endl;

	// ���������� � ������ � ������ ����� �������� ����������� ����

Start:
	// ������ �������� ���� �������
	SetConsoleTitleW(L"Quan");

	// ������ "QUAN" ���������
	SetConsoleTextAttribute(hConsole, 11);
	cout << "  ____    _     _      ___      _    _"       << endl;
	cout << " |    |  | |   | |    / _ \\    | |  / |"     << endl;
	cout << " | || |  | |   | |   / /_\\ \\   | | /  |"    << endl;
	cout << " | || \\  | |___| |  /  ___  \\  | |//| |"    << endl;
	cout << " |___\\_\\ \\_______/ /__/   \\__\\ |_ / |_|" << "   BY MILIKOVV" << "\n\n\n";
	SetConsoleTextAttribute(hConsole, 15);

	// ��������� ���� ��� ������ � ������
	cout << "Your IP is " << user.ip << " Type \"help\" to learn about new commands." << endl;

	// ������� ������, ���� ����������� �������
	string command;
	// � ������, � ������� �������� ����� �������, ����������� ��������
	vector<string> commands;


	// ������ ����� �������
	while (true)
	{
		// ���������� ���� ������ �� ����� (�����������)
		SetConsoleTextAttribute(hConsole, 15);
		// ������� ������, ��� ������ � ���� ����� ������
		commands.clear();

		// ���������� �������
		cout << "\r>> ";
		getline(cin, command);
		commands.push_back("");
		// ��������� ������ � �������� �� ���������� � ���������� �� � ������
		for (int i(0), j(0); i < command.size(); ++i)
		{
			if (command[i] == ' ')
			{
				commands.push_back("");
				++j; continue;
			}
			commands[j] += command[i];
		}
		// ���� ������� ������, ������������ � ������ �����
		if (commands[0] == "") continue;
		cout << endl;
	

		// ����� ������ ���������� ��������� ������
		SetConsoleTextAttribute(hConsole, 7);

		// ��������� �������
		if (commands[0] == "exit") c_exit(commands);

		// �� ��������� � ��������� ������� ��-�� goto
		else if (commands[0] == "clear")
		{
			if (commands.size() == 1)
			{
				system("cls");
				goto Start;
			}
			else
				print("Invalid arguments. Check help to learn how to use this command", FOREGROUND_RED);
		}

		else if (commands[0] == "help") c_help(commands);

		else if (commands[0] == "connect") { if (!c_connect(commands)) goto Start; }

		else if (commands[0] == "setts") c_settings(commands);

		else if (commands[0] == "test") c_test(commands);

		// ���� ��������� ������� �� �������� �� � ����� �� ������ ����
		else print("Command \"" + commands[0] + "\" does not exist!", FOREGROUND_RED);

		cout << endl;

	}

	cout << endl; return 0;
}