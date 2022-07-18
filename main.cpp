// udpclient.cpp

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <winsock2.h>
#include <conio.h>
#include <string>
#pragma comment (lib, "Ws2_32.lib")

using namespace std;

#define PORT_NUMBER 55555
#define HOST_NAME "Wths00680"


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
	string szServerstr;

	//	最初のポート番号とか、ホスト名は
	//	いちいち書くのがめんどいから、マクロにしただけ
	{
		//printf("PORT:");
		//gets_s(szBuf);	
		//port = atoi(szBuf);	
		//printf("SERVER:");
	}

	
	printf("Press Any Key:");
	gets_s(szServer);
	//szServerstr = szServer;	

	port = PORT_NUMBER;
	szServerstr = HOST_NAME;

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

	//	string 型の方のszServerstrを.c_str()でcharにする。
	lpHostEnt = gethostbyname(szServerstr.c_str());
	if (lpHostEnt == NULL) {

		addr = inet_addr(szServerstr.c_str());	//	ここも
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


	//	add
	//	ノンブロッキング処理
	u_long val = 1;
	ioctlsocket(s, FIONBIO, &val);

	while (1) {
		//	recvfrom
		{
			fromlen = (int)sizeof(from);
			nRtn = recvfrom(s,
				szBuf,
				(int)sizeof(szBuf) - 1,
				0,
				(SOCKADDR*)&from,
				&fromlen);

			//	add
			if (nRtn < 1) {
				if (errno == EAGAIN) {
					printf("MADA KONAI\n");
				}
			}
			else {
				printf("Recv:");
				//printf("%s>%s\n", inet_ntoa(from.sin_addr), szBuf);
				printf("%s\n", szBuf);
				szBuf[nRtn] = '\0';
			}

			if (strcmp(szBuf, "end") == 0) {
				printf(" Terminate Server\n");
				break;
			}
		}

		printf("Send:");
		{		
			//	こいつはブロックしてくる。
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
				printf("終了します\n");
				_getch();
				break;
			}
		}


		
	}
	closesocket(s);
	WSACleanup();
	return 0;

}