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

	// ����һ���½��̣��������ö�д�ܵ��������ɹ��󣬽��̻��Զ��ٴ���һ��д�ܵ��˿�
	BOOL bRet = ::CreateProcess(
		this->m_pszAdbPath,
		pszCmdline,
		 // ��Unicode�汾�д˲�������Ϊ�����ַ�������Ϊ�˲����ᱻ�޸�    
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
		// ��Ϊ���ǿ�ʼ�Ѿ�������һ��д�ܵ��˿ڣ����ڽ��̴���ʱ���Զ�������һ��д�ܵ��˿�
		// ����Ӧ�رն���Ĺܵ�д�˿�
		CloseHandle(hWrite);

		DWORD dwReadLen = 0;
		while (dwReadLen != -1)
		{
			// �鿴�ܵ����Ƿ�������
			PeekNamedPipe(hRead, NULL, NULL, NULL, &dwReadLen, NULL);
			TRACE(L"\nlen=%d\n", dwReadLen);
			if (0 != dwReadLen)
			{
				// �ӹܵ��ж�ȡ����
				char *szReadBuff = new char[dwReadLen]{ 0 };
				// �ӹܵ��ж�ȡ����
				ReadFile(hRead, szReadBuff, dwReadLen, &dwReadLen, NULL);
				*szRet += szReadBuff;
				TRACE(L"��ȡ������:%s\n", CString(szReadBuff));
				delete szReadBuff;
			}
			else
			{
				DWORD dwExitCode;
				// �鿴ָ�������Ƿ��ѽ���
				GetExitCodeProcess(pi.hProcess, &dwExitCode);
				// ��Ϊ���г����˳����ˣ����ܵ��л������ݵ���������Ա����ڹܵ���û�����ݵ���
				// �����жϳ����Ƿ��ѽ�������ʱ������ֹѭ����
				if (dwExitCode != STILL_ACTIVE)
				{
					// �رչܵ�
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

