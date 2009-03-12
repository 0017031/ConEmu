#include "header.h"

CConEmuPipe::CConEmuPipe()
{
	for (int i=0; i<MAXCMDCOUNT; i++)
		hEventCmd[i] = NULL;
	hEventAlive=NULL;
	hEventReady=NULL;
	hMapping=NULL;
	dwMaxDataSize = 0;
	lpMap = NULL;
	lpCursor = NULL;
	sMapName[0] = 0;
	nPID = 0;
}

CConEmuPipe::~CConEmuPipe()
{
	for (int i=0; i<MAXCMDCOUNT; i++)
		SafeCloseHandle(hEventCmd[i]);
	SafeCloseHandle(hEventAlive);
	SafeCloseHandle(hEventReady);
	if (lpMap)
		UnmapViewOfFile(lpMap);
	SafeCloseHandle(hMapping);
}

#define CREATEEVENT(fmt,h) \
		wsprintf(szEventName, fmt, dwCurProcId ); \
		h = OpenEvent(EVENT_ALL_ACCESS, FALSE, szEventName); \
		if (h==INVALID_HANDLE_VALUE) h=NULL;

//#pragma message("warning: �������� �������� abSilent, ����� ������ ��� �� ��������?")
BOOL CConEmuPipe::Init(BOOL abSilent)
{
    if (!gConEmu.isFar() && !gConEmu.mn_TopProcessID) {
	    gConEmu.DnDstep(_T("Pipe: FAR not active"));
	    return FALSE;
	}

	// ���������� ��� �����, � �� ����� ������� ������������?
	DWORD dwCurProcId = gConEmu.mn_TopProcessID;
	nPID = dwCurProcId;
	wsprintf(sMapName, CONEMUMAPPING, dwCurProcId);
	
	WCHAR szEventName[64];

	CREATEEVENT(CONEMUDRAGFROM, hEventCmd[CMD_DRAGFROM]);
	CREATEEVENT(CONEMUDRAGTO, hEventCmd[CMD_DRAGTO]);
	CREATEEVENT(CONEMUREQTABS, hEventCmd[CMD_REQTABS]);
	CREATEEVENT(CONEMUSETWINDOW, hEventCmd[CMD_SETWINDOW]);
	CREATEEVENT(CONEMUPOSTMACRO, hEventCmd[CMD_POSTMACRO]);
	CREATEEVENT(CONEMUEXIT, hEventCmd[CMD_EXIT]);
	CREATEEVENT(CONEMUALIVE, hEventAlive);
	CREATEEVENT(CONEMUREADY, hEventReady);


	if (!hEventAlive || !hEventReady || 
		!hEventCmd[CMD_DRAGFROM] || !hEventCmd[CMD_DRAGTO] || 
		!hEventCmd[CMD_REQTABS] || !hEventCmd[CMD_SETWINDOW] || 
		!hEventCmd[CMD_POSTMACRO] || !hEventCmd[CMD_EXIT] )
	{
		if (!abSilent)
			MBoxA(_T("CreateEvent failed"));
		return FALSE;
	}

	return TRUE;
}

BOOL CConEmuPipe::Execute(int nCmd)
{
	if (nCmd<0 || nCmd>MAXCMDCOUNT) {
		TCHAR szError[128];
		swprintf(szError, _T("Invalid command id (%i)!"), nCmd);
		MBoxA(szError);
		return FALSE;
	}
	if (!hEventCmd[nCmd]) {
		TCHAR szError[128];
		swprintf(szError, _T("Command %i was not created by plugin!"), nCmd);
		MBoxA(szError);
		return FALSE;
	}
	SetEvent(hEventCmd[nCmd]);
	return TRUE;
}

LPBYTE CConEmuPipe::GetPtr()
{
	return lpCursor;
}

BOOL CConEmuPipe::Read(LPVOID pData, DWORD nSize, DWORD* nRead)
{
	if (nRead) *nRead = 0; // ���� �������

	if (hMapping==NULL) {
		// ����� ���������������� FileMapping
		
		hMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, // Read/write permission. 
			FALSE,                             // Do not inherit the name
			sMapName);            // of the mapping object. 
		if (hMapping == NULL) {
			hMapping = INVALID_HANDLE_VALUE;
			// ���� ������
			MessageBox(ghWnd, _T("Can't open filemapping"), sMapName, MB_ICONSTOP);
			return FALSE;
		}
		lpMap = (LPBYTE)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0,0,0);
		if (lpMap==NULL) {
			CloseHandle(hMapping); hMapping = INVALID_HANDLE_VALUE;
			MessageBox(ghWnd, _T("Can't map view of file"), sMapName, MB_ICONSTOP);
			return FALSE;
		}
		dwMaxDataSize = *((DWORD*)lpMap);
		lpCursor = lpMap+4;
	}
	if (hMapping==INVALID_HANDLE_VALUE || !hMapping) {
		if (nRead) *nRead=0;
		return FALSE;
	}

	if (!dwMaxDataSize)
		nSize = 0; else
	if ((lpCursor-lpMap+nSize)>dwMaxDataSize)
		nSize = dwMaxDataSize - (lpCursor-lpMap);

	if (nSize) {
		memmove(pData, lpCursor, nSize);
		lpCursor += nSize;
	}
	if (nRead) *nRead = nSize;

	return (nSize!=0);
}
