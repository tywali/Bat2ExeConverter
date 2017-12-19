#include <stdio.h>
#include <windows.h>
#include "batcmd.h"

#define BUFSIZE 4096

HANDLE hChildStdinRd, hChildStdinWr, hChildStdinWrDup,
       hChildStdoutRd, hChildStdoutWr, hChildStdoutRdDup,
       hInputFile, hStdout;

BOOL CreateChildProcess(VOID);
VOID WriteToPipe(VOID);
VOID ReadFromPipe(VOID);
VOID ErrorExit(const char *);
VOID ErrMsg(LPTSTR, BOOL);

int main(int argc, char *argv[]) {
	/*DWORD dwRead;
	CHAR chBuf[BUFSIZE];
	
	ZeroMemory(chBuf, BUFSIZE);
	
	HANDLE hBat = CreateFile("example.bat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	ReadFile(hBat, chBuf, BUFSIZE, &dwRead, NULL);
	printf("%s", chBuf);
	CloseHandle(hBat);
	return 0;*/
	
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
    /*
    CHAR cmds[] = "@echo off\n"
                  "REM Windows 下无效\n"
                  "REM set PHP_FCGI_CHILDREN=5\n"
                  "REM 每个进程处理的最大请求数，或设置为 Windows 环境变量\n"
                  "set PHP_FCGI_MAX_REQUESTS=1000\n"
                  "echo Starting PHP FastCGI...\n"
                  "RunHiddenConsole D:/PHPDev/PHP/php-cgi.exe -b 127.0.0.1:9090 -c D:/PHPDev/PHP/php.ini\n"
                  "echo Starting nginx...\n"
                  "RunHiddenConsole D:/PHPDev/nginx-1.11.3/nginx.exe -p D:/PHPDev/nginx-1.11.3\n";
    */
    /*
    CHAR cmds[] = "@ECHO ON\n"
                    "cd..\n"
                    "dir\n"; 
    */

    WriteFile(hChildStdinWrDup, cmdStr, sizeof(cmdStr), &dwWritten, NULL);

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

VOID ErrorExit (const char *lpszMessage) {
    fprintf(stderr, "%s\n", lpszMessage);
    ExitProcess(0);
}
