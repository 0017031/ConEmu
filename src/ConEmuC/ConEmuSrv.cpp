
#include "ConEmuC.h"


/*
!! �� ��������� ���������������� ������ ������������ ��������� MAP ������ ��� ����� (Info), ���� �� ������
   ��������� � �������������� MAP, "ConEmuFileMapping.%08X.N", ��� N == nCurDataMapIdx
   ���������� ��� ��� ����, ����� ��� ���������� ������� ������� ����� ���� ������������� ��������� ����� ������
   � ����� ����������� ������ �� ���� �������

!! ��� ������, � ������� ����� �������� ������ � ������� ������ ������� �������. �.�. ��� � ������ ��������
   ��� ����� ��������� - ������� � ��������� ������� ���� �� ������.

1. ReadConsoleData ������ ����� �����������, ������ ������ ������ 30K? ���� ������ - �� � �� �������� ������ ������.
2. ReadConsoleInfo & ReadConsoleData ������ ���������� TRUE ��� ������� ��������� (��������� ������ ���������� ��������� ������ ��� ���������)
3. ReloadFullConsoleInfo �������� ����� ReadConsoleInfo & ReadConsoleData ������ ���� ���. ������� ������� ������ ��� ������� ���������. �� ����� �������� Tick

4. � RefreshThread ����� ������� hRefresh 10�� (������) ��� hExit.
-- ���� ���� ������ �� ��������� ������� - 
   ������� � TRUE ��������� ���������� bChangingSize
   ������ ������ � ������ ����� �����
-- ����� ReloadFullConsoleInfo
-- ����� ���, ���� bChangingSize - ���������� Event hSizeChanged.
5. ����� ��� ����������� ������. ��� ������ ��� �� �����, �.�. ��� ������ ������� ����� � RefreshThread.
6. ������� ����� ������� �� ������ ���� ������ ������, � ���������� ������ � RefreshThread � ����� ������� hSizeChanged
*/


int CreateMapHeader()
{
	int iRc = 0;
	wchar_t szMapName[64];
	int nConInfoSize = sizeof(CESERVER_REQ_CONINFO_HDR);
	
	_ASSERTE(srv.hFileMapping == NULL);
	
	wsprintf(szMapName, CECONMAPNAME, (DWORD)ghConWnd);
	srv.hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, 
		gpNullSecurity, PAGE_READWRITE, 0, nConInfoSize, szMapName);
	if (!srv.hFileMapping) {
		DWORD dwErr = GetLastError();
		_printf ("Can't create console data file mapping. ErrCode=0x%08X\n", dwErr, szMapName);
		iRc = CERR_CREATEMAPPINGERR; goto wrap;
	}
	
	srv.pConsoleInfo = (CESERVER_REQ_CONINFO_HDR*)MapViewOfFile(srv.hFileMapping, FILE_MAP_ALL_ACCESS,0,0,0);
	if (!srv.pConsoleInfo) {
		DWORD dwErr = GetLastError();
		_printf ("Can't map console info. ErrCode=0x%08X\n", dwErr, szMapName);
		iRc = CERR_MAPVIEWFILEERR; goto wrap;
	}
	memset(srv.pConsoleInfo, 0, nConInfoSize);
	
	srv.pConsoleInfo->nServerPID = GetCurrentProcessId();

wrap:	
	return iRc;
}

static void CloseMapData()
{
	if (srv.pConsoleData) {
		UnmapViewOfFile(srv.pConsoleData);
		srv.pConsoleData = NULL;
	}
	if (srv.hFileMappingData) {
		CloseHandle(srv.hFileMappingData);
		srv.hFileMappingData = NULL;
	}
	if (srv.pConsoleDataCopy) {
		Free(srv.pConsoleDataCopy);
		srv.pConsoleDataCopy = NULL;
	}
}

