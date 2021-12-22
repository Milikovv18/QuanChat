
#include <iostream>
using namespace std;

#include <Windows.h>
#include <Wininet.h>
#pragma comment(lib, "wininet.lib")

#include <sstream> // ��� getline

#include "Something.h"

int startIP(string IP_ADDRESS = "");
int startTor();
int getLocalIPs();

string ip;


// ����� ������ Quan
int main()
{
	// ������ �������� ���� �������
	SetConsoleTitleW(L"QuanHost");

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


	// ������ "QUAN" ���������
	SetConsoleTextAttribute(hConsole, 13);
	cout << "  ____    _     _   ____   ____   _______  "       << endl;
	cout << " |    |  | |   | | |    | |  __| |__   __|"     << endl;
	cout << " | || |  | |___| | | || | | |__     | |     " << endl;
	cout << " | || \\  |  ___  | | || | |__  |    | |     "    << endl;
	cout << " |___\\_\\ |_|   |_| |____| |____|    |_|" << "   BY MILIKOVV" << "\n\n\n";
	SetConsoleTextAttribute(hConsole, 15);

	// ��������� ���� ��� ������ � ������
	cout << "Your external IP is " << ip << " ("; getLocalIPs(); cout << ")\n\n";

	cout << "Start as Tor hidden service? [y/n] ";
	char wtf = (char)_getch(); cout << wtf;

	cout << "\n\n--------------------------------------------------------------------------" << endl << endl;;

	if (wtf == 'y')
		startTor();

	else if (wtf == 'n')
		startIP();

	else
		print("Server not started, exiting", FOREGROUND_RED);
	
	cin.ignore();
	return 0;
}