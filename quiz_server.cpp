#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#pragma comment(lib,"ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h> 

#define BUF_SIZE 100
#define NAME_SIZE 20
#define MAX_CLNT 256
#define Q_SIZE 400


struct c_socket {
	SOCKET socket;
	char addr[NAME_SIZE];
	char name[NAME_SIZE];
	int score = 0;
}typedef C_SOCKET;

struct quiz {
	char question[Q_SIZE];
	char answer[Q_SIZE];
}typedef QUIZ;

QUIZ test_quiz[] = { //학교
   {"정지연은 18학번이다. ","예"},
   {"학교 앞에 쿠니치가 있다.", "예"},
   {"컴퓨터 네트워크는 3학년 2학기 때 배운다","아니오"}
};

QUIZ test_quiz2[] = { //애니
   {"뚱이는 귀엽다." ,"예"},
   {"스폰지밥은 오징어이다. ", "아니오"},
   {"다람이 출신은 멕시코이다. ","아니오"}
};

int quiz1_len = sizeof(test_quiz) / sizeof(quiz);
int quiz2_len = sizeof(test_quiz2) / sizeof(quiz);

unsigned WINAPI HandleClnt(void * arg);
void quiz_MSG(char * msg, int len, SOCKET *s);
void ErrorHandling(char * msg);
void game_start();

int clntCnt = 0;
C_SOCKET clntSocks[MAX_CLNT];
HANDLE hMutex;


char text[1000];
int quiz_num = -1;
int question_num = 0;
int question_flag = -1;

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;
	int clntAdrSz;
	HANDLE  hThread;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");
	hMutex = CreateMutex(NULL, FALSE, NULL);
	hServSock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(argv[1]));

	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	while (1) {
		clntAdrSz = sizeof(clntAdr);
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSz);

		WaitForSingleObject(hMutex, INFINITE);
		clntSocks[clntCnt].socket = hClntSock;
		recv(hClntSock, clntSocks[clntCnt].name, sizeof(clntSocks[clntCnt].name), 0);//클라이언트의 닉네임
		strcpy(clntSocks[clntCnt].addr, inet_ntoa(clntAdr.sin_addr));//클라이언트의 IP주소
		clntCnt++;
		if (clntCnt >= 2) {
			char * msg = "            *****퀴즈를 시작하겠습니다.*****\n       모든 문제는 \"예\" 또는 \"아니오\"로 답해주세요.       \n       주제를 먼저 정해주세요. 1번 학교, 2번 애니, 3번 시스템 종료       \n";
			for (int i = 0; i < clntCnt; i++)
				send(clntSocks[i].socket, msg, strlen(msg), 0);
		}
		ReleaseMutex(hMutex);
		hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClnt,
			(void*)&hClntSock, 0, NULL);
		printf("Connected client IP: %s \n",
			inet_ntoa(clntAdr.sin_addr));
	}
	closesocket(hServSock);
	WSACleanup();

	

	return 0;
}

unsigned WINAPI HandleClnt(void * arg)
{
	SOCKET hClntSock = *((SOCKET*)arg);
	int strLen = 0, i;
	char msg[BUF_SIZE];

	while ((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0)
		quiz_MSG(msg, strLen, &hClntSock);


	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0; i < clntCnt; i++)
	{
		if (hClntSock == clntSocks[i].socket)
		{
			while (i++ < clntCnt - 1)
				clntSocks[i] = clntSocks[i + 1];
			break;
		}
	}
	clntCnt--;
	ReleaseMutex(hMutex);
	closesocket(hClntSock);
	return 0;
}

void game_start() {
	
}

