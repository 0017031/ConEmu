
/*
Copyright (c) 2009-2010 Maximus5
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define SHOWDEBUGSTR

#include "ConEmuC.h"
#include "..\common\ConsoleAnnotation.h"

#define DEBUGSTRINPUTPIPE(s) DEBUGSTR(s)
#define DEBUGSTRCHANGES(s) DEBUGSTR(s)

#define MAX_EVENTS_PACK 20

#ifdef _DEBUG
	#define ASSERT_UNWANTED_SIZE
#else
	#undef ASSERT_UNWANTED_SIZE
#endif

BOOL ProcessInputMessage(MSG &msg, INPUT_RECORD &r);
BOOL SendConsoleEvent(INPUT_RECORD* pr, UINT nCount);
BOOL ReadInputQueue(INPUT_RECORD *prs, DWORD *pCount);
BOOL WriteInputQueue(const INPUT_RECORD *pr);
BOOL IsInputQueueEmpty();
BOOL WaitConsoleReady(); // ���������� ����� ����� ������� ������� �����
DWORD WINAPI InputThread(LPVOID lpvParam);


// ���������� ������ �����, ����� ����� ���� ���������� ���������� ������� GUI ����
void ServerInitFont()
{
	// ������ ������ � Lucida. ����������� ��� ���������� ������.
	if (srv.szConsoleFont[0]) {
		// ��������� ��������� ������� ������ ������!
		LOGFONT fnt = {0};
		lstrcpynW(fnt.lfFaceName, srv.szConsoleFont, LF_FACESIZE);
		srv.szConsoleFont[0] = 0; // ����� �������. ���� ����� ���� - ��� ����� ����������� � FontEnumProc
		HDC hdc = GetDC(NULL);
		EnumFontFamiliesEx(hdc, &fnt, (FONTENUMPROCW) FontEnumProc, (LPARAM)&fnt, 0);
		DeleteDC(hdc);
	}
	if (srv.szConsoleFont[0] == 0) {
		lstrcpyW(srv.szConsoleFont, L"Lucida Console");
		srv.nConFontWidth = 4; srv.nConFontHeight = 6;
	}
	if (srv.nConFontHeight<6) srv.nConFontHeight = 6;
	if (srv.nConFontWidth==0 && srv.nConFontHeight==0) {
		srv.nConFontWidth = 4; srv.nConFontHeight = 6;
	} else if (srv.nConFontWidth==0) {
		srv.nConFontWidth = srv.nConFontHeight * 2 / 3;
	} else if (srv.nConFontHeight==0) {
		srv.nConFontHeight = srv.nConFontWidth * 3 / 2;
	}
	if (srv.nConFontHeight<6 || srv.nConFontWidth <4) {
		srv.nConFontWidth = 4; srv.nConFontHeight = 6;
	}

	//if (!srv.bDebuggerActive || gbAttachMode) {

	if (ghLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.before");
	SetConsoleFontSizeTo(ghConWnd, srv.nConFontHeight, srv.nConFontWidth, srv.szConsoleFont);
	if (ghLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.after");

	//}
}

// ������� ����������� ������� � ����
int ServerInit()
{
	int iRc = 0;
	BOOL bConRc = FALSE;
	DWORD dwErr = 0;
	wchar_t szComSpec[MAX_PATH+1], szSelf[MAX_PATH+3];
	wchar_t* pszSelf = szSelf+1;
	//HWND hDcWnd = NULL;
	//HMODULE hKernel = GetModuleHandleW (L"kernel32.dll");
	
	srv.osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&srv.osv);
	TODO("���������, � � ����� �� ������?");
	if (srv.osv.dwMajorVersion == 6 && srv.osv.dwMinorVersion == 1)
		srv.bReopenHandleAllowed = FALSE;
	else
		srv.bReopenHandleAllowed = TRUE;

	if (!gnConfirmExitParm) {
		gbAlwaysConfirmExit = TRUE; gbAutoDisableConfirmExit = TRUE;
	}

	// ����� � ������� ����� ������ � ����� ������, ����� ����� ���� �������� � ���������� ������� �������
	if (!gbNoCreateProcess && !srv.bDebuggerActive)
		ServerInitFont();

	// �������� �� ��������� ��������� �����
	if (!gbNoCreateProcess /*&& !(gbParmBufferSize && gnBufferHeight == 0)*/) {
		HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
		DWORD dwFlags = 0;
		bConRc = GetConsoleMode(h, &dwFlags);
		dwFlags |= (ENABLE_QUICK_EDIT_MODE|ENABLE_EXTENDED_FLAGS|ENABLE_INSERT_MODE);
		bConRc = SetConsoleMode(h, dwFlags);
	}

	//2009-08-27 ������� �����
	if (!srv.hConEmuGuiAttached) {
		wchar_t szTempName[MAX_PATH];
		wsprintfW(szTempName, CEGUIRCONSTARTED, (DWORD)ghConWnd);
		//srv.hConEmuGuiAttached = OpenEvent(EVENT_ALL_ACCESS, FALSE, szTempName);
		//if (srv.hConEmuGuiAttached == NULL)
   		srv.hConEmuGuiAttached = CreateEvent(gpNullSecurity, TRUE, FALSE, szTempName);
		_ASSERTE(srv.hConEmuGuiAttached!=NULL);
		//if (srv.hConEmuGuiAttached) ResetEvent(srv.hConEmuGuiAttached); -- ����. ����� ��� ���� �������/����������� � GUI
	}
	
	// ���� 10, ����� �� ������������� ������� ��� �� ������� ���������� ("dir /s" � �.�.)
	srv.nMaxFPS = 100;

#ifdef _DEBUG
	if (ghFarInExecuteEvent)
		SetEvent(ghFarInExecuteEvent);
