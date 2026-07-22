#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include<Windows.h>
#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h> // сокращение от ip help appi
#include <FormatLastError.h>
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "FormatLastError.lib")
using namespace std;


CHAR* FormatLastError(DWORD dwError, CHAR szError[]);
#define MTU  1500
// Maximum transfer unit- макс блок данных который можно предать по сети. Для сетей семейства enternet
//mtu составляеь 1500 byte

void main()
{

	INT iResult; //переменная которая хранит результаты работы функции
	DWORD dwError;
	CHAR szError[256] = {};

	// 0 инициализация winsock
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult)
	{
		cout << "WSAStartup failed with error: " << iResult << endl;
		return;
	}

	//1 Задаем параметры подключения:
	addrinfo hints;
	addrinfo* target;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;			//tcp/ip
	hints.ai_socktype = SOCK_STREAM;	//sock-stream and ipproto_tcp/ip - говорят о том что мы будем подключатся по протоколу tcp
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo("127.0.0.1", "27015", &hints, &target);

	if (iResult)
	{
		cout << "getaddrinfo() fiald with error" << iResult << endl;
		WSACleanup();
		return;
	}

	//2 cоздаем сокет при помощи которого будем подключатся к серверу

	SOCKET connect_socket = socket(target->ai_family, target->ai_socktype, target->ai_protocol);
	dwError = WSAGetLastError();
	if (connect_socket == INVALID_SOCKET)
	{
		cout << FormatLastError(dwError, szError) << endl;
		cout << "Socket creating failed with error" << WSAGetLastError() << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//3 Подключ к серверу
	iResult = connect(connect_socket, target->ai_addr, target->ai_addrlen);
	dwError = WSAGetLastError();
	freeaddrinfo(target);
	if (iResult)
	{
		cout << FormatLastError(dwError, szError) << endl;
		cout << "Unable to connect to server. ERROR: " << WSAGetLastError() << endl;
		closesocket(connect_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	// 4 отправка данных на сервер
	CHAR send_buffer[MTU] = "Привет Сервер";
	do
	{
		iResult = send(connect_socket, send_buffer, strlen(send_buffer), 0);
		dwError = WSAGetLastError();
		if (iResult == SOCKET_ERROR)
		{
			cout << "Send failed with error: " << WSAGetLastError() << endl;
			cout << FormatLastError(dwError, szError) << endl;
			closesocket(connect_socket);
			WSACleanup();
			return;
		}
		cout << "Send" << iResult << "Bytes" << endl;

		//5 Получение данных от сервера
		CHAR recv_buffer[MTU] = {};
		iResult = recv(connect_socket, recv_buffer, MTU, NULL);
		dwError = WSAGetLastError();
		if (iResult > 0) cout << iResult << "Byte received. Message: " << recv_buffer << endl;
		else if (iResult == 0)cout << "Nothing received." << endl;
		else cout << "Received failed with error: " << WSAGetLastError() << endl << FormatLastError(dwError, szError);

		ZeroMemory(send_buffer, strlen(send_buffer));

		cout << "Введите сообщение:"; 
		SetConsoleCP(1251);
		cin.getline(send_buffer, MTU);
		SetConsoleCP(866);

	} while (strcmp(send_buffer,"exit"));
	
	// 6 завершаем сеан работы с сервером и освобождаем ресурсы
	iResult = shutdown(connect_socket, SD_BOTH);
	dwError = WSAGetLastError();
	if (iResult == SOCKET_ERROR) cout << "Shutdown failed with error: " << WSAGetLastError() << endl << FormatLastError(dwError, szError);
	closesocket(connect_socket);
	WSACleanup();


	//объект wsadata занимает ресурсы памяти, и поэтому,  после того 
	// как winsock больше не нужен, ти ресурсы нужно освободить
	WSACleanup();
	
}