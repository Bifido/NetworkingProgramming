#pragma once
#include <string>
#include <stdint.h>


class IpAddress
{
public:
	//string must be "x.x.x.x:p"
	IpAddress(const std::string& ipAddressWhitPort);
	//string must be "x.x.x.x"
	IpAddress(const std::string& ipAddress, uint16_t port);
	IpAddress(uint32_t address, uint16_t port);
	~IpAddress();

	//Return the ip and port in network format big-endian
	uint32_t GetIpAddressForNetwork();
	uint16_t GetIpPortForNetwork();
	//Return the ip and port in host format
	//big-endian or little-endian in base of host system
	uint32_t GetIpAddressForHost();
	uint16_t GetIpPortForHost();

	//string must be "x.x.x.x:p"
	void SetIpAddress(const std::string& ipAddressWhitPort);
	//string must be "x.x.x.x"
	void SetIpAddress(const std::string& ipAddress, uint16_t port);
	void SetIpAddress(uint32_t address, uint16_t port);

private:
	typedef struct in_address{
		union
		{
			struct{
				uint8_t ucByte1, ucByte2, ucByte3, ucByte4;
			}ucByte;
			uint32_t uiAddress;
		};
	};

	in_address m_ipAddress;
	uint16_t m_port;
};