#endif

	// ������� MapFile ��� ��������� (�����!!!)
	iRc = CreateMapHeader();
	if (iRc != 0)
		goto wrap;

	// ������� ������� ��� Colorer
	CreateColorerHeader(); // ������ �� ������������ - �� �����������
	
	//if (hKernel) {
	//    pfnGetConsoleKeyboardLayoutName = (FGetConsoleKeyboardLayoutName)GetProcAddress (hKernel, "GetConsoleKeyboardLayoutNameW");
	//    pfnGetConsoleProcessList = (FGetConsoleProcessList)GetProcAddress (hKernel, "GetConsoleProcessList");
	//}
	
	srv.csProc = new MSection();

	// ������������� ���� ������
	wsprintfW(srv.szPipename, CESERVERPIPENAME, L".", gnSelfPID);
	wsprintfW(srv.szInputname, CESERVERINPUTNAME, L".", gnSelfPID);
	wsprintfW(srv.szQueryname, CESERVERQUERYNAME, L".", gnSelfPID);

	srv.nMaxProcesses = START_MAX_PROCESSES; srv.nProcessCount = 0;
	srv.pnProcesses = (DWORD*)Alloc(START_MAX_PROCESSES, sizeof(DWORD));
	srv.pnProcessesCopy = (DWORD*)Alloc(START_MAX_PROCESSES, sizeof(DWORD));
	MCHKHEAP;
	if (srv.pnProcesses == NULL || srv.pnProcessesCopy == NULL) {
		_printf ("Can't allocate %i DWORDS!\n", srv.nMaxProcesses);
		iRc = CERR_NOTENOUGHMEM1; goto wrap;
	}
	CheckProcessCount(TRUE); // ������� ������� ����
	// � ��������, ��������� ����� ����� ���� ������ �� ����, ����� ����������� � GUI 
	// �� ������ ���� ��������� � ������� �� ���������!
	_ASSERTE(srv.bDebuggerActive || srv.nProcessCount<=2); 


	// ��������� ���� ��������� ������� (����������, ����, � ��.)
	srv.hInputEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	if (srv.hInputEvent) srv.hInputThread = CreateThread( 
		NULL,              // no security attribute 
		0,                 // default stack size 
		InputThread,       // thread proc
		NULL,              // thread parameter 
		0,                 // not suspended 
		&srv.dwInputThread);      // returns thread ID 
	if (srv.hInputEvent == NULL || srv.hInputThread == NULL) 
	{
		dwErr = GetLastError();
		_printf("CreateThread(InputThread) failed, ErrCode=0x%08X\n", dwErr); 
		iRc = CERR_CREATEINPUTTHREAD; goto wrap;
	}
	SetThreadPriority(srv.hInputThread, THREAD_PRIORITY_ABOVE_NORMAL);
	
	srv.nMaxInputQueue = 255;
	srv.pInputQueue = (INPUT_RECORD*)Alloc(srv.nMaxInputQueue, sizeof(INPUT_RECORD));
	srv.pInputQueueEnd = srv.pInputQueue+srv.nMaxInputQueue;
	srv.pInputQueueWrite = srv.pInputQueue;
	srv.pInputQueueRead = srv.pInputQueueEnd;
	
	// ��������� ���� ��������� ������� (����������, ����, � ��.)
	srv.hInputPipeThread = CreateThread( 
		NULL,              // no security attribute 
		0,                 // default stack size 
		InputPipeThread,   // thread proc
		NULL,              // thread parameter 
		0,                 // not suspended 
		&srv.dwInputPipeThreadId);      // returns thread ID 

	if (srv.hInputPipeThread == NULL) 
	{
		dwErr = GetLastError();
		_printf("CreateThread(InputPipeThread) failed, ErrCode=0x%08X\n", dwErr);
		iRc = CERR_CREATEINPUTTHREAD; goto wrap;
	}
	SetThreadPriority(srv.hInputPipeThread, THREAD_PRIORITY_ABOVE_NORMAL);

	//InitializeCriticalSection(&srv.csChangeSize);
	//InitializeCriticalSection(&srv.csConBuf);
	//InitializeCriticalSection(&srv.csChar);

	if (!gbAttachMode) {
		DWORD dwRead = 0, dwErr = 0; BOOL lbCallRc = FALSE;
		HWND hConEmuWnd = FindConEmuByPID();
		if (hConEmuWnd) {
			//UINT nMsgSrvStarted = RegisterWindowMessage(CONEMUMSG_SRVSTARTED);
			//DWORD_PTR nRc = 0;
			//SendMessageTimeout(hConEmuWnd, nMsgSrvStarted, (WPARAM)ghConWnd, gnSelfPID, 
			//	SMTO_BLOCK, 500, &nRc);
			
			_ASSERTE(ghConWnd!=NULL);
				
		    wchar_t szServerPipe[MAX_PATH];
		    wsprintf(szServerPipe, CEGUIPIPENAME, L".", (DWORD)hConEmuWnd);
		    CESERVER_REQ In, Out;
		    ExecutePrepareCmd(&In, CECMD_CMDSTARTSTOP, sizeof(CESERVER_REQ_HDR)+sizeof(DWORD));
		    In.dwData[0] = (DWORD)ghConWnd;
		    
		    lbCallRc = CallNamedPipe(szServerPipe, &In, In.hdr.nSize, &Out, sizeof(Out), &dwRead, 1000);
		    if (!lbCallRc) {
		    	dwErr = GetLastError();
		    	_ASSERTE(lbCallRc);
	    	}
		}
		if (srv.hConEmuGuiAttached) {
			WaitForSingleObject(srv.hConEmuGuiAttached, 500);
		}
		
		CheckConEmuHwnd();
	}




	if (gbNoCreateProcess && (gbAttachMode || srv.bDebuggerActive)) {
		if (!srv.bDebuggerActive && !IsWindowVisible(ghConWnd)) {
			PRINT_COMSPEC(L"Console windows is not visible. Attach is unavailable. Exiting...\n", 0);
			DisableAutoConfirmExit();
			//srv.nProcessStartTick = GetTickCount() - 2*CHECK_ROOTSTART_TIMEOUT; // ������ nProcessStartTick �� �����. �������� ������ �� �������
			return CERR_RUNNEWCONSOLE;
		}
		
		if (srv.dwRootProcess == 0 && !srv.bDebuggerActive) {
			// ����� ���������� ���������� PID ��������� ��������.
			// ������������ ����� ���� cmd (comspec, ���������� �� FAR)
			DWORD dwParentPID = 0, dwFarPID = 0;
			DWORD dwServerPID = 0; // ����� � ���� ������� ��� ���� ������?
			
			_ASSERTE(!srv.bDebuggerActive);
			
			if (srv.nProcessCount >= 2 && !srv.bDebuggerActive) {
				HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
				if (hSnap != INVALID_HANDLE_VALUE) {
					PROCESSENTRY32 prc = {sizeof(PROCESSENTRY32)};
					if (Process32First(hSnap, &prc)) {
						do {
							for (UINT i = 0; i < srv.nProcessCount; i++) {
								if (prc.th32ProcessID != gnSelfPID
									&& prc.th32ProcessID == srv.pnProcesses[i])
								{
									if (lstrcmpiW(prc.szExeFile, L"conemuc.exe")==0) {
										CESERVER_REQ* pIn = ExecuteNewCmd(CECMD_ATTACH2GUI, 0);
										CESERVER_REQ* pOut = ExecuteSrvCmd(prc.th32ProcessID, pIn, ghConWnd);
										if (pOut) dwServerPID = prc.th32ProcessID;
										ExecuteFreeResult(pIn); ExecuteFreeResult(pOut);
										// ���� ������� ������� ��������� - �������
										if (dwServerPID)
											break;
									}
									if (!dwFarPID && lstrcmpiW(prc.szExeFile, L"far.exe")==0) {
										dwFarPID = prc.th32ProcessID;
									}
									if (!dwParentPID)
										dwParentPID = prc.th32ProcessID;
								}
							}
							// ���� ��� ��������� ������� � ������� - �������, ������� ������ �� �����
							if (dwServerPID)
								break;
						} while (Process32Next(hSnap, &prc));
					}
					CloseHandle(hSnap);
					
					if (dwFarPID) dwParentPID = dwFarPID;
				}
			}
			
			if (dwServerPID) {
				AllowSetForegroundWindow(dwServerPID);
				PRINT_COMSPEC(L"Server was already started. PID=%i. Exiting...\n", dwServerPID);
				DisableAutoConfirmExit(); // ������ ��� ����?
				// ������ nProcessStartTick �� �����. �������� ������ �� �������
				//srv.nProcessStartTick = GetTickCount() - 2*CHECK_ROOTSTART_TIMEOUT;
				return CERR_RUNNEWCONSOLE;
			}
			
			if (!dwParentPID) {
				_printf ("Attach to GUI was requested, but there is no console processes:\n", 0, GetCommandLineW());
				return CERR_CARGUMENT;
			}
			
			// ����� ������� HANDLE ��������� ��������
			srv.hRootProcess = OpenProcess(PROCESS_QUERY_INFORMATION|SYNCHRONIZE, FALSE, dwParentPID);
			if (!srv.hRootProcess) {
				dwErr = GetLastError();
				wchar_t* lpMsgBuf = NULL;
				
   				FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL );
				
				_printf("Can't open process (%i) handle, ErrCode=0x%08X, Description:\n", 
					dwParentPID, dwErr, (lpMsgBuf == NULL) ? L"<Unknown error>" : lpMsgBuf);
				
				if (lpMsgBuf) LocalFree(lpMsgBuf);
				return CERR_CREATEPROCESS;
			}
			srv.dwRootProcess = dwParentPID;

			// ��������� ������ ����� ConEmuC ����������!
			wchar_t szSelf[MAX_PATH+100];
			wchar_t* pszSelf = szSelf+1;
			if (!GetModuleFileName(NULL, pszSelf, MAX_PATH)) {
				dwErr = GetLastError();
				_printf("GetModuleFileName failed, ErrCode=0x%08X\n", dwErr);
				return CERR_CREATEPROCESS;
			}
			if (wcschr(pszSelf, L' ')) {
				*(--pszSelf) = L'"';
				lstrcatW(pszSelf, L"\"");
			}
			
			wsprintf(pszSelf+wcslen(pszSelf), L" /ATTACH /PID=%i", dwParentPID);

			PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
			STARTUPINFOW si; memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
			
			PRINT_COMSPEC(L"Starting modeless:\n%s\n", pszSelf);
		
			// CREATE_NEW_PROCESS_GROUP - ����, ��������� �������� Ctrl-C
			BOOL lbRc = CreateProcessW(NULL, pszSelf, NULL,NULL, TRUE, 
					NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
			dwErr = GetLastError();
			if (!lbRc)
			{
				_printf("Can't create process, ErrCode=0x%08X! Command to be executed:\n", dwErr, pszSelf);
				return CERR_CREATEPROCESS;
			}
			//delete psNewCmd; psNewCmd = NULL;
			AllowSetForegroundWindow(pi.dwProcessId);
			PRINT_COMSPEC(L"Modeless server was started. PID=%i. Exiting...\n", pi.dwProcessId);
			SafeCloseHandle(pi.hProcess); SafeCloseHandle(pi.hThread);
			DisableAutoConfirmExit(); // ������ ������� ������ ���������, ����� �� ����������� bat �����
			// ������ nProcessStartTick �� �����. �������� ������ �� �������
			//srv.nProcessStartTick = GetTickCount() - 2*CHECK_ROOTSTART_TIMEOUT;
			return CERR_RUNNEWCONSOLE;

		} else {
			// ����� ������� HANDLE ��������� ��������
			DWORD dwFlags = PROCESS_QUERY_INFORMATION|SYNCHRONIZE;
			if (srv.bDebuggerActive)
				dwFlags |= PROCESS_VM_READ;
			srv.hRootProcess = OpenProcess(dwFlags, FALSE, srv.dwRootProcess);
			if (!srv.hRootProcess) {
				dwErr = GetLastError();
				wchar_t* lpMsgBuf = NULL;
				
   				FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL );
				
				_printf ("Can't open process (%i) handle, ErrCode=0x%08X, Description:\n", 
					srv.dwRootProcess, dwErr, (lpMsgBuf == NULL) ? L"<Unknown error>" : lpMsgBuf);
				
				if (lpMsgBuf) LocalFree(lpMsgBuf);
				return CERR_CREATEPROCESS;
			}
			
			if (srv.bDebuggerActive) {
				wchar_t szTitle[64];
				wsprintf(szTitle, L"Debug PID=%i", srv.dwRootProcess);
				SetConsoleTitleW(szTitle);
			}
		}
	}


	srv.hAllowInputEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (!srv.hAllowInputEvent) SetEvent(srv.hAllowInputEvent);

	TODO("����� ���������, ����� ComSpecC ��� ����?");
	if (GetEnvironmentVariable(L"ComSpec", szComSpec, MAX_PATH)) {
		wchar_t* pszSlash = wcsrchr(szComSpec, L'\\');
		if (pszSlash) {
			if (_wcsnicmp(pszSlash, L"\\conemuc.", 9)) {
				// ���� ��� �� �� - ��������� � ComSpecC
				SetEnvironmentVariable(L"ComSpecC", szComSpec);
			}
		}
	}
	if (GetModuleFileName(NULL, pszSelf, MAX_PATH)) {
		wchar_t *pszShort = NULL;
		if (pszSelf[0] != L'\\')
			pszShort = GetShortFileNameEx(pszSelf);
		if (!pszShort && wcschr(pszSelf, L' ')) {
			*(--pszSelf) = L'"';
			lstrcatW(pszSelf, L"\"");
		}
		if (pszShort) {
			SetEnvironmentVariable(L"ComSpec", pszShort);
			free(pszShort);
		} else {
			SetEnvironmentVariable(L"ComSpec", pszSelf);
		}
	}

	//srv.bContentsChanged = TRUE;
	srv.nMainTimerElapse = 10;
	srv.bConsoleActive = TRUE; TODO("������������ ���������� ������� Activate/Deactivate");
	//srv.bNeedFullReload = FALSE; srv.bForceFullSend = TRUE;
	srv.nTopVisibleLine = -1; // ���������� ��������� �� ��������

	


	//// ������ ������ � Lucida. ����������� ��� ���������� ������.
	//if (srv.szConsoleFont[0]) {
	//    // ��������� ��������� ������� ������ ������!
	//    LOGFONT fnt = {0};
	//    lstrcpynW(fnt.lfFaceName, srv.szConsoleFont, LF_FACESIZE);
	//    srv.szConsoleFont[0] = 0; // ����� �������. ���� ����� ���� - ��� ����� ����������� � FontEnumProc
	//    HDC hdc = GetDC(NULL);
	//    EnumFontFamiliesEx(hdc, &fnt, (FONTENUMPROCW) FontEnumProc, (LPARAM)&fnt, 0);
	//    DeleteDC(hdc);
	//}
	//if (srv.szConsoleFont[0] == 0) {
	//    lstrcpyW(srv.szConsoleFont, L"Lucida Console");
	//    srv.nConFontWidth = 4; srv.nConFontHeight = 6;
	//}
	//if (srv.nConFontHeight<6) srv.nConFontHeight = 6;
	//if (srv.nConFontWidth==0 && srv.nConFontHeight==0) {
	//    srv.nConFontWidth = 4; srv.nConFontHeight = 6;
	//} else if (srv.nConFontWidth==0) {
	//    srv.nConFontWidth = srv.nConFontHeight * 2 / 3;
	//} else if (srv.nConFontHeight==0) {
	//    srv.nConFontHeight = srv.nConFontWidth * 3 / 2;
	//}
	//if (srv.nConFontHeight<6 || srv.nConFontWidth <4) {
	//    srv.nConFontWidth = 4; srv.nConFontHeight = 6;
	//}
	//if (srv.szConsoleFontFile[0])
	//  AddFontResourceEx(srv.szConsoleFontFile, FR_PRIVATE, NULL);
	if (!srv.bDebuggerActive || gbAttachMode) {
		// ���, ������
		/*if (ghLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.before");
		SetConsoleFontSizeTo(ghConWnd, srv.nConFontHeight, srv.nConFontWidth, srv.szConsoleFont);
		if (ghLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.after");*/
	} else {
		SetWindowPos(ghConWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
	}
	if (gbParmBufferSize && gcrBufferSize.X && gcrBufferSize.Y) {
		SMALL_RECT rc = {0};
		SetConsoleSize(gnBufferHeight, gcrBufferSize, rc, ":ServerInit.SetFromArg"); // ����� ����������? ���� ����� ��� �������
	} else {
		HANDLE hOut = (HANDLE)ghConOut;
		CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}}; // ���������� �������� ��������� ���
		if (GetConsoleScreenBufferInfo(hOut, &lsbi)) {
			srv.crReqSizeNewSize = lsbi.dwSize;
			gcrBufferSize.X = lsbi.dwSize.X;
			if (lsbi.dwSize.Y > lsbi.dwMaximumWindowSize.Y) {
				// �������� �����
				gcrBufferSize.Y = (lsbi.srWindow.Bottom - lsbi.srWindow.Top + 1);
				gnBufferHeight = lsbi.dwSize.Y;
			} else {
				// ����� ��� ���������!
				gcrBufferSize.Y = lsbi.dwSize.Y;
				gnBufferHeight = 0;
			}
		}
	}

	if (IsIconic(ghConWnd)) { // ������ ����� ����������!
		WINDOWPLACEMENT wplCon = {sizeof(wplCon)};
		GetWindowPlacement(ghConWnd, &wplCon);
		wplCon.showCmd = SW_RESTORE;
		SetWindowPlacement(ghConWnd, &wplCon);
	}

	// ����� �������� ������� ��������� �������
	ReloadFullConsoleInfo(TRUE);


	//
	srv.hRefreshEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	if (!srv.hRefreshEvent) {
		dwErr = GetLastError();
		_printf("CreateEvent(hRefreshEvent) failed, ErrCode=0x%08X\n", dwErr); 
		iRc = CERR_REFRESHEVENT; goto wrap;
	}
	srv.hDataSentEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	if (!srv.hDataSentEvent) {
		dwErr = GetLastError();
		_printf("CreateEvent(hDataSentEvent) failed, ErrCode=0x%08X\n", dwErr); 
		iRc = CERR_REFRESHEVENT; goto wrap;
	}
	// !! Event ����� ��������� � ���������� ����� !!
	srv.hReqSizeChanged = CreateEvent(NULL,TRUE,FALSE,NULL);
	if (!srv.hReqSizeChanged) {
		dwErr = GetLastError();
		_printf("CreateEvent(hReqSizeChanged) failed, ErrCode=0x%08X\n", dwErr); 
		iRc = CERR_REFRESHEVENT; goto wrap;
	}




	if (gbAttachMode) {
		// ���� Refresh �� ������ ���� ��������, ����� � ������� ����� ������� ������ �� �������
		// �� ����, ��� ���������� ������ (��� ������, ������� ������ ���������� GUI ��� ������)
		_ASSERTE(srv.dwRefreshThread==0);
		HWND hDcWnd = Attach2Gui(5000);

		// 090719 ��������� � ������� ��� ������ ������. ����� �������� � GUI - TID ���� �����
		//// ���� ��� �� ����� ������� (-new_console) � �� /ATTACH ��� ������������ �������
		//if (!gbNoCreateProcess)
		//	SendStarted();

		if (!hDcWnd) {
			_printf("Available ConEmu GUI window not found!\n");
			iRc = CERR_ATTACHFAILED; goto wrap;
		}
	}



	
	// ��������� ���� ���������� �� ��������
	srv.hRefreshThread = CreateThread( 
		NULL,              // no security attribute 
		0,                 // default stack size 
		RefreshThread,     // thread proc
		NULL,              // thread parameter 
		0,                 // not suspended 
		&srv.dwRefreshThread); // returns thread ID 

	if (srv.hRefreshThread == NULL) 
	{
		dwErr = GetLastError();
		_printf("CreateThread(RefreshThread) failed, ErrCode=0x%08X\n", dwErr); 
		iRc = CERR_CREATEREFRESHTHREAD; goto wrap;
	}
	
	
	//srv.nMsgHookEnableDisable = RegisterWindowMessage(L"ConEmuC::HookEnableDisable");
	// The client thread that calls SetWinEventHook must have a message loop in order to receive events.");
	srv.hWinEventThread = CreateThread( NULL, 0, WinEventThread, NULL, 0, &srv.dwWinEventThread);
	if (srv.hWinEventThread == NULL) 
	{
		dwErr = GetLastError();
		_printf("CreateThread(WinEventThread) failed, ErrCode=0x%08X\n", dwErr); 
		iRc = CERR_WINEVENTTHREAD; goto wrap;
	}

	// ��������� ���� ��������� ������  
	srv.hServerThread = CreateThread( NULL, 0, ServerThread, NULL, 0, &srv.dwServerThreadId);
	if (srv.hServerThread == NULL) 
	{
		dwErr = GetLastError();
		_printf("CreateThread(ServerThread) failed, ErrCode=0x%08X\n", dwErr); 
		iRc = CERR_CREATESERVERTHREAD; goto wrap;
	}


	SendStarted();

	CheckConEmuHwnd();


