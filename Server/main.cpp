#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <iostream>
#include <Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h> // сокращение от ip help appi

#include <FormatLastError.h>
#include <Messages.h>
#define MTU 1500 
using std::cin;
using std::cout;
using std::endl;

#pragma comment(lib,"WS2_32.lib")
#pragma comment(lib,"FormatLastError.lib")

#define MAX_CONNECTIONS 3
HANDLE g_hThreads[MAX_CONNECTIONS + 1] = {};
DWORD g_dwThreadsIDs[MAX_CONNECTIONS + 1] = {};
SOCKET g_hSockets[MAX_CONNECTIONS + 1] = {};
INT n = 0;

VOID ShowActiveClients();
VOID ClientHandler(SOCKET client_socket);

void main()
{
	setlocale(LC_ALL, "");
	cout << "Server" << endl;


	INT iResult = 0;
	DWORD dwError = 0;
	CHAR szError[256] = {};

	// 0 Инициализация
	WSADATA wsaData;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	//1 Параметры подключения
	addrinfo hints;
	addrinfo* target;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE; // Соединение работает в режиме "listening"


	iResult = getaddrinfo(NULL, "27015", &hints, &target); // null - 0.0.0.0 сервер будет прослушивать порт 27015
	//на всех доступных ip адресах, то есть на всех сетевых картах установленных на компьюторе
	if (iResult) // "0" false, true this all what not 0
	{
		cout << " getaddrinfo() falied with error:" << endl;
		cout << "Функция getaddrinfo() завершилась с ошибкой: " << endl;
		WSACleanup();
		return;
	}

	// 2 Создаем socket сервер:

	SOCKET listen_socket = socket(target->ai_family, target->ai_socktype, target->ai_protocol);
	dwError = WSAGetLastError();
	if (listen_socket == INVALID_SOCKET)
	{
		cout << "LISTEN SOCKET cretion falied with error" << endl;
		cout << "Создание LISTEN SOCKET завершилось с ошибкой: " << endl;
		cout << FormatLastError(dwError, szError) << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//3 BIND -- Привязываем серверный сокет к интерфейсу, который он будет слушать 
	iResult = bind(listen_socket, target->ai_addr, target->ai_addrlen);
	dwError = WSAGetLastError();
	if (iResult)
	{
		cout << "BIND falied with error:" << WSAGetLastError() << endl;
		cout << "Привязка сокета к порту завершилась с ошибкой" << WSAGetLastError() << endl;
		cout << FormatLastError(dwError, szError) << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	// 4 запускаем прослушивание порта:
	if (listen(listen_socket, MAX_CONNECTIONS) == SOCKET_ERROR)
	{
		cout << FormatLastError(WSAGetLastError(), szError) << endl;
		cout << "Listen falied with error: " << WSAGetLastError() << endl;
		cout << "Прослушивать порт невозможно изза оошибки " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//5 принимаем подключение от клиентов
	do
	{
		ShowActiveClients();
		sockaddr_in client_address;
		int client_address_len = sizeof(client_address);
		SOCKET client_socket = accept(listen_socket, (SOCKADDR*)&client_address, &client_address_len);
		//cout << "Accept DONE" << endl;
		cout << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << endl;

		if (client_socket == INVALID_SOCKET)
		{
			cout << FormatLastError(WSAGetLastError(), szError) << endl;
			cout << "Accept falied wiht error: " << WSAGetLastError() << endl;
			cout << "Не удалось принять подключение от сервера" << WSAGetLastError() << endl;
			closesocket(listen_socket);
			freeaddrinfo(target);
			WSACleanup();
			return;
		}


		if (n < MAX_CONNECTIONS)
		{
			//ClientHandler(client_socket);
			g_hSockets[n] = client_socket;
			g_hThreads[n] = CreateThread
			(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ClientHandler,
				(LPVOID)g_hSockets[n],
				NULL,
				g_dwThreadsIDs + n
			);
			n++;
		}
		else
		{
			iResult = send(client_socket, DECLINE_MESSAGE, strlen(DECLINE_MESSAGE), NULL);
			if (iResult == SOCKET_ERROR) FormatLastError(WSAGetLastError(), szError);
			iResult = shutdown(client_socket, SD_BOTH);
			cout << " - DECLINE" << endl;
		}

	} while (true);

	//9 освободить ресурсы
	closesocket(listen_socket);
	freeaddrinfo(target);
	WSACleanup();

}
VOID ShowActiveClients()
{
	Sleep(100);
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hConsole, &info);
	SetConsoleCursorPosition(hConsole, COORD{ 25,0 });
	cout << "\t\t\t\t\t";
	SetConsoleCursorPosition(hConsole, COORD{ 25,0 });
	cout << "Количество подлюченных клиентов: " << n;
	SetConsoleCursorPosition(hConsole, info.dwCursorPosition);
}
INT GetClientPosition(DWORD dwID)
{
	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (g_dwThreadsIDs[i] == dwID)return i;
	}
}
VOID Shift(INT position)
{
	for (INT i = position; i < MAX_CONNECTIONS; i++)
	{
		g_hThreads[i] = g_hThreads[i + 1];
		g_hSockets[i] = g_hSockets[i + 1];
		g_dwThreadsIDs[i] = g_dwThreadsIDs[i + 1];
	}
}
VOID Broadcast(CHAR send_buffer[], INT sender_index)
{
	CHAR szError[256] = {};
	for (INT i = 0; i < n; i++)
	{
		if (i != sender_index)
		{
			INT iResult = send(g_hSockets[i], send_buffer, strlen(send_buffer), NULL);
			if (iResult == SOCKET_ERROR)
			{
				cout << FormatLastError(WSAGetLastError(), szError) << endl;
				cout << " Send() failed with error: " << WSAGetLastError() << endl;
				cout << "При отправка данных возникла ошибка" << WSAGetLastError() << endl;
			}
		}
	}
}
VOID ClientHandler(SOCKET client_socket)
{
	SOCKADDR_IN client_address;
	client_address.sin_family = AF_INET;
	INT namelen = sizeof(client_address);
	getpeername(client_socket, (SOCKADDR*)&client_address, &namelen);
	CHAR sz_client_address[32] = {};
	CHAR sz_client_connected[32] = {};
	sprintf(sz_client_address, "%s:%d  -  ", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
	sprintf(sz_client_connected, " %s CONNECTED", sz_client_address);
	cout << "Client " << sz_client_connected << endl;
	INT iResult = 0;
	DWORD dwError = 0;
	CHAR szError[256] = {};
	// 6 получение данных от клиента:
	CHAR recv_buffer[MTU] = {};
	CHAR send_buffer[MTU] = {};
	do
	{ 
		ZeroMemory(send_buffer, MTU);
		ZeroMemory(recv_buffer, MTU);
		iResult = recv(client_socket, recv_buffer, MTU, NULL);
		if (iResult > 0)
		{
			//cout << iResult << "Bytes received. Message: " << recv_buffer << endl;
			cout << sz_client_address << recv_buffer << ". (" << iResult << " Bytes);";
			cout << "\tThreadID: " << GetCurrentThreadId() << "\tPosition:" << GetClientPosition(GetCurrentThreadId());
			cout << endl;
		}
		else if (iResult == 0)
			cout << "Nothing received, connection closing\n нет данных от клиента, закрываем соединение" << endl;
		else
		{
			cout << FormatLastError(WSAGetLastError(), szError) << endl;
			cout << "recevied failed with error: " << WSAGetLastError() << endl;
			cout << "При получении данных возникла ошибка: " << WSAGetLastError() << endl;
			break;
		}
		////7 Отправка данных клиенту
		sprintf(send_buffer, "%s%s\n", sz_client_address, recv_buffer);
		Broadcast(send_buffer, GetClientPosition(GetCurrentThreadId()));
		
	} while (true);

	//cin.get(); ожидает нажатия клавиши enter;

	//8 закрываем соедин с клиентом
	cout << "Press ENTER to close connection" << endl;
	//cin.get();
	iResult = shutdown(client_socket, SD_BOTH);
	if (iResult) cout << FormatLastError(WSAGetLastError(), szError) << endl;
	if (iResult) cout << "shutdown failed with error: " << WSAGetLastError() << endl;
	INT index = GetClientPosition((DWORD)GetCurrentThreadId);									//!!!!!!!!!!!!!!!!!!!!!!
	HANDLE hCurrentThread = g_hThreads[index];
	closesocket(client_socket);
	CloseHandle(hCurrentThread);
	n--;
	ShowActiveClients();
}         