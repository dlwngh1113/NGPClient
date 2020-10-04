#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include<WS2tcpip.h>
#include<iostream>
#include<fstream>
#include<string>
#pragma comment(lib, "ws2_32")

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 512

void err_quit(const char* msg);
void err_display(const char* msg);
void input_data(char* dest, std::ifstream& in, std::string& s);

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
	const char* testData[] = {
		"안녕하세요",
		"반가워요",
		"오늘따라 할 이야기가 많을 것 같네요",
		"저도 그렇네요"
	};
	int len;

	while (true) {
		ZeroMemory(buf, BUFSIZE);
		std::cout << "서버에 전송할 파일 이름을 입력하세요:";
		std::string fileName;
		std::cin >> fileName;

		std::ifstream in(fileName);
		if (!in) {
			std::cout << "존재하지 않는 파일 이름입니다.\n";
			continue;
		}

		fileName += '\n';
		const auto beg = in.tellg();
		in.seekg(0, in.end);
		const auto end = in.tellg();
		len = end - beg + fileName.size();
		if (len > BUFSIZE) {
			std::cout << "파일 크기가 최대 버퍼 사이즈를 넘기 때문에 읽지 않습니다.\n";
			continue;
		}
		in.seekg(0, in.beg);

		//파일 길이 전송
		retval = send(sock, (char*)&len, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		//파일 내용, 이름 전송
		input_data(buf, in, fileName);
		retval = send(sock, buf, len, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		std::cout << buf << std::endl;

		printf("[TCP클라이언트] %d+%d바이트를 보냈습니다\n",
			sizeof(int), retval);
		in.close();
	}
	closesocket(sock);

	WSACleanup();
	return 0;
}

void input_data(char* dest, std::ifstream& in, std::string& s)
{
	strcpy(dest, s.c_str());
	int idx = s.size();
	while (in) {
		dest[idx] = static_cast<char>(in.get());
		++idx;
	}
}