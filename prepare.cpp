#include <stdio.h>
#include <windows.h>
#include "tstring.h"

#define BUFSIZE 4096

int prepareCompileFolder() {
	system("cmd.exe /c rd compile /s /q");
	system("cmd.exe /c mkdir compile");
	system("cmd.exe /c mkdir compile\\obj");
	system("cmd.exe /c mkdir compile\\exe");
	system("cmd.exe /c xcopy template\\*.* compile\\");
	
	return 0;
}

int createIncludeFile(LPCSTR batFileName) {
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE], chFileBuf[BUFSIZE * 2], chReplBuf[BUFSIZE * 2], *pDes;
	CHAR cmdVar[] = "CHAR cmdStr[] = \"\"\r\n\"";
	
	ZeroMemory(chBuf, BUFSIZE);
	ZeroMemory(chFileBuf, BUFSIZE * 2);
	ZeroMemory(chReplBuf, BUFSIZE * 2);
	
	HANDLE hBat = CreateFile(batFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	ReadFile(hBat, chBuf, BUFSIZE, &dwRead, NULL);
	//printf("%s", chBuf);
	
	HANDLE hInc = CreateFile("compile\\batcmd.h", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	pDes = strrepl(chBuf, chReplBuf, sizeof(chReplBuf), "\r\n", "\\n\"\r\n\"");
	dwRead = sprintf(chFileBuf, "%s%s\";", cmdVar, pDes);
	WriteFile(hInc, chFileBuf, dwRead, &dwWritten, NULL);
	
	CloseHandle(hBat);
	CloseHandle(hInc);
	
	return 0;
}

int createResourceFile(LPCTSTR iniFileName) {
	CHAR chBuf[BUFSIZE], chRcBuf[BUFSIZE], chReplBuf[BUFSIZE], *pDes;
	DWORD dwRead, dwWritten;
	WIN32_FIND_DATA FileData;
	HANDLE hSearch;

	ZeroMemory(chBuf, sizeof(chBuf));
	ZeroMemory(chRcBuf, sizeof(chRcBuf));
	ZeroMemory(chReplBuf, sizeof(chReplBuf));
	 
	HANDLE hRc = CreateFile("compile/Bat2Exe_private.rc", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	ReadFile(hRc, chRcBuf, BUFSIZE, &dwRead, NULL);
	CloseHandle(hRc);
	
	GetPrivateProfileString("info", "CompanyName", "", chBuf, BUFSIZE, iniFileName);
	pDes = strrepl(chRcBuf, chReplBuf, sizeof(chReplBuf), "CompanyName_EXE", chBuf);
	
	memcpy(chRcBuf, chReplBuf, sizeof(chRcBuf));
	GetPrivateProfileString("info", "FileVersion", "1.0.0.0", chBuf, BUFSIZE, iniFileName);
	pDes = strrepl(chRcBuf, chReplBuf, sizeof(chReplBuf), "FileVersion_EXE", chBuf);

	memcpy(chRcBuf, chReplBuf, sizeof(chRcBuf));
	ZeroMemory(chReplBuf, sizeof(chReplBuf));
	pDes = strrepl(chBuf, chReplBuf, sizeof(chReplBuf), ".", ",");
	pDes = strrepl(chRcBuf, chReplBuf, sizeof(chReplBuf), "FILE_VERSION_EXE", pDes);
	
	memcpy(chRcBuf, chReplBuf, sizeof(chRcBuf));
	GetPrivateProfileString("info", "ProductVersion", "1.0.0.0", chBuf, BUFSIZE, iniFileName);
	pDes = strrepl(chRcBuf, chReplBuf, sizeof(chReplBuf), "ProductVersion_EXE", chBuf);
	
	memcpy(chRcBuf, chReplBuf, sizeof(chRcBuf));
	ZeroMemory(chReplBuf, sizeof(chReplBuf));
	pDes = strrepl(chBuf, chReplBuf, sizeof(chReplBuf), ".", ",");
	pDes = strrepl(chRcBuf, chReplBuf, sizeof(chReplBuf), "PRODUCT_VERSION_EXE", pDes);
	
	memcpy(chRcBuf, chReplBuf, sizeof(chRcBuf));
	GetPrivateProfileString("info", "FileDescription", "", chBuf, BUFSIZE, iniFileName);
	pDes = strrepl(chRcBuf, chReplBuf, sizeof(chReplBuf), "FileDescription_EXE", chBuf);
	
	memcpy(chRcBuf, chReplBuf, sizeof(chRcBuf));
	GetPrivateProfileString("info", "InternalName", "", chBuf, BUFSIZE, iniFileName);
	pDes = strrepl(chRcBuf, chReplBuf, sizeof(chReplBuf), "InternalName_EXE", chBuf);
	
	memcpy(chRcBuf, chReplBuf, sizeof(chRcBuf));
	GetPrivateProfileString("info", "LegalCopyright", "", chBuf, BUFSIZE, iniFileName);
	pDes = strrepl(chRcBuf, chReplBuf, sizeof(chReplBuf), "LegalCopyright_EXE", chBuf);
	
	memcpy(chRcBuf, chReplBuf, sizeof(chRcBuf));
	GetPrivateProfileString("info", "LegalTrademarks", "", chBuf, BUFSIZE, iniFileName);
	pDes = strrepl(chRcBuf, chReplBuf, sizeof(chReplBuf), "LegalTrademarks_EXE", chBuf);
	
	memcpy(chRcBuf, chReplBuf, sizeof(chRcBuf));
	GetPrivateProfileString("info", "OriginalFilename", "", chBuf, BUFSIZE, iniFileName);
	pDes = strrepl(chRcBuf, chReplBuf, sizeof(chReplBuf), "OriginalFilename_EXE", chBuf);
	
	memcpy(chRcBuf, chReplBuf, sizeof(chRcBuf));
	GetPrivateProfileString("info", "ProductName", "", chBuf, BUFSIZE, iniFileName);
	pDes = strrepl(chRcBuf, chReplBuf, sizeof(chReplBuf), "ProductName_EXE", chBuf);
	
	hRc = CreateFile("compile/Bat2Exe_private.rc", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	WriteFile(hRc, pDes, strlen(pDes), &dwWritten, NULL);
	CloseHandle(hRc);
	
	ZeroMemory(chBuf, sizeof(chBuf));
	ZeroMemory(chRcBuf, sizeof(chRcBuf));
	GetPrivateProfileString("info", "IconPath", "", chBuf, BUFSIZE, iniFileName);
	if (strlen(chBuf) > 0) {
		hSearch = FindFirstFile(chBuf, &FileData); 
		if (hSearch != INVALID_HANDLE_VALUE) {
			sprintf(chRcBuf, "cmd.exe /c copy %s compile\\Bat2Exe.ico /Y", chBuf);
			system(chRcBuf);
		}
	} 
	
	return 0;
}

int afterCompile(LPCTSTR iniFileName) {
	CHAR chBuf[BUFSIZE], chCmd[BUFSIZE];
	
	ZeroMemory(chBuf, sizeof(chBuf));
	ZeroMemory(chCmd, sizeof(chCmd));
	GetPrivateProfileString("info", "OutputFile", "Bat2Exe.exe", chBuf, BUFSIZE, iniFileName);
	sprintf(chCmd, "cmd.exe /c copy compile\\exe\\Bat2Exe.exe %s /Y", chBuf);
	system(chCmd);
}

