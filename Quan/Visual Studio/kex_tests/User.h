#pragma once
#include <string>

// ��������� ���������� ���� � �������
struct User
{
	std::string userName = "DefaultUser";  // ��� ������������
	std::string password = "";             // ������ ������������
	std::string ip = "127.0.0.1";          // IP ����� ������������
	bool decryptMsg = true;                // �������������� �� ���������� ���������
	bool soundNotify = true;               // �������� �� �������� �����������, ����� ���� �� � ������
	bool manualTor = false;                // ������ ���� �������
	bool authenticated = false;            // ������������ ��������������� �� �������
	//bool tor = false;
};