wrap:
	return iRc;
}

// ��������� ��� ���� � ������� �����������
void ServerDone(int aiRc)
{
	gbQuit = true;
	// �� ������ ������ - �������� �������
	if (ghExitQueryEvent) SetEvent(ghExitQueryEvent);
	if (ghQuitEvent) SetEvent(ghQuitEvent);

	// ���������� ��������, ����� ������������ ������� ���� ����������	
	if (srv.bDebuggerActive) {
		if (pfnDebugActiveProcessStop) pfnDebugActiveProcessStop(srv.dwRootProcess);
		srv.bDebuggerActive = FALSE;
	}

	// ������ ������� ����� �� ��� ����, � ����� ����� �����
	if (srv.dwWinEventThread && srv.hWinEventThread)
		PostThreadMessage(srv.dwWinEventThread, WM_QUIT, 0, 0);
	//if (srv.dwInputThreadId && srv.hInputThread)
	//	PostThreadMessage(srv.dwInputThreadId, WM_QUIT, 0, 0);
	// ����������� ���� ��������� ����
	HANDLE hPipe = CreateFile(srv.szPipename,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		DEBUGSTR(L"All pipe instances closed?\n");
	}
	// ����������� ���� �����
	if (srv.hInputPipe && srv.hInputPipe != INVALID_HANDLE_VALUE) {
		//DWORD dwSize = 0;
		//BOOL lbRc = WriteFile(srv.hInputPipe, &dwSize, sizeof(dwSize), &dwSize, NULL);
		/*BOOL lbRc = DisconnectNamedPipe(srv.hInputPipe);
		CloseHandle(srv.hInputPipe);
		srv.hInputPipe = NULL;*/
		TODO("������ �� ��������, ������ ����� �� Overlapped ����������");
	}
	//HANDLE hInputPipe = CreateFile(srv.szInputname,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	//if (hInputPipe == INVALID_HANDLE_VALUE) {
	//	DEBUGSTR(L"Input pipe was not created?\n");
	//} else {
	//	MSG msg = {NULL}; msg.message = 0xFFFF; DWORD dwOut = 0;
	//	WriteFile(hInputPipe, &msg, sizeof(msg), &dwOut, 0);
	//}


	// ��������� ����������� � �������
	if (srv.dwWinEventThread && srv.hWinEventThread) {
		// �������� ��������, ���� ���� ���� ����������
		if (WaitForSingleObject(srv.hWinEventThread, 500) != WAIT_OBJECT_0) {
			#pragma warning( push )
			#pragma warning( disable : 6258 )
			TerminateThread ( srv.hWinEventThread, 100 ); // ��� ��������� �� �����...
			#pragma warning( pop )
		}
		SafeCloseHandle(srv.hWinEventThread);
		srv.dwWinEventThread = 0;
	}
	if (srv.hInputThread) {
		// �������� ��������, ���� ���� ���� ����������
		if (WaitForSingleObject(srv.hInputThread, 500) != WAIT_OBJECT_0) {
			#pragma warning( push )
			#pragma warning( disable : 6258 )
			TerminateThread ( srv.hInputThread, 100 ); // ��� ��������� �� �����...
			#pragma warning( pop )
		}
		SafeCloseHandle(srv.hInputThread);
		srv.dwInputThread = 0;
	}
	if (srv.hInputPipeThread) {
		// �������� ��������, ���� ���� ���� ����������
		if (WaitForSingleObject(srv.hInputPipeThread, 50) != WAIT_OBJECT_0) {
			#pragma warning( push )
			#pragma warning( disable : 6258 )
			TerminateThread ( srv.hInputPipeThread, 100 ); // ��� ��������� �� �����...
			#pragma warning( pop )
		}
		SafeCloseHandle(srv.hInputPipeThread);
		srv.dwInputPipeThreadId = 0;
	}
	SafeCloseHandle(srv.hInputPipe);
	SafeCloseHandle(srv.hInputEvent);

	if (srv.hServerThread) {
		// �������� ��������, ���� ���� ���� ����������
		if (WaitForSingleObject(srv.hServerThread, 500) != WAIT_OBJECT_0) {
			#pragma warning( push )
			#pragma warning( disable : 6258 )
			TerminateThread ( srv.hServerThread, 100 ); // ��� ��������� �� �����...
			#pragma warning( pop )
		}
		SafeCloseHandle(srv.hServerThread);
	}
	SafeCloseHandle(hPipe);
	if (srv.hRefreshThread) {
		if (WaitForSingleObject(srv.hRefreshThread, 100)!=WAIT_OBJECT_0) {
			_ASSERT(FALSE);
			#pragma warning( push )
			#pragma warning( disable : 6258 )
			TerminateThread(srv.hRefreshThread, 100);
			#pragma warning( pop )
		}
		SafeCloseHandle(srv.hRefreshThread);
	}
	
	if (srv.hRefreshEvent) {
		SafeCloseHandle(srv.hRefreshEvent);
	}
	if (srv.hDataSentEvent) {
		SafeCloseHandle(srv.hDataSentEvent);
	}
	//if (srv.hChangingSize) {
	//    SafeCloseHandle(srv.hChangingSize);
	//}
	// ��������� ��� ����
	//srv.bWinHookAllow = FALSE; srv.nWinHookMode = 0;
	//HookWinEvents ( -1 );
	
	if (gpStoredOutput) { Free(gpStoredOutput); gpStoredOutput = NULL; }
	if (srv.pszAliases) { Free(srv.pszAliases); srv.pszAliases = NULL; }
	//if (srv.psChars) { Free(srv.psChars); srv.psChars = NULL; }
	//if (srv.pnAttrs) { Free(srv.pnAttrs); srv.pnAttrs = NULL; }
	//if (srv.ptrLineCmp) { Free(srv.ptrLineCmp); srv.ptrLineCmp = NULL; }
	//DeleteCriticalSection(&srv.csConBuf);
	//DeleteCriticalSection(&srv.csChar);
	//DeleteCriticalSection(&srv.csChangeSize);

	SafeCloseHandle(srv.hAllowInputEvent);

	SafeCloseHandle(srv.hRootProcess);
	SafeCloseHandle(srv.hRootThread);


	if (srv.csProc) {
		delete srv.csProc;
		srv.csProc = NULL;
	}

	if (srv.pnProcesses) {
		Free(srv.pnProcesses); srv.pnProcesses = NULL;
	}
	if (srv.pnProcessesCopy) {
		Free(srv.pnProcessesCopy); srv.pnProcessesCopy = NULL;
	}
	if (srv.pInputQueue) {
		Free(srv.pInputQueue); srv.pInputQueue = NULL;
	}

	CloseMapHeader();
	SafeCloseHandle(srv.hColorerMapping);
}



