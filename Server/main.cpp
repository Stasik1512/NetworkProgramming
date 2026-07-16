#include <iostream>

#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h> // сокращение от ip help appi
#define MTU 1500 
using namespace std;

#pragma comment(lib,"WS2_32.lib")
void main()
{
	setlocale(LC_ALL, "");
	cout << "Server" << endl;


	INT iResult = 0;

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
		WSACleanup();
		return;
	}

	// 2 Создаем socket сервер:

	SOCKET listen_socket = socket(target->ai_family, target->ai_socktype, target->ai_protocol);
	if (listen_socket == INVALID_SOCKET)
	{
		cout << "LISTEN SOCKET cretion falied with error" << endl;
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//3 BIND -- Привязываем серверный сокет к интерфейсу, который он будет слушать 
	iResult = bind(listen_socket, target->ai_addr, target->ai_addrlen);
	if (iResult)
	{
		cout << "BIND falied with error:" << WSAGetLastError() << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	// 4 запускаем прослушивание порта:
	if (listen(listen_socket, 1) == SOCKET_ERROR)
	{
		cout << "Listen falied with error: " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}

	//5 принимаем подключение от клиентов
	SOCKET client_socket = accept(listen_socket, NULL, NULL);

	if (client_socket == INVALID_SOCKET)
	{
		cout << "Accept falied wiht error: " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		freeaddrinfo(target);
		WSACleanup();
		return;
	}
	// 6 получение данных от клиента:
	CHAR recv_buffer[MTU] = {};
	iResult = recv(client_socket, recv_buffer, MTU, NULL);
	if (iResult > 0)
	{
		cout << iResult << "Bytes received.Message: " << recv_buffer << endl;
	}
	else if (iResult == 0) cout << "Nothing received, connection closing" << endl;
	else cout << " Recevied failed with error:" << WSAGetLastError() << endl;
	
	//7 Отправка данных клиенту
	CHAR send_buffer[MTU] = {};
	sprintf(send_buffer, "Привет Клиент, ваше сообщение: %s", recv_buffer);
	iResult = send(client_socket, send_buffer, strlen(send_buffer), NULL);
	if (iResult == SOCKET_ERROR)
	{
		cout << " Send() failed with error: " << WSAGetLastError() << endl;
	}

	cin.get(); //ожидает нажатия клавиши enter;

	//8 закрываем соедин с клиентом
	iResult = shutdown(client_socket, SD_BOTH);
	if (iResult) cout << "shutdown failed with error: " << WSAGetLastError() << endl;
	

	//9 освободить ресурсы
	closesocket(client_socket);
	closesocket(listen_socket);
	freeaddrinfo(target);
	WSACleanup();

}