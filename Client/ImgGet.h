#pragma once
#include <winsock2.h>
#include <windows.h> 
#include <wlanapi.h>
#include <iostream>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")

class ImgGet
{
public:
	ImgGet(const std::string &ip, int port);

	~ImgGet();

	void close();

	std::string  Recv();

	int init();

private:
	std::string  m_ip;
	int m_port;
	SOCKET m_socket;
	char *m_data;

};
