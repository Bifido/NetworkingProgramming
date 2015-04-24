#pragma once

#include <windows.networking.sockets.h>
#include "IpAddress.h"

class Socket
{
public:
	Socket(int af, int type, int protocol);
	~Socket();

	void Connect(IpAddress ipAddress);

private:
	SOCKET m_socket;
	int m_af;
	int m_type;
	int m_protocol;

	sockaddr a;
	sockaddr_in b;
};

Socket::Socket(int af, int type, int protocol)
	:m_af(af), m_type(type), m_protocol(protocol)
{
	
}

