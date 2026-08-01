#include<Windows.h>
#include <iostream>
using namespace std;

BOOL finish = FALSE;
HANDLE g_hMutex = NULL;
void Plus()
{
	while (!finish)
	{
		WaitForSingleObject(g_hMutex, INFINITE);
		cout << " + ";
		Sleep(100);
		ReleaseMutex(g_hMutex);
	}
}
void Minus()
{
	while (!finish)
	{
		WaitForSingleObject(g_hMutex, INFINITE);
		cout << " - ";
		Sleep(100);
		ReleaseMutex(g_hMutex);
	}
}

//	#define - определить
//	#definition - определение
//	Директива #define создает макроопределение (макрос)
#define WINDOWS_THREADS
void main()
{
#if defined WINDOWS_THREADS
	//ЕСЛИ определено WINDOWS_THREADS, то нижеследующий код будет виден компилятору, 
	// до диективы #endif
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	//Plus();
	//Minus();
	g_hMutex = CreateMutex(NULL, NULL, "Mutex");
	HANDLE hThreads[2] = {};
	hThreads[0] = CreateThread
	(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)Plus, // ОСНОВ
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
	finish = TRUE;
	WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);
	CloseHandle(hThreads[0]);
	CloseHandle(hThreads[1]);
	CloseHandle(g_hMutex);
#endif // WINDOWS_THREADS

}