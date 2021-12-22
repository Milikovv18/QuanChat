#pragma once
#include <vector>
#include <iomanip>
#include "User.h"

// ������� ������ ������ User, ��� ��������
// ��������� ���� � ��������, ������� ����� �������
User user;

// ����� �� ���������
void c_exit(vector<string> commands)
{
	if (commands.size() == 1)
		exit(0);
	else
		print("Invalid arguments. Check help to learn how to use this command", FOREGROUND_RED);
}


// ������� �� ��������
void c_help(vector<string> commands)
{
	if (commands.size() == 1)
	{
		cout << "Command     Arguments                Description" << endl;
		cout << "-------------------------------------------------------------------------------------------" << endl;
		cout << "exit                                 Exit from Quan" << endl;
		cout << "connect     host_address [name]      Connect to a local server" << endl;
		cout << "setts       [parameter newValue]     Show current settings or modify parameter" << endl;
		cout << "             -UserName               Change your username" << endl;
		cout << "             -Password               Change password for current username" << endl;
		cout << "             -DecryptMsg             Decrypt messages from server" << endl;
		cout << "             -SoundNotify            Make \"beep\" when receives decryptable message in the background" << endl;
		cout << "             -ManualTor              Run Tor manually instead of auto running" << endl;
		cout << "test                                 Key generation and asymmetric/symmetric encryption test" << endl;
		cout << "clear                                Clear the screen and return to main menu" << endl;
	}
	else
		print("Invalid arguments. Check help to learn how to use this command", FOREGROUND_RED);
}


// ����������� ������������ � ���������� �������
bool c_connect(vector<string> commands)
{
	// ������������ � ������� �� ���������� ������
	if (commands.size() == 2)
	{
		system("cls");
		connectTo(commands[1], user.userName);
	}

	// ������������ � ������� �� ���������� ������
	// � ���������� ����, ������ ����, ������� �������� � ����������
	else if (commands.size() == 3)
	{
		system("cls");
		connectTo(commands[1], commands[2]);
	}

	else
	{
		print("Invalid arguments. Check help to learn how to use this command", FOREGROUND_RED);
		return 1;
	}
	system("cls");
	return 0;
}


// �������� �������
void c_settings(vector<string> commands)
{
	// ���� ������� ������� ��� ����������
	if (commands.size() == 1)
	{
		// ��� ������������
		cout << "       UserName:     " << user.userName << endl;

		// ������ ������������ ���� ����������, ����� NONE
		if (user.password != "")
			cout << "       Password:     " << user.password << endl;
		else
			cout << "       Password:     NONE" << endl;

		// IP ������������
		cout << "          My IP:     " << user.ip << endl;

		// ����� � ����������
		cout << "Encryption type:     SIDHp751 & RC5" << endl;

		// �������������� �� ���������
		cout << "     DecryptMsg:     " << std::boolalpha << user.decryptMsg << endl;

		// �������� �� �������� �����������, ����� ���� �� � ������
		cout << "    SoundNotify:     " << std::boolalpha << user.soundNotify << endl;

		// ���� ����-�� �� ��������, ����� ���-�� �������������� ����������� �� �� �����
		cout << "      ManualTor:     " << std::boolalpha << user.manualTor << endl;

		cout << "\n* To change bool values type \"0\" or \"1\" *" << endl;
	}

	// ���� ������� ������� � ��������������� �����������
	else if (commands.size() == 3)
	{
		// �������� ��� ������������
		if (commands[1] == "UserName")
		{
			// ��������� ��� �� ������������ �������
			for (int i(0); i < commands[2].size(); ++i)
				if (!isalnum((unsigned char)commands[2][i]))
				{
					print("UserName must contain only symbols a-z, A-Z and 0-9", FOREGROUND_RED);
					return;
				}

			user.userName = commands[2];
			cout << "Parameter UserName changed to \"" << commands[2] << "\"" << endl;
		}

		// �������� ������ ������������
		else if (commands[1] == "Password")
		{
			user.password = commands[2];
			cout << "Parameter Password changed to \"" << commands[2] << "\"" << endl;
		}

		else if (commands[1] == "DecryptMsg")
		{
			istringstream(commands[2]) >> user.decryptMsg;
			cout << "Parameter DecryptMsg changed to \"" << commands[2] << "\"" << endl;
		}

		else if (commands[1] == "SoundNotify")
		{
			istringstream(commands[2]) >> user.soundNotify;
			cout << "Parameter SoundNotify changed to \"" << commands[2] << "\"" << endl;
		}

		else if (commands[1] == "ManualTor")
		{
			istringstream(commands[2]) >> user.manualTor;
			cout << "Parameter ManualTor changed to \"" << commands[2] << "\"" << endl;
		}

		// ������ ������ �������� �� ���������
		else print("Invalid parameter \"" + commands[1] + "\"", FOREGROUND_RED);
	}
	else
		print("Invalid arguments. Check help to learn how to use this command", FOREGROUND_RED);
}


// ���� ��������� ������ � ����������
void c_test(vector<string> commands)
{
	if (commands.size() == 1)
	{
		// ���� �������������� ���������� SIDH
		bool result = !testSIDH();
		cout << "Testing asimmetric key encryption ";
		if (result)
			print("PASSED", FOREGROUND_GREEN);
		else
			print("FAILED", FOREGROUND_RED);
		cout << "\n\n";

		// ���� ������������� ���������� RC5
		result = !RC5test("Quan - it's almost easy C++ chat app, that uses newest postquantum algorithms of asimmetric encryption SIDH with 188 byte key (1504 bit). In addition, RC5 - 32 / 12 / 188 with CBC used for simmetric encryption. Quan has 2 modes of operation: simple sockets networking and Tor based.");
		cout << "Testing simmetric key encryption ";
		if (result)
			print("PASSED", FOREGROUND_GREEN);
		else
			print("FAILED", FOREGROUND_RED);
		cout << "\n\n";
	}
	else
		print("Invalid arguments. Check help to learn how to use this command", FOREGROUND_RED);
}