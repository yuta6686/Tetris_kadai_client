// udpclient.cpp

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <winsock2.h>
#include <conio.h>
#pragma comment (lib, "Ws2_32.lib")

int main()
{
	WSADATA wsaData;
	SOCKET s;
	SOCKADDR_IN addrin;
	SOCKADDR_IN from;	//	add 
	HOSTENT* lpHostEnt;
	unsigned int addr;
	int fromlen;		//	add
	int nRtn, nNo = 5;
	u_short port;
	char szBuf[1024], szServer[64];

	printf("PORT:");
	gets_s(szBuf);
	port = atoi(szBuf);
	printf("SERVER:");
	gets_s(szServer);


	nRtn = WSAStartup(MAKEWORD(1, 1), &wsaData);
	if (nRtn != 0) {
		perror("WSAStartup Error\n");
		return -1;
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("Socket Error\n");
		WSACleanup();
		return -2;
	}
	lpHostEnt = gethostbyname(szServer);
	if (lpHostEnt == NULL) {
		addr = inet_addr(szServer);
		lpHostEnt = gethostbyaddr((char*)&addr, 4, AF_INET);
		if (lpHostEnt == NULL) {
			perror("Server not identified\n");
			_getch();
			return -3;
		}
	}


	memset(&addrin, 0, sizeof(addrin));
	memcpy(&(addrin.sin_addr),
		lpHostEnt->h_addr_list[0],
		lpHostEnt->h_length);
	addrin.sin_port = htons(port);
	addrin.sin_family = AF_INET;
	//addrin.sin_addr.s_addr = *((unsigned long *)lpHostEnt->h_addr);

	while (1) {
		printf("Send:");
		gets_s(szBuf);
		nRtn = sendto(s, szBuf, (int)strlen(szBuf) + 1, 0,
			(LPSOCKADDR)&addrin, sizeof(addrin));
		if (nRtn != (int)strlen(szBuf) + 1) {
			perror("Send Srror\n");
			closesocket(s);
			WSACleanup();
			return -4;
		}
		if (strcmp(szBuf, "end") == 0) {
			printf("èIóπÇµÇ‹Ç∑\n");
			_getch();
			break;
		}

	}
	closesocket(s);
	WSACleanup();
	return 0;

}