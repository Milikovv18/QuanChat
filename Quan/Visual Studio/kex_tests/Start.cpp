
#include <iostream>
using namespace std;

#include <Windows.h>
#include <Wininet.h>
#pragma comment(lib, "wininet.lib")

#include <sstream> // Для getline

#include "Something.h"
#include "CommandHandler.h"


// Самое начало Quan
int main()
{
	SetConsoleTextAttribute(hConsole, 15);

	// Если прога запустилась, значит все библии загружены)
	cout << "Libraries loaded" << endl;
	cout << "Checking internet connection... ";

	// Проверяем наличие связи с гуглом (подключен ли инет)
	// Если ее нет, вырубаем прогу, так как дальнейшее ее использование бессмысленно
	if (InternetCheckConnection("http://google.com", FLAG_ICC_FORCE_CONNECTION, 0))
		print("OK", FOREGROUND_GREEN);
	else
	{
		print("ERROR", FOREGROUND_RED);
		return 1;
	}

	cout << "Obtaining your IP address... ";

	// Получаем IP адрес запустившего прогу
	if (getIp())
		print("OK", FOREGROUND_GREEN);
	else
	{
		print("ERROR", FOREGROUND_RED);
		return 1;
	}

	cout << "Starting..." << endl << endl;

	// Приступаем к работе с прогой после загрузки необходимой инфы

Start:
	// Меняем название окна консоли
	SetConsoleTitleW(L"Quan");

	// Рисуем "QUAN" палочками
	SetConsoleTextAttribute(hConsole, 11);
	cout << "  ____    _     _      ___      _    _"       << endl;
	cout << " |    |  | |   | |    / _ \\    | |  / |"     << endl;
	cout << " | || |  | |   | |   / /_\\ \\   | | /  |"    << endl;
	cout << " | || \\  | |___| |  /  ___  \\  | |//| |"    << endl;
	cout << " |___\\_\\ \\_______/ /__/   \\__\\ |_ / |_|" << "   BY MILIKOVV" << "\n\n\n";
	SetConsoleTextAttribute(hConsole, 15);

	// Некоторая инфа про работу с прогой
	cout << "Your IP is " << user.ip << " Type \"help\" to learn about new commands." << endl;

	// Создаем строку, куда считываются команды
	string command;
	// И вектор, в котором хранятся части команды, разделенные пробелом
	vector<string> commands;


	// Начало цикла комманд
	while (true)
	{
		// Сбрасываем цвет шрифта на белый (стандартный)
		SetConsoleTextAttribute(hConsole, 15);
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
	

		// Вывод команд выделяется сероватым цветом
		SetConsoleTextAttribute(hConsole, 7);

		// Обработка комманд
		if (commands[0] == "exit") c_exit(commands);

		// Не выносится в отдельную функцию из-за goto
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

		// Если введенная команда не совапала ни с одной из списка выше
		else print("Command \"" + commands[0] + "\" does not exist!", FOREGROUND_RED);

		cout << endl;

	}

	cout << endl; return 0;
}