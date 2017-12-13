#include <stdio.h>
#include <windows.h>
#include "tstring.h"
#include "prepare.h"

#define BUFSIZE 4096

HANDLE hChildStdinRd, hChildStdinWr, hChildStdinWrDup,
       hChildStdoutRd, hChildStdoutWr, hChildStdoutRdDup,
       hInputFile, hStdout;

int createIncludeFile(LPCSTR batFileName) {
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE], chFileBuf[BUFSIZE * 2], chReplBuf[BUFSIZE * 2], *pDes;
	CHAR cmdVar[] = "CHAR cmdStr[] = \"\"\r\n\"";
	
	ZeroMemory(chBuf, BUFSIZE);
	ZeroMemory(chFileBuf, BUFSIZE * 2);
	ZeroMemory(chReplBuf, BUFSIZE * 2);
	
	HANDLE hBat = CreateFile(batFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	ReadFile(hBat, chBuf, BUFSIZE, &dwRead, NULL);
	printf("%s", chBuf);
	
	HANDLE hInc = CreateFile("compile\\batcmd.h", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	pDes = strrepl(chBuf, chReplBuf, sizeof(chReplBuf), "\r\n", "\\n\"\r\n\"");
	dwRead = sprintf(chFileBuf, "%s%s\";", cmdVar, pDes);
	WriteFile(hInc, chFileBuf, dwRead, &dwWritten, NULL);
	
	CloseHandle(hBat);
	CloseHandle(hInc);
	
	return 0;
}

VOID ErrorExit (const char *lpszMessage) {
    fprintf(stderr, "%s\n", lpszMessage);
    ExitProcess(0);
}

BOOL CreateChildProcess() {
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bFuncRetn = FALSE;

    // Set up members of the PROCESS_INFORMATION structure.
    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

    // Set up members of the STARTUPINFO structure.
    // 设定DOS进程的标准输入、输出和错误信息的管道
    // 使用前面创建的值，DOS窗口的输入输出都会被定向到本应用中
    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = hChildStdoutWr;
    siStartInfo.hStdOutput = hChildStdoutWr;
    siStartInfo.hStdInput = hChildStdinRd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    char cmdLine[] = "cmd";
    // Create the child process.
    bFuncRetn = CreateProcess(NULL,
                              cmdLine,       // command line
                              NULL,          // process security attributes
                              NULL,          // primary thread security attributes
                              TRUE,          // handles are inherited
                              0,             // creation flags
                              NULL,          // use parent's environment
                              NULL,          // use parent's current directory
                              &siStartInfo,  // STARTUPINFO pointer
                              &piProcInfo);  // receives PROCESS_INFORMATION

    if (bFuncRetn == 0)
        ErrorExit("CreateProcess failed");
    else {
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
        return bFuncRetn;
    }
}

VOID WriteToPipe(VOID) {
    DWORD dwRead, dwWritten;
    CHAR chBuf[BUFSIZE];
    CHAR cmds[] = "cd compile\n"
                    "mingw32-make.exe -f Makefile.win all\n"
                    ""; 

    WriteFile(hChildStdinWrDup, cmds, sizeof(cmds), &dwWritten, NULL);

    // Close the pipe handle so the child process stops reading.
    if (! CloseHandle(hChildStdinWrDup))
        ErrorExit("Close pipe failed");
}

VOID ReadFromPipe(VOID) {
    DWORD dwRead, dwWritten;
    CHAR chBuf[BUFSIZE];

    // Close the write end of the pipe before reading from the
    // read end of the pipe.
    if (!CloseHandle(hChildStdoutWr))
        ErrorExit("CloseHandle failed");

    // Read output from the child process, and write to parent's STDOUT.
    // 获取子线程，即DOS窗口的输出，显示到标准输出设备上
    for (;;) {
        if( !ReadFile( hChildStdoutRdDup, chBuf, BUFSIZE, &dwRead,
                       NULL) || dwRead == 0) break;
        if (! WriteFile(hStdout, chBuf, dwRead, &dwWritten, NULL))
            break;
    }
}

int createBatExe() {
    // SECURITY_ATTRIBUTES结构包含一个对象的安全描述符，并指定检索到指定这个结构的句柄是否是可继承的。
    // 这个结构为很多函数创建对象时提供安全性设置
    SECURITY_ATTRIBUTES saAttr;
    BOOL fSuccess;

    // Set the bInheritHandle flag so pipe handles are inherited.
    // 设置句柄为可继承的，使得子线程可以使用父线程
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Get the handle to the current STDOUT.
    // 取得当前应用的标准输出句柄，对于Windows控制台应用来说，一般是输出到屏幕
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    // Create a pipe for the child process's STDOUT.
    // 创建一个用于输出操作的匿名管道。
    if (! CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
        ErrorExit("Stdout pipe creation failed\n");

    // Create noninheritable read handle and close the inheritable read handle.
    // 将输出管道的句柄绑定到当前进程
    fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdoutRd,
                               GetCurrentProcess(), &hChildStdoutRdDup , 0,
                               FALSE,
                               DUPLICATE_SAME_ACCESS);
    if( !fSuccess )
        ErrorExit("DuplicateHandle failed");
    CloseHandle(hChildStdoutRd);

    // Create a pipe for the child process's STDIN.
    // 创建一个用于输入操作的匿名管道。
    if (! CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0))
        ErrorExit("Stdin pipe creation failed\n");

    // Duplicate the write handle to the pipe so it is not inherited.
    // 将输入管道的句柄绑定到当前进程
    fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdinWr,
                               GetCurrentProcess(), &hChildStdinWrDup, 0,
                               FALSE,                  // not inherited
                               DUPLICATE_SAME_ACCESS);
    if (! fSuccess)
        ErrorExit("DuplicateHandle failed");

    CloseHandle(hChildStdinWr);

    // Now create the child process.
    // 创建DOS子进程
    fSuccess = CreateChildProcess();
    if (! fSuccess)
        ErrorExit("Create process failed");

    // Write to pipe that is the standard input for a child process.
    WriteToPipe();

    // Read from pipe that is the standard output for child process.
    ReadFromPipe();

    return 0;
}

int main(int argc, char *argv[]) {
	createIncludeFile("example.bat");
	CopyFile("template/Bat2Exe_private.rc", "compile/Bat2Exe_private.rc", false);
	CopyFile("template/main.cpp", "compile/main.cpp", false);
	CopyFile("template/Bat2Exe.ico", "compile/Bat2Exe.ico", false);
	DeleteFile("compile/obj/main.o");
	createResourceFile("./compile.ini");
	createBatExe();
	
	return 0;
}

