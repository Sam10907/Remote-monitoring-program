/*向使用者取得最高權限來執行regedit.cpp所編譯成的regedit1.exe*/ 
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include "strsafe.h"
#pragma warning( disable : 4996 )

int _tmain(int argc, _TCHAR* argv[])
{
	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO); 
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = "runas";	
	ShExecInfo.lpFile = "regedit1.exe";
	ShExecInfo.lpParameters = "";
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	return 0;
}