static BOOL RecreateMapData()
{
	BOOL lbRc = FALSE;
	DWORD dwErr = 0;
	DWORD nNewIndex = 0;
	DWORD nMaxSize = (srv.sbi.dwMaximumWindowSize.X * srv.sbi.dwMaximumWindowSize.Y * 2) * sizeof(CHAR_INFO);
	wchar_t szErr[255]; szErr[0] = 0;
	wchar_t szMapName[64];
	
	_ASSERTE(srv.pConsoleInfo);
	if (!srv.pConsoleInfo) {
		lstrcpyW(szErr, L"ConEmuC: RecreateMapData failed, srv.pConsoleInfo is NULL");
		goto wrap;
	}
	
	if (srv.pConsoleData)
		CloseMapData();
		
		
	srv.pConsoleDataCopy = (CHAR_INFO*)Alloc(nMaxSize,1);
	if (!srv.pConsoleDataCopy) {
		wsprintf (szErr, L"ConEmuC: Alloc(%i) failed, pConsoleDataCopy is null", nMaxSize);
		goto wrap;
	}
	

	nNewIndex = srv.pConsoleInfo->nCurDataMapIdx;
	nNewIndex++;
	
	wsprintf(szMapName, CECONMAPNAME L".%i", (DWORD)ghConWnd, nNewIndex);

	
	srv.hFileMappingData = CreateFileMapping(INVALID_HANDLE_VALUE, 
		gpNullSecurity, PAGE_READWRITE, 0, nMaxSize, szMapName);
	if (!srv.hFileMappingData) {
		dwErr = GetLastError();
		wsprintf (szErr, L"ConEmuC: CreateFileMapping(%s) failed. ErrCode=0x%08X", szMapName, dwErr);
		goto wrap;
	}
	
	srv.pConsoleData = (CHAR_INFO*)MapViewOfFile(srv.hFileMappingData, FILE_MAP_ALL_ACCESS,0,0,0);
	if (!srv.pConsoleData) {
		dwErr = GetLastError();
		CloseHandle(srv.hFileMappingData); srv.hFileMappingData = NULL;
		wsprintf (szErr, L"ConEmuC: MapViewOfFile(%s) failed. ErrCode=0x%08X", szMapName, dwErr);
		goto wrap;
	}
	memset(srv.pConsoleData, 0, nMaxSize);
	
	srv.nConsoleDataSize = nMaxSize;
	
	// Done
	srv.pConsoleInfo->nCurDataMapIdx = nNewIndex;
	srv.pConsoleInfo->nCurDataMaxSize = nMaxSize;

	lbRc = TRUE;
wrap:
	if (!lbRc && szErr[0])
		SetConsoleTitle(szErr);
	return lbRc;
}

void CloseMapHeader()
{
	CloseMapData();
	
	if (srv.pConsoleInfo) {
		UnmapViewOfFile(srv.pConsoleInfo);
		srv.pConsoleInfo = NULL;
	}
	if (srv.hFileMapping) {
		CloseHandle(srv.hFileMapping);
		srv.hFileMapping = NULL;
	}
}