// ��������� ������ ���� ������� � gpStoredOutput
void CmdOutputStore()
{
	// � Win7 �������� ����������� � ������ �������� - ��������� ���������� ����� ���������!!!
	// � �����, ����� ������ telnet'� ������������!
	if (srv.bReopenHandleAllowed) {
		ghConOut.Close();
	}
	
	WARNING("� ��� ��� ����� �� ������ � RefreshThread!!!");
	DEBUGSTR(L"--- CmdOutputStore begin\n");
	CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}};
	// !!! ��� ���������� �������� ��������� ��� � �������, 
	//     � �� ����������������� �������� MyGetConsoleScreenBufferInfo
	if (!GetConsoleScreenBufferInfo(ghConOut, &lsbi)) {
		if (gpStoredOutput) { Free(gpStoredOutput); gpStoredOutput = NULL; }
		return; // �� ������ �������� ���������� � �������...
	}
	int nOneBufferSize = lsbi.dwSize.X * lsbi.dwSize.Y * 2; // ������ ��� ������� �������!
	// ���� ��������� ���������� ������� ���������� ������
	if (gpStoredOutput && gpStoredOutput->hdr.cbMaxOneBufferSize < (DWORD)nOneBufferSize) {
		Free(gpStoredOutput); gpStoredOutput = NULL;
	}
	if (gpStoredOutput == NULL) {
		// �������� ������: ��������� + ����� ������ (�� �������� ������)
		gpStoredOutput = (CESERVER_CONSAVE*)Alloc(sizeof(CESERVER_CONSAVE_HDR)+nOneBufferSize,1);
		_ASSERTE(gpStoredOutput!=NULL);
		if (gpStoredOutput == NULL)
			return; // �� ������ �������� ������
		gpStoredOutput->hdr.cbMaxOneBufferSize = nOneBufferSize;
	}
	// ��������� sbi
	//memmove(&gpStoredOutput->hdr.sbi, &lsbi, sizeof(lsbi));
	gpStoredOutput->hdr.sbi = lsbi;
	// ������ ������ ������
	COORD coord = {0,0};
	DWORD nbActuallyRead = 0;
	DWORD nReadLen = lsbi.dwSize.X * lsbi.dwSize.Y;

	// [Roman Kuzmin]
	// In FAR Manager source code this is mentioned as "fucked method". Yes, it is.
	// Functions ReadConsoleOutput* fail if requested data size exceeds their buffer;
	// MSDN says 64K is max but it does not say how much actually we can request now.
	// Experiments show that this limit is floating and it can be much less than 64K.
	// The solution below is not optimal when a user sets small font and large window,
	// but it is safe and practically optimal, because most of users set larger fonts
	// for large window and ReadConsoleOutput works OK. More optimal solution for all
	// cases is not that difficult to develop but it will be increased complexity and
	// overhead often for nothing, not sure that we really should use it.

	if (!ReadConsoleOutputCharacter(ghConOut, gpStoredOutput->Data, nReadLen, coord, &nbActuallyRead)
		|| (nbActuallyRead != nReadLen))
	{
		DEBUGSTR(L"--- Full block read failed: read line by line\n");
		wchar_t* ConCharNow = gpStoredOutput->Data;
		nReadLen = lsbi.dwSize.X;
		for(int y = 0; y < (int)lsbi.dwSize.Y; y++, coord.Y++)
		{
			ReadConsoleOutputCharacter(ghConOut, ConCharNow, nReadLen, coord, &nbActuallyRead);
				ConCharNow += lsbi.dwSize.X;
		}
	}
	DEBUGSTR(L"--- CmdOutputStore end\n");
}

void CmdOutputRestore()
{
	if (gpStoredOutput) {
		TODO("������������ ����� ������� (������������ �����) ����� �������");
		// ������, ��� ������ ������� ����� ���������� �� ������� ���������� ���������� �������.
		// ������ � ��� � ������� ����� ������� ����� ���������� ������� ����������� ������ (����������� FAR).
		// 1) ���� ������� ����� �������
		// 2) ����������� � ������ ����� ������� (�� ������� ����������� ���������� �������)
		// 3) ���������� ������� �� ���������� ������� (���� �� ������ ��� ����������� ������ ������)
		// 4) ������������ ���������� ����� �������. ������, ��� ��� �����
		//    ��������� ��������� ������� ��� � ������ ������-�� ��� ��������� ������...
	}
}


HWND FindConEmuByPID()
{
	HWND hConEmuWnd = NULL;
	DWORD dwGuiThreadId = 0, dwGuiProcessId = 0;

	// � ����������� ������� PID GUI ������� ����� ���������
	if (srv.dwGuiPID == 0) {
		// GUI ����� ��� "������" � �������� ��� � ���������, ��� ��� ������� � ����� Snapshoot
		
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
		if (hSnap != INVALID_HANDLE_VALUE) {
			PROCESSENTRY32 prc = {sizeof(PROCESSENTRY32)};
			if (Process32First(hSnap, &prc)) {
				do {
					if (prc.th32ProcessID == gnSelfPID) {
						srv.dwGuiPID = prc.th32ParentProcessID;
						break;
					}
				} while (Process32Next(hSnap, &prc));
			}
			CloseHandle(hSnap);
		}
	}
	
	if (srv.dwGuiPID) {
		HWND hGui = NULL;
		while ((hGui = FindWindowEx(NULL, hGui, VirtualConsoleClassMain, NULL)) != NULL) {
			dwGuiThreadId = GetWindowThreadProcessId(hGui, &dwGuiProcessId);
			if (dwGuiProcessId == srv.dwGuiPID) {
				hConEmuWnd = hGui;
				
				break;
			}
		}
	}

	return hConEmuWnd;
}

void CheckConEmuHwnd()
{
	//HWND hWndFore = GetForegroundWindow();
	//HWND hWndFocus = GetFocus();
	DWORD dwGuiThreadId = 0, dwGuiProcessId = 0;

	if (ghConEmuWnd == NULL) {
		SendStarted(); // �� � ���� ��������, � ��������� �������� � ������ ������� �������������
	}
	// GUI ����� ��� "������" � �������� ��� � ���������, ��� ��� ������� � ����� Snapshoot
	if (ghConEmuWnd == NULL) {
		ghConEmuWnd = FindConEmuByPID();
	}
	if (ghConEmuWnd == NULL) { // ���� �� ������ �� �������...
		ghConEmuWnd = GetConEmuHWND(TRUE/*abRoot*/);
	}
	if (ghConEmuWnd) {
		// ���������� ���������� ����� � ������������ ����
		SetConEmuEnvVar(ghConEmuWnd);

		dwGuiThreadId = GetWindowThreadProcessId(ghConEmuWnd, &dwGuiProcessId);

		AllowSetForegroundWindow(dwGuiProcessId);

		//if (hWndFore == ghConWnd || hWndFocus == ghConWnd)
		//if (hWndFore != ghConEmuWnd)

		if (GetForegroundWindow() == ghConWnd)
			SetForegroundWindow(ghConEmuWnd); // 2009-09-14 ������-�� ���� ���� ghConWnd ?

	} else {
		// �� � ��� ����. ��� ����� ������ ���, ��� gui ���������
		//_ASSERTE(ghConEmuWnd!=NULL);
	}
}

