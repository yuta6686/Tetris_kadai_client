// udpclient.cpp

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <winsock2.h>
#include <conio.h>
#include <time.h>
#include <string>
#include <vector>
#pragma comment (lib, "Ws2_32.lib")

using namespace std;


#define PORT_NUMBER 55555
#define HOST_NAME "Wths00680"


//	Tetris側のマクロコピー  
#define FIELD_H         20      // フィールドの高さ
#define FIELD_W         10      // フィールドの幅
#define FIELD_X         5       // フィールド表示桁位置
#define FIELD_Y         2       // フィールド表示行位置
#define GAME_CLEAR_PONT 100     // ゲームクリアになる得点
#define EXPAND_POINT    4       // 連続削除時の点数の拡大率
#define FALL_TIMING     300     // ブロックが自動落下するタイミング(m秒)

#define CURSOR_ERASE	printf("\033[?25l")		// カーソル消去
#define CURSOR_DISPLAY	printf("\033[?25h")		// カーソル表示
#define COLOR_RESET		printf("\033[0m")		// 色設定のリセット
#define WINDOW_CLEAR	printf("\033[2J")		// 画面クリア

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
#define PIECE_DEL       BLACK           // 削除中のピースの色

//	Tetris側のグローバル変数コピー
int     Field[FIELD_H][FIELD_W];        // フィールド
int     Block[BLOCK_SIZE][BLOCK_SIZE];  // 表示するブロック
int     Block_X, Block_Y;   // ブロック位置(フィールド内相対位置)
int     Point = 0;          // 得点


//	プロトタイプ宣言
void set_text_color(int color);
void set_back_color(int color);
void set_cursor_pos(int x, int y);

void print_field(void);
void print_block(void);
int input_key(void);

std::vector<std::string> split(std::string str, char del) {
	int first = 0;
	int last = str.find_first_of(del);

	std::vector<std::string> result;

	while (first < str.size()) {
		std::string subStr(str, first, last - first);

		result.push_back(subStr);

		first = last + 1;
		last = str.find_first_of(del, first);

		if (last == std::string::npos) {
			last = str.size();
		}
	}

	return result;
}

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

				//	区切り文字
				char del = ',';

				//	区切ったstring型の文字列の配列
				vector<string> subStr = split(szBuf, del);
				for (int i = 0; i < 200; i++) {
					Field[i / 10][i % 10] = stoi(subStr[i]);
				}
				for (int i = 200; i < 200 + 4 * 4; i++) {
					Block[(i - 200) / 4][(i - 200) % 4] = stoi(subStr[i]);
				}
				Block_X = stoi(subStr[216]);
				Block_Y = stoi(subStr[217]);

				print_field();      // フィールドの表示
				print_block();      // アクティブブロックの表示

				//printf("Recv:");
				//printf("%s>%s\n", inet_ntoa(from.sin_addr), szBuf);
				//printf("%s\n", szBuf);
				//printf("%d", subStr.size());
				szBuf[nRtn] = '\0';
			}

			if (strcmp(szBuf, "end") == 0) {
				printf(" Terminate Server\n");
				break;
			}
		}


		{
			//	こいつはブロックしてくる。
			//gets_s(szBuf);			

			int key = input_key();


			if (key != 0) {
				sprintf(szBuf, "%c", key);

				//printf("Send:%s\n",szBuf);

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



	}
	closesocket(s);
	WSACleanup();
	return 0;

}

//==============================================================
// ダイレクトキー入力
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
			key = 's';  // ↓キーと同じ動きにする
			break;
		}
	}
	switch (key) {
	case 0xe0:
		switch (_getch()) {
		case 0x4b:  // ←キー
			key = 'a';
			break;
		case 0x4d:  // →キー
			key = 'd';
			break;
		case 0x50:  // ↓キー
			key = 's';
			break;
		default:
			key = 0;
		}
		break;
	case 0x1b:  // ESCキー
		key = '*';
		break;
	case '*':   // 終了
	case 'a':   // 左移動
	case 'd':   // 右移動
	case 's':   // 下移動
	case ' ':   // 高速落下
	case 'x':   // 回転
		break;
	default:
		key = 0;
		break;
	}
	return key;
}
//=============================================
// 前景色設定(0以上15以下の色番号を指定、範囲外は補正)
//=============================================
void set_text_color(int color)
{
	if (color < 0) {
		color = 30;
	}
	else if (color < 8) {
		color += 30;
	}
	else if (color < 16) {
		color = (color - 8) + 90;
	}
	else {
		color = 97;
	}
	printf("\033[%dm", color);
}
//=============================================
// 背景色設定(0以上15以下の色番号を指定、範囲外は補正)
//=============================================
void set_back_color(int color)
{
	if (color < 0) {
		color = 40;
	}
	else if (color < 8) {
		color += 40;
	}
	else if (color < 16) {
		color = (color - 8) + 100;
	}
	else {
		color = 107;
	}
	printf("\033[%dm", color);
}
//=============================================
// カーソル位置設定
//=============================================
void set_cursor_pos(int x, int y)
{
	printf("\033[%d;%dH", y, x);
}
//=================================
void print_field(void)
{
	int         x = FIELD_X, y = FIELD_Y;

	// フィールドの表示(固定化したブロック含む)
	set_back_color(DARK_GRAY);
	for (int i = 0; i < FIELD_H; i++) {
		set_cursor_pos(x, y++);
		for (int j = 0; j < FIELD_W; j++) {
			if (Field[i][j] != EMPTY) {
				set_text_color(Field[i][j]);
				printf("■");
			}
			else {
				printf("  ");
			}
		}
	}
	COLOR_RESET;
	printf("\n\n得点：%4d", Point);
}

void print_block(void)
{
	int         x = FIELD_X, y = FIELD_Y;

	x += Block_X * 2;
	y += Block_Y;
	for (int i = 0; i < BLOCK_SIZE; i++) {
		for (int j = 0; j < BLOCK_SIZE; j++) {
			set_cursor_pos(x + (j * 2), y + i);
			if (Block[i][j] != EMPTY) {
				set_text_color(Block[i][j]);
				printf("■");
			}
		}
	}
	COLOR_RESET;
}