// ���������� TRUE - ���� ������ ������ ������� ������� (��� ����� ��������� � �������)
BOOL CorrectVisibleRect(CONSOLE_SCREEN_BUFFER_INFO* pSbi)
{
	BOOL lbChanged = FALSE;
	_ASSERTE(gcrBufferSize.Y<200); // ������ ������� �������

	// ���������� �������������� ���������
	SHORT nLeft = 0;
	SHORT nRight = pSbi->dwSize.X - 1;
	SHORT nTop = pSbi->srWindow.Top;
	SHORT nBottom = pSbi->srWindow.Bottom;

	if (gnBufferHeight == 0) {
		// ������ ��� ��� �� ������ ������������ �� ��������� ������ BufferHeight
		if (pSbi->dwMaximumWindowSize.Y < pSbi->dwSize.Y) {
			// ��� ���������� �������� �����, �.�. ������ ������ ������ ����������� ����������� ������� ����

			// ������ ���������� ��������. �������� VBinDiff ������� ������ ���� �����,
			// �������������� ��� ������ ���������, � ��� ������ ��������� ��...
			//_ASSERTE(pSbi->dwMaximumWindowSize.Y >= pSbi->dwSize.Y);

			gnBufferHeight = pSbi->dwSize.Y; 
		}
	}

	// ���������� ������������ ��������� ��� �������� ������
	if (gnBufferHeight == 0) {
		nTop = 0; 
		nBottom = pSbi->dwSize.Y - 1;

	} else if (srv.nTopVisibleLine!=-1) {
		// � ��� '���������' ������ ������� ����� ���� �������������
		nTop = srv.nTopVisibleLine;
		nBottom = min( (pSbi->dwSize.Y-1), (srv.nTopVisibleLine+gcrBufferSize.Y-1) );

	} else {
		// ������ ������������ ������ ������ �� ������������� � GUI �������
		// ������ �� ��� ��������� ������� ���, ����� ������ ��� �����

		if (pSbi->dwCursorPosition.Y == pSbi->srWindow.Bottom)
		{
			// ���� ������ ��������� � ������ ������� ������ (������������, ��� ����� ���� ������������ ������� ������)
			nTop = pSbi->dwCursorPosition.Y - gcrBufferSize.Y + 1; // ���������� ������� ����� �� �������
		} else {
			// ����� - ���������� ����� (��� ����) ����������, ����� ������ ���� �����
			if ((pSbi->dwCursorPosition.Y < pSbi->srWindow.Top) || (pSbi->dwCursorPosition.Y > pSbi->srWindow.Bottom)) {
				nTop = pSbi->dwCursorPosition.Y - gcrBufferSize.Y + 1;
			}
		}
		// ��������� �� ������ �� �������
		if (nTop<0) nTop = 0;

		// ������������ ������ ������� �� ������� + �������� ������ ������� �������
		nBottom = (nTop + gcrBufferSize.Y - 1);
		// ���� �� ��������� ��� �������� �� ������� ������ (���� �� ������ ��?)
		if (nBottom >= pSbi->dwSize.Y) {
			// ������������ ���
			nBottom = pSbi->dwSize.Y - 1;
			// � ���� �� ��������� �������
			nTop = max(0, (nBottom - gcrBufferSize.Y + 1));
		}
	}

#ifdef _DEBUG
	if ((pSbi->srWindow.Bottom - pSbi->srWindow.Top)>pSbi->dwMaximumWindowSize.Y) {
		_ASSERTE((pSbi->srWindow.Bottom - pSbi->srWindow.Top)<pSbi->dwMaximumWindowSize.Y);
	}
#endif

	if (nLeft != pSbi->srWindow.Left
		|| nRight != pSbi->srWindow.Right
		|| nTop != pSbi->srWindow.Top
		|| nBottom != pSbi->srWindow.Bottom)
		lbChanged = TRUE;

	return lbChanged;
}






