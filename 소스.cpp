#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include<WS2tcpip.h>
#include<iostream>
#include<string>
#pragma comment(lib, "ws2_32")

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 1024

void err_quit(const char* msg);
void err_display(const char* msg);
void send_fileName(SOCKET sock, char* buf, std::string& fileName, int& len);
void send_file(SOCKET sock, FILE* fp, char* buf, int& len);

int main(int argc, char* argv[])
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET sock = socket(AF_INET, SOCK_STREAM, NULL);
	if (sock == INVALID_SOCKET)err_quit("socket()");

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serverAddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR)err_quit("connect()");

	char buf[BUFSIZE + 1];
	int len;

	//패킷 데이터 처리
	while (true) {
		std::cout << "서버에 전송할 파일 이름을 입력하세요:";
		std::string fileName;
		std::cin >> fileName;

		//바이너리 읽기 모드로 파일 오픈
		FILE* fp = fopen(fileName.c_str(), "rb");
		if (!fp) {
			std::cout << "존재하지 않는 파일 이름입니다.\n";
			continue;
		}

		send_fileName(sock, buf, fileName, len);

		send_file(sock, fp, buf, len);
	}
	closesocket(sock);

	WSACleanup();
	return 0;
}

void send_fileName(SOCKET sock, char* buf, std::string& fileName, int& len)
{
	int retval;
	len = fileName.size();

	//파일 이름 길이 전송
	retval = send(sock, (char*)&len, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return;
	}

	//파일 이름 전송
	strcpy(buf, fileName.c_str());
	int sentSize = 0;
	sentSize = send(sock, buf, fileName.size(), 0);
	std::cout << buf << std::endl;
}

void send_file(SOCKET sock, FILE* fp, char* buf, int& len)
{
	int retval;
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	rewind(fp);

	//파일 길이 전송
	retval = send(sock, (char*)&len, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return;
	}

	//파일 내용 전송
	while (len > 0) {
		int toSend = BUFSIZE <= len ? BUFSIZE : len;
		fread(buf, toSend, 1, fp);
		retval = send(sock, buf, toSend, 0);

		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		len -= retval;
	}
	fclose(fp);
}

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}