HWND Attach2Gui(DWORD nTimeout)
{
	// ���� Refresh �� ������ ���� ��������, ����� � ������� ����� ������� ������ �� �������
	// �� ����, ��� ���������� ������ (��� ������, ������� ������ ���������� GUI ��� ������)
	_ASSERTE(srv.dwRefreshThread==0);

	HWND hGui = NULL, hDcWnd = NULL;
	//UINT nMsg = RegisterWindowMessage(CONEMUMSG_ATTACH);
	BOOL bNeedStartGui = FALSE;
	DWORD dwErr = 0;

	if (!srv.pConsoleInfo) {
		_ASSERTE(srv.pConsoleInfo!=NULL);
	} else {
		// ����� GUI �� ������� ������� ���������� �� ������� �� ���������� ������ (�� ��������� ��������)
		srv.pConsoleInfo->nPacketId = 0;
	}
	
	hGui = FindWindowEx(NULL, hGui, VirtualConsoleClassMain, NULL);
	if (!hGui)
	{
		DWORD dwGuiPID = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
		if (hSnap != INVALID_HANDLE_VALUE) {
			PROCESSENTRY32 prc = {sizeof(PROCESSENTRY32)};
			if (Process32First(hSnap, &prc)) {
				do {
					for (UINT i = 0; i < srv.nProcessCount; i++) {
						if (lstrcmpiW(prc.szExeFile, L"conemu.exe")==0) {
							dwGuiPID = prc.th32ProcessID;
							break;
						}
					}
					if (dwGuiPID) break;
				} while (Process32Next(hSnap, &prc));
			}
			CloseHandle(hSnap);
			
			if (!dwGuiPID) bNeedStartGui = TRUE;
		}
	}
	
	
	if (bNeedStartGui) {
		wchar_t szSelf[MAX_PATH+100];
		wchar_t* pszSelf = szSelf+1, *pszSlash = NULL;
		if (!GetModuleFileName(NULL, pszSelf, MAX_PATH)) {
			dwErr = GetLastError();
			_printf ("GetModuleFileName failed, ErrCode=0x%08X\n", dwErr);
			return NULL;
		}
		pszSlash = wcsrchr(pszSelf, L'\\');
		if (!pszSlash) {
			_printf ("Invalid GetModuleFileName, backslash not found!\n", 0, pszSelf);
			return NULL;
		}
		pszSlash++;
		if (wcschr(pszSelf, L' ')) {
			*(--pszSelf) = L'"';
			lstrcpyW(pszSlash, L"ConEmu.exe\"");
		} else {
			lstrcpyW(pszSlash, L"ConEmu.exe");
		}
		
		lstrcatW(pszSelf, L" /detached");
		
		PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
		STARTUPINFOW si; memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
		
		PRINT_COMSPEC(L"Starting GUI:\n%s\n", pszSelf);
	
		// CREATE_NEW_PROCESS_GROUP - ����, ��������� �������� Ctrl-C
		BOOL lbRc = CreateProcessW(NULL, pszSelf, NULL,NULL, TRUE, 
				NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
		dwErr = GetLastError();
		if (!lbRc)
		{
			_printf ("Can't create process, ErrCode=0x%08X! Command to be executed:\n", dwErr, pszSelf);
			return NULL;
		}
		//delete psNewCmd; psNewCmd = NULL;
		AllowSetForegroundWindow(pi.dwProcessId);
		PRINT_COMSPEC(L"Detached GUI was started. PID=%i, Attaching...\n", pi.dwProcessId);
		WaitForInputIdle(pi.hProcess, nTimeout);
		SafeCloseHandle(pi.hProcess); SafeCloseHandle(pi.hThread);
		
		if (nTimeout > 1000) nTimeout = 1000;
	}
	
	
	DWORD dwStart = GetTickCount(), dwDelta = 0, dwCur = 0;
	CESERVER_REQ In = {{0}};
	_ASSERTE(sizeof(CESERVER_REQ_STARTSTOP) >= sizeof(CESERVER_REQ_STARTSTOPRET));
	ExecutePrepareCmd(&In, CECMD_ATTACH2GUI, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_STARTSTOP));
	In.StartStop.nStarted = 0;
	In.StartStop.hWnd = ghConWnd;
	In.StartStop.dwPID = gnSelfPID;
	//In.StartStop.dwInputTID = srv.dwInputPipeThreadId;
	In.StartStop.nSubSystem = gnImageSubsystem;
	In.StartStop.bRootIsCmdExe = gbRootIsCmdExe;
	In.StartStop.bUserIsAdmin = IsUserAdmin();
	HANDLE hOut = (HANDLE)ghConOut;
	if (!GetConsoleScreenBufferInfo(hOut, &In.StartStop.sbi)) {
		_ASSERTE(FALSE);
	} else {
		srv.crReqSizeNewSize = In.StartStop.sbi.dwSize;
	}

	// � ������� "���������" ������ ����� ��� ����������, � ��� ������
	// ������� �������� - ����� ���-����� �������� �� ���������
	//BOOL lbNeedSetFont = TRUE;
	// ����� ��������. ����� ��� ������...
	hGui = NULL;
	// ���� � ������� ���� �� ��������� (GUI ��� ��� �� �����������) ������� ���
	while (!hDcWnd && dwDelta <= nTimeout) {
		while ((hGui = FindWindowEx(NULL, hGui, VirtualConsoleClassMain, NULL)) != NULL) {
			//if (lbNeedSetFont) {
			//	lbNeedSetFont = FALSE;
			//	
			//    if (ghLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.before");
			//    SetConsoleFontSizeTo(ghConWnd, srv.nConFontHeight, srv.nConFontWidth, srv.szConsoleFont);
			//    if (ghLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.after");
			//}
		
			WARNING("���������� �� ������� �����!!! AttachRequested");
			//hDcWnd = (HWND)SendMessage(hGui, nMsg, (WPARAM)ghConWnd, (LPARAM)gnSelfPID);
			//if (hDcWnd != NULL) {
			wchar_t szPipe[64];
			wsprintf(szPipe, CEGUIPIPENAME, L".", (DWORD)hGui);
			CESERVER_REQ *pOut = ExecuteCmd(szPipe, &In, GUIATTACH_TIMEOUT, ghConWnd);
			if (pOut) {
				//ghConEmuWnd = hGui;
				ghConEmuWnd = pOut->StartStopRet.hWnd;
				hDcWnd = pOut->StartStopRet.hWndDC;

				//DisableAutoConfirmExit();

				// � ��������, ������� ����� ������������� ����������� �������. � ���� ������ �� �������� �� �����
				// �� ������ �����, ������� ���������� ��� ������� � Win7 ����� ���������� ��������
				if (gbForceHideConWnd)
					ShowWindow(ghConWnd, SW_HIDE);

				COORD crNewSize = {(SHORT)pOut->StartStopRet.nWidth, (SHORT)pOut->StartStopRet.nHeight};
				SMALL_RECT rcWnd = {In.StartStop.sbi.srWindow.Top};
				SetConsoleSize((USHORT)pOut->StartStopRet.nBufferHeight, crNewSize, rcWnd, "Attach2Gui:Ret");
				
				// ���������� ���������� ����� � ������������ ����
				SetConEmuEnvVar(ghConEmuWnd);
				CheckConEmuHwnd();

				ExecuteFreeResult(pOut);
				
				break;
			}
		}
		if (hDcWnd) break;

		dwCur = GetTickCount(); dwDelta = dwCur - dwStart;
		if (dwDelta > nTimeout) break;
		
		Sleep(500);
		dwCur = GetTickCount(); dwDelta = dwCur - dwStart;
	}
	
	return hDcWnd;
}




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
	
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD crMax = GetLargestConsoleWindowSize(h);
	if (crMax.X < 80 || crMax.Y < 25) {
		#ifdef _DEBUG
		DWORD dwErr = GetLastError();
		_ASSERTE(crMax.X >= 80 && crMax.Y >= 25);
		#endif
		if (crMax.X < 80) crMax.X = 80;
		if (crMax.Y < 25) crMax.Y = 25;
	}
	TODO("�������� � nConDataSize ������ ����������� ��� �������� crMax �����");
	int nConDataSize = 0;
	
	_ASSERTE(srv.hFileMapping == NULL);
	
	wsprintf(szMapName, CECONMAPNAME, (DWORD)ghConWnd);
	srv.hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, 
		gpNullSecurity, PAGE_READWRITE, 0, nConInfoSize+nConDataSize, szMapName);
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
	// ������-�� ������ � �� ���� - �������� ��������� zero-initialized
	// �� �� ������ ������ ������� (������ ���������, �� ������ - ������)
	memset(srv.pConsoleInfo, 0, nConInfoSize);
	srv.pConsoleInfo->cbSize = nConInfoSize; // ��� - ������ ������ ���������!
	srv.pConsoleInfo->cbExtraSize = 0; TODO("����� ��������� ������ ������ ��� CESERVER_REQ_CONINFO_DATA[]");
	srv.pConsoleInfo->bConsoleActive = TRUE; // ���� FALSE - ����� ������� ������� ������
	srv.pConsoleInfo->nLogLevel = (ghLogSize!=NULL) ? 1 : 0;
//	srv.pConsoleInfo->nFarReadIdx = -1;
	srv.pConsoleInfo->nServerPID = GetCurrentProcessId();
	srv.pConsoleInfo->nGuiPID = srv.dwGuiPID;
	srv.pConsoleInfo->nProtocolVersion = CESERVER_REQ_VER;
	
	// ������������ ������ ������
	srv.pConsoleInfo->crMaxConSize = crMax;

wrap:	
	return iRc;
}