static BOOL ReadConsoleInfo()
{
	BOOL lbRc = TRUE;
	BOOL lbChanged = srv.pConsoleData ? FALSE : TRUE; // ��� ������� ������ - ����� TRUE
	//CONSOLE_SELECTION_INFO lsel = {0}; // GetConsoleSelectionInfo
	CONSOLE_CURSOR_INFO lci = {0}; // GetConsoleCursorInfo
	DWORD ldwConsoleCP=0, ldwConsoleOutputCP=0, ldwConsoleMode=0;
	CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}}; // MyGetConsoleScreenBufferInfo

	HANDLE hOut = (HANDLE)ghConOut;
	if (hOut == INVALID_HANDLE_VALUE)
		hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// ����� ��������� �������� ��� �������� ComSpec � ���������� ������ ������
	MCHKHEAP;

	if (!GetConsoleCursorInfo(hOut, &lci)) { srv.dwCiRc = GetLastError(); if (!srv.dwCiRc) srv.dwCiRc = -1; } else {
		srv.dwCiRc = 0;
		if (memcmp(&srv.ci, &lci, sizeof(srv.ci))) {
			srv.ci = lci;
			lbChanged = TRUE;
		}
	}

	ldwConsoleCP = GetConsoleCP();
	if (srv.dwConsoleCP!=ldwConsoleCP) {
		srv.dwConsoleCP = ldwConsoleCP; lbChanged = TRUE;
	}

	ldwConsoleOutputCP = GetConsoleOutputCP();
	if (srv.dwConsoleOutputCP!=ldwConsoleOutputCP) {
		srv.dwConsoleOutputCP = ldwConsoleOutputCP; lbChanged = TRUE;
	}

	ldwConsoleMode = 0;	GetConsoleMode(ghConIn, &ldwConsoleMode);
	if (srv.dwConsoleMode!=ldwConsoleMode) {
		srv.dwConsoleMode = ldwConsoleMode; lbChanged = TRUE;
	}

	MCHKHEAP;

	if (!MyGetConsoleScreenBufferInfo(hOut, &lsbi)) {
		srv.dwSbiRc = GetLastError(); if (!srv.dwSbiRc) srv.dwSbiRc = -1;
		lbRc = FALSE;
	} else {
		// ���������� ���������� ����� �������� ������ ������
		if (!NTVDMACTIVE // �� ��� ���������� 16������ ���������� - ��� �� ��� ������ ���������, ����� �������� ������ ��� �������� 16���
			/*&& (gcrBufferSize.Y != lsbi.dwSize.Y || gnBufferHeight != 0)*/ // ��� ����� �������� �������
			&& (lsbi.srWindow.Top == 0 // ��� ���� ������������ ������� ������
			    && lsbi.dwSize.Y == (lsbi.srWindow.Bottom - lsbi.srWindow.Top + 1)))
		{
			// ��� ������, ��� ��������� ���, � ���������� ���������� �������� ������ ������
			gnBufferHeight = 0; gcrBufferSize = lsbi.dwSize;
		}

		if (memcmp(&srv.sbi, &lsbi, sizeof(srv.sbi))) {
			srv.sbi = lsbi;
			lbChanged = TRUE;
		}
	}

	if (!gnBufferHeight && srv.sbi.dwSize.Y > 200) {
		_ASSERTE(srv.sbi.dwSize.Y <= 200);
		DEBUGLOGSIZE(L"!!! srv.sbi.dwSize.Y > 200 !!! in ConEmuC.ReloadConsoleInfo");
		//	Sleep(10);
		//} else {
		//	break; // OK
	}
	
	// ����� ������ ������, ����� ������ ���� �������������� ����������
	srv.pConsoleInfo->hConWnd = ghConWnd;
	srv.pConsoleInfo->nServerPID = GetCurrentProcessId();
	srv.pConsoleInfo->nInputTID = srv.dwInputThreadId;
    srv.pConsoleInfo->dwCiSize = sizeof(srv.ci);
	srv.pConsoleInfo->ci = srv.ci;
    srv.pConsoleInfo->dwConsoleCP = srv.dwConsoleCP;
	srv.pConsoleInfo->dwConsoleOutputCP = srv.dwConsoleOutputCP;
	srv.pConsoleInfo->dwConsoleMode = srv.dwConsoleMode;
	srv.pConsoleInfo->dwSbiSize = sizeof(srv.sbi);
	srv.pConsoleInfo->sbi = srv.sbi;


//wrap:
	// ���� ���� ����������� (WinXP+) - ������� �������� ������ ��������� �� �������
	//CheckProcessCount(); -- ��� ������ ���� ������� !!!
	GetProcessCount(srv.pConsoleInfo->nProcesses, countof(srv.pConsoleInfo->nProcesses));
	_ASSERTE(srv.pConsoleInfo->nProcesses[0]);
	
	return lbChanged;
}

// !! test test !!

// !!! ������� ������� ������ ������ ������� ��������, ��� ���� ������ �������� �������� �����
// !!! ������ 1000 ��� ������ ������ �������� 140x76 �������� 100��.
// !!! ������ 1000 ��� �� ������ (140x1) �������� 30��.
// !!! ������. �� ��������� ���������� ���� � ������ ��������� ������ ������ ��� ���������.
// !!! � ������� �� ���� ���������� ������ - ������������ � ������� ���� ������������� ������.


