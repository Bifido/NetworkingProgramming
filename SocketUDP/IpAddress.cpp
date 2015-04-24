#include "IpAddress.h"
#include <Winsock2.h>

IpAddress::IpAddress(const std::string& ipAddressWhitPort)
{
	SetIpAddress(ipAddressWhitPort);
}
IpAddress::IpAddress(const std::string& ipAddress, uint16_t port)
{
	SetIpAddress(ipAddress, port);
}
IpAddress::IpAddress(uint32_t address, uint16_t port)
{
	SetIpAddress(address, port);
}
IpAddress::~IpAddress()
{}

uint32_t IpAddress::GetIpAddressForNetwork()
{
	return m_ipAddress.uiAddress;
}
uint16_t IpAddress::GetIpPortForNetwork()
{
	return m_port;
}

uint32_t IpAddress::GetIpAddressForHost()
{
	return ntohl(m_ipAddress.uiAddress);
}
uint16_t IpAddress::GetIpPortForHost()
{
	return ntohs(m_port);
}

//string must be "x.x.x.x:p"
void IpAddress::SetIpAddress(const std::string& ipAddressWhitPort)
{
	size_t index = ipAddressWhitPort.find(':');
	std::string addr = ipAddressWhitPort.substr(0, index);
	uint16_t port = atoi(ipAddressWhitPort.substr(index + 1).c_str());

	SetIpAddress(addr, port);
}
//string must be "x.x.x.x"
void IpAddress::SetIpAddress(const std::string& ipAddress, uint16_t port)
{
	size_t dotIndex;
	std::string addr = ipAddress;

	uint8_t* address[] = { &(m_ipAddress.ucByte.ucByte1), &(m_ipAddress.ucByte.ucByte2),
		&(m_ipAddress.ucByte.ucByte3), &(m_ipAddress.ucByte.ucByte4) };

	for (uint8_t i = 0; i < 4; ++i)
	{
		//Find the first dot
		dotIndex = addr.find('.');
		//Take the number before the dot and put in the ipAddress
		*(address[i]) = atoi(addr.substr(0, dotIndex).c_str());
		//Cut the readed number and dot
		addr = addr.substr(dotIndex + 1);
	}

	m_port = htons(port);
	m_ipAddress.uiAddress = htonl(m_ipAddress.uiAddress);
}

void IpAddress::SetIpAddress(uint32_t address, uint16_t port)
{
	m_port = htons(port);
	m_ipAddress.uiAddress = htonl(address);
}
