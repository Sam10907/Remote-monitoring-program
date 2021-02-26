/*將程式執行畫面隱藏起來*/ 
//

#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <string.h>
#pragma warning( disable : 4996 )
using namespace std;

void exeCmd(const char*);

int _tmain(int argc, _TCHAR* argv[])
{
	/*char path[1000];
    GetCurrentDirectoryA(1000, path);
    strcat(path, "\\client.exe");*/
    exeCmd("C:\\Windows\\System32\\networktlist\\client.exe");
	//system("pause");
	return 0;
}
int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
    _tmain(0,NULL);
    return 0;
}
void exeCmd(const char* cmd) {
    char cmdPath[1000] = "c:\\windows\\system32\\cmd.exe /C ";
    strcat(cmdPath, cmd);
    //printf("%s\n",cmdPath);
    HANDLE hPipeRead, hPipeWrite;

    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
    saAttr.bInheritHandle = TRUE; // Pipe handles are inherited by child process.
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe to get results from child's stdout.
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        printf("pipe error\n");
        return;
    }

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    ZeroMemory(&si, sizeof(STARTUPINFOA));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput = hPipeWrite;
    si.hStdError = hPipeWrite;
    si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing.
                              // Requires STARTF_USESHOWWINDOW in dwFlags.

    PROCESS_INFORMATION pi = { 0 };
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    BOOL fSuccess = CreateProcessA(NULL, (LPSTR)cmdPath, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    if (!fSuccess)
    {
        CloseHandle(hPipeWrite);
        CloseHandle(hPipeRead);
        printf("create process error\n");
        DWORD errCode = GetLastError();
        printf("%lu\n", errCode);
        return;
    }
    /*u_long iMode = 1;
    ioctlsocket(sfd, FIONBIO, &iMode);*/
    bool bProcessEnded = false;
    for (; !bProcessEnded;)
    {
        // Give some timeslice (50 ms), so we won't waste 100% CPU.
        bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

        // Even if process exited - we continue reading, if
        // there is some data available over pipe.
        for (;;)
        {
            char buf[1024];
            DWORD dwRead = 0;
            DWORD dwAvail = 0;

            if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL)) {
                printf("nothing\n");
                break;
            }

            if (!dwAvail) { // No data available, return
                printf("nothing1\n");
                break;
            }

            if (!::ReadFile(hPipeRead, buf, min(sizeof(buf), dwAvail), &dwRead, NULL) || !dwRead) {
                // Error, the child process might ended
                printf("nothing3\n");
                break;
            }
            printf("%s", buf);
        }
    } //for
    /*iMode = 0;
    ioctlsocket(sfd, FIONBIO, &iMode);*/
    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