static BOOL ReadConsoleData()
{
	BOOL lbRc = FALSE, lbChanged = FALSE;
	
#ifdef _DEBUG
	CONSOLE_SCREEN_BUFFER_INFO dbgSbi = srv.sbi;
#endif

	USHORT TextWidth=0, TextHeight=0;
	DWORD TextLen=0;
	COORD bufSize, bufCoord;
	SMALL_RECT rgn;
	
	_ASSERTE(srv.sbi.srWindow.Left == 0);
	_ASSERTE(srv.sbi.srWindow.Right == (srv.sbi.dwSize.X - 1));

	TextWidth  = srv.sbi.dwSize.X;
	TextHeight = (srv.sbi.srWindow.Bottom - srv.sbi.srWindow.Top + 1);
	TextLen = TextWidth * TextHeight;
	
	DWORD nCurSize = TextLen * sizeof(CHAR_INFO);
	
	if (!srv.pConsoleData || srv.nConsoleDataSize < nCurSize)
	{
		// ���� MapFile ��� �� ����������, ��� ��� �������� ������ �������
		if (!RecreateMapData())
		{
			// ��� �� ������� ����������� MapFile - �� � ��������� �� �����...
			goto wrap;
		}
		
		_ASSERTE(srv.nConsoleDataSize >= nCurSize);
	}

	HANDLE hOut = (HANDLE)ghConOut;
	if (hOut == INVALID_HANDLE_VALUE)
		hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	lbRc = FALSE;
	
	if (nCurSize <= MAX_CONREAD_SIZE)
	{
		bufSize.X = TextWidth; bufSize.Y = TextHeight;
		bufCoord.X = 0; bufCoord.Y = 0;
		rgn = srv.sbi.srWindow;
		if (ReadConsoleOutput(hOut, srv.pConsoleDataCopy, bufSize, bufCoord, &rgn))
			lbRc = TRUE;
	}
	
	if (!lbRc)
	{
		// �������� ������ ���������
		bufSize.X = TextWidth; bufSize.Y = 1;
		bufCoord.X = 0; bufCoord.Y = 0;
		rgn = srv.sbi.srWindow;
		CHAR_INFO* pLine = srv.pConsoleDataCopy;
		for(int y = 0; y < (int)TextHeight; y++, rgn.Top++, pLine+=TextWidth)
		{
			rgn.Bottom = rgn.Top;
			ReadConsoleOutput(hOut, pLine, bufSize, bufCoord, &rgn);
		}
	}
	
	if (memcmp(srv.pConsoleData, srv.pConsoleDataCopy, nCurSize)) {
		memmove(srv.pConsoleData, srv.pConsoleDataCopy, nCurSize);
		lbChanged = TRUE;
	}
	
wrap:
	return lbChanged;
}




// abForceSend ������������ � TRUE, ����� ��������������
// ����������� GUI �� �������� (�� ���� 1 ���).
BOOL ReloadFullConsoleInfo(BOOL abForceSend)
{
	BOOL lbChanged = abForceSend;
	DWORD dwCurThId = GetCurrentThreadId();

	// ������ ���������� ������ � ������� ���� (RefreshThread)
	// ����� �������� ����������
	if (srv.dwRefreshThread && dwCurThId != srv.dwRefreshThread) {
		ResetEvent(srv.hDataSentEvent);
		
		SetEvent(srv.hRefreshEvent);
		// ��������, ���� ��������� RefreshThread
		HANDLE hEvents[2] = {ghQuitEvent, srv.hRefreshEvent};
		DWORD nWait = WaitForMultipleObjects ( 2, hEvents, FALSE, RELOAD_INFO_TIMEOUT );
		lbChanged = (nWait == (WAIT_OBJECT_0+1));
		
		return lbChanged;
	}

	DWORD nPacketID = srv.pConsoleInfo->nPacketId;
	
	if (ReadConsoleInfo())
		lbChanged = TRUE;

	// ���� ������ ���������� � ������� ������ �������, � ����� ������ ����� ������ �� ��������...
	//if (nPacketID == srv.pConsoleInfo->nPacketId)
	//{
	if (ReadConsoleData())
		lbChanged = TRUE;
	//}

	if (lbChanged)
	{
		// ��������� ������� � Tick
		if (nPacketID == srv.pConsoleInfo->nPacketId) {
			srv.pConsoleInfo->nPacketId++;
			TODO("����� �������� �� multimedia tick");
			srv.pConsoleInfo->nSrvUpdateTick = GetTickCount();
		}
	}
	
	SetEvent(srv.hDataSentEvent);

	return lbChanged;
}





