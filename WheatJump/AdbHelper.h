#pragma once
class AdbHelper
{
public:
	AdbHelper();
	
	AdbHelper(LPWSTR pszAdbPath)
	{
		this->m_pszAdbPath = pszAdbPath;
	}

	BOOL Exec(LPWSTR pszCmdline, CString *szRet = NULL);

	void SetAdbPath(LPWSTR pszAdbPath)
	{
		this->m_pszAdbPath = pszAdbPath;
	}
    BOOL Screenshot(LPWSTR pszSavePath);

	BOOL Copy(LPWSTR pszRemotePath, LPWSTR pszLocalPath);
	~AdbHelper()
	{ 
		
	}
private:
	LPWSTR m_pszAdbPath;
};

