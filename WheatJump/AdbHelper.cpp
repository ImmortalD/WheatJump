#include "stdafx.h"
#include "AdbHelper.h"


AdbHelper::AdbHelper()
{
}

BOOL AdbHelper::Exec(LPWSTR pszCmdline)
{
	PROCESS_INFORMATION pi;

	STARTUPINFO si;
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;

	BOOL bRet = ::CreateProcess(
		this->m_pszAdbPath,
		pszCmdline, // 在Unicode版本中此参数不能为常量字符串，因为此参数会被修改    
		NULL,
		NULL,
		FALSE,
		NULL,
		NULL,
		NULL,
		&si,
		&pi);

	int error = 0;

	if (bRet)
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
	}
	else
	{
		int error = GetLastError();
		printf("error code:%d/n", error);
	}

	return error == 0;
}

BOOL AdbHelper::Screenshot(LPWSTR pszSavePath)
{
	int nLen = wcslen(pszSavePath);
	WCHAR *szCommand = L" shell screencap -p ";
	int nCmdLen = wcslen(szCommand);

	WCHAR *szCommandAndArg = new WCHAR[nCmdLen + nLen + 1];
	memset(szCommandAndArg,0, nCmdLen + nLen + 1);

	StrCpyW(szCommandAndArg, szCommand);
	StrCpyW(szCommandAndArg + nCmdLen, pszSavePath);
	 
	return this->Exec(L" shell screencap -p /sdcard/jump.png");
}

 
BOOL AdbHelper::Copy(LPWSTR pszRemotePath, LPWSTR pszLocalPath)
{
	int nLen = wcslen(pszRemotePath) + wcslen(pszLocalPath);
	WCHAR *szCommand = new WCHAR[nLen + 1];
	memset(szCommand, 0, nLen + 1);

	StrCpyW(szCommand, L" pull ");
	StrCpyW(szCommand + 6, pszRemotePath);
	StrCpyW(szCommand + 6 + wcslen(pszRemotePath), L" ");
	StrCpyW(szCommand + 6 + wcslen(pszRemotePath)+1 , pszLocalPath);
	return this->Exec(szCommand);
}