DWORD WINAPI WinEventThread(LPVOID lpvParam)
{
	//DWORD dwErr = 0;
	//HANDLE hStartedEvent = (HANDLE)lpvParam;

	// �� ������ ������
	srv.dwWinEventThread = GetCurrentThreadId();


	// �� ��������� - ����� ������ StartStop.
	// ��� ��������� � ������� FAR'� - ������� ��� �������
	//srv.bWinHookAllow = TRUE; srv.nWinHookMode = 1;
	//HookWinEvents ( TRUE );
	_ASSERTE(srv.hWinHookStartEnd==NULL);

	// "�����" (Start/End)
	srv.hWinHookStartEnd = SetWinEventHook(EVENT_CONSOLE_START_APPLICATION,EVENT_CONSOLE_END_APPLICATION,
		NULL, (WINEVENTPROC)WinEventProc, 0,0, WINEVENT_OUTOFCONTEXT /*| WINEVENT_SKIPOWNPROCESS ?*/);

	if (!srv.hWinHookStartEnd) {
		PRINT_COMSPEC(L"!!! HookWinEvents(StartEnd) FAILED, ErrCode=0x%08X\n", GetLastError());
		return 1; // �� ������� ���������� ���, ������ � ���� ���� ���, �������
	}

	PRINT_COMSPEC(L"WinEventsHook(StartEnd) was enabled\n", 0);



	//
	//SetEvent(hStartedEvent); hStartedEvent = NULL; // ����� �� ����� �� ���������

	MSG lpMsg;
	//while (GetMessage(&lpMsg, NULL, 0, 0)) -- ������� �� Peek ����� ���������� ���� ��������
	while (TRUE)
	{
		if (!PeekMessage(&lpMsg, 0,0,0, PM_REMOVE)) {
			Sleep(10);
			continue;
		}
		// 	if (lpMsg.message == srv.nMsgHookEnableDisable) {
		// 		srv.bWinHookAllow = (lpMsg.wParam != 0);
		//HookWinEvents ( srv.bWinHookAllow ? srv.nWinHookMode : 0 );
		// 		continue;
		// 	}
		MCHKHEAP;
		if (lpMsg.message == WM_QUIT) {
			//          lbQuit = TRUE;
			break;
		}
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
		MCHKHEAP;
	}

	// ������� ���
	//HookWinEvents ( FALSE );
	if (/*abEnabled == -1 &&*/ srv.hWinHookStartEnd) {
		UnhookWinEvent(srv.hWinHookStartEnd); srv.hWinHookStartEnd = NULL;
		PRINT_COMSPEC(L"WinEventsHook(StartEnd) was disabled\n", 0);
	}

	MCHKHEAP;

	return 0;
}


