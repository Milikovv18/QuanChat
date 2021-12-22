#pragma once
#include <string>

// Структура содержащая инфу о клиенте
struct User
{
	std::string userName = "DefaultUser";  // Имя пользователя
	std::string password = "";             // Пароль пользователя
	std::string ip = "127.0.0.1";          // IP адрес пользователя
	bool decryptMsg = true;                // Расшифровывать ли полученные сообщения
	bool soundNotify = true;               // Включены ли звуковые уведомления, когда окно не в фокусе
	bool manualTor = false;                // Запуск Тора вручную
	bool authenticated = false;            // Пользователь зарегистрирован на сервере
	//bool tor = false;
};