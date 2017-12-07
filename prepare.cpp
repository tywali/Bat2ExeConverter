#include <stdio.h>
#include <windows.h>
#include "tstring.h"

#define BUFSIZE 4096

int createResourceFile(LPCTSTR iniFileName) {
	CHAR chBuf[BUFSIZE], chRcBuf[BUFSIZE], chReplBuf[BUFSIZE], *pDes;
	DWORD dwRead, dwWritten;

	GetPrivateProfileString("info", "CompanyName", "", chBuf, BUFSIZE, iniFileName);
	printf("\r\n%s\r\n", chBuf);
	
	HANDLE hRc = CreateFile("compile/Bat2Exe_private.rc", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	ReadFile(hRc, chRcBuf, BUFSIZE, &dwRead, NULL);
	CloseHandle(hRc);
	
	pDes = strrepl(chRcBuf, chReplBuf, sizeof(chReplBuf), "CompanyName_EXE", chBuf);

	hRc = CreateFile("compile/Bat2Exe_private.rc", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	WriteFile(hRc, pDes, strlen(pDes), &dwWritten, NULL);
	CloseHandle(hRc);
	
	return 0;
}