void quiz_MSG(char * msg, int len, SOCKET *s)
{
	int i;
	char text[1000];
	int que_len = 0;
	char answer[100];
	char tempStr[NAME_SIZE + BUF_SIZE + 2];
	strcpy_s(tempStr, msg);
	char* token;
	char* temp;
	char* answer_sock = "";
	char seps[] = " ,\t\n";
	int max_score = 0;
	char tmp_name[NAME_SIZE];
	token = strtok_s(tempStr, seps, &temp);
	token = strtok_s(NULL, seps, &temp);
	WaitForSingleObject(hMutex, INFINITE);

	if (question_flag == -1) {

		if (strcmp(token, "1") == 0) {
			for (i = 0; i < clntCnt; i++) {
				sprintf(text, "1번 주제를 선택하였습니다. 시작하겠습니다.\n");
				send(clntSocks[i].socket, text, strlen(text), 0);
			}
			for (i = 0; i < clntCnt; i++) {
				sprintf(text, "%d번 문제. %s\n", question_num+1,test_quiz[question_num].question);
				send(clntSocks[i].socket, text, strlen(text), 0);
			}
			quiz_num = token[0] - '0';
			question_flag++;
		}
		else if (strcmp(token, "2") == 0) {
			for (i = 0; i < clntCnt; i++) {
				sprintf(text, "2번 주제를 선택하였습니다. 시작하겠습니다.\n");
				send(clntSocks[i].socket, text, strlen(text), 0);
			}
			for (i = 0; i < clntCnt; i++) {
				sprintf(text, "%d번 문제. %s\n", question_num + 1,test_quiz2[question_num].question);
				send(clntSocks[i].socket, text, strlen(text), 0);
			}
			quiz_num = token[0] - '0';
			question_flag++;
		}
		else if (strcmp(token, "3") == 0) {
			for (i = 0; i < clntCnt; i++) {
				sprintf(text, "퀴즈 시스템을 종료합니다.\n");
				send(clntSocks[i].socket, text, strlen(text), 0);
			}
			exit(1);
		}
		else {
			for (i = 0; i < clntCnt; i++) {
				sprintf(text, "다시 선택 해주세요.\n");
				send(clntSocks[i].socket, text, strlen(text), 0);
			}
		}
	}
	else if (quiz_num != -1) {
		if (quiz_num == 1 && question_num < quiz1_len -1) {
			if (strcmp(test_quiz[question_num].answer, token) == 0) {
				for (i = 0; i < clntCnt; i++) {
					if (*s == clntSocks[i].socket) {
						strcpy(tmp_name, clntSocks[i].name);
						clntSocks[i].score += 100;
					}
				}
				for (i = 0; i < clntCnt; i++) {
					sprintf(text, "%s님께서 답을 맞추셨습니다!!!!!!!!\n", tmp_name);
					send(clntSocks[i].socket, text, strlen(text), 0);
				}
				question_num++;
				for (i = 0; i < clntCnt; i++) {
					sprintf(text, "%d번 문제. %s\n", question_num+1 ,test_quiz[question_num].question);
					send(clntSocks[i].socket, text, strlen(text), 0);
				}
			}
			else {
				for (i = 0; i < clntCnt; i++) {
					sprintf(text, "틀렸습니다!\n");
					send(clntSocks[i].socket, text, strlen(text), 0);
				}
			}
		}
		else if (quiz_num == 2 && question_num < quiz2_len - 1) {
			for (i = 0; i < clntCnt; i++) {
				if (*s == clntSocks[i].socket) {
					strcpy(tmp_name, clntSocks[i].name);
					clntSocks[i].score += 100;
				}
			}
			if (strcmp(test_quiz2[question_num].answer, token) == 0) {
				for (i = 0; i < clntCnt; i++) {
					sprintf(text, " %s님께서 답을 맞추셨습니다!!!!!!!!\n", tmp_name);
					send(clntSocks[i].socket, text, strlen(text), 0);
				}
				question_num++;
				for (i = 0; i < clntCnt; i++) {
					sprintf(text, "%d번 문제. %s\n", question_num +1 ,test_quiz2[question_num].question);
					send(clntSocks[i].socket, text, strlen(text), 0);
				}
			}
			else {
				for (i = 0; i < clntCnt; i++) {
					sprintf(text, "틀렸습니다!\n");
					send(clntSocks[i].socket, text, strlen(text), 0);
				}
			}
		}
		else {
			int score;
			for (int i = 0; i < clntCnt; i++) {
				if (max_score < clntSocks[i].score) {
					strcpy(tmp_name, clntSocks[i].name);
					clntSocks[i].score += 100;
					score = clntSocks[i].score;
				}
			}

			for (i = 0; i < clntCnt; i++) {
				sprintf(text, "%s님께서 %d점으로 1등하셨습니다. \n", tmp_name,score);
				send(clntSocks[i].socket, text, strlen(text), 0);

				sprintf(text, "다른 주제 번호 골라주세요\n");
				send(clntSocks[i].socket, text, strlen(text), 0);
				quiz_num = -1;
				question_num = 0;
				question_flag = -1;
				
			}
		}
	}
	
	
	ReleaseMutex(hMutex);
}

void ErrorHandling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

