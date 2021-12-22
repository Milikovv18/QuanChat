#pragma once
#include <vector>
#include <iomanip>
#include <vector>
#include <string>


using namespace std;

void print(string text, WORD style, bool newStr = true);
void print(wstring text, WORD style, bool newStr = true);

extern struct Client clients[MAX_CLIENTS];
extern bool connected[MAX_CLIENTS + 1];
extern int connected_int;

// Выход из программы
void c_exit(vector<string> commands)
{
	if (commands.size() == 1)
	{
		for (int i(0); i < MAX_CLIENTS; ++i)
			if (connected[i])
				shutdown(clients[i].socket, 2);
		exit(0);
	}
	else
		print("Invalid arguments. Check help to learn how to use this command", FOREGROUND_RED, false);
}


// Справка по командам
void c_help(vector<string> commands)
{
	if (commands.size() != 1)
	{
		print("Invalid arguments. Check help to learn how to use this command", FOREGROUND_RED, false);
		return;
	}

	cout << "Command     Description" << endl;
	cout << "---------------------------------------------------------------------------------------" << endl;
	cout << "stop        Close all sockets and stop the server" << endl;
	cout << "stat        Show list of connected users with their names, ips, sockets and auth states" << endl;
	cout << "ban         Ban users by their names (enter names, separated by spaces)" << endl;
	cout << "banall      Ban all users connected to server" << endl;
}


void c_stat(vector<string> commands)
{
	if (commands.size() != 1)
	{
		print("Invalid arguments. Check help to learn how to use this command", FOREGROUND_RED, false);
		return;
	}

	if (connected_int == 0)
	{
		cout << "No users connected(";
		return;
	}

	cout << " #   UserName        isAuth     IPv4            Socket" << endl;
	cout << "--------------------------------------------------------" << endl;

	for (int i(0), j(0); i < MAX_CLIENTS; ++i)
		if (connected[i])
		{
			cout << setw(2) << i << "   " << left << setw(15) << clients[i].name.substr(0, 15) << " " << setw(6) << boolalpha << clients[i].authenticated << "     " << setw(15) << clients[i].ip << " " << right << setw(6) << clients[i].socket << endl;
			++j;
		}
}


int ban(SOCKET socket);
void c_ban(vector<string> commands)
{
	if (commands.size() < 2)
	{
		print("Invalid arguments. Check help to learn how to use this command", FOREGROUND_RED, false);
		return;
	}

	for (int num(1); num < commands.size(); ++num)
	{
		if (num > 1) cout << endl;

		int i(0);
		for (; i < MAX_CLIENTS; ++i)
			if (connected[i] && commands[num] == clients[i].name) break;

		if (i == MAX_CLIENTS)
		{
			print("User " + commands[num] + " not connected to the server", FOREGROUND_RED, false);
			continue;
		}

		ban(clients[i].socket);
		cout << "User " << commands[num] << " has been banned from this server";
	}
}


void c_banall(vector<string> commands)
{
	if (commands.size() != 1)
	{
		print("Invalid arguments. Check help to learn how to use this command", FOREGROUND_RED, false);
		return;
	}

	if (!connected_int)
	{
		print("No users are connected", FOREGROUND_RED, false);
		return;
	}

	for (int i(0); i < MAX_CLIENTS; ++i)
		if (connected[i])
		{
			cout << "User " << clients[i].name << " has been banned from this server";
			ban(clients[i].socket);
		}
}


extern string filename;
extern __int64 filesize;
extern __int64 wrote;
void c_filestat(vector<string> commands)
{
	if (commands.size() != 1)
	{
		print("Invalid arguments. Check help to learn how to use this command", FOREGROUND_RED, false);
		return;
	}

	if (wrote)
		cout << "Downloading file " << filename << " " << wrote / filesize * 100 << "% -- " << wrote << "/" << filesize;
	else
		cout << "No files downloading now";
}


void mainHandler()
{
	// Создаем строку, куда считываются команды
	string command;
	// И вектор, в котором хранятся части команды, разделенные пробелом
	vector<string> commands;

	while (true)
	{
		// Очищаем вектор, для записи в него новых команд
		commands.clear();

		// Считывание комманд
		cout << "\r>> ";
		getline(cin, command);
		commands.push_back("");
		// Разбиваем строку с командой на подкоманды и засовываем их в вектор
		for (int i(0), j(0); i < command.size(); ++i)
		{
			if (command[i] == ' ')
			{
				commands.push_back("");
				++j; continue;
			}
			commands[j] += command[i];
		}
		// Если команда пустая, возвращаемся в начало цикла
		if (commands[0] == "") continue;
		cout << endl;


		// Обработка комманд
		if (commands[0] == "stop") c_exit(commands);

		else if (commands[0] == "help") c_help(commands);

		else if (commands[0] == "stat") c_stat(commands);

		else if (commands[0] == "ban") c_ban(commands);

		else if (commands[0] == "banall") c_banall(commands);

		else if (commands[0] == "filestat") c_filestat(commands);

		// Если введенная команда не совапала ни с одной из списка выше
		else print("Command \"" + commands[0] + "\" does not exist!", FOREGROUND_RED, false);

		cout << endl << endl;
	}
}