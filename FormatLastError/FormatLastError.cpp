#include "FormatLastError.h"
#include <iostream>
CHAR* FormatLastError(DWORD dwError, CHAR szError[])
{
	ZeroMemory(szError, strlen(szError));
	LPSTR lpError = NULL;
	FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		(LPSTR)&lpError,
		256,
		NULL
	);
	sprintf(szError, "Error# %i: %s: ", dwError, lpError);
	// strcpy(szError, lpError);
	return szError;
}
