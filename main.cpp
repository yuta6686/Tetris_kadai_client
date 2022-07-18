// udpclient.cpp

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <winsock2.h>
#include <conio.h>
#include <time.h>
#include <string>
#pragma comment (lib, "Ws2_32.lib")

using namespace std;

#define PORT_NUMBER 55555
#define HOST_NAME "Wths00680"


//	Tetris���̃}�N���R�s�[  
#define FIELD_H         20      // �t�B�[���h�̍���
#define FIELD_W         10      // �t�B�[���h�̕�
#define FIELD_X         5       // �t�B�[���h�\�����ʒu
#define FIELD_Y         2       // �t�B�[���h�\���s�ʒu
#define GAME_CLEAR_PONT 100     // �Q�[���N���A�ɂȂ链�_
#define EXPAND_POINT    4       // �A���폜���̓_���̊g�嗦
#define FALL_TIMING     300     // �u���b�N��������������^�C�~���O(m�b)

#define CURSOR_ERASE	printf("\033[?25l")		// �J�[�\������
#define CURSOR_DISPLAY	printf("\033[?25h")		// �J�[�\���\��
#define COLOR_RESET		printf("\033[0m")		// �F�ݒ�̃��Z�b�g
#define WINDOW_CLEAR	printf("\033[2J")		// ��ʃN���A

#define BLACK			0
#define DARK_RED		1
#define DARK_GREEN		2
#define DARK_YELLOW		3
#define DARK_BLUE		4
#define DARK_MAGENTA	5
#define DARK_CYAN		6
#define LIGHT_GRAY		7
#define DARK_GRAY		8
#define LIGHT_RED		9
#define LIGHT_GREEN		10
#define LIGHT_YELLOW	11
#define LIGHT_BLUE		12
#define LIGHT_MAGENTA	13
#define LIGHT_CYAN		14
#define WHITE			15

#define BLOCK_SIZE		4
#define BLOCK_TYPE		7
#define EMPTY           -1
#define PIECE0          DARK_RED
#define PIECE1          DARK_GREEN
#define PIECE2          DARK_YELLOW
#define PIECE3          DARK_BLUE
#define PIECE4          DARK_MAGENTA
#define PIECE5          DARK_CYAN
#define PIECE6          LIGHT_GRAY
#define PIECE_DEL       BLACK           // �폜���̃s�[�X�̐F

//	Tetris���̃O���[�o���ϐ��R�s�[
int     Field[FIELD_H][FIELD_W];        // �t�B�[���h
int     Block[BLOCK_SIZE][BLOCK_SIZE];  // �\������u���b�N
int     Block_X, Block_Y;   // �u���b�N�ʒu(�t�B�[���h�����Έʒu)
int     Point = 0;          // ���_


//	�v���g�^�C�v�錾
int input_key(void);

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

	//	�ŏ��̃|�[�g�ԍ��Ƃ��A�z�X�g����
	//	�������������̂��߂�ǂ�����A�}�N���ɂ�������
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

	//	string �^�̕���szServerstr��.c_str()��char�ɂ���B
	lpHostEnt = gethostbyname(szServerstr.c_str());
	if (lpHostEnt == NULL) {

		addr = inet_addr(szServerstr.c_str());	//	������
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
	//	�m���u���b�L���O����
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

		
		{		
			//	�����̓u���b�N���Ă���B
			//gets_s(szBuf);			

			int key = input_key();


			if (key != 0) {
				sprintf(szBuf, "%c", key);

				printf("Send:%s\n",szBuf);

				nRtn = sendto(s, szBuf, (int)strlen(szBuf) + 1, 0,
					(LPSOCKADDR)&addrin, sizeof(addrin));
				if (nRtn != (int)strlen(szBuf) + 1) {
					perror("Send Srror\n");
					closesocket(s);
					WSACleanup();
					return -4;
				}
				if (strcmp(szBuf, "end") == 0) {
					printf("�I�����܂�\n");
					_getch();
					break;
				}
			}
		}


		
	}
	closesocket(s);
	WSACleanup();
	return 0;

}

//==============================================================
// �_�C���N�g�L�[����
//==============================================================
int input_key(void)
{
	int     key;
	int     now_time, pre_time;

	now_time = pre_time = clock();
	while (true) {
		if (_kbhit()) {
			key = _getch();
			break;
		}
		now_time = clock();
		if (now_time - pre_time > FALL_TIMING) {
			key = 's';  // ���L�[�Ɠ��������ɂ���
			break;
		}
	}
	switch (key) {
	case 0xe0:
		switch (_getch()) {
		case 0x4b:  // ���L�[
			key = 'a';
			break;
		case 0x4d:  // ���L�[
			key = 'd';
			break;
		case 0x50:  // ���L�[
			key = 's';
			break;
		default:
			key = 0;
		}
		break;
	case 0x1b:  // ESC�L�[
		key = '*';
		break;
	case '*':   // �I��
	case 'a':   // ���ړ�
	case 'd':   // �E�ړ�
	case 's':   // ���ړ�
	case ' ':   // ��������
	case 'x':   // ��]
		break;
	default:
		key = 0;
		break;
	}
	return key;
}