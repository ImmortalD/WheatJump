#include "stdafx.h"
#include "AdbHelper.h"


AdbHelper::AdbHelper()
{
}
#include <string>
using namespace std;
 
BOOL AdbHelper::Exec(LPWSTR pszCmdline, CString *szRet)
{
	HANDLE hRead=NULL, hWrite=NULL;
	SECURITY_ATTRIBUTES sa;
	if (NULL != szRet)
	{
		
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = 0;
		sa.bInheritHandle = TRUE;
		CreatePipe(&hRead, &hWrite, &sa, 0);
	}

	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	if (NULL != szRet)
	{
		si.hStdOutput = si.hStdError = hWrite;
	}

	// 创建一个新进程，并且设置读写管道，创建成功后，进程会自动再创建一个写管道端口
	BOOL bRet = ::CreateProcess(
		this->m_pszAdbPath,
		pszCmdline,
		 // 在Unicode版本中此参数不能为常量字符串，因为此参数会被修改    
		NULL,
		NULL,
		TRUE,
		NULL,
		NULL,
		NULL,
		&si,
		&pi);

	if (NULL != szRet)
	{
		// 因为我们开始已经创建了一个写管道端口，现在进程创键时又自动创键了一个写管道端口
		// 所以应关闭多余的管道写端口
		CloseHandle(hWrite);

		DWORD dwReadLen = 0;
		while (dwReadLen != -1)
		{
			// 查看管道中是否有数据
			PeekNamedPipe(hRead, NULL, NULL, NULL, &dwReadLen, NULL);
			TRACE(L"\nlen=%d\n", dwReadLen);
			if (0 != dwReadLen)
			{
				// 从管道中读取数据
				char *szReadBuff = new char[dwReadLen]{ 0 };
				// 从管道中读取数据
				ReadFile(hRead, szReadBuff, dwReadLen, &dwReadLen, NULL);
				*szRet += szReadBuff;
				TRACE(L"读取数据是:%s\n", CString(szReadBuff));
				delete szReadBuff;
			}
			else
			{
				DWORD dwExitCode;
				// 查看指定进程是否已结束
				GetExitCodeProcess(pi.hProcess, &dwExitCode);
				// 因为会有程序退出来了，但管道中还有数据的情况，所以必须在管道中没有数据的情
				// 况下判断程序是否已结束，这时才能中止循环！
				if (dwExitCode != STILL_ACTIVE)
				{
					// 关闭管道
					CloseHandle(hRead);
					break;
				}
			}
		}
	}
	else
	{
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
	
	return bRet;
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