int CreateColorerHeader()
{
	int iRc = -1;
	wchar_t szMapName[64];
	DWORD dwErr = 0;
	//int nConInfoSize = sizeof(CESERVER_REQ_CONINFO_HDR);
	DWORD nMapCells = 0, nMapSize = 0;
	HWND lhConWnd = NULL;

	_ASSERTE(srv.hColorerMapping == NULL);

	
	lhConWnd = GetConsoleWindow();
	if (!lhConWnd) {
		_ASSERTE(lhConWnd != NULL);
		dwErr = GetLastError();
		_printf ("Can't create console data file mapping. Console window is NULL.\n");
		iRc = CERR_COLORERMAPPINGERR;
		return 0;
	}

	
	COORD crMaxSize = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));
	nMapCells = max(crMaxSize.X,200) * max(crMaxSize.Y,200);
	nMapSize = nMapCells * sizeof(AnnotationInfo) + sizeof(AnnotationHeader);
	
	
	wsprintf(szMapName, AnnotationShareName, sizeof(AnnotationInfo), (DWORD)lhConWnd);
	
	srv.hColorerMapping = CreateFileMapping(INVALID_HANDLE_VALUE, 
		gpNullSecurity, PAGE_READWRITE, 0, nMapSize, szMapName);
	
	if (!srv.hColorerMapping) {
		dwErr = GetLastError();
		_printf ("Can't create colorer data mapping. ErrCode=0x%08X\n", dwErr, szMapName);
		iRc = CERR_COLORERMAPPINGERR;
	} else {
		// ��������� �������� �������� ���������� � �������, ����� ���������!
		AnnotationHeader* pHdr = (AnnotationHeader*)MapViewOfFile(srv.hColorerMapping, FILE_MAP_ALL_ACCESS,0,0,0);
		if (!pHdr) {
			dwErr = GetLastError();
			_printf ("Can't map colorer data mapping. ErrCode=0x%08X\n", dwErr, szMapName);
			iRc = CERR_COLORERMAPPINGERR;
			CloseHandle(srv.hColorerMapping); srv.hColorerMapping = NULL;
		} else {
			pHdr->struct_size = sizeof(AnnotationHeader);
			pHdr->bufferSize = nMapCells;
			pHdr->locked = 0; pHdr->flushCounter = 0;
			// � ������� - ������ �� �����
			UnmapViewOfFile(pHdr);
		}
		// OK
		iRc = 0;
	}

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
	DWORD nMaxCells = (srv.sbi.dwMaximumWindowSize.X * srv.sbi.dwMaximumWindowSize.Y);
	DWORD nMaxSize = (nMaxCells * 2) * sizeof(CHAR_INFO)+sizeof(CESERVER_REQ_CONINFO_DATA);
	wchar_t szErr[255]; szErr[0] = 0;
	wchar_t szMapName[64];

	// ��� ��������� ����� �� ����������� GetLargestConsoleWindowSize & nMaxSize
	COORD crMaxSize = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));
	if (crMaxSize.X < srv.sbi.dwMaximumWindowSize.X) crMaxSize.X = srv.sbi.dwMaximumWindowSize.X;
	if (crMaxSize.Y < srv.sbi.dwMaximumWindowSize.Y) crMaxSize.Y = srv.sbi.dwMaximumWindowSize.Y;
	if (nMaxCells < (DWORD)(crMaxSize.X * crMaxSize.Y)) {
		nMaxCells = (crMaxSize.X * crMaxSize.Y);
		nMaxSize = (nMaxCells * 2) * sizeof(CHAR_INFO)+sizeof(CESERVER_REQ_CONINFO_DATA);
	}
	
	_ASSERTE(srv.pConsoleInfo);
	if (!srv.pConsoleInfo) {
		lstrcpyW(szErr, L"ConEmuC: RecreateMapData failed, srv.pConsoleInfo is NULL");
		goto wrap;
	}
	
	if (srv.pConsoleData)
		CloseMapData();
		
		
	srv.pConsoleDataCopy = (CESERVER_REQ_CONINFO_DATA*)Alloc(nMaxSize,1);
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
	
	srv.pConsoleData = (CESERVER_REQ_CONINFO_DATA*)MapViewOfFile(srv.hFileMappingData, FILE_MAP_ALL_ACCESS,0,0,0);
	if (!srv.pConsoleData) {
		dwErr = GetLastError();
		CloseHandle(srv.hFileMappingData); srv.hFileMappingData = NULL;
		wsprintf (szErr, L"ConEmuC: MapViewOfFile(%s) failed. ErrCode=0x%08X", szMapName, dwErr);
		goto wrap;
	}
	memset(srv.pConsoleData, 0, nMaxSize);
	srv.pConsoleData->crBufSize.X = crMaxSize.X; // srv.sbi.dwMaximumWindowSize.X;
	srv.pConsoleData->crBufSize.Y = crMaxSize.Y; // srv.sbi.dwMaximumWindowSize.Y;
	
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

	ldwConsoleMode = 0;	
	#ifdef _DEBUG
	BOOL lbConModRc =
	#endif
	GetConsoleMode(/*ghConIn*/GetStdHandle(STD_INPUT_HANDLE), &ldwConsoleMode);
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
			#ifdef ASSERT_UNWANTED_SIZE
			if (srv.crReqSizeNewSize.X != lsbi.dwSize.X) {
				LogSize(NULL, ":ReadConsoleInfo(AssertWidth)");
				wchar_t szTitle[64], szInfo[128];
				wsprintf(szTitle, L"ConEmuC, PID=%i", GetCurrentProcessId());
				wsprintf(szInfo, L"Size set by server: {%ix%i}\nCurrent size: {%ix%i}",
					srv.crReqSizeNewSize.X, srv.crReqSizeNewSize.Y, lsbi.dwSize.X, lsbi.dwSize.Y);
				MessageBox(NULL, szInfo, szTitle, MB_OK|MB_SETFOREGROUND|MB_SYSTEMMODAL);
			}
			#endif

			if (ghLogSize) LogSize(NULL, ":ReadConsoleInfo");
		
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
	//srv.pConsoleInfo->nInputTID = srv.dwInputThreadId;
	srv.pConsoleInfo->nReserved0 = 0;
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

	HANDLE hOut = NULL;
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
	DWORD nHdrSize = sizeof(CESERVER_REQ_CONINFO_DATA)-sizeof(CHAR_INFO);
	
	if (!srv.pConsoleData || srv.nConsoleDataSize < (nCurSize+nHdrSize))
	{
		// ���� MapFile ��� �� ����������, ��� ��� �������� ������ �������
		if (!RecreateMapData())
		{
			// ��� �� ������� ����������� MapFile - �� � ��������� �� �����...
			goto wrap;
		}
		
		_ASSERTE(srv.nConsoleDataSize >= (nCurSize+nHdrSize));
	}

	hOut = (HANDLE)ghConOut;
	if (hOut == INVALID_HANDLE_VALUE)
		hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	lbRc = FALSE;
	
	if (nCurSize <= MAX_CONREAD_SIZE)
	{
		bufSize.X = TextWidth; bufSize.Y = TextHeight;
		bufCoord.X = 0; bufCoord.Y = 0;
		rgn = srv.sbi.srWindow;
		if (ReadConsoleOutput(hOut, srv.pConsoleDataCopy->Buf, bufSize, bufCoord, &rgn))
			lbRc = TRUE;
	}
	
	if (!lbRc)
	{
		// �������� ������ ���������
		bufSize.X = TextWidth; bufSize.Y = 1;
		bufCoord.X = 0; bufCoord.Y = 0;
		rgn = srv.sbi.srWindow;
		CHAR_INFO* pLine = srv.pConsoleDataCopy->Buf;
		for(int y = 0; y < (int)TextHeight; y++, rgn.Top++, pLine+=TextWidth)
		{
			rgn.Bottom = rgn.Top;
			ReadConsoleOutput(hOut, pLine, bufSize, bufCoord, &rgn);
		}
	}

	// ���� - �� ��� ���������� � ������������ ��������
	//srv.pConsoleDataCopy->crBufSize.X = TextWidth;
	//srv.pConsoleDataCopy->crBufSize.Y = TextHeight;
	
	if (memcmp(srv.pConsoleData->Buf, srv.pConsoleDataCopy->Buf, nCurSize)) {
		memmove(srv.pConsoleData->Buf, srv.pConsoleDataCopy->Buf, nCurSize);
		lbChanged = TRUE;
	}
	// ���� - �� ��� ���������� � ������������ ��������
	//srv.pConsoleData->crBufSize = srv.pConsoleDataCopy->crBufSize;
	
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

	//if (!lbChanged && srv.nFarInfoLastIdx != srv.pConsoleInfo->nFarInfoIdx)
	//	lbChanged = TRUE;

	if (lbChanged)
	{
		// ��������� ������� � Tick
		if (nPacketID == srv.pConsoleInfo->nPacketId) {
			srv.pConsoleInfo->nPacketId++;
			TODO("����� �������� �� multimedia tick");
			srv.pConsoleInfo->nSrvUpdateTick = GetTickCount();
//			srv.nFarInfoLastIdx = srv.pConsoleInfo->nFarInfoIdx;
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
	srv.hWinHookStartEnd = SetWinEventHook(
		//EVENT_CONSOLE_LAYOUT, -- � ���������, EVENT_CONSOLE_LAYOUT ����� ������� �� ����� ������
		//                      -- "�����" � ��� scroll & resize & focus, � �� ����� ���� ����� �����
		//                      -- ��� ��� ��� �������������� ����� ������ ��� �� ��������
		EVENT_CONSOLE_START_APPLICATION,
		EVENT_CONSOLE_END_APPLICATION,
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
			// �� �� ���� ������� ��������: (idChild == CONSOLE_APPLICATION_16BIT)
			// ������ ��� �� �������� �����, ����� 16��� (��� DOS) ���������� ����� 
			// ����������� ����� ������ �� ����� ������ ��������.
			if (idChild == CONSOLE_APPLICATION_16BIT) {
				if (ghLogSize) {
					char szInfo[64]; wsprintfA(szInfo, "NTVDM started, PID=%i", idObject);
					LogSize(NULL, szInfo);
				}

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
				if (ghLogSize) {
					char szInfo[64]; wsprintfA(szInfo, "NTVDM stopped, PID=%i", idObject);
					LogSize(NULL, szInfo);
				}

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
	case EVENT_CONSOLE_LAYOUT: //0x4005
		{
		//The console layout has changed.
		//EVENT_CONSOLE_LAYOUT, -- � ���������, EVENT_CONSOLE_LAYOUT ����� ������� �� ����� ������
		//                      -- "�����" � ��� scroll & resize & focus, � �� ����� ���� ����� �����
		//                      -- ��� ��� ��� �������������� ����� ������ ��� �� ��������
		#ifdef _DEBUG
		DEBUGSTR(L"EVENT_CONSOLE_LAYOUT\n");
		#endif
		}
		return; // ���������� �� ������� � ����
	}
}














DWORD WINAPI RefreshThread(LPVOID lpvParam)
{
	DWORD nWait = 0, dwConWait = 0;
	HANDLE hEvents[2] = {ghQuitEvent, srv.hRefreshEvent};
	DWORD nDelta = 0;
	DWORD nLastUpdateTick = 0; //GetTickCount();
	DWORD nLastConHandleTick = GetTickCount();
	BOOL  /*lbEventualChange = FALSE,*/ lbForceSend = FALSE, lbChanged = FALSE; //, lbProcessChanged = FALSE;
	DWORD dwTimeout = 10; // ������������� ������ ���������� �� ���� (��������, �������,...)

	while (TRUE)
	{
		nWait = WAIT_TIMEOUT;
		lbForceSend = FALSE;
		MCHKHEAP;

		// Alwas update con handle, ������ �������
		if ((GetTickCount() - nLastConHandleTick) > UPDATECONHANDLE_TIMEOUT) {
			WARNING("!!! � Win7 �������� ����������� � ������ �������� - ��������� ���������� ����� ���������!!!");
			// � �����, ����� ������ telnet'� ������������!
			if (srv.bReopenHandleAllowed) {
				ghConOut.Close();
				nLastConHandleTick = GetTickCount();
			}
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
		if (srv.nRequestChangeSize) {
			// AVP ������... �� ����� � �� �����
			//DWORD dwSusp = 0, dwSuspErr = 0;
			//if (srv.hRootThread) {
			//	WARNING("A 64-bit application can suspend a WOW64 thread using the Wow64SuspendThread function");
			//	// The handle must have the THREAD_SUSPEND_RESUME access right
			//	dwSusp = SuspendThread(srv.hRootThread);
			//	if (dwSusp == (DWORD)-1) dwSuspErr = GetLastError();
			//}
			
			SetConsoleSize(srv.nReqSizeBufferHeight, srv.crReqSizeNewSize, srv.rReqSizeNewRect, srv.sReqSizeLabel);
	
			//if (srv.hRootThread) {
			//	ResumeThread(srv.hRootThread);
			//}
			
			SetEvent(srv.hReqSizeChanged);
		}
		
		
		
		// ��������� ���������� ��������� � �������.
		// ������� �������� ghExitQueryEvent, ���� ��� �������� �����������.
		//lbProcessChanged = CheckProcessCount();
		// ������� ����������� ������ ����� �������� CHECK_PROCESSES_TIMEOUT (������ ������ �� ������ �������)
		// #define CHECK_PROCESSES_TIMEOUT 500
		CheckProcessCount();
		
		
		

		// ��������� ��������
		if (srv.nMaxFPS>0) {
			dwTimeout = 1000 / srv.nMaxFPS;
			// ���� 50, ����� �� ������������� ������� ��� �� ������� ���������� ("dir /s" � �.�.)
			if (dwTimeout < 10) dwTimeout = 10;
		} else {
			dwTimeout = 100;
		}
		// !!! ����� ������� ������ ���� �����������, �� ����� ��� ������� ���������
			//		HANDLE hOut = (HANDLE)ghConOut;
			//		if (hOut == INVALID_HANDLE_VALUE) hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			//		TODO("���������, � ���������� ��������� �� ��������� � �������?");
			//		-- �� ��������� (������) � ������� �� ���������
			//		dwConWait = WaitForSingleObject ( hOut, dwTimeout );
			//#ifdef _DEBUG
			//		if (dwConWait == WAIT_OBJECT_0) {
			//			DEBUGSTRCHANGES(L"STD_OUTPUT_HANDLE was set\n");
			//		}
			//#endif
			//		
		nWait = WaitForMultipleObjects ( 2, hEvents, FALSE, dwTimeout/*dwTimeout*/ );
		if (nWait == WAIT_OBJECT_0) {
			break; // ����������� ���������� ����
		}// else if (nWait == WAIT_TIMEOUT && dwConWait == WAIT_OBJECT_0) {
		//	nWait = (WAIT_OBJECT_0+1);
		//}

		//lbEventualChange = (nWait == (WAIT_OBJECT_0+1))/* || lbProcessChanged*/;
		//lbForceSend = (nWait == (WAIT_OBJECT_0+1));

		// ����� �� ������� ��������� ����������� ���������
		//if (!lbForceSend) {
		if (!srv.pConsoleInfo->bConsoleActive) {
			DWORD nCurTick = GetTickCount();
			nDelta = nCurTick - nLastUpdateTick;
			// #define MAX_FORCEREFRESH_INTERVAL 1000
			if (nDelta > MAX_FORCEREFRESH_INTERVAL) {
				lbForceSend = TRUE;
			} else {
				// ����� �� ������� ���������
				continue;
			}
		}

		
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



			


		/* ****************** */
		/* ���������� ������� */
		/* ****************** */
		lbChanged = ReloadFullConsoleInfo(FALSE/*lbForceSend*/);

		
		// ��� ���������� - ��������� ��������� tick
		if (lbChanged)
			nLastUpdateTick = GetTickCount();


		MCHKHEAP
	}

	return 0;
}



//// srv.hInputThread && srv.dwInputThreadId
//DWORD WINAPI InputThread(LPVOID lpvParam) 
//{
//	MSG msg;
//	//while (GetMessage(&msg,0,0,0))
//	INPUT_RECORD rr[MAX_EVENTS_PACK];
//	
//	while (TRUE) {
//		if (!PeekMessage(&msg,0,0,0,PM_REMOVE)) {
//			Sleep(10);
//			continue;
//		}
//		if (msg.message == WM_QUIT)
//			break;
//
//		if (ghQuitEvent) {
//			if (WaitForSingleObject(ghQuitEvent, 0) == WAIT_OBJECT_0)
//				break;
//		}
//		if (msg.message == WM_NULL) {
//			_ASSERTE(msg.message != WM_NULL);
//			continue;
//		}
//
//		//if (msg.message == INPUT_THREAD_ALIVE_MSG) {
//		//	//pRCon->mn_FlushOut = msg.wParam;
//		//	TODO("INPUT_THREAD_ALIVE_MSG");
//		//	continue;
//		//
//		//} else {
//
//		// ��������� ����� ���������, ����� ��� ���������� � �������?
//		UINT nCount = 0;
//		while (nCount < MAX_EVENTS_PACK)
//		{
//			if (msg.message == WM_NULL) {
//				_ASSERTE(msg.message != WM_NULL);
//			} else {
//				if (ProcessInputMessage(msg, rr[nCount]))
//					nCount++;
//			}
//			if (!PeekMessage(&msg,0,0,0,PM_REMOVE))
//				break;
//			if (msg.message == WM_QUIT)
//				break;
//		}
//		if (nCount && msg.message != WM_QUIT) {
//			SendConsoleEvent(rr, nCount);
//		}
//		//}
//	}
//
//	return 0;
//}

BOOL WriteInputQueue(const INPUT_RECORD *pr)
{
	INPUT_RECORD* pNext = srv.pInputQueueWrite;
	// ���������, ���� �� ��������� ����� � ������
	if (srv.pInputQueueRead != srv.pInputQueueEnd) {
		if (srv.pInputQueueRead < srv.pInputQueueEnd
			&& ((srv.pInputQueueWrite+1) == srv.pInputQueueRead))
		{
			return FALSE;
		}
	}
	// OK
	*pNext = *pr;
	srv.pInputQueueWrite++;
	if (srv.pInputQueueWrite >= srv.pInputQueueEnd)
		srv.pInputQueueWrite = srv.pInputQueue;
	SetEvent(srv.hInputEvent);

	// ��������� ��������� ������, ���� �� ����� ����� ��� ����
	if (srv.pInputQueueRead == srv.pInputQueueEnd)
		srv.pInputQueueRead = pNext;
	return TRUE;
}

BOOL IsInputQueueEmpty()
{
	if (srv.pInputQueueRead != srv.pInputQueueEnd
		&& srv.pInputQueueRead != srv.pInputQueueWrite)
		return FALSE;
	return TRUE;
}

BOOL ReadInputQueue(INPUT_RECORD *prs, DWORD *pCount)
{
	DWORD nCount = 0;
	
	if (!IsInputQueueEmpty())
	{
		DWORD n = *pCount;
		INPUT_RECORD *pSrc = srv.pInputQueueRead;
		INPUT_RECORD *pEnd = (srv.pInputQueueRead < srv.pInputQueueWrite) ? srv.pInputQueueWrite : srv.pInputQueueEnd;
		INPUT_RECORD *pDst = prs;
		while (n && pSrc < pEnd) {
			*pDst = *pSrc; nCount++;
			n--; pSrc++; pDst++;
		}
		if (pSrc == srv.pInputQueueEnd)
			pSrc = srv.pInputQueue;
		TODO("�������� ������ ������ ������, ���� ������� ��� �����");
		//
		srv.pInputQueueRead = pSrc;
	}
	
	*pCount = nCount;
	return (nCount>0);
}

DWORD WINAPI InputThread(LPVOID lpvParam)
{
	HANDLE hEvents[2] = {ghQuitEvent, srv.hInputEvent};
	DWORD dwWait = 0;
	INPUT_RECORD ir[100];
	
	while ((dwWait = WaitForMultipleObjects ( 2, hEvents, FALSE, INPUT_QUEUE_TIMEOUT )) != WAIT_OBJECT_0)
	{
		if (IsInputQueueEmpty())
			continue;
		
		// ���� �� ����� - ��� ����� �������
		WaitConsoleReady();

		// ������ � �����
		DWORD nInputCount = sizeof(ir)/sizeof(ir[0]);
		if (ReadInputQueue(ir, &nInputCount)) {
			_ASSERTE(nInputCount>0);
			SendConsoleEvent(ir, nInputCount);
		}

		// ���� �� ����� ������ � ������� � ������ ��� ���-�� ��������� - ����������
		if (!IsInputQueueEmpty())
			SetEvent(srv.hInputEvent);
	}
	
	return 1;
}

DWORD WINAPI InputPipeThread(LPVOID lpvParam) 
{ 
	BOOL fConnected, fSuccess; 
	//DWORD nCurInputCount = 0;
	//DWORD srv.dwServerThreadId;
	//HANDLE hPipe = NULL; 
	DWORD dwErr = 0;


	// The main loop creates an instance of the named pipe and 
	// then waits for a client to connect to it. When the client 
	// connects, a thread is created to handle communications 
	// with that client, and the loop is repeated. 

	while (!gbQuit)
	{
		MCHKHEAP;
		srv.hInputPipe = CreateNamedPipe( 
			srv.szInputname,          // pipe name 
			PIPE_ACCESS_INBOUND,      // goes from client to server only
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			PIPEBUFSIZE,              // output buffer size 
			PIPEBUFSIZE,              // input buffer size 
			0,                        // client time-out
			gpNullSecurity);          // default security attribute 

		if (srv.hInputPipe == INVALID_HANDLE_VALUE) 
		{
			dwErr = GetLastError();
			_ASSERTE(srv.hInputPipe != INVALID_HANDLE_VALUE);
			_printf("CreatePipe failed, ErrCode=0x%08X\n", dwErr);
			Sleep(50);
			//return 99;
			continue;
		}

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 

		fConnected = ConnectNamedPipe(srv.hInputPipe, NULL) ? 
		TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

		MCHKHEAP;
		if (fConnected) 
		{ 
			//TODO:
			DWORD cbBytesRead; //, cbWritten;
			MSG imsg; memset(&imsg,0,sizeof(imsg));
			while (!gbQuit && (fSuccess = ReadFile( 
					srv.hInputPipe,        // handle to pipe 
					&imsg,        // buffer to receive data 
					sizeof(imsg), // size of buffer 
					&cbBytesRead, // number of bytes read 
					NULL)) != FALSE)        // not overlapped I/O 
			{
				// ������������� ����������� ���������� ����
				if (gbQuit)
					break;

				MCHKHEAP;
				if (imsg.message) {
					#ifdef _DEBUG
						switch (imsg.message) {
							case WM_KEYDOWN: case WM_SYSKEYDOWN: DEBUGSTRINPUTPIPE(L"ConEmuC: Recieved key down\n"); break;
							case WM_KEYUP: case WM_SYSKEYUP: DEBUGSTRINPUTPIPE(L"ConEmuC: Recieved key up\n"); break;
							default: DEBUGSTRINPUTPIPE(L"ConEmuC: Recieved input\n");
						}
					#endif

					INPUT_RECORD r;
					ProcessInputMessage(imsg, r);
					//SendConsoleEvent(&r, 1);
					if (!WriteInputQueue(&r)) {
						WARNING("���� ����� ���������� - �����? ���� ���� ����� ����� ����� - ����� ��������� GUI �� ������ � pipe...");
					}

					MCHKHEAP;
				}
				// next
				memset(&imsg,0,sizeof(imsg));
				MCHKHEAP;
			}
			SafeCloseHandle(srv.hInputPipe);

		} 
		else 
			// The client could not connect, so close the pipe. 
			SafeCloseHandle(srv.hInputPipe);
	} 
	MCHKHEAP;
	return 1; 
} 

BOOL ProcessInputMessage(MSG &msg, INPUT_RECORD &r)
{
	memset(&r, 0, sizeof(r));
	BOOL lbOk = FALSE;

	if (!UnpackInputRecord(&msg, &r)) {
		_ASSERT(FALSE);
		
	} else {
		TODO("������� ��������� ����� ���������, ����� ��� ���������� � �������?");

		#define ALL_MODIFIERS (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED|SHIFT_PRESSED)
		#define CTRL_MODIFIERS (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)

		bool lbProcessEvent = false;
		bool lbIngoreKey = false;
		if (r.EventType == KEY_EVENT && r.Event.KeyEvent.bKeyDown &&
			(r.Event.KeyEvent.wVirtualKeyCode == 'C' || r.Event.KeyEvent.wVirtualKeyCode == VK_CANCEL)
			&&      ( // ������������ ������ Ctrl
					(r.Event.KeyEvent.dwControlKeyState & CTRL_MODIFIERS) &&
					((r.Event.KeyEvent.dwControlKeyState & ALL_MODIFIERS) 
					== (r.Event.KeyEvent.dwControlKeyState & CTRL_MODIFIERS))
					)
			)
		{
			lbProcessEvent = true;

			DWORD dwMode = 0;
			GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &dwMode);

			// CTRL+C (and Ctrl+Break?) is processed by the system and is not placed in the input buffer
			if ((dwMode & ENABLE_PROCESSED_INPUT) == ENABLE_PROCESSED_INPUT)
				lbIngoreKey = lbProcessEvent = true;
			else
				lbProcessEvent = false;


			if (lbProcessEvent) {

				BOOL lbRc = FALSE;
				DWORD dwEvent = (r.Event.KeyEvent.wVirtualKeyCode == 'C') ? CTRL_C_EVENT : CTRL_BREAK_EVENT;
				//&& (srv.dwConsoleMode & ENABLE_PROCESSED_INPUT)

				//The SetConsoleMode function can disable the ENABLE_PROCESSED_INPUT mode for a console's input buffer, 
				//so CTRL+C is reported as keyboard input rather than as a signal. 
				// CTRL+BREAK is always treated as a signal
				if ( // ������������ ������ Ctrl
					(r.Event.KeyEvent.dwControlKeyState & CTRL_MODIFIERS) &&
					((r.Event.KeyEvent.dwControlKeyState & ALL_MODIFIERS) 
					== (r.Event.KeyEvent.dwControlKeyState & CTRL_MODIFIERS))
					)
				{
					// ����� ��������, ������� �� ��������� ������� � ������ CREATE_NEW_PROCESS_GROUP
					// ����� � ��������������� ������� (WinXP SP3) ������ �����, � ��� ���������
					// �� Ctrl-Break, �� ������� ���������� Ctrl-C
					lbRc = GenerateConsoleCtrlEvent(dwEvent, 0);

					// ��� ������� (Ctrl+C) � ����� ����������(!) ����� �� ���� �� ������ ���������� ������� C � ������� Ctrl
				}
			}

			if (lbIngoreKey)
				return FALSE;
		}

		#ifdef _DEBUG
		if (r.EventType == KEY_EVENT && r.Event.KeyEvent.bKeyDown &&
			r.Event.KeyEvent.wVirtualKeyCode == VK_F11)
		{
			DEBUGSTR(L"  ---  F11 recieved\n");
		}
		#endif
		#ifdef _DEBUG
		if (r.EventType == MOUSE_EVENT) {
			static DWORD nLastEventTick = 0;
			if (nLastEventTick && (GetTickCount() - nLastEventTick) > 2000) {
				OutputDebugString(L".\n");
			}
			wchar_t szDbg[60]; 
			wsprintf(szDbg, L"ConEmuC.MouseEvent(X=%i,Y=%i,Btns=0x%04x,Moved=%i)\n", r.Event.MouseEvent.dwMousePosition.X, r.Event.MouseEvent.dwMousePosition.Y, r.Event.MouseEvent.dwButtonState, (r.Event.MouseEvent.dwEventFlags & MOUSE_MOVED));
			DEBUGLOGINPUT(szDbg);
			nLastEventTick = GetTickCount();
		}
		#endif

		// ���������, ����� ���� ��������� ���������� ������������
		if (r.EventType == KEY_EVENT
			|| (r.EventType == MOUSE_EVENT 
			&& (r.Event.MouseEvent.dwButtonState || r.Event.MouseEvent.dwEventFlags 
			|| r.Event.MouseEvent.dwEventFlags == DOUBLE_CLICK)))
		{
			srv.dwLastUserTick = GetTickCount();
		}

		lbOk = TRUE;
		//SendConsoleEvent(&r, 1);
	}
	
	return lbOk;
}

// ���������� ����� ����� ������� ������� �����
BOOL WaitConsoleReady()
{
	// ���� ������ ���� ������ - ������������ ��������� � ����� �������
	if (srv.bInSyncResize)
		WaitForSingleObject(srv.hAllowInputEvent, MAX_SYNCSETSIZE_WAIT);

	DWORD nCurInputCount = 0, cbWritten = 0;
	//INPUT_RECORD irDummy[2] = {{0},{0}};

	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE); // ��� ��� ghConIn
	
	// 27.06.2009 Maks - If input queue is not empty - wait for a while, to avoid conflicts with FAR reading queue
	// 19.02.2010 Maks - ������ �� GetNumberOfConsoleInputEvents
	//if (PeekConsoleInput(hIn, irDummy, 1, &(nCurInputCount = 0)) && nCurInputCount > 0) {
	if (GetNumberOfConsoleInputEvents(hIn, &(nCurInputCount = 0)) && nCurInputCount > 0) {
		DWORD dwStartTick = GetTickCount();
		WARNING("Do NOT wait, but place event in Cyclic queue");
		do {
			Sleep(5);
			//if (!PeekConsoleInput(hIn, irDummy, 1, &(nCurInputCount = 0)))
			if (!GetNumberOfConsoleInputEvents(hIn, &(nCurInputCount = 0)))
				nCurInputCount = 0;
		} while ((nCurInputCount > 0) && ((GetTickCount() - dwStartTick) < MAX_INPUT_QUEUE_EMPTY_WAIT));
	}
	
	return (nCurInputCount == 0);
}

BOOL SendConsoleEvent(INPUT_RECORD* pr, UINT nCount)
{
	if (!nCount || !pr) {
		_ASSERTE(nCount>0 && pr!=NULL);
		return FALSE;
	}

	BOOL fSuccess = FALSE;

	//// ���� ������ ���� ������ - ������������ ��������� � ����� �������
	//if (srv.bInSyncResize)
	//	WaitForSingleObject(srv.hAllowInputEvent, MAX_SYNCSETSIZE_WAIT);

	//DWORD nCurInputCount = 0, cbWritten = 0;
	//INPUT_RECORD irDummy[2] = {{0},{0}};

	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE); // ��� ��� ghConIn

	// 02.04.2010 Maks - ���������� � WaitConsoleReady
	//// 27.06.2009 Maks - If input queue is not empty - wait for a while, to avoid conflicts with FAR reading queue
	//// 19.02.2010 Maks - ������ �� GetNumberOfConsoleInputEvents
	////if (PeekConsoleInput(hIn, irDummy, 1, &(nCurInputCount = 0)) && nCurInputCount > 0) {
	//if (GetNumberOfConsoleInputEvents(hIn, &(nCurInputCount = 0)) && nCurInputCount > 0) {
	//	DWORD dwStartTick = GetTickCount();
	//	WARNING("Do NOT wait, but place event in Cyclic queue");
	//	do {
	//		Sleep(5);
	//		//if (!PeekConsoleInput(hIn, irDummy, 1, &(nCurInputCount = 0)))
	//		if (!GetNumberOfConsoleInputEvents(hIn, &(nCurInputCount = 0)))
	//			nCurInputCount = 0;
	//	} while ((nCurInputCount > 0) && ((GetTickCount() - dwStartTick) < MAX_INPUT_QUEUE_EMPTY_WAIT));
	//}

	INPUT_RECORD* prNew = NULL;
	int nAllCount = 0;
	for (UINT n = 0; n < nCount; n++) {
		if (pr[n].EventType != KEY_EVENT) {
			nAllCount++;
		} else {
			if (!pr[n].Event.KeyEvent.wRepeatCount) {
				_ASSERTE(pr[n].Event.KeyEvent.wRepeatCount!=0);
				pr[n].Event.KeyEvent.wRepeatCount = 1;
			}
			nAllCount += pr[n].Event.KeyEvent.wRepeatCount;
		}
	}
	if (nAllCount > (int)nCount) {
		prNew = (INPUT_RECORD*)malloc(sizeof(INPUT_RECORD)*nAllCount);
		if (prNew) {
			INPUT_RECORD* ppr = prNew;
			INPUT_RECORD* pprMod = NULL;
			for (UINT n = 0; n < nCount; n++) {
				*(ppr++) = pr[n];
				if (pr[n].EventType == KEY_EVENT) {
					UINT nCurCount = pr[n].Event.KeyEvent.wRepeatCount;
					
					if (nCurCount > 1) {
						pprMod = (ppr-1);
						pprMod->Event.KeyEvent.wRepeatCount = 1;
						for (UINT i = 1; i < nCurCount; i++) {
							*(ppr++) = *pprMod;
						}
					}
				}
			}
			pr = prNew;
			_ASSERTE(nAllCount == (ppr-prNew));
			nCount = (UINT)(ppr-prNew);
		}
	}

	DWORD cbWritten = 0;
	fSuccess = WriteConsoleInput(hIn, pr, nCount, &cbWritten);
	_ASSERTE(fSuccess && cbWritten==nCount);
	
	if (prNew) free(prNew);

	return fSuccess;
} 