//Minimum supported client Windows 2000 Professional 
//Minimum supported server Windows 2000 Server 
void WINAPI WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (hwnd != ghConWnd) {
		// ���� ��� �� ���� ���� - �������
		return;
	}

	//BOOL bNeedConAttrBuf = FALSE;
	//CESERVER_CHAR ch = {{0,0}};
	#ifdef _DEBUG
	WCHAR szDbg[128];
	#endif

	switch(event)
	{
	case EVENT_CONSOLE_START_APPLICATION:
		//A new console process has started. 
		//The idObject parameter contains the process identifier of the newly created process. 
		//If the application is a 16-bit application, the idChild parameter is CONSOLE_APPLICATION_16BIT and idObject is the process identifier of the NTVDM session associated with the console.

		#ifdef _DEBUG
		#ifndef WIN64
			_ASSERTE(CONSOLE_APPLICATION_16BIT==1);
		#endif
		wsprintfW(szDbg, L"EVENT_CONSOLE_START_APPLICATION(PID=%i%s)\n", idObject, 
			(idChild == CONSOLE_APPLICATION_16BIT) ? L" 16bit" : L"");
		DEBUGSTR(szDbg);
		#endif

		if (((DWORD)idObject) != gnSelfPID) {
			CheckProcessCount(TRUE);
			/*
			EnterCriticalSection(&srv.csProc);
			srv.nProcesses.push_back(idObject);
			LeaveCriticalSection(&srv.csProc);
			*/

			#ifndef WIN64
			_ASSERTE(CONSOLE_APPLICATION_16BIT==1);
			if (idChild == CONSOLE_APPLICATION_16BIT) {
				srv.bNtvdmActive = TRUE;
				srv.nNtvdmPID = idObject;
				SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

				// ��� ������ ������ ��� ������... ����� ������� �� ������� �� 16��� ����������...
				// ��������� �������� ������ - 25/28/50 �����. ��� ������ - ������ 28
				// ������ - ������ 80 ��������
			}
			#endif
			//
			//HANDLE hIn = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
			//                  0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			//if (hIn != INVALID_HANDLE_VALUE) {
			//  HANDLE hOld = ghConIn;
			//  ghConIn = hIn;
			//  SafeCloseHandle(hOld);
			//}
		}
		return; // ���������� ������ �� ���������

	case EVENT_CONSOLE_END_APPLICATION:
		//A console process has exited. 
		//The idObject parameter contains the process identifier of the terminated process.

		#ifdef _DEBUG
		wsprintfW(szDbg, L"EVENT_CONSOLE_END_APPLICATION(PID=%i%s)\n", idObject, 
			(idChild == CONSOLE_APPLICATION_16BIT) ? L" 16bit" : L"");
		DEBUGSTR(szDbg);
		#endif

		if (((DWORD)idObject) != gnSelfPID) {
			CheckProcessCount(TRUE);

			#ifndef WIN64
			_ASSERTE(CONSOLE_APPLICATION_16BIT==1);
			if (idChild == CONSOLE_APPLICATION_16BIT) {
				//DWORD ntvdmPID = idObject;
				//dwActiveFlags &= ~CES_NTVDM;
				srv.bNtvdmActive = FALSE;
				//TODO: �������� ����� ������� ������� NTVDM?
				SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
			}
			#endif

			//
			//HANDLE hIn = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
			//                  0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			//if (hIn != INVALID_HANDLE_VALUE) {
			//  HANDLE hOld = ghConIn;
			//  ghConIn = hIn;
			//  SafeCloseHandle(hOld);
			//}
		}
		return; // ���������� ������ �� ���������
	}
}














