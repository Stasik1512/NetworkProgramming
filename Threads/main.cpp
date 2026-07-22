#include<Windows.h>
#include <iostream>
using namespace std;

void Plus()
{
	while (true)
	{
		cout << " + ";

	}
}
void Minus()
{
	while (true)
	{
		cout << " - ";

	}
}
void main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	//Plus();
	//Minus();
	HANDLE hThreads[2] = {};
	hThreads[0] = CreateThread
	(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)Plus, // нямнб
		NULL,
		0,
		NULL
	);
	hThreads[1] = CreateThread
	(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)Minus,
		NULL,
		0,
		NULL
	);
	cin.get();
	WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);
	CloseHandle(hThreads[0]);
	CloseHandle(hThreads[1]);
}