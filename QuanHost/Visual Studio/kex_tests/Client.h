#pragma once

#include <string>
#include <thread>

#define MAX_CLIENTS 10

struct Client {
	std::string name;
	int socket;
	unsigned char key[188]{};
	bool authenticated = false;
	std::string ip;
	std::thread thread;
	std::thread fileThread;
	int fileA = -1;
};