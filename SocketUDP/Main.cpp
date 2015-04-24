//apre socket udp sulla porta 12500(invio) 12501 (ricezione)
//invia messaggio "ciao"
//chiude socket
#include <ws2def.h>
//#include <winsock.h>
#include <windows.networking.sockets.h>
#include <stdio.h>

int main()
{
	WSADATA wsaData;
	int err;
	SOCKET sock = INVALID_SOCKET;

	err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err != 0) {
		wprintf(L"WSAStartup failed: %d\n", err);
		return 1;
	}
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sock == INVALID_SOCKET)
		wprintf(L"socket function failed with error = %d\n", WSAGetLastError());
	else {
		wprintf(L"socket function succeeded\n");


		// Close the socket to release the resources associated
		// Normally an application calls shutdown() before closesocket 
		//   to  disables sends or receives on a socket first
		// This isn't needed in this simple sample
		err = closesocket(sock);
		if (err == SOCKET_ERROR) {
			wprintf(L"closesocket failed with error = %d\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
	}

	WSACleanup();

	return 0;
	

	/*
		*****int sock = ....

		struct INEDADDR{
			int port;
			union{
				int
				int
				int
				int
			}address
		}

		*****INEDADDR addr;
		addr.port = 12500 non va bene perche' vuole il numero in bigendian
		*****addr.port = HTONS(12500); converte il formato dalla macchina host alla tipologia voluta dal network

		HTONS(x){
			return x; se e' gia' bigendian
		}
		else
		{
			x inverte i byte in bigendian
		}
		bigendian
		|31..24|23..16|15..8|7..0|
		littleendian
		|0..7|8..15| per i 16 bit
		|0..7|8..15|24..31|16..23| per i 32 bit

		dal formato host al formato network
		HTONS per i short
		HTONL per i long
		HTONLL per i long long (non tutti ce l'hanno)

		dal formato network al formato host
		NTOHS per i short
		NTOHL per i long
		NTOHLL per i long long (non tutti ce l'hanno)

		ochhio quando si fa int= 1200;
		e poi char* che il risultato sara' diverso

		? ip = 127.0.0.1 -> da shiftare per memorizzarla in un intero
		*****addr.addr = HTONL(ip);
		addr.addr = "localhost"; da convertire da stringa a numerico e poi con HTONL
		addr.addr = "127.0.0.1"; da convertire da stringa a numerico e poi con HTONL


		*****bind(sock, (inedaddr)addr); collega la socket ad un certo indirizzo
		possibili problemi: porta gia' in uso ed errori vari
		riprovare con un altra porta oppure settare la porta in modo da riusarla in parallelo
		SO_TCPNODELAY per togliere il delay del tcp
		SO_RECIVETIMEOUT per risettare il timeout di default
		SO_RECIVEBUFFER per allargare il buffer
		SO_NONBLOCK socket bloccanti o no

		*****Sendto(sock,byte,dest,out); funzione da usare per udp, funzione non bloccante tranne quando il buffer e' pieno
		*****reciveFrom(sock,buffer,sizemax, (*)from) from di tipo INEDADDR e torna il numero di byte letti

		*/


};