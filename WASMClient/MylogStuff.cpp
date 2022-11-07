#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

HANDLE  hLog = NULL;
char szMyExePath[MAX_PATH], szMyLogFile[MAX_PATH];
int nMyPathLen;
extern ULONGLONG systemStartTime;

HANDLE WINAPI MyLogCreateFile(char* psz, BOOL fNoPrev)
{

	if (!fNoPrev)
	{
		char szWrk[MAX_PATH], * psz2;
		strcpy(szWrk, psz);
		psz2 = strrchr(szWrk, '.');
		if (psz2)
		{
			*psz2 = 0;
			strcat(szWrk, "_prev.log");
			remove(szWrk);
			rename(psz, szWrk);
		}
	}

	HANDLE h = CreateFile(psz, GENERIC_WRITE, FILE_SHARE_READ, 0,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

//	char szLogBuffer[128];
//	DWORD dwRes;
//	sprintf(szLogBuffer, "Console Started ...\r\n\r\n");
//	WriteFile(h, szLogBuffer, (int)strlen(szLogBuffer), &dwRes, 0);

	return h;
}

void MyWriteLog(const char* fmt, ...)
{
	char szLogBuffer2[1014];
	char szLogBuffer[1024];
	DWORD dwRes;
	va_list va;
	va_start(va, fmt);
	vsprintf(szLogBuffer2, fmt, va);
	va_end(va);

	sprintf(szLogBuffer, "%8d: %s\r\n", (int)(GetTickCount64() - systemStartTime), szLogBuffer2);

	if (hLog == NULL) {
		GetModuleFileName(NULL, szMyExePath, MAX_PATH);
		nMyPathLen = (int)strlen(szMyExePath) - 3;
		strcpy(szMyLogFile, szMyExePath);
		strcpy(&szMyLogFile[nMyPathLen], "log");;
		hLog = MyLogCreateFile(szMyLogFile, FALSE);
		if (hLog == INVALID_HANDLE_VALUE) {
			hLog = NULL;
			return;
		}
	}
	WriteFile(hLog, szLogBuffer, (int)strlen(szLogBuffer), &dwRes, 0);
}