DWORD WINAPI QueryPipeThread(LPVOID lpvParam) 
{ 
	BOOL fConnected, fSuccess; 
	//DWORD nCurInputCount = 0;
	//DWORD srv.dwServerThreadId;
	//HANDLE srv.hQueryPipe = NULL; 
	DWORD dwErr = 0;


	// The main loop creates an instance of the named pipe and 
	// then waits for a client to connect to it. When the client 
	// connects, a thread is created to handle communications 
	// with that client, and the loop is repeated. 

	for (;;) 
	{
		MCHKHEAP;
		srv.hQueryPipe = CreateNamedPipe( 
			srv.szQueryname,          // pipe name 
			PIPE_ACCESS_INBOUND,      // goes from client to server only
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			PIPEBUFSIZE,              // output buffer size 
			PIPEBUFSIZE,              // input buffer size 
			0,                        // client time-out
			gpNullSecurity);          // default security attribute 

		if (srv.hQueryPipe == INVALID_HANDLE_VALUE) 
		{
			dwErr = GetLastError();
			_ASSERTE(srv.hQueryPipe != INVALID_HANDLE_VALUE);
			srv.hQueryPipe = NULL;
			_printf("CreatePipe failed, ErrCode=0x%08X\n", dwErr);
			Sleep(50);
			//return 99;
			continue;
		}

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 

		fConnected = ConnectNamedPipe(srv.hQueryPipe, NULL) ? 
			TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

		MCHKHEAP;
		if (fConnected) 
		{ 
			//TODO:
			DWORD cbBytesRead; //, cbWritten;
			//MSG imsg; memset(&imsg,0,sizeof(imsg));
			CESERVER_REQ iReq;

			while ((fSuccess = ReadFile( 
					srv.hQueryPipe,        // handle to pipe 
					&iReq,        // buffer to receive data 
					sizeof(iReq), // size of buffer 
					&cbBytesRead, // number of bytes read 
					NULL)) != FALSE)        // not overlapped I/O 
			{
				// ������������� ����������� ���������� ����
				if (iReq.hdr.nCmd == 0xFFFF) {
					SafeCloseHandle(srv.hQueryPipe);
					break;
				}
				MCHKHEAP;
				//TODO:

			}
		} 
		else 
			// The client could not connect, so close the pipe. 
			SafeCloseHandle(srv.hQueryPipe);
	} 
	MCHKHEAP;
	return 1; 
} 