DWORD WINAPI RefreshThread(LPVOID lpvParam)
{
	DWORD nWait = 0;
	HANDLE hEvents[2] = {ghQuitEvent, srv.hRefreshEvent};
	DWORD nDelta = 0;
	DWORD nLastUpdateTick = GetTickCount();
	DWORD nLastConHandleTick = nLastUpdateTick;
	BOOL  lbEventualChange = FALSE, lbForceSend = FALSE, lbChanged = FALSE, lbProcessChanged = FALSE;
	DWORD dwTimeout = 10; // ������������� ������ ���������� �� ���� (��������, �������,...)


	while (TRUE)
	{
		nWait = WAIT_TIMEOUT;
		lbForceSend = FALSE;
		MCHKHEAP;

		// Alwas update con handle, ������ �������
		if ((GetTickCount() - nLastConHandleTick) > UPDATECONHANDLE_TIMEOUT) {
			WARNING("!!! MS - ������. � Win7 �������� ����������� � ������ �������� - ��������� ���������� ����� ���������!!!");
			#ifdef _DEBUG
			HANDLE hHandle = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
				0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if (hHandle)
				CloseHandle(hHandle);
			#endif
			// � �����, ����� ������ telnet'� ������������!
			ghConOut.Close();
			nLastConHandleTick = GetTickCount();
		}
		
		// ������� ��������� CECMD_SETCONSOLECP
		if (srv.hLockRefreshBegin) {
			// ���� ������� ������� ���������� ���������� - 
			// ����� ���������, ���� ��� (hLockRefreshBegin) ����� ����������
			SetEvent(srv.hLockRefreshReady);
			while (srv.hLockRefreshBegin
				&& WaitForSingleObject(srv.hLockRefreshBegin, 10) == WAIT_TIMEOUT)
				SetEvent(srv.hLockRefreshReady);
		}
		
		
		// �� ������ ���� �������� ������ �� ��������� ������� �������
		if (srv.bRequestChangeSize) {
			srv.bRequestChangeSize = FALSE;
			SetConsoleSize(srv.nReqSizeBufferHeight, srv.crReqSizeNewSize, srv.rReqSizeNewRect, srv.sReqSizeLabel);
			SetEvent(srv.hReqSizeChanged);
		}
		
		
		// ��������� ���������� ��������� � �������.
		// ������� �������� ghExitQueryEvent, ���� ��� �������� �����������.
		lbProcessChanged = CheckProcessCount();
		
		
		// ��������� ��������
		if (srv.nMaxFPS>0) {
			dwTimeout = 1000 / srv.nMaxFPS;
			// ���� 50, ����� �� ������������� ������� ��� �� ������� ���������� ("dir /s" � �.�.)
			if (dwTimeout < 10) dwTimeout = 10;
		} else {
			dwTimeout = 100;
		}
		// !!! ����� ������� ������ ���� �����������, �� ����� ��� ������� ���������
		nWait = WaitForMultipleObjects ( 2, hEvents, FALSE, dwTimeout );
		if (nWait == WAIT_OBJECT_0)
			break; // ����������� ���������� ����
		lbEventualChange = (nWait == (WAIT_OBJECT_0+1)) || lbProcessChanged;
		
		#ifdef _DEBUG
		if (nWait == (WAIT_OBJECT_0+1)) {
			DEBUGSTR(L"*** hRefreshEvent was set, checking console...\n");
		}
		#endif

		
		if (ghConEmuWnd && !IsWindow(ghConEmuWnd)) {
			ghConEmuWnd = NULL;
			EmergencyShow();
		}
		
		
		// 17.12.2009 Maks - �������� ������
		//if (ghConEmuWnd && GetForegroundWindow() == ghConWnd) {
		//	if (lbFirstForeground || !IsWindowVisible(ghConWnd)) {
		//		DEBUGSTR(L"...SetForegroundWindow(ghConEmuWnd);\n");
		//		SetForegroundWindow(ghConEmuWnd);
		//		lbFirstForeground = FALSE;
		//	}
		//}
		


		// ���� ����� - �������� ������� ��������� � �������
		if (pfnGetConsoleKeyboardLayoutName)
			CheckKeyboardLayout();



		if (!lbForceSend) {
			DWORD nCurTick = GetTickCount();
			nDelta = nCurTick - nLastUpdateTick;
			if (nDelta > MAX_FORCEREFRESH_INTERVAL) {
				lbForceSend = TRUE;
			}
		}
			


		/* ****************** */
		/* ���������� ������� */
		/* ****************** */
		lbChanged = ReloadFullConsoleInfo(lbForceSend);

		
		// ��� ���������� - ��������� ��������� tick
		if (lbChanged)
			nLastUpdateTick = GetTickCount();


		MCHKHEAP
	}

	return 0;
}
