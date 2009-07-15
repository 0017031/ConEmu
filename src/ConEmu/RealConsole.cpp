
// WARNING!!! �������� ��������� ������� !!!

#define SHOWDEBUGSTR

#include "Header.h"
#include <Tlhelp32.h>

#define DEBUGSTRDRAW(s) //DEBUGSTR(s)
#define DEBUGSTRINPUT(s) DEBUGSTR(s)
#define DEBUGSTRSIZE(s) //DEBUGSTR(s)
#define DEBUGSTRPROC(s) //DEBUGSTR(s)
#define DEBUGSTRCMD(s) //DEBUGSTR(s)
#define DEBUGSTRPKT(s) //DEBUGSTR(s)

WARNING("��� ������� ������ ������ ������ ����� '���������' �� ����� �� ����, �� ���������� ��������� ������");

WARNING("���� � ��������� ���������� ������ ����� ���������� ��� Alt-F7, Enter. ��������, ���� ����� ���");
// ������ �� �������������� ������ ������ ��������� - ������ ����� ������� ����������� ����������.
// ������ ������ �����������, �� ����� �� ������ "..." �����������

//PRAGMA_ERROR("��� ������� ���������� F:\\VCProject\\FarPlugin\\PPCReg\\compile.cmd - Enter � ������� �� ������");

TODO("CES_CONMANACTIVE deprecated");

WARNING("����� ���� ������ ServerThread? � MonitorThread ��������������� ������ ��� ������� ��������, � ����� �� ���� �����");

WARNING("� ������ VCon ������� ����� BYTE[256] ��� �������� ������������ ������ (Ctrl,...,Up,PgDn,Add,� ��.");
WARNING("�������������� ����� �������� � ����� {VKEY,wchar_t=0}, � ������� ��������� ��������� wchar_t �� WM_CHAR/WM_SYSCHAR");
WARNING("��� WM_(SYS)CHAR �������� wchar_t � ������, � ������ ��������� VKEY");
WARNING("��� ������������ WM_KEYUP - �����(� ������) wchar_t �� ����� ������, ������ � ������� UP");
TODO("� ������������ - ��������� �������� isKeyDown, � ������� �����");
WARNING("��� ������������ �� ������ ������� (�� �������� � � �������� ������ ������� - ����������� ����� ���� ������� � ����� ���������) ��������� �������� caps, scroll, num");
WARNING("� ����� ���������� �������/������� ��������� ����� �� �� ���������� Ctrl/Shift/Alt");

WARNING("����� ����� ��������������� ���������� ������ ������� ���������� (OK), �� ����������� ���������� ������� �� �������� � GUI - ��� ������� ���������� ������� � ������ ������");


//������, ��� ���������, ������ �������, ���������� �����, � ��...

#define VCURSORWIDTH 2
#define HCURSORHEIGHT 2

#define Free SafeFree
#define Alloc calloc

#define Assert(V) if ((V)==FALSE) { TCHAR szAMsg[MAX_PATH*2]; wsprintf(szAMsg, _T("Assertion (%s) at\n%s:%i"), _T(#V), _T(__FILE__), __LINE__); Box(szAMsg); }

#ifdef _DEBUG
#define HEAPVAL MCHKHEAP
#else
#define HEAPVAL 
#endif

#ifdef _DEBUG
    #define FORCE_INVALIDATE_TIMEOUT 3000
#else
    #define FORCE_INVALIDATE_TIMEOUT 300
#endif


#define MOUSE_EVENT_MOVE      (WM_APP+10)
#define MOUSE_EVENT_CLICK     (WM_APP+11)
#define MOUSE_EVENT_DBLCLICK  (WM_APP+12)
#define MOUSE_EVENT_WHEELED   (WM_APP+13)
#define MOUSE_EVENT_HWHEELED  (WM_APP+14)
#define MOUSE_EVENT_FIRST MOUSE_EVENT_MOVE
#define MOUSE_EVENT_LAST MOUSE_EVENT_HWHEELED

#define INPUT_THREAD_ALIVE_MSG (WM_APP+100)


CRealConsole::CRealConsole(CVirtualConsole* apVCon)
{
    mp_VCon = apVCon;
    memset(Title,0,sizeof(Title)); memset(TitleCmp,0,sizeof(TitleCmp));
    mp_tabs = NULL; mn_tabsCount = 0; ms_PanelTitle[0] = 0; mn_ActiveTab = 0;
    memset(&m_PacketQueue, 0, sizeof(m_PacketQueue));
    
    mn_FlushIn = mn_FlushOut = 0;

    mr_LeftPanel = mr_RightPanel = MakeRect(-1,-1);

    wcscpy(Title, L"ConEmu");
    wcscpy(ms_PanelTitle, Title);
    mn_Progress = -1; mn_PreWarningProgress = -1; // ��������� ���

    hPictureView = NULL; mb_PicViewWasHidden = FALSE;

    mh_MonitorThread = NULL; mn_MonitorThreadID = 0; 
    mh_InputThread = NULL; mn_InputThreadID = 0;
    
    mn_ConEmuC_PID = 0; mh_ConEmuC = NULL; mh_ConEmuCInput = NULL;
    
    mb_NeedStartProcess = FALSE; mb_IgnoreCmdStop = FALSE;

    ms_ConEmuC_Pipe[0] = 0; ms_ConEmuCInput_Pipe[0] = 0; ms_VConServer_Pipe[0] = 0;
    mh_TermEvent = CreateEvent(NULL,TRUE/*MANUAL - ������������ � ���������� �����!*/,FALSE,NULL); ResetEvent(mh_TermEvent);
    mh_MonitorThreadEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    //mh_EndUpdateEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    //WARNING("mh_Sync2WindowEvent ������");
    //mh_Sync2WindowEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    //mh_ConChanged = CreateEvent(NULL,FALSE,FALSE,NULL);
    mh_PacketArrived = CreateEvent(NULL,FALSE,FALSE,NULL);
    //mh_CursorChanged = NULL;
    //mb_Detached = FALSE;
    m_Args.pszSpecialCmd = NULL;
    mb_FullRetrieveNeeded = FALSE;
    memset(&m_LastMouse, 0, sizeof(m_LastMouse));
    mb_DataChanged = FALSE;

    mn_ProgramStatus = 0; mn_FarStatus = 0;
    isShowConsole = false;
    mb_ConsoleSelectMode = false;
    mn_ProcessCount = 0; mn_FarPID = 0; mn_InRecreate = 0; mb_ProcessRestarted = FALSE;
    mn_LastSetForegroundPID = 0;
    mh_ServerSemaphore = NULL;
    memset(mh_ServerThreads, 0, sizeof(mh_ServerThreads)); mh_ActiveServerThread = NULL;
    memset(mn_ServerThreadsId, 0, sizeof(mn_ServerThreadsId));

    ZeroStruct(con); //WARNING! �������� CriticalSection, ������� ����� ��������� ����� InitializeCriticalSection(&csCON);
    mb_BuferModeChangeLocked = FALSE;

	ZeroStruct(m_Args);

    mn_LastInvalidateTick = 0;

    //InitializeCriticalSection(&csPRC); ncsTPRC = 0;
    //InitializeCriticalSection(&csCON); con.ncsT = 0;
    //InitializeCriticalSection(&csPKT); ncsTPKT = 0;

    mn_LastProcessedPkt = 0;

    hConWnd = NULL; mh_GuiAttached = NULL; mn_Focused = -1;
    mn_LastVKeyPressed = 0;
    mh_LogInput = NULL; mpsz_LogInputFile = NULL; mpsz_LogPackets = NULL; mn_LogPackets = 0;

    m_UseLogs = gSet.isAdvLogging;

    mb_PluginDetected = FALSE; mn_FarPID_PluginDetected = 0;

    lstrcpy(ms_Editor, L"edit ");
    MultiByteToWideChar(CP_ACP, 0, "�������������� ", -1, ms_EditorRus, 32);
    lstrcpy(ms_Viewer, L"view ");
    MultiByteToWideChar(CP_ACP, 0, "�������� ", -1, ms_ViewerRus, 32);
    lstrcpy(ms_TempPanel, L"{Temporary panel");
    MultiByteToWideChar(CP_ACP, 0, "{��������� ������", -1, ms_TempPanelRus, 32);


    SetTabs(NULL,1); // ��� ������ - ���������� ������� Console, � ��� ��� ����������
    
    PreInit(FALSE); // ������ ���������������� ���������� ��������...
}

CRealConsole::~CRealConsole()
{
    StopThread();
    
    MCHKHEAP

    if (con.pConChar)
        { Free(con.pConChar); con.pConChar = NULL; }
    if (con.pConAttr)
        { Free(con.pConAttr); con.pConAttr = NULL; }

    if (m_Args.pszSpecialCmd)
        { Free(m_Args.pszSpecialCmd); m_Args.pszSpecialCmd = NULL; }


    SafeCloseHandle(mh_ConEmuC); mn_ConEmuC_PID = 0;
    SafeCloseHandle(mh_ConEmuCInput);
    
    SafeCloseHandle(mh_ServerSemaphore);
    SafeCloseHandle(mh_GuiAttached);

    //DeleteCriticalSection(&csPRC);
    //DeleteCriticalSection(&csCON);
    //DeleteCriticalSection(&csPKT);
    
    
    if (mp_tabs) Free(mp_tabs);
    mp_tabs = NULL; mn_tabsCount = 0; mn_ActiveTab = 0;

    // 
    CloseLogFiles();
}

BOOL CRealConsole::PreCreate(RConStartArgs *args)
	//(BOOL abDetached, LPCWSTR asNewCmd /*= NULL*/, , BOOL abAsAdmin /*= FALSE*/)
{
    mb_NeedStartProcess = FALSE;

    if (args->pszSpecialCmd && !m_Args.pszSpecialCmd) {
        _ASSERTE(args->bDetached == FALSE);
        int nLen = lstrlenW(args->pszSpecialCmd);
        m_Args.pszSpecialCmd = (wchar_t*)Alloc(nLen+1,2);
        if (!m_Args.pszSpecialCmd)
            return FALSE;
        lstrcpyW(m_Args.pszSpecialCmd, args->pszSpecialCmd);
    }

	m_Args.bRunAsAdministrator = args->bRunAsAdministrator;

    if (args->bDetached) {
        // ���� ������ �� ������ - ������ ��������� ��������� ����
        if (!PreInit()) { //TODO: ������-�� PreInit() ��� �������� ������...
            return FALSE;
        }
        m_Args.bDetached = TRUE;
    } else
    if (gSet.nAttachPID) {
        // Attach - only once
        DWORD dwPID = gSet.nAttachPID; gSet.nAttachPID = 0;
        if (!AttachPID(dwPID)) {
            return FALSE;
        }
    } else {
        mb_NeedStartProcess = TRUE;
    }

    if (!StartMonitorThread()) {
        return FALSE;
    }

    return TRUE;
}


void CRealConsole::DumpConsole(HANDLE ahFile)
{
    DWORD dw = 0;
    if (con.pConChar && con.pConAttr) {
        WriteFile(ahFile, con.pConChar, con.nTextWidth * con.nTextHeight * 2, &dw, NULL);
        WriteFile(ahFile, con.pConAttr, con.nTextWidth * con.nTextHeight * 2, &dw, NULL);
    }
}

BOOL CRealConsole::SetConsoleSize(COORD size, DWORD anCmdID/*=CECMD_SETSIZE*/)
{
    if (!this) return FALSE;
    //_ASSERTE(hConWnd && ms_ConEmuC_Pipe[0]);

    if (!hConWnd || ms_ConEmuC_Pipe[0] == 0) {
        // 19.06.2009 Maks - ��� ������������� ����� ���� ��� �� �������
        //Box(_T("Console was not created (CRealConsole::SetConsoleSize)"));
		DEBUGSTRSIZE(L"SetConsoleSize skipped (!hConWnd)\n");
        return false; // ������� ���� �� �������?
    }
    
    _ASSERTE(size.X>=MIN_CON_WIDTH && size.Y>=MIN_CON_HEIGHT);
    
    if (size.X</*4*/MIN_CON_WIDTH)
        size.X = /*4*/MIN_CON_WIDTH;
    if (size.Y</*3*/MIN_CON_HEIGHT)
        size.Y = /*3*/MIN_CON_HEIGHT;

    bool lbRc = false;
    BOOL fSuccess = FALSE;
    DWORD dwRead = 0;
    int nInSize = sizeof(CESERVER_REQ_HDR)+sizeof(USHORT)+sizeof(COORD)+sizeof(SHORT)+sizeof(SMALL_RECT);
    CESERVER_REQ *pIn = (CESERVER_REQ*)calloc(nInSize,1);
    _ASSERTE(pIn);
    if (!pIn) return false;
    pIn->hdr.nSize = nInSize;
    int nOutSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_RETSIZE);
    CESERVER_REQ *pOut = (CESERVER_REQ*)calloc(nOutSize,1);
    _ASSERTE(pOut);
    if (!pOut) { free(pIn); return false; }
    pOut->hdr.nSize = nOutSize;
    SMALL_RECT rect = {0};

    pIn->hdr.nVersion = CESERVER_REQ_VER;
    _ASSERTE(anCmdID==CECMD_SETSIZE || anCmdID==CECMD_SETSIZESYNC || anCmdID==CECMD_CMDSTARTED || anCmdID==CECMD_CMDFINISHED);
    pIn->hdr.nCmd = anCmdID;
    pIn->hdr.nSrcThreadId = GetCurrentThreadId();



    // ��� ������ BufferHeight ����� �������� ��� � ������� ������������� (����� ������ ������� ����������?)
    if (con.bBufferHeight) {
        // global flag of the first call which is:
        // *) after getting all the settings
        // *) before running the command
        static bool s_isFirstCall = true;

        // case: buffer mode: change buffer
        CONSOLE_SCREEN_BUFFER_INFO sbi = con.m_sbi;

        sbi.dwSize.X = size.X; // new
        if (s_isFirstCall)
        {
            // first call: buffer height = from settings
            s_isFirstCall = false;
            sbi.dwSize.Y = max(gSet.DefaultBufferHeight, size.Y);
        }
        else
        {
            if (sbi.dwSize.Y == sbi.srWindow.Bottom - sbi.srWindow.Top + 1)
                // no y-scroll: buffer height = new window height
                sbi.dwSize.Y = size.Y;
            else
                // y-scroll: buffer height = old buffer height
                sbi.dwSize.Y = max(sbi.dwSize.Y, size.Y);
        }

        rect.Top = sbi.srWindow.Top;
        rect.Left = sbi.srWindow.Left;
        rect.Right = rect.Left + size.X - 1;
        rect.Bottom = rect.Top + size.Y - 1;
        if (rect.Right >= sbi.dwSize.X)
        {
            int shift = sbi.dwSize.X - 1 - rect.Right;
            rect.Left += shift;
            rect.Right += shift;
        }
        if (rect.Bottom >= sbi.dwSize.Y)
        {
            int shift = sbi.dwSize.Y - 1 - rect.Bottom;
            rect.Top += shift;
            rect.Bottom += shift;
        }

        // � size �������� ������� �������
        size.Y = TextHeight();
    }

    _ASSERTE(size.Y>0 && size.Y<200);

    // ������������� ��������� ��� �������� � ConEmuC
    //*((USHORT*)pIn->Data) = con.bBufferHeight ? gSet.DefaultBufferHeight : 0;
    //memmove(pIn->Data + sizeof(USHORT), &size, sizeof(COORD));
    //SHORT nSendTopLine = (gSet.AutoScroll || !con.bBufferHeight) ? -1 : con.nTopVisibleLine;
    //memmove(pIn->Data + sizeof(USHORT)+sizeof(COORD), &nSendTopLine, sizeof(SHORT));
    //// ������� �������������
    //memmove(pIn->Data + sizeof(USHORT) + sizeof(COORD) + sizeof(SHORT), &rect, sizeof(rect));
    pIn->SetSize.nBufferHeight = con.bBufferHeight ? gSet.DefaultBufferHeight : 0;
    pIn->SetSize.size = size;
    pIn->SetSize.nSendTopLine = (gSet.AutoScroll || !con.bBufferHeight) ? -1 : con.nTopVisibleLine;
    pIn->SetSize.rcWindow = rect;

	DEBUGSTRSIZE(L"SetConsoleSize.CallNamedPipe\n");
    fSuccess = CallNamedPipe(ms_ConEmuC_Pipe, pIn, pIn->hdr.nSize, pOut, pOut->hdr.nSize, &dwRead, 500);

    if (fSuccess && dwRead>=(DWORD)nOutSize) {
		DEBUGSTRSIZE(L"SetConsoleSize.fSuccess == TRUE\n");
        if (pOut->hdr.nCmd == pIn->hdr.nCmd) {
            con.nPacketIdx = pOut->SetSizeRet.nNextPacketId;
            CONSOLE_SCREEN_BUFFER_INFO sbi = {{0,0}};
            memmove(&sbi, &pOut->SetSizeRet.SetSizeRet, sizeof(sbi));
            if (con.bBufferHeight) {
                _ASSERTE((sbi.srWindow.Bottom-sbi.srWindow.Top)<200);
                if (sbi.dwSize.X == size.X && sbi.dwSize.Y == pIn->SetSize.nBufferHeight)
                    lbRc = TRUE;
            } else {
                if (sbi.dwSize.Y > 200) {
                    _ASSERTE(sbi.dwSize.Y<200);
                }
                if (sbi.dwSize.X == size.X && sbi.dwSize.Y == size.Y)
                    lbRc = TRUE;
            }
            //if (sbi.dwSize.X == size.X && sbi.dwSize.Y == size.Y) {
            //    con.m_sbi = sbi;
            //    if (sbi.dwSize.X == con.m_sbi.dwSize.X && sbi.dwSize.Y == con.m_sbi.dwSize.Y) {
            //        SetEvent(mh_ConChanged);
            //    }
            //    lbRc = true;
            //}
            if (lbRc) { // ��������� ����� ������ nTextWidth/nTextHeight. ����� ������������� �������� ������� ������...
				DEBUGSTRSIZE(L"SetConsoleSize.lbRc == TRUE\n");
                con.nChange2TextWidth = sbi.dwSize.X;
                con.nChange2TextHeight = con.bBufferHeight ? (sbi.srWindow.Bottom-sbi.srWindow.Top+1) : sbi.dwSize.Y;
                if (con.nChange2TextHeight > 200) {
                    _ASSERTE(con.nChange2TextHeight<=200);
                }
            }
        }
    }

    if (lbRc && isActive()) {
        // update size info
        //if (!gSet.isFullScreen && !IsZoomed(ghWnd) && !IsIconic(ghWnd))
        if (gConEmu.isWindowNormal())
        {
            gSet.UpdateSize(size.X, size.Y);
        }
    }

	DEBUGSTRSIZE(L"SetConsoleSize.finalizing\n");
	if (anCmdID == CECMD_SETSIZESYNC) {
		// ConEmuC ������ ��� ������� ���������
		if (con.nTextWidth == size.X && con.nTextHeight == size.Y) {
			DEBUGSTRSIZE(L"SetConsoleSize.RetrieveConsoleInfo is not needed, size already match\n");
		} else {
			DEBUGSTRSIZE(L"SetConsoleSize.RetrieveConsoleInfo\n");
			if (!RetrieveConsoleInfo(size.Y)) {
				DEBUGSTRSIZE(L"SetConsoleSize.RetrieveConsoleInfo - height failed, waiting\n");
				Sleep(300);
				// ����, ���� mn_MonitorThreadID ������� ���������
				DWORD dwStartTick = GetTickCount();
				DWORD nTimeout = 2000, nSteps = 0;
				#ifdef _DEBUG
				nTimeout = 2000;
				#endif
				while (!RetrieveConsoleInfo(size.Y)) {
					DEBUGSTRSIZE(L"SetConsoleSize.RetrieveConsoleInfo - height failed, waiting\n");
					nSteps ++;
					Sleep(300);
					if ((GetTickCount() - dwStartTick) > nTimeout)
						break;
				}
				#ifdef _DEBUG
				nTimeout = 2000;
				#endif
			} else {
				DEBUGSTRSIZE(L"SetConsoleSize.RetrieveConsoleInfo - height ok, exiting\n");
			}
		}
		//if (GetCurrentThreadId() != mn_MonitorThreadID) {
		//	// ����, ���� mn_MonitorThreadID ������� ���������
		//	DWORD dwStartTick = GetTickCount();
		//	DWORD nTimeout = 300;
		//	#ifdef _DEBUG
		//	nTimeout = 300;
		//	#endif
		//	while (size.X != con.nTextWidth || size.Y != con.nTextHeight) {
		//		Sleep(10);
		//		if ((GetTickCount() - dwStartTick) > nTimeout)
		//			break;
		//	}
		//}
	}

    return lbRc;
}

// �������� ������ ������� �� ������� ���� (��������)
void CRealConsole::SyncConsole2Window()
{
    if (!this)
        return;

    //2009-06-17 ��������� ���. ����� ������� � �������� ������ ������������� �� ������
    /*
    if (GetCurrentThreadId() != mn_MonitorThreadID) {
        RECT rcClient; GetClientRect(ghWnd, &rcClient);
        _ASSERTE(rcClient.right>250 && rcClient.bottom>200);

        // ��������� ������ ������ �������
        RECT newCon = gConEmu.CalcRect(CER_CONSOLE, rcClient, CER_MAINCLIENT);

        if (newCon.right==TextWidth() && newCon.bottom==TextHeight())
            return; // ������� �� ��������

        SetEvent(mh_Sync2WindowEvent);
        return;
    }
    */

    DEBUGLOGFILE("SyncConsoleToWindow\n");

    RECT rcClient; GetClientRect(ghWnd, &rcClient);
    //_ASSERTE(rcClient.right>140 && rcClient.bottom>100);

    // ��������� ������ ������ �������
    RECT newCon = gConEmu.CalcRect(CER_CONSOLE, rcClient, CER_MAINCLIENT);
    _ASSERTE(newCon.right>20 && newCon.bottom>6);

    SetConsoleSize(MakeCoord(newCon.right, newCon.bottom));
}

BOOL CRealConsole::AttachConemuC(HWND ahConWnd, DWORD anConemuC_PID)
{
    DWORD dwErr = 0;
    HANDLE hProcess = NULL;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|SYNCHRONIZE, FALSE, anConemuC_PID);
    if (!hProcess) {
        DisplayLastError(L"Can't open ConEmuC process! Attach is impossible!", dwErr = GetLastError());
        return FALSE;
    }

    //// ������� "���������" ������� //2009-05-14 ������ ������� �������������� � GUI, �� ������ �� ������� ����� ��������� ������� �������
    //wsprintfW(ms_ConEmuC_Pipe, CE_CURSORUPDATE, mn_ConEmuC_PID);
    //mh_CursorChanged = CreateEvent ( NULL, FALSE, FALSE, ms_ConEmuC_Pipe );
    //if (!mh_CursorChanged) {
    //    ms_ConEmuC_Pipe[0] = 0;
    //    DisplayLastError(L"Can't create event!");
    //    return FALSE;
    //}

    SetHwnd(ahConWnd);
    ProcessUpdate(&anConemuC_PID, 1);

    mh_ConEmuC = hProcess;
    mn_ConEmuC_PID = anConemuC_PID;

    CreateLogFiles();

    // ��� ����� ��� ���������� ConEmuC
    wsprintfW(ms_ConEmuC_Pipe, CESERVERPIPENAME, L".", mn_ConEmuC_PID);
    wsprintfW(ms_ConEmuCInput_Pipe, CESERVERINPUTNAME, L".", mn_ConEmuC_PID);
    MCHKHEAP

    //SetConsoleSize(MakeCoord(TextWidth,TextHeight));
    RECT rcWnd; GetClientRect(ghWnd, &rcWnd);
    RECT rcCon = gConEmu.CalcRect(CER_CONSOLE, rcWnd, CER_MAINCLIENT);
    SetConsoleSize(MakeCoord(rcCon.right,rcCon.bottom));

    // ����������� ���� MonitorThread
    SetEvent(mh_MonitorThreadEvent);

    return TRUE;
}

BOOL CRealConsole::AttachPID(DWORD dwPID)
{
    TODO("AttachPID ���� �������� �� �����");
    return FALSE;

#ifdef ALLOW_ATTACHPID
    #ifdef MSGLOGGER
        TCHAR szMsg[100]; wsprintf(szMsg, _T("Attach to process %i"), (int)dwPID);
        DEBUGSTRPROC(szMsg);
    #endif
    BOOL lbRc = AttachConsole(dwPID);
    if (!lbRc) {
        DEBUGSTRPROC(_T(" - failed\n"));
        BOOL lbFailed = TRUE;
        DWORD dwErr = GetLastError();
        if (/*dwErr==0x1F || dwErr==6 &&*/ dwPID == -1)
        {
            // ���� ConEmu ����������� �� FAR'� �������� - �� ������������ ������� - CMD.EXE, � �� ��� ������ ����� ������. �� ���� ����������� �� �������
            HWND hConsole = FindWindowEx(NULL,NULL,_T("ConsoleWindowClass"),NULL);
            if (hConsole && IsWindowVisible(hConsole)) {
                DWORD dwCurPID = 0;
                if (GetWindowThreadProcessId(hConsole,  &dwCurPID)) {
                    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,dwCurPID);
                    dwErr = GetLastError();
                    if (AttachConsole(dwCurPID))
                        lbFailed = FALSE;
                    else
                        dwErr = GetLastError();
                }
            }
        }
        if (lbFailed) {
            TCHAR szErr[255];
            wsprintf(szErr, _T("AttachConsole failed (PID=%i)!"), dwPID);
            DisplayLastError(szErr, dwErr);
            return FALSE;
        }
    }
    DEBUGSTRPROC(_T(" - OK"));

    TODO("InitHandler � GUI �������� ��� � �� �����...");
    //InitHandlers(FALSE);

    // ���������� ������� ������ ��� ��������� ������.
    CConEmuPipe pipe;
    
    //DEBUGSTRPROC(_T("CheckProcesses\n"));
    //gConEmu.CheckProcesses(0,TRUE);
    
    if (pipe.Init(_T("DefFont.in.attach"), TRUE))
        pipe.Execute(CMD_DEFFONT);

    return TRUE;
#endif
}

BOOL CRealConsole::FlushInputQueue(DWORD nTimeout /*= 500*/)
{
	if (!this) return FALSE;
	
	if (nTimeout > 1000) nTimeout = 1000;
	DWORD dwStartTick = GetTickCount();
	
	mn_FlushOut = mn_FlushIn;
	mn_FlushIn++;
	
	TODO("�������� ��������� ���� � �� ����������");
	PostThreadMessage(mn_InputThreadID, INPUT_THREAD_ALIVE_MSG, mn_FlushIn, 0);
	
	while (mn_FlushOut != mn_FlushIn) {
		WaitForSingleObject(mh_InputThread, 100);
		
		DWORD dwCurTick = GetTickCount();
		DWORD dwDelta = dwCurTick - dwStartTick;
		if (dwDelta > nTimeout) break;
	}
	
	return (mn_FlushOut == mn_FlushIn);
}

void CRealConsole::PostConsoleEvent(INPUT_RECORD* piRec)
{
	if (!this) return;
	_ASSERTE(mn_InputThreadID!=0);
	
    if (piRec->EventType == MOUSE_EVENT) {
        if (piRec->Event.MouseEvent.dwEventFlags == MOUSE_MOVED) {
            if (m_LastMouse.dwEventFlags != 0
             && m_LastMouse.dwButtonState     == piRec->Event.MouseEvent.dwButtonState 
             && m_LastMouse.dwControlKeyState == piRec->Event.MouseEvent.dwControlKeyState
             && m_LastMouse.dwMousePosition.X == piRec->Event.MouseEvent.dwMousePosition.X
             && m_LastMouse.dwMousePosition.Y == piRec->Event.MouseEvent.dwMousePosition.Y)
            {
                //#ifdef _DEBUG
                //wchar_t szDbg[60];
                //wsprintf(szDbg, L"!!! Skipping ConEmu.Mouse event at: {%ix%i}\n", m_LastMouse.dwMousePosition.X, m_LastMouse.dwMousePosition.Y);
                //DEBUGSTRINPUT(szDbg);
                //#endif
                return; // ��� ������� ������. �������� ����� ������� �� ����, ������ �� ��������
            }
        }
        // ��������
        m_LastMouse.dwMousePosition   = piRec->Event.MouseEvent.dwMousePosition;
        m_LastMouse.dwEventFlags      = piRec->Event.MouseEvent.dwEventFlags;
        m_LastMouse.dwButtonState     = piRec->Event.MouseEvent.dwButtonState;
        m_LastMouse.dwControlKeyState = piRec->Event.MouseEvent.dwControlKeyState;

        //#ifdef _DEBUG
        //wchar_t szDbg[60];
        //wsprintf(szDbg, L"ConEmu.Mouse event at: {%ix%i}\n", m_LastMouse.dwMousePosition.X, m_LastMouse.dwMousePosition.Y);
        //DEBUGSTRINPUT(szDbg);
        //#endif
    }
    

    UINT nMsg = 0; WPARAM wParam = 0; LPARAM lParam = 0;
    if (piRec->EventType == KEY_EVENT) {
    	nMsg = piRec->Event.KeyEvent.bKeyDown ? WM_KEYDOWN : WM_KEYUP;
    	
		lParam |= (WORD)piRec->Event.KeyEvent.uChar.UnicodeChar;
		lParam |= ((BYTE)piRec->Event.KeyEvent.wVirtualKeyCode) << 16;
		lParam |= ((BYTE)piRec->Event.KeyEvent.wVirtualScanCode) << 24;
		
        wParam |= (WORD)piRec->Event.KeyEvent.dwControlKeyState;
        wParam |= ((DWORD)piRec->Event.KeyEvent.wRepeatCount & 0xFF) << 16;
    
    } else if (piRec->EventType == MOUSE_EVENT) {
		switch (piRec->Event.MouseEvent.dwEventFlags) {
			case MOUSE_MOVED:
				nMsg = MOUSE_EVENT_MOVE;
				break;
			case 0:
				nMsg = MOUSE_EVENT_CLICK;
				break;
			case DOUBLE_CLICK:
				nMsg = MOUSE_EVENT_DBLCLICK;
				break;
			case MOUSE_WHEELED:
				DEBUGSTRINPUT(L"MOUSE_WHEELED.Posted\n");
				nMsg = MOUSE_EVENT_WHEELED;
				break;
			case /*MOUSE_HWHEELED*/ 0x0008:
				nMsg = MOUSE_EVENT_HWHEELED;
				break;
			default:
				_ASSERT(FALSE);
		}
		
    	lParam = ((WORD)piRec->Event.MouseEvent.dwMousePosition.X)
    	       | (((DWORD)(WORD)piRec->Event.MouseEvent.dwMousePosition.Y) << 16);
		
		// max 0x0010/*FROM_LEFT_4ND_BUTTON_PRESSED*/
		wParam |= ((DWORD)piRec->Event.MouseEvent.dwButtonState) & 0xFF;
		
		// max - ENHANCED_KEY == 0x0100
		wParam |= (((DWORD)piRec->Event.MouseEvent.dwControlKeyState) & 0xFFFF) << 8;
		
		if (nMsg == MOUSE_EVENT_WHEELED || nMsg == MOUSE_EVENT_HWHEELED) {
    		// HIWORD() - short (direction[1/-1])*count*120
    		short nWheel = (short)((((DWORD)piRec->Event.MouseEvent.dwButtonState) & 0xFFFF0000) >> 16);
    		char  nCount = nWheel / 120;
    		wParam |= ((DWORD)nCount) << 24;
		}
		
    
    } else if (piRec->EventType == FOCUS_EVENT) {
    	nMsg = piRec->Event.FocusEvent.bSetFocus ? WM_SETFOCUS : WM_KILLFOCUS;
    	
    } else {
    	_ASSERT(FALSE);
    }
    _ASSERTE(nMsg!=0);
    
    TODO("�������� ��������� ���� � �� ���������� ��� �������������");
    PostThreadMessage(mn_InputThreadID, nMsg, wParam, lParam);
}

DWORD CRealConsole::InputThread(LPVOID lpParameter)
{
    CRealConsole* pRCon = (CRealConsole*)lpParameter;
    
    MSG msg;
    while (GetMessage(&msg,0,0,0)) {
    	if (msg.message == WM_QUIT) break;
    	if (WaitForSingleObject(pRCon->mh_TermEvent, 0) == WAIT_OBJECT_0) break;

    	if (msg.message == INPUT_THREAD_ALIVE_MSG) {
    		pRCon->mn_FlushOut = msg.wParam;
    		continue;
    	
    	} else if (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) {
    		INPUT_RECORD r = {KEY_EVENT};
    		
    		// lParam
            r.Event.KeyEvent.bKeyDown = (msg.message == WM_KEYDOWN);
            r.Event.KeyEvent.uChar.UnicodeChar = (WCHAR)(msg.lParam & 0xFFFF);
            r.Event.KeyEvent.wVirtualKeyCode   = (((DWORD)msg.lParam) & 0xFF0000) >> 16;
            r.Event.KeyEvent.wVirtualScanCode  = (((DWORD)msg.lParam) & 0xFF000000) >> 24;
            
            // wParam. ���� ��� ��� ����� ���� max(ENHANCED_KEY==0x0100)
            r.Event.KeyEvent.dwControlKeyState = ((DWORD)msg.wParam & 0xFFFF);
            
            r.Event.KeyEvent.wRepeatCount = ((DWORD)msg.wParam & 0xFF0000) >> 16;
            
            pRCon->SendConsoleEvent(&r);
            
    	} else if (msg.message >= MOUSE_EVENT_FIRST && msg.message <= MOUSE_EVENT_LAST) {
    		INPUT_RECORD r = {MOUSE_EVENT};
    		
    		switch (msg.message) {
    			case MOUSE_EVENT_MOVE:
    				r.Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
    				break;
    			case MOUSE_EVENT_CLICK:
    				r.Event.MouseEvent.dwEventFlags = 0;
    				break;
    			case MOUSE_EVENT_DBLCLICK:
    				r.Event.MouseEvent.dwEventFlags = DOUBLE_CLICK;
    				break;
    			case MOUSE_EVENT_WHEELED:
    				r.Event.MouseEvent.dwEventFlags = MOUSE_WHEELED;
					DEBUGSTRINPUT(L"MOUSE_WHEELED.Sending\n");
    				break;
    			case MOUSE_EVENT_HWHEELED:
    				r.Event.MouseEvent.dwEventFlags = /*MOUSE_HWHEELED*/ 0x0008;
    				break;
    		}
    		
    		r.Event.MouseEvent.dwMousePosition.X = LOWORD(msg.lParam);
    		r.Event.MouseEvent.dwMousePosition.Y = HIWORD(msg.lParam);
    		
    		// max 0x0010/*FROM_LEFT_4ND_BUTTON_PRESSED*/
    		r.Event.MouseEvent.dwButtonState = ((DWORD)msg.wParam) & 0xFF;
    		
    		// max - ENHANCED_KEY == 0x0100
    		r.Event.MouseEvent.dwControlKeyState = (((DWORD)msg.wParam) & 0xFFFF00) >> 8;
    		
    		if (msg.message == MOUSE_EVENT_WHEELED || msg.message == MOUSE_EVENT_HWHEELED) {
	    		// HIWORD() - short (direction[1/-1])*count*120
	    		short nDir = (/*signed*/ char)((((DWORD)msg.wParam) & 0xFF000000) >> 24);
	    		r.Event.MouseEvent.dwButtonState |= ((WORD)(nDir*120)) << 16;
			}
			
			pRCon->SendConsoleEvent(&r);
			
    	} else if (msg.message == WM_SETFOCUS || msg.message == WM_KILLFOCUS) {
	        INPUT_RECORD r = {FOCUS_EVENT};
	        
	        r.Event.FocusEvent.bSetFocus = (msg.message == WM_SETFOCUS);
	        
	        pRCon->SendConsoleEvent(&r);
	        
    	}
    }
    
    return 0;
}

DWORD CRealConsole::MonitorThread(LPVOID lpParameter)
{
    CRealConsole* pRCon = (CRealConsole*)lpParameter;

    if (pRCon->mb_NeedStartProcess) {
        _ASSERTE(pRCon->mh_ConEmuC==NULL);
        pRCon->mb_NeedStartProcess = FALSE;
        if (!pRCon->StartProcess()) {
            DEBUGSTRPROC(L"### Can't start process\n");
            return 0;
        }
    }
    
    //_ASSERTE(pRCon->mh_ConChanged!=NULL);
    // ���� ���� ����������� - ��������� "�����" ��� ��� ��� ���������...
    //_ASSERTE(pRCon->mb_Detached || pRCon->mh_ConEmuC!=NULL);

    BOOL bDetached = pRCon->m_Args.bDetached;
    //TODO: � ��� �� ����� ������ �������...
    #define IDEVENT_TERM  0
    #define IDEVENV_MONITORTHREADEVENT 1
    //#define IDEVENT_SYNC2WINDOW 2
    //#define IDEVENT_CURSORCHANGED 4
    #define IDEVENT_PACKETARRIVED 2
    #define IDEVENT_CONCLOSED 3
    #define EVENTS_COUNT (IDEVENT_CONCLOSED+1)
    HANDLE hEvents[EVENTS_COUNT];
    hEvents[IDEVENT_TERM] = pRCon->mh_TermEvent;
    //hEvents[IDEVENT_CONCHANGED] = pRCon->mh_ConChanged;
    hEvents[IDEVENV_MONITORTHREADEVENT] = pRCon->mh_MonitorThreadEvent; // ������������, ����� ������� Update & Invalidate
    //hEvents[IDEVENT_SYNC2WINDOW] = pRCon->mh_Sync2WindowEvent;
    //hEvents[IDEVENT_CURSORCHANGED] = pRCon->mh_CursorChanged;
    hEvents[IDEVENT_PACKETARRIVED] = pRCon->mh_PacketArrived;
    hEvents[IDEVENT_CONCLOSED] = pRCon->mh_ConEmuC;
    DWORD  nEvents = /*bDetached ? (IDEVENT_SYNC2WINDOW+1) :*/ countof(hEvents);
    if (hEvents[IDEVENT_CONCLOSED] == NULL) nEvents --;
    _ASSERT(EVENTS_COUNT==countof(hEvents)); // ��������� �����������

    DWORD  nWait = 0;
    BOOL   bLoop = TRUE, bIconic = FALSE, bFirst = TRUE, bActive = TRUE;

    DWORD nElapse = max(10,gSet.nMainTimerElapse);
    
    TODO("���� �� ����������� ��� F10 � ���� - �������� ���� �� ����������������...")
    while (TRUE/*bLoop*/)
    {
        gSet.Performance(tPerfInterval, TRUE); // ������ �������� ������. �� ������� �� ���������� ����� ���������

        if (hEvents[IDEVENT_CONCLOSED] == NULL && pRCon->mh_ConEmuC /*&& pRCon->mh_CursorChanged*/) {
            bDetached = FALSE;
            //hEvents[IDEVENT_CURSORCHANGED] = pRCon->mh_CursorChanged;
            hEvents[IDEVENT_CONCLOSED] = pRCon->mh_ConEmuC;
            nEvents = countof(hEvents);
        }

        
        bActive = pRCon->isActive();
        bIconic = IsIconic(ghWnd);

        // � ����������������/���������� ������ - ��������� �������
        nWait = WaitForMultipleObjects(nEvents, hEvents, FALSE, (bIconic || !bActive) ? max(1000,nElapse) : nElapse);
        _ASSERTE(nWait!=(DWORD)-1);

        if (nWait == IDEVENT_TERM /*|| !bLoop*/ || nWait == IDEVENT_CONCLOSED) {
            if (nWait == IDEVENT_CONCLOSED) {
                DEBUGSTRPROC(L"### ConEmuC.exe was terminated\n");
            }
            break; // ���������� ���������� ����
        }

        // ��������, ��� ConEmuC ���
        if (pRCon->mh_ConEmuC) {
            DWORD dwExitCode = 0;
            #ifdef _DEBUG
            BOOL fSuccess = 
            #endif
            GetExitCodeProcess(pRCon->mh_ConEmuC, &dwExitCode);
            if (dwExitCode!=STILL_ACTIVE) {
                pRCon->StopSignal();
                return 0;
            }
        }

        // ���� ������� �� ������ ���� �������� - �� �� ���-�� ��������� 
        if (!pRCon->isShowConsole && !gSet.isConVisible)
        {
            /*if (foreWnd == hConWnd)
                SetForegroundWindow(ghWnd);*/
            if (IsWindowVisible(pRCon->hConWnd))
                ShowWindow(pRCon->hConWnd, SW_HIDE);
        }

        // ������ ������� ������ � ��� �����, � ������� ��� ���������. ����� ����� ��������������� ��� Update (InitDC)
        // ��������� ��������� �������� �������
        /*if (nWait == (IDEVENT_SYNC2WINDOW)) {
            pRCon->SetConsoleSize(pRCon->m_ReqSetSize);
            //SetEvent(pRCon->mh_ReqSetSizeEnd);
            //continue; -- � ����� ������� ���������� � ���
        }*/
        

        DWORD dwT1 = GetTickCount();

        try {   
            //ResetEvent(pRCon->mh_EndUpdateEvent);
            
            
            if (!bDetached && bFirst) {
                bFirst = FALSE;
                DWORD dwAlive = 0;
				// �������� ��������� ���������� � �������
                if (!pRCon->RetrieveConsoleInfo(0)) {
                  // ������ ����� ����, ���� ConEmuC ��� ������ � ��������
                  if (pRCon->mh_ConEmuC) {
                      dwAlive = WaitForSingleObject(pRCon->mh_ConEmuC,0);
                      if (dwAlive != WAIT_OBJECT_0) {
                          _ASSERT(FALSE);
                      }
                  }
                }
            }

            if (nWait==IDEVENT_PACKETARRIVED || pRCon->isPackets()) {
                //MSectionLock cs; cs.Lock(&pRCon->csPKT,�� TRUE);
                CESERVER_REQ* pPkt = NULL;
                while ((pPkt = pRCon->PopPacket()) != NULL) {
                    pRCon->ApplyConsoleInfo(pPkt);
                    free(pPkt);
                }
            }

            //if ((nWait == IDEVENT_SYNC2WINDOW || !WaitForSingleObject(hEvents[IDEVENT_SYNC2WINDOW],0))
            //    && pRCon->hConWnd)
            //{
            //    pRCon->SyncConsole2Window();
            //}

            if (pRCon->hConWnd 
                && GetWindowText(pRCon->hConWnd, pRCon->TitleCmp, countof(pRCon->TitleCmp)-2)
                && wcscmp(pRCon->Title, pRCon->TitleCmp))
            {
                pRCon->OnTitleChanged();
            } else if (bActive) {
                if (wcscmp(pRCon->Title, gConEmu.GetTitle()))
                    gConEmu.UpdateTitle(pRCon->TitleCmp);
            }

            bool lbIsActive = pRCon->isActive();

            // �� con.m_sbi ���������, �������� �� ���������
            bool lbForceUpdate = pRCon->CheckBufferSize();
            if (lbForceUpdate && lbIsActive) // ������ �������� ����������� ���� ��� �������
                gConEmu.OnSize(-1); // ������� � ������� ���� ������ �� ���������� �������
            //if (!lbForceUpdate && (nWait == (WAIT_OBJECT_0+1)))
            //    lbForceUpdate = true;

            // 04.06.2009 Maks - ������, EndUpdateEvent �� ������ ���������!
            // 04.06.2009 Maks - ���� ������� ������� - Invalidate ������� ��� VCon ��� �������������
            if ((nWait == (WAIT_OBJECT_0+1)) || lbForceUpdate || pRCon->mb_DataChanged) {
                pRCon->mb_DataChanged = FALSE;
                if (lbIsActive) {
                    gConEmu.m_Child.Validate(); // �������� ������
                    pRCon->LogString("mp_VCon->Update from CRealConsole::MonitorThread");
                    if (pRCon->mp_VCon->Update(lbForceUpdate))
                        gConEmu.m_Child.Redraw();
                    pRCon->mn_LastInvalidateTick = GetTickCount();
                }
            } else if (lbIsActive) {
                if (gSet.isCursorBlink) {
                    // ��������, ������� ����� ������� ��������?
                    bool lbNeedBlink = false;
                    pRCon->mp_VCon->UpdateCursor(lbNeedBlink);
                    // UpdateCursor Invalidate �� �����
                    if (lbNeedBlink) {
                        pRCon->LogString("Invalidating from CRealConsole::MonitorThread.1");
                        gConEmu.m_Child.Redraw();
                        /*gConEmu.m_Child.Validate(); // �������� ������
                        gConEmu.m_Child.Invalidate();
                        UpdateWindow(ghWndDC);*/
                        pRCon->mn_LastInvalidateTick = GetTickCount();
                    }
                } else if (((GetTickCount() - pRCon->mn_LastInvalidateTick) > FORCE_INVALIDATE_TIMEOUT)) {
                    DEBUGSTRDRAW(L"+++ Force invalidate by timeout\n");
                    pRCon->LogString("Invalidating from CRealConsole::MonitorThread.2");
                    gConEmu.m_Child.Redraw();
                    /*gConEmu.m_Child.Validate(); // �������� ������
                    gConEmu.m_Child.Invalidate();
                    UpdateWindow(ghWndDC);*/ //2009-06-24. ����������, ��� ���� �� ��� - � Zeroes1 �� �������������� ���������
                    pRCon->mn_LastInvalidateTick = GetTickCount();
                }
            }

            //SetEvent(pRCon->mh_EndUpdateEvent);
        } catch(...) {
            bLoop = FALSE;
        }

        /*if (nWait == (WAIT_OBJECT_0+2)) {
            SetEvent(pRCon->mh_ReqSetSizeEnd);
        }*/

        DWORD dwT2 = GetTickCount();
        DWORD dwD = max(10,(dwT2 - dwT1));
        nElapse = (DWORD)(nElapse*0.7 + dwD*0.3);
        if (nElapse > 1000) nElapse = 1000; // ������ ������� - �� �����! ����� ������ ������ �� �����

        if (!bLoop) {
            #ifdef _DEBUG
            _ASSERT(FALSE);
            #endif
            pRCon->Box(_T("Exception triggered in CRealConsole::MonitorThread"));
            bLoop = true;
        }

        gSet.Performance(tPerfInterval, FALSE);
    }
   
    // ���������� ��������� ����� ���� �������
    DEBUGSTRPROC(L"About to terminate main server thread (MonitorThread)\n");
    if (pRCon->ms_VConServer_Pipe[0]) // ������ ���� �� ���� ���� ���� ��������
    {   
        pRCon->StopSignal(); // ��� ������ ���� ���������, �� �� ������ ������
        //
        HANDLE hPipe = INVALID_HANDLE_VALUE;
        DWORD dwWait = 0;
        // ����������� �����, ����� ���� ������� �����������
        for (int i=0; i<MAX_SERVER_THREADS; i++) {
            DEBUGSTRPROC(L"Touching our server pipe\n");
            HANDLE hServer = pRCon->mh_ActiveServerThread;
            hPipe = CreateFile(pRCon->ms_VConServer_Pipe,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
            if (hPipe == INVALID_HANDLE_VALUE) {
                DEBUGSTRPROC(L"All pipe instances closed?\n");
                break;
            }
            DEBUGSTRPROC(L"Waiting server pipe thread\n");
            dwWait = WaitForSingleObject(hServer, 200); // �������� ���������, ���� ���� ����������
            // ������ ������� ���� - ��� ����� ���� �����������
            CloseHandle(hPipe);
            hPipe = INVALID_HANDLE_VALUE;
        }
        // ������� ��������, ���� ��� ���� ����������
        DEBUGSTRPROC(L"Checking server pipe threads are closed\n");
        WaitForMultipleObjects(MAX_SERVER_THREADS, pRCon->mh_ServerThreads, TRUE, 500);
        for (int i=0; i<MAX_SERVER_THREADS; i++) {
            if (WaitForSingleObject(pRCon->mh_ServerThreads[i],0) != WAIT_OBJECT_0) {
                DEBUGSTRPROC(L"### Terminating mh_ServerThreads\n");
                TerminateThread(pRCon->mh_ServerThreads[i],0);
            }
            CloseHandle(pRCon->mh_ServerThreads[i]);
            pRCon->mh_ServerThreads[i] = NULL;
        }
    }
    
    // Finalize
    //SafeCloseHandle(pRCon->mh_MonitorThread);

    DEBUGSTRPROC(L"Leaving MonitorThread\n");
    
    return 0;
}

BOOL CRealConsole::PreInit(BOOL abCreateBuffers/*=TRUE*/)
{
    // ���������������� ���������� m_sbi, m_ci, m_sel
    RECT rcWnd; GetClientRect(ghWnd, &rcWnd);
    // isBufferHeight ������������ ������, �.�. con.m_sbi ��� �� ���������������!
    if (gSet.ForceBufferHeight && gSet.DefaultBufferHeight && !con.bBufferHeight)
        SetBufferHeightMode(TRUE);

    //if (con.bBufferHeight) {
    //  // ��������������� ������ ���� �� ������ ������������ ������ ���������
    //  if (!gConEmu.ActiveCon()->RCon()->isBufferHeight())
    //      rcWnd.right -= GetSystemMetrics(SM_CXVSCROLL);
    //} else {
    //  // // ��������������� ������ ���� �� ������ ����������� ������ ���������
    //  if (gConEmu.ActiveCon()->RCon()->isBufferHeight())
    //      rcWnd.right += GetSystemMetrics(SM_CXVSCROLL);
    //}
    RECT rcCon;
    if (IsIconic(ghWnd))
        rcCon = MakeRect(gSet.wndWidth, gSet.wndHeight);
    else
        rcCon = gConEmu.CalcRect(CER_CONSOLE, rcWnd, CER_MAINCLIENT);
    _ASSERTE(rcCon.right!=0 && rcCon.bottom!=0);
    if (con.bBufferHeight) {
        con.m_sbi.dwSize = MakeCoord(rcCon.right,gSet.DefaultBufferHeight);
    } else {
        con.m_sbi.dwSize = MakeCoord(rcCon.right,rcCon.bottom);
    }
    con.m_sbi.wAttributes = 7;
    con.m_sbi.srWindow.Right = rcCon.right-1; con.m_sbi.srWindow.Bottom = rcCon.bottom-1;
    con.m_sbi.dwMaximumWindowSize = con.m_sbi.dwSize;
    con.m_ci.dwSize = 15; con.m_ci.bVisible = TRUE;
    if (!InitBuffers(0))
        return FALSE;
    return TRUE;
}

BOOL CRealConsole::StartMonitorThread()
{
    BOOL lbRc = FALSE;

    _ASSERT(mh_MonitorThread==NULL);
    _ASSERT(mh_InputThread==NULL);
    //_ASSERTE(mb_Detached || mh_ConEmuC!=NULL); -- ������� ������ ��������� � MonitorThread

    mh_MonitorThread = CreateThread(NULL, 0, MonitorThread, (LPVOID)this, 0, &mn_MonitorThreadID);
    
    mh_InputThread = CreateThread(NULL, 0, InputThread, (LPVOID)this, 0, &mn_InputThreadID);

    if (mh_MonitorThread == NULL || mh_InputThread == NULL) {
        DisplayLastError(_T("Can't create console thread!"));
    } else {
        //lbRc = SetThreadPriority(mh_MonitorThread, THREAD_PRIORITY_ABOVE_NORMAL);
        lbRc = TRUE;
    }

    return lbRc;
}

BOOL CRealConsole::StartProcess()
{
    BOOL lbRc = FALSE;


    if (!mb_ProcessRestarted) {
        if (!PreInit())
            return FALSE;
    }

    
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        wchar_t szInitConTitle[255];

        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW|STARTF_USECOUNTCHARS/*|STARTF_USEPOSITION*/;
        si.lpTitle = wcscpy(szInitConTitle, CEC_INITTITLE);
        si.dwXCountChars = con.m_sbi.dwSize.X;
        si.dwYCountChars = con.m_sbi.dwSize.Y;
        si.wShowWindow = gSet.isConVisible ? SW_SHOWNORMAL : SW_HIDE;
        //RECT rcDC; GetWindowRect(ghWndDC, &rcDC);
        //si.dwX = rcDC.left; si.dwY = rcDC.top;
        ZeroMemory( &pi, sizeof(pi) );

        int nStep = (m_Args.pszSpecialCmd!=NULL) ? 2 : 1;
        wchar_t* psCurCmd = NULL;
        while (nStep <= 2)
        {
            MCHKHEAP
            /*if (!*gSet.GetCmd()) {
                gSet.psCurCmd = _tcsdup(gSet.Buffer Height == 0 ? _T("far") : _T("cmd"));
                nStep ++;
            }*/

            LPCWSTR lpszCmd = NULL;
            if (m_Args.pszSpecialCmd)
                lpszCmd = m_Args.pszSpecialCmd;
            else
                lpszCmd = gSet.GetCmd();

            int nLen = _tcslen(lpszCmd);
            TCHAR *pszSlash=NULL;
            nLen += _tcslen(gConEmu.ms_ConEmuExe) + 256 + MAX_PATH;
            psCurCmd = (wchar_t*)malloc(nLen*sizeof(wchar_t));
            _ASSERTE(psCurCmd);
            wcscpy(psCurCmd, L"\"");
            wcscat(psCurCmd, gConEmu.ms_ConEmuExe);
            pszSlash = wcsrchr(psCurCmd, _T('\\'));
            MCHKHEAP
            wcscpy(pszSlash+1, L"ConEmuC.exe\" ");

			if (m_Args.bRunAsAdministrator) {
				m_Args.bDetached = TRUE;
				wcscat(pszSlash, L" /ATTACH ");
			}

            int nWndWidth = con.m_sbi.dwSize.X;
            int nWndHeight = con.m_sbi.dwSize.Y;
            GetConWindowSize(con.m_sbi, nWndWidth, nWndHeight);
            wsprintf(psCurCmd+wcslen(psCurCmd), L"/BW=%i /BH=%i /BZ=%i \"/FN=%s\" /FW=%i /FH=%i", 
                nWndWidth, nWndHeight,
                (con.bBufferHeight ? gSet.DefaultBufferHeight : 0),
                gSet.ConsoleFont.lfFaceName, gSet.ConsoleFont.lfWidth, gSet.ConsoleFont.lfHeight);
            /*if (gSet.FontFile[0]) { --  ����������� ������ �� ������� �� ��������!
                wcscat(psCurCmd, L" \"/FF=");
                wcscat(psCurCmd, gSet.FontFile);
                wcscat(psCurCmd, L"\"");
            }*/
            if (gSet.isAdvLogging) wcscat(psCurCmd, L" /LOG");
            wcscat(psCurCmd, L" /CMD ");
            wcscat(psCurCmd, lpszCmd);
            MCHKHEAP

			DWORD dwLastError = 0;

            #ifdef MSGLOGGER
            DEBUGSTRPROC(psCurCmd);DEBUGSTRPROC(_T("\n"));
            #endif
            
            //try {
				if (!m_Args.bRunAsAdministrator) {
					LockSetForegroundWindow ( LSFW_LOCK );

					lbRc = CreateProcess(NULL, psCurCmd, NULL, NULL, FALSE, 
						NORMAL_PRIORITY_CLASS|
						CREATE_DEFAULT_ERROR_MODE|CREATE_NEW_CONSOLE
						//|CREATE_NEW_PROCESS_GROUP - ����! ��������� ����������� Ctrl-C
						, NULL, NULL, &si, &pi);
					if (!lbRc)
						dwLastError = GetLastError();
					DEBUGSTRPROC(_T("CreateProcess finished\n"));

					LockSetForegroundWindow ( LSFW_UNLOCK );

				} else {
					LPCWSTR pszCmd = psCurCmd;
					wchar_t szExec[MAX_PATH+1];
					if (NextArg(&pszCmd, szExec) != 0) {
						lbRc = FALSE;
						dwLastError = -1;
					} else {
						// ������-�� �������.
                        // ����������� GlobalAlloc �� ������ (����� ��� ���� �� ��������...) ������� �����
                        // ���� �� ��������: CreateProcessAsUser with an unrestricted administrator token
                        // http://weblogs.asp.net/kennykerr/archive/2006/09/29/Windows-Vista-for-Developers-_1320_-Part-4-_1320_-User-Account-Control.aspx

						SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
						sei.hwnd = NULL; //ghWnd;
						sei.fMask = 0;//SEE_MASK_NO_CONSOLE; //SEE_MASK_NOCLOSEPROCESS; -- ������ ����� ���������� ��� - ������� ����������� � ����� �������
						wchar_t szVerb[10];
						sei.lpVerb = wcscpy(szVerb, L"runas");
						sei.lpFile = szExec;
						sei.lpParameters = pszCmd;
						sei.nShow = SW_SHOWNORMAL;
						lbRc = gConEmu.GuiShellExecuteEx(&sei);
						//lbRc = 32 < (int)::ShellExecute(0, // owner window
						//	L"runas",
						//	L"C:\\Windows\\Notepad.exe",
						//	0, // params
						//	0, // directory
						//	SW_SHOWNORMAL);
						// ������ ������� ������
						dwLastError = GetLastError();
					}
				}
            //} catch(...) {
            //    lbRc = FALSE;
            //}

            

            if (lbRc)
            {
				if (!m_Args.bRunAsAdministrator) {
					ProcessUpdate(&pi.dwProcessId, 1);

					AllowSetForegroundWindow(pi.dwProcessId);
				}
                SetForegroundWindow(ghWnd);

                DEBUGSTRPROC(_T("CreateProcess OK\n"));
                lbRc = TRUE;

                /*if (!AttachPID(pi.dwProcessId)) {
                    DEBUGSTRPROC(_T("AttachPID failed\n"));
                    return FALSE;
                }
                DEBUGSTRPROC(_T("AttachPID OK\n"));*/

                break; // OK, ���������
            } else {
                //Box("Cannot execute the command.");
                //DWORD dwLastError = GetLastError();
                DEBUGSTRPROC(_T("CreateProcess failed\n"));
                int nLen = _tcslen(psCurCmd);
                TCHAR* pszErr=(TCHAR*)Alloc(nLen+100,sizeof(TCHAR));
                
                if (0==FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, dwLastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                    pszErr, 1024, NULL))
                {
                    wsprintf(pszErr, _T("Unknown system error: 0x%x"), dwLastError);
                }
                
                nLen += _tcslen(pszErr);
                TCHAR* psz=(TCHAR*)Alloc(nLen+100,sizeof(TCHAR));
                int nButtons = MB_OK|MB_ICONEXCLAMATION|MB_SETFOREGROUND;
                
                _tcscpy(psz, _T("Cannot execute the command.\r\n"));
                _tcscat(psz, psCurCmd); _tcscat(psz, _T("\r\n"));
                _tcscat(psz, pszErr);
                if (m_Args.pszSpecialCmd == NULL)
                {
                    if (psz[_tcslen(psz)-1]!=_T('\n')) _tcscat(psz, _T("\r\n"));
                    if (!gSet.psCurCmd && StrStrI(gSet.GetCmd(), gSet.GetDefaultCmd())==NULL) {
                        _tcscat(psz, _T("\r\n\r\n"));
                        _tcscat(psz, _T("Do You want to simply start "));
                        _tcscat(psz, gSet.GetDefaultCmd());
                        _tcscat(psz, _T("?"));
                        nButtons |= MB_YESNO;
                    }
                }
                MCHKHEAP
                //Box(psz);
                int nBrc = MessageBox(NULL, psz, _T("ConEmu"), nButtons);
                Free(psz); Free(pszErr);
                if (nBrc!=IDYES) {
                    // ??? ����� ���� ���� ��������� ��������. ������ ��� ��������� �������� ����!
                    //gConEmu.Destroy();
					CloseConsole();
                    return FALSE;
                }
                // ��������� ����������� �������...
                if (m_Args.pszSpecialCmd == NULL)
                {
                    gSet.psCurCmd = _tcsdup(gSet.GetDefaultCmd());
                }
                nStep ++;
                MCHKHEAP
                if (psCurCmd) free(psCurCmd); psCurCmd = NULL;
            }
        }

        MCHKHEAP
        if (psCurCmd) free(psCurCmd); psCurCmd = NULL;
        MCHKHEAP

        //TODO: � ������ �� ���?
        SafeCloseHandle(pi.hThread); pi.hThread = NULL;
        //CloseHandle(pi.hProcess); pi.hProcess = NULL;
        mn_ConEmuC_PID = pi.dwProcessId;
        mh_ConEmuC = pi.hProcess; pi.hProcess = NULL;

		if (!m_Args.bRunAsAdministrator) {
			CreateLogFiles();
        
			//// ������� "���������" ������� //2009-05-14 ������ ������� �������������� � GUI, �� ������ �� ������� ����� ��������� ������� �������
			//wsprintfW(ms_ConEmuC_Pipe, CE_CURSORUPDATE, mn_ConEmuC_PID);
			//mh_CursorChanged = CreateEvent ( NULL, FALSE, FALSE, ms_ConEmuC_Pipe );
	        
			// ��� ����� ��� ���������� ConEmuC
			wsprintfW(ms_ConEmuC_Pipe, CESERVERPIPENAME, L".", mn_ConEmuC_PID);
			wsprintfW(ms_ConEmuCInput_Pipe, CESERVERINPUTNAME, L".", mn_ConEmuC_PID);
			MCHKHEAP
		}

    return lbRc;
}



void CRealConsole::OnMouse(UINT messg, WPARAM wParam, int x, int y)
{
    #ifndef WM_MOUSEHWHEEL
    #define WM_MOUSEHWHEEL                  0x020E
    #endif

    if (!this || !hConWnd)
        return;

    //BOOL lbStdMode = FALSE;
    //if (!con.bBufferHeight)
    //    lbStdMode = TRUE;
    if (con.bBufferHeight) {
        TODO("������ ��������� ���������, �� ������� ���������� �� �������...");
        if (messg == WM_MOUSEWHEEL) {
            SHORT nDir = (SHORT)HIWORD(wParam);
            if (nDir > 0) {
                OnScroll(SB_LINEUP);
                //OnScroll(SB_PAGEUP);
            } else if (nDir < 0) {
                OnScroll(SB_LINEDOWN);
                //OnScroll(SB_PAGEDOWN);
            }
        }
        return;
    }

    //if (lbStdMode)
    {
        INPUT_RECORD r; memset(&r, 0, sizeof(r));
        r.EventType = MOUSE_EVENT;
        
        // Mouse Buttons
        if (messg != WM_LBUTTONUP && (messg == WM_LBUTTONDOWN || messg == WM_LBUTTONDBLCLK || isPressed(VK_LBUTTON)))
            r.Event.MouseEvent.dwButtonState |= FROM_LEFT_1ST_BUTTON_PRESSED;
        if (messg != WM_RBUTTONUP && (messg == WM_RBUTTONDOWN || messg == WM_RBUTTONDBLCLK || isPressed(VK_RBUTTON)))
            r.Event.MouseEvent.dwButtonState |= RIGHTMOST_BUTTON_PRESSED;
        if (messg != WM_MBUTTONUP && (messg == WM_MBUTTONDOWN || messg == WM_MBUTTONDBLCLK || isPressed(VK_MBUTTON)))
            r.Event.MouseEvent.dwButtonState |= FROM_LEFT_2ND_BUTTON_PRESSED;
        if (messg != WM_XBUTTONUP && (messg == WM_XBUTTONDOWN || messg == WM_XBUTTONDBLCLK)) {
            if ((HIWORD(wParam) & 0x0001/*XBUTTON1*/))
                r.Event.MouseEvent.dwButtonState |= 0x0008/*FROM_LEFT_3ND_BUTTON_PRESSED*/;
            else if ((HIWORD(wParam) & 0x0002/*XBUTTON2*/))
                r.Event.MouseEvent.dwButtonState |= 0x0010/*FROM_LEFT_4ND_BUTTON_PRESSED*/;
        }

        // Key modifiers
        if (GetKeyState(VK_CAPITAL) & 1)
            r.Event.MouseEvent.dwControlKeyState |= CAPSLOCK_ON;
        if (GetKeyState(VK_NUMLOCK) & 1)
            r.Event.MouseEvent.dwControlKeyState |= NUMLOCK_ON;
        if (GetKeyState(VK_SCROLL) & 1)
            r.Event.MouseEvent.dwControlKeyState |= SCROLLLOCK_ON;
        if (isPressed(VK_LMENU))
            r.Event.MouseEvent.dwControlKeyState |= LEFT_ALT_PRESSED;
        if (isPressed(VK_RMENU))
            r.Event.MouseEvent.dwControlKeyState |= RIGHT_ALT_PRESSED;
        if (isPressed(VK_LCONTROL))
            r.Event.MouseEvent.dwControlKeyState |= LEFT_CTRL_PRESSED;
        if (isPressed(VK_RCONTROL))
            r.Event.MouseEvent.dwControlKeyState |= RIGHT_CTRL_PRESSED;
        if (isPressed(VK_SHIFT))
            r.Event.MouseEvent.dwControlKeyState |= SHIFT_PRESSED;

        if (messg == WM_LBUTTONDBLCLK || messg == WM_RBUTTONDBLCLK || messg == WM_MBUTTONDBLCLK)
            r.Event.MouseEvent.dwEventFlags = DOUBLE_CLICK;
        else if (messg == WM_MOUSEMOVE)
            r.Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
        else if (messg == WM_MOUSEWHEEL) {
            r.Event.MouseEvent.dwEventFlags = MOUSE_WHEELED;
            r.Event.MouseEvent.dwButtonState |= (0xFFFF0000 & wParam);
        } else if (messg == WM_MOUSEHWHEEL) {
            r.Event.MouseEvent.dwEventFlags = 8; //MOUSE_HWHEELED
            r.Event.MouseEvent.dwButtonState |= (0xFFFF0000 & wParam);
        }

        //TODO("� ����� ������ �� �������� ��������� ���������� ��������, � �� ������� ��������");
        //COORD crMouse = MakeCoord(x/gSet.Log Font.lfWidth, y/gSet.Log Font.lfHeight);
        COORD crMouse = mp_VCon->ClientToConsole(x,y);

        // ��� ������� ����� ������ ������� ����� ��������� � ������ ���������� �����������. �������� ���.
        if (gSet.isRSelFix) {
            BOOL lbRBtnDrag = (r.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED) == RIGHTMOST_BUTTON_PRESSED;
            if (con.bRBtnDrag && !lbRBtnDrag) {
                con.bRBtnDrag = FALSE;
            } else if (con.bRBtnDrag) {
            	#ifdef _DEBUG
                SHORT nXDelta = crMouse.X - con.crRBtnDrag.X;
                #endif
                SHORT nYDelta = crMouse.Y - con.crRBtnDrag.Y;
                if (nYDelta < -1 || nYDelta > 1) {
                    // ���� ����� ����������� ����� ������ ����� 1 ������
                    SHORT nYstep = (nYDelta < -1) ? -1 : 1;
                    SHORT nYend = crMouse.Y; // - nYstep;
                    crMouse.Y = con.crRBtnDrag.Y + nYstep;
                    // �������� ����������� ������
                    while (crMouse.Y != nYend) {
                        #ifdef _DEBUG
                        wchar_t szDbg[60]; wsprintf(szDbg, L"+++ Add right button drag: {%ix%i}\n", crMouse.X, crMouse.Y);
                        DEBUGSTRINPUT(szDbg);
                        #endif
                        r.Event.MouseEvent.dwMousePosition = crMouse;
                        PostConsoleEvent ( &r );
                        crMouse.Y += nYstep;
                    }
                }
            }
            if (lbRBtnDrag) {
                con.bRBtnDrag = TRUE;
                con.crRBtnDrag = crMouse;
            }
        }
        
        r.Event.MouseEvent.dwMousePosition = crMouse;

        if (mn_FarPID && mn_FarPID != mn_LastSetForegroundPID) {
            //DWORD dwFarPID = GetFarPID();
            //if (dwFarPID)
            AllowSetForegroundWindow(mn_FarPID);
            mn_LastSetForegroundPID = mn_FarPID;
        }

        // �������� ������� � ������� ����� ConEmuC
        PostConsoleEvent ( &r );

        /*DWORD dwWritten = 0;
        if (!WriteConsoleInput(hConIn(), &r, 1, &dwWritten)) {
            DisplayLastError(L"SendMouseEvent failed!");
        }*/
    }
    //else {
    //       /*���� ���������� ��� - ����� �����:
    //       1) ����� ���� ������ �� ������� �� �������
    //       2) ������ ���� �������� "�����" ���������� �������
    //       3) �������� � ���������� ����(?)*/
    //       /* ��! ����� �� �������� ��������� ������ */
    //       static bool bInitialized = false;
    //       if (!bInitialized && (messg == WM_LBUTTONDOWN || messg == WM_RBUTTONDOWN))
    //       {
    //           bInitialized = true; // ��������� ��� �������� ���� ���
    //           /*if (gConEmu.isConmanAlternative()) {
    //               POSTMESSAGE(hConWnd, WM_COMMAND, SC_PASTE_SECRET, 0, TRUE);
    //           } else {*/
    //               POSTMESSAGE(hConWnd, WM_COMMAND, (messg == WM_LBUTTONDOWN) ? SC_MARK_SECRET : SC_PASTE_SECRET, 0, TRUE);
    //               if (messg == WM_RBUTTONDOWN)
    //                   return; // ���������� SC_PASTE_SECRET
    //           //}
    //       }

    //       RECT conRect;
    //       GetClientRect(hConWnd, &conRect);
    //       short newX = MulDiv(x, conRect.right, klMax<uint>(1, mp_VCon->Width));
    //       short newY = MulDiv(y, conRect.bottom, klMax<uint>(1, mp_VCon->Height));

    //       POSTMESSAGE(hConWnd, messg, wParam, MAKELPARAM( newX, newY ), TRUE);
    //   }
}

void CRealConsole::SendConsoleEvent(INPUT_RECORD* piRec)
{
    if (piRec->EventType == MOUSE_EVENT) {
        if (piRec->Event.MouseEvent.dwEventFlags == MOUSE_MOVED) {
            if (m_LastMouse.dwEventFlags != 0
             && m_LastMouse.dwButtonState     == piRec->Event.MouseEvent.dwButtonState 
             && m_LastMouse.dwControlKeyState == piRec->Event.MouseEvent.dwControlKeyState
             && m_LastMouse.dwMousePosition.X == piRec->Event.MouseEvent.dwMousePosition.X
             && m_LastMouse.dwMousePosition.Y == piRec->Event.MouseEvent.dwMousePosition.Y)
            {
                #ifdef _DEBUG
                wchar_t szDbg[60];
                wsprintf(szDbg, L"!!! Skipping ConEmu.Mouse event at: {%ix%i}\n", m_LastMouse.dwMousePosition.X, m_LastMouse.dwMousePosition.Y);
                DEBUGSTRINPUT(szDbg);
                #endif
                return; // ��� ������� ������. �������� ����� ������� �� ����, ������ �� ��������
            }
        }
        // ��������
        m_LastMouse.dwMousePosition   = piRec->Event.MouseEvent.dwMousePosition;
        m_LastMouse.dwEventFlags      = piRec->Event.MouseEvent.dwEventFlags;
        m_LastMouse.dwButtonState     = piRec->Event.MouseEvent.dwButtonState;
        m_LastMouse.dwControlKeyState = piRec->Event.MouseEvent.dwControlKeyState;

        #ifdef _DEBUG
        wchar_t szDbg[60];
        wsprintf(szDbg, L"ConEmu.Mouse event at: {%ix%i}\n", m_LastMouse.dwMousePosition.X, m_LastMouse.dwMousePosition.Y);
        DEBUGSTRINPUT(szDbg);
        #endif
    }

    WARNING("��������� ������� ����� ������������, ���� ConEmuC �� ���� �� ������� ����� (�������� Focus)");
    WARNING("����� ��������� �������������� � ���� RealConsole, � �� ������ � ������� ����. ����� GUI ����� ���������������!");

    DWORD dwErr = 0, dwMode = 0;
    BOOL fSuccess = FALSE;

    // ���� ����. ��������, ��� ConEmuC ���
    DWORD dwExitCode = 0;
    fSuccess = GetExitCodeProcess(mh_ConEmuC, &dwExitCode);
    if (dwExitCode!=STILL_ACTIVE) {
        //DisplayLastError(L"ConEmuC was terminated");
        return;
    }

    LogInput(piRec);

    TODO("���� ���� � ����� ������ �� �������� � ������� 10 ������ (������?) - ������� VirtualConsole ������� ������");
    if (mh_ConEmuCInput==NULL || mh_ConEmuCInput==INVALID_HANDLE_VALUE) {
        // Try to open a named pipe; wait for it, if necessary. 
        int nSteps = 10;
        while ((nSteps--) > 0) 
        { 
          mh_ConEmuCInput = CreateFile( 
             ms_ConEmuCInput_Pipe,// pipe name 
             GENERIC_WRITE, 
             0,              // no sharing 
             NULL,           // default security attributes
             OPEN_EXISTING,  // opens existing pipe 
             0,              // default attributes 
             NULL);          // no template file 

          // Break if the pipe handle is valid. 
          if (mh_ConEmuCInput != INVALID_HANDLE_VALUE) 
             break; 

          // Exit if an error other than ERROR_PIPE_BUSY occurs. 
          dwErr = GetLastError();
          if (dwErr != ERROR_PIPE_BUSY) 
          {
            TODO("���������, ���� �������� ���� � ����� ������, �� ������ ���� ��� mh_ConEmuC");
            dwErr = WaitForSingleObject(mh_ConEmuC, 100);
            if (dwErr == WAIT_OBJECT_0) {
                return;
            }
            continue;
            //DisplayLastError(L"Could not open pipe", dwErr);
            //return 0;
          }

          // All pipe instances are busy, so wait for 0.1 second.
          if (!WaitNamedPipe(ms_ConEmuCInput_Pipe, 100) ) 
          {
            dwErr = WaitForSingleObject(mh_ConEmuC, 100);
            if (dwErr == WAIT_OBJECT_0) {
                DEBUGSTRINPUT(L" - FAILED!\n");
                return;
            }
            //DisplayLastError(L"WaitNamedPipe failed"); 
            //return 0;
          }
        }
        if (mh_ConEmuCInput == NULL || mh_ConEmuCInput == INVALID_HANDLE_VALUE) {
            // �� ��������� ��������� �����. ��������, ConEmuC ��� �� ����������
            DEBUGSTRINPUT(L" - mh_ConEmuCInput not found!\n");
            return;
        }

        // The pipe connected; change to message-read mode. 
        dwMode = PIPE_READMODE_MESSAGE; 
        fSuccess = SetNamedPipeHandleState( 
          mh_ConEmuCInput,    // pipe handle 
          &dwMode,  // new pipe mode 
          NULL,     // don't set maximum bytes 
          NULL);    // don't set maximum time 
        if (!fSuccess) 
        {
          DEBUGSTRINPUT(L" - FAILED!\n");
          DWORD dwErr = GetLastError();
          SafeCloseHandle(mh_ConEmuCInput);
          if (!IsDebuggerPresent())
            DisplayLastError(L"SetNamedPipeHandleState failed", dwErr);
          return;
        }
    }
    
    // ���� ����. ��������, ��� ConEmuC ���
    dwExitCode = 0;
    fSuccess = GetExitCodeProcess(mh_ConEmuC, &dwExitCode);
    if (dwExitCode!=STILL_ACTIVE) {
        //DisplayLastError(L"ConEmuC was terminated");
        return;
    }
    
    DWORD dwSize = sizeof(INPUT_RECORD), dwWritten;
    fSuccess = WriteFile ( mh_ConEmuCInput, piRec, dwSize, &dwWritten, NULL);
    if (!fSuccess) {
        DisplayLastError(L"Can't send console event");
        return;
    }
}


void CRealConsole::StopSignal()
{
    if (!this)
        return;

    if (mn_ProcessCount) {
        MSectionLock SPRC; SPRC.Lock(&csPRC, TRUE);
        m_Processes.clear();
        SPRC.Unlock();
        mn_ProcessCount = 0;
    }

    SetEvent(mh_TermEvent);

    if (!mn_InRecreate)
        gConEmu.OnVConTerminated(mp_VCon);
}

void CRealConsole::StopThread(BOOL abRecreating)
{
#ifdef _DEBUG
    /*
        HeapValidate(mh_Heap, 0, NULL);
    */
#endif

    _ASSERT(abRecreating==mb_ProcessRestarted);

    DEBUGSTRPROC(L"Entering StopThread\n");

    // ������� ��������� ����� ��������
    if (mh_MonitorThread) {
        // ����������� ������ � ���������� ����
        StopSignal(); //SetEvent(mh_TermEvent);
    }
    
    if (mh_InputThread) {
    	PostThreadMessage(mn_InputThreadID, WM_QUIT, 0, 0);
    }
    
    
    // � ������ ����� ����� ����������
    if (mh_MonitorThread) {
        if (WaitForSingleObject(mh_MonitorThread, 300) != WAIT_OBJECT_0) {
            DEBUGSTRPROC(L"### Main Thread wating timeout, terminating...\n");
            TerminateThread(mh_MonitorThread, 1);
        } else {
            DEBUGSTRPROC(L"Main Thread closed normally\n");
        }
        SafeCloseHandle(mh_MonitorThread);
    }
    
    if (mh_InputThread) {
        if (WaitForSingleObject(mh_InputThread, 300) != WAIT_OBJECT_0) {
            DEBUGSTRPROC(L"### Input Thread wating timeout, terminating...\n");
            TerminateThread(mh_InputThread, 1);
        } else {
            DEBUGSTRPROC(L"Input Thread closed normally\n");
        }
        SafeCloseHandle(mh_InputThread);
    }
    
    if (!abRecreating) {
        SafeCloseHandle(mh_TermEvent);
        SafeCloseHandle(mh_MonitorThreadEvent);
        SafeCloseHandle(mh_PacketArrived);
    }
    
    
    if (abRecreating) {
        hConWnd = NULL;
        mn_ConEmuC_PID = 0;
        SafeCloseHandle(mh_ConEmuC);
        SafeCloseHandle(mh_ConEmuCInput);
        // ��� ����� ��� ���������� ConEmuC
        ms_ConEmuC_Pipe[0] = 0;
        ms_ConEmuCInput_Pipe[0] = 0;
    }
#ifdef _DEBUG
    /*
        HeapValidate(mh_Heap, 0, NULL);
    */
#endif

    DEBUGSTRPROC(L"Leaving StopThread\n");
}


void CRealConsole::Box(LPCTSTR szText)
{
#ifdef _DEBUG
    _ASSERT(FALSE);
#endif
    MessageBox(NULL, szText, _T("ConEmu"), MB_ICONSTOP);
}


BOOL CRealConsole::isBufferHeight()
{
    if (!this)
        return FALSE;

    return con.bBufferHeight;
}

BOOL CRealConsole::isConSelectMode()
{
    if (!this) return false;
    return mb_ConsoleSelectMode;
}

BOOL CRealConsole::isDetached()
{
    if (this == NULL) return FALSE;
    if (!m_Args.bDetached) return FALSE;
    // ������ ������ �� ���������� - ������������� �� ������
    //_ASSERTE(!mb_Detached || (mb_Detached && (hConWnd==NULL)));
    return (mh_ConEmuC == NULL);
}

BOOL CRealConsole::isFar()
{
    if (!this) return false;
    return GetFarPID()!=0;
}

BOOL CRealConsole::isWindowVisible()
{
	if (!this) return FALSE;
	if (!hConWnd) return FALSE;
	return IsWindowVisible(hConWnd);
}

LPCTSTR CRealConsole::GetTitle()
{
    // �� ������ mn_ProcessCount==0, � ������ � ������� ���������� ��� �����
    if (!this /*|| !mn_ProcessCount*/)
        return NULL;
    return Title;
}

LRESULT CRealConsole::OnScroll(int nDirection)
{
    // SB_LINEDOWN / SB_LINEUP / SB_PAGEDOWN / SB_PAGEUP
    TODO("���������� � ������� �����");
    POSTMESSAGE(hConWnd, WM_VSCROLL, nDirection, NULL, FALSE);
    return 0;
}

void CRealConsole::OnKeyboard(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
    //LRESULT result = 0;

#ifdef _DEBUG
    if (wParam != VK_LCONTROL && wParam != VK_RCONTROL && wParam != VK_CONTROL &&
        wParam != VK_LSHIFT && wParam != VK_RSHIFT && wParam != VK_SHIFT &&
        wParam != VK_LMENU && wParam != VK_RMENU && wParam != VK_MENU &&
        wParam != VK_LWIN && wParam != VK_RWIN)
    {
        wParam = wParam;
    }
    if (wParam == VK_CONTROL || wParam == VK_LCONTROL || wParam == VK_RCONTROL || wParam == 'C') {
        if (messg == WM_KEYDOWN || messg == WM_KEYUP || messg == WM_CHAR) {
            wchar_t szDbg[128];
            if (messg == WM_KEYDOWN)
                wsprintf(szDbg, L"WM_KEYDOWN(%i,0x%08X)\n", wParam, lParam);
            else if (messg == WM_KEYUP)
                wsprintf(szDbg, L"WM_KEYUP(%i,0x%08X)\n", wParam, lParam);
            else
                wsprintf(szDbg, L"WM_CHAR(%i,0x%08X)\n", wParam, lParam);
            DEBUGSTRINPUT(szDbg);
        }
    }
#endif

    if (mb_ConsoleSelectMode && messg == WM_KEYDOWN && ((wParam == VK_ESCAPE) || (wParam == VK_RETURN)))
        mb_ConsoleSelectMode = false; //TODO: ����� ���-�� �� ������� ����������?

    // �������� ��������� 
    {
        if (messg == WM_SYSKEYDOWN) 
            if (wParam == VK_INSERT && lParam & (1<<29)/*����. ��� 29-� ���, � �� ����� 29*/)
                mb_ConsoleSelectMode = true;

        static bool isSkipNextAltUp = false;
        if (messg == WM_SYSKEYDOWN && wParam == VK_RETURN && lParam & (1<<29)/*����. ��� 29-� ���, � �� ����� 29*/)
        {
            if (gSet.isSentAltEnter)
            {
                TODO("���������� � SendConsoleInput");
                OnKeyboard(hConWnd, WM_KEYDOWN, VK_MENU, 0);
                OnKeyboard(hConWnd, WM_KEYDOWN, VK_RETURN, 0);
                OnKeyboard(hConWnd, WM_KEYUP, VK_RETURN, 0);
                OnKeyboard(hConWnd, WM_KEYUP, VK_MENU, 0);
            }
            else
            {
                if (isPressed(VK_SHIFT))
                    return;

                if (!gSet.isFullScreen)
                    gConEmu.SetWindowMode(rFullScreen);
                else
                    gConEmu.SetWindowMode(gConEmu.isWndNotFSMaximized ? rMaximized : rNormal);

                isSkipNextAltUp = true;
            }
        }
        else if (messg == WM_SYSKEYDOWN && wParam == VK_SPACE && lParam & (1<<29)/*����. ��� 29-� ���, � �� ����� 29*/ && !isPressed(VK_SHIFT))
        {   // ����, ��� ��������� ���� ����� ����������
            RECT rect, cRect;
            GetWindowRect(ghWnd, &rect);
            GetClientRect(ghWnd, &cRect);
            WINDOWINFO wInfo;   GetWindowInfo(ghWnd, &wInfo);
            gConEmu.ShowSysmenu(ghWnd, ghWnd, rect.right - cRect.right - wInfo.cxWindowBorders, rect.bottom - cRect.bottom - wInfo.cyWindowBorders);
        }
        else if (messg == WM_KEYUP && wParam == VK_MENU && isSkipNextAltUp) isSkipNextAltUp = false;
        else if (messg == WM_SYSKEYDOWN && wParam == VK_F9 && lParam & (1<<29)/*����. ��� 29-� ���, � �� ����� 29*/ && !isPressed(VK_SHIFT))
            gConEmu.SetWindowMode((IsZoomed(ghWnd)||(gSet.isFullScreen&&gConEmu.isWndNotFSMaximized)) ? rNormal : rMaximized);
        else {
            INPUT_RECORD r = {KEY_EVENT};

            WORD nCaps = 1 & (WORD)GetKeyState(VK_CAPITAL);
            WORD nNum = 1 & (WORD)GetKeyState(VK_NUMLOCK);
            WORD nScroll = 1 & (WORD)GetKeyState(VK_SCROLL);
            WORD nLAlt = 0x8000 & (WORD)GetKeyState(VK_LMENU);
            WORD nRAlt = 0x8000 & (WORD)GetKeyState(VK_RMENU);
            WORD nLCtrl = 0x8000 & (WORD)GetKeyState(VK_LCONTROL);
            WORD nRCtrl = 0x8000 & (WORD)GetKeyState(VK_RCONTROL);
            WORD nShift = 0x8000 & (WORD)GetKeyState(VK_SHIFT);

            if (messg == WM_CHAR || messg == WM_SYSCHAR) {
                if (((WCHAR)wParam) <= 32 || mn_LastVKeyPressed == 0)
                    return; // ��� ��� ����������
                r.Event.KeyEvent.bKeyDown = TRUE;
                r.Event.KeyEvent.uChar.UnicodeChar = (WCHAR)wParam;
                r.Event.KeyEvent.wRepeatCount = 1; TODO("0-15 ? Specifies the repeat count for the current message. The value is the number of times the keystroke is autorepeated as a result of the user holding down the key. If the keystroke is held long enough, multiple messages are sent. However, the repeat count is not cumulative.");
                r.Event.KeyEvent.wVirtualKeyCode = mn_LastVKeyPressed;
            } else {
                mn_LastVKeyPressed = wParam & 0xFFFF;
                //POSTMESSAGE(hConWnd, messg, wParam, lParam, FALSE);
                if ((wParam >= VK_F1 && wParam <= /*VK_F24*/ VK_SCROLL) || wParam <= 32 ||
                    (wParam >= VK_LSHIFT/*0xA0*/ && wParam <= /*VK_RMENU=0xA5*/ 0xB7 /*=VK_LAUNCH_APP2*/) ||
                    (wParam >= VK_LWIN/*0x5B*/ && wParam <= VK_APPS/*0x5D*/) ||
                    /*(wParam >= VK_NUMPAD0 && wParam <= VK_DIVIDE) ||*/ //TODO:
                    (wParam >= VK_PRIOR/*0x21*/ && wParam <= VK_HELP/*0x2F*/) ||
                    nLCtrl || nRCtrl ||
                    ((nLAlt || nRAlt) && !(nLCtrl || nRCtrl || nShift) && (wParam >= VK_NUMPAD0/*0x60*/ && wParam <= VK_NUMPAD9/*0x69*/)) || // ���� Alt-����� ��� ���������� NumLock
                    FALSE)
                {
                    r.Event.KeyEvent.wRepeatCount = 1; TODO("0-15 ? Specifies the repeat count for the current message. The value is the number of times the keystroke is autorepeated as a result of the user holding down the key. If the keystroke is held long enough, multiple messages are sent. However, the repeat count is not cumulative.");
                    r.Event.KeyEvent.wVirtualKeyCode = mn_LastVKeyPressed;
                    r.Event.KeyEvent.uChar.UnicodeChar = 0;
                    if (!nLCtrl && !nRCtrl) {
                        if (wParam == VK_ESCAPE || wParam == VK_RETURN || wParam == VK_BACK || wParam == VK_TAB || wParam == VK_SPACE
                            || FALSE)
                            r.Event.KeyEvent.uChar.UnicodeChar = wParam;
                    }
                    mn_LastVKeyPressed = 0; // ����� �� ������������ WM_(SYS)CHAR
                } else {
                    return;
                }
                r.Event.KeyEvent.bKeyDown = (messg == WM_KEYDOWN || messg == WM_SYSKEYDOWN);
            }

            r.Event.KeyEvent.wVirtualScanCode = ((DWORD)lParam & 0xFF0000) >> 16; // 16-23 - Specifies the scan code. The value depends on the OEM.
            // 24 - Specifies whether the key is an extended key, such as the right-hand ALT and CTRL keys that appear on an enhanced 101- or 102-key keyboard. The value is 1 if it is an extended key; otherwise, it is 0.
            // 29 - Specifies the context code. The value is 1 if the ALT key is held down while the key is pressed; otherwise, the value is 0.
            // 30 - Specifies the previous key state. The value is 1 if the key is down before the message is sent, or it is 0 if the key is up.
            // 31 - Specifies the transition state. The value is 1 if the key is being released, or it is 0 if the key is being pressed.
            r.Event.KeyEvent.dwControlKeyState = 0;
            if (((DWORD)lParam & (DWORD)(1 << 24)) != 0)
                r.Event.KeyEvent.dwControlKeyState |= ENHANCED_KEY;
            if ((nCaps & 1) == 1)
                r.Event.KeyEvent.dwControlKeyState |= CAPSLOCK_ON;
            if ((nNum & 1) == 1)
                r.Event.KeyEvent.dwControlKeyState |= NUMLOCK_ON;
            if ((nScroll & 1) == 1)
                r.Event.KeyEvent.dwControlKeyState |= SCROLLLOCK_ON;
            if (nLAlt & 0x8000)
                r.Event.KeyEvent.dwControlKeyState |= LEFT_ALT_PRESSED;
            if (nRAlt & 0x8000)
                r.Event.KeyEvent.dwControlKeyState |= RIGHT_ALT_PRESSED;
            if (nLCtrl & 0x8000)
                r.Event.KeyEvent.dwControlKeyState |= LEFT_CTRL_PRESSED;
            if (nRCtrl & 0x8000)
                r.Event.KeyEvent.dwControlKeyState |= RIGHT_CTRL_PRESSED;
            if (nShift & 0x8000)
                r.Event.KeyEvent.dwControlKeyState |= SHIFT_PRESSED;


            #ifdef _DEBUG
            if (r.EventType == KEY_EVENT && r.Event.KeyEvent.bKeyDown &&
                r.Event.KeyEvent.wVirtualKeyCode == VK_F11)
            {
                DEBUGSTRINPUT(L"  ---  F11 sending\n");
            }
            #endif

            
            if (mn_FarPID && mn_FarPID != mn_LastSetForegroundPID) {
                //DWORD dwFarPID = GetFarPID();
                //if (dwFarPID)
                AllowSetForegroundWindow(mn_FarPID);
                mn_LastSetForegroundPID = mn_FarPID;
            }
            
            PostConsoleEvent(&r);

            if (messg == WM_CHAR || messg == WM_SYSCHAR) {
                // � ����� �������� ����������
                r.Event.KeyEvent.bKeyDown = FALSE;
                PostConsoleEvent(&r);
            }
        }
    }

    /*if (IsDebuggerPresent()) {
        if (hWnd ==ghWnd)
            DEBUGSTRINPUT(L"   focused ghWnd\n"); else
        if (hWnd ==hConWnd)
            DEBUGSTRINPUT(L"   focused hConWnd\n"); else
        if (hWnd ==ghWndDC)
            DEBUGSTRINPUT(L"   focused ghWndDC\n"); 
        else
            DEBUGSTRINPUT(L"   focused UNKNOWN\n"); 
    }*/

    return;
}

void CRealConsole::OnWinEvent(DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    _ASSERTE(hwnd!=NULL);
    
    if (hConWnd == NULL && event == EVENT_CONSOLE_START_APPLICATION && idObject == (LONG)mn_ConEmuC_PID)
        SetHwnd ( hwnd );

    _ASSERTE(hConWnd!=NULL && hwnd==hConWnd);
    
    //TODO("!!! ������� ��������� ������� � ��������� m_Processes");
    //
    //AddProcess(idobject), � �������� idObject �� ������ ���������
    // �� ������, ��� ��������� ������ Ntvdm
    
    TODO("��� ���������� �� ������� NTVDM - ����� ������� ���� �������");
    switch(event)
    {
    case EVENT_CONSOLE_START_APPLICATION:
        //A new console process has started. 
        //The idObject parameter contains the process identifier of the newly created process. 
        //If the application is a 16-bit application, the idChild parameter is CONSOLE_APPLICATION_16BIT and idObject is the process identifier of the NTVDM session associated with the console.
        {   
            if (mn_InRecreate>=1)
                mn_InRecreate = 0; // �������� ������� ������� ������������
                
            //WARNING("��� ����� ���������, ���� ��������� �� ����������: ������� ����� ���� �������� � ����� ������");
            //Process Add(idObject);
            
            // ���� �������� 16������ ���������� - ���������� �������� ��������� ������ ��������, ����� ����� �������
            _ASSERTE(CONSOLE_APPLICATION_16BIT==1);
            if (idChild == CONSOLE_APPLICATION_16BIT) {
				DEBUGSTRPROC(L"16 bit application STARTED\n");
                mn_ProgramStatus |= CES_NTVDM;
				if (gOSVer.dwMajorVersion>5 || (gOSVer.dwMajorVersion==5 && gOSVer.dwMinorVersion>=1))
					mb_IgnoreCmdStop = TRUE;
                SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
            }
        } break;
    case EVENT_CONSOLE_END_APPLICATION:
        //A console process has exited. 
        //The idObject parameter contains the process identifier of the terminated process.
        {
            //WARNING("��� ����� ���������, ���� ��������� �� ����������: ������� ����� ���� ������ � ����� ������");
            //Process Delete(idObject);
            
            //
            if (idChild == CONSOLE_APPLICATION_16BIT) {
                //gConEmu.gbPostUpdateWindowSize = true;
				DEBUGSTRPROC(L"16 bit application TERMINATED\n");
                mn_ProgramStatus &= ~CES_NTVDM;
				SyncConsole2Window(); // ����� ������ �� 16bit ������ ������ �� ����������� ������� �� ������� GUI
                TODO("������-�� ������ �� ���������, ��� 16��� �� �������� � ������ ��������");
                SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
            }
        } break;
    }
}


DWORD CRealConsole::ServerThread(LPVOID lpvParam) 
{ 
    CRealConsole *pRCon = (CRealConsole*)lpvParam;
    BOOL fConnected = FALSE;
    DWORD dwErr = 0;
    HANDLE hPipe = NULL; 
    HANDLE hWait[2] = {NULL,NULL};
    DWORD dwTID = GetCurrentThreadId();

    _ASSERTE(pRCon->hConWnd!=NULL);
    _ASSERTE(pRCon->ms_VConServer_Pipe[0]!=0);
    _ASSERTE(pRCon->mh_ServerSemaphore!=NULL);
    //wsprintf(pRCon->ms_VConServer_Pipe, CEGUIPIPENAME, L".", (DWORD)pRCon->hConWnd); //��� mn_ConEmuC_PID

    // The main loop creates an instance of the named pipe and 
    // then waits for a client to connect to it. When the client 
    // connects, a thread is created to handle communications 
    // with that client, and the loop is repeated. 
    
    hWait[0] = pRCon->mh_TermEvent;
    hWait[1] = pRCon->mh_ServerSemaphore;

    // ���� �� ����������� ���������� �������
    do {
        while (!fConnected)
        { 
            _ASSERTE(hPipe == NULL);

            // ��������� ���������� ��������, ��� �������� �������
            dwErr = WaitForMultipleObjects ( 2, hWait, FALSE, INFINITE );
            if (dwErr == WAIT_OBJECT_0) {
                return 0; // ������� �����������
            }

            for (int i=0; i<MAX_SERVER_THREADS; i++) {
                if (pRCon->mn_ServerThreadsId[i] == dwTID) {
                    pRCon->mh_ActiveServerThread = pRCon->mh_ServerThreads[i]; break;
                }
            }

            hPipe = CreateNamedPipe( 
                pRCon->ms_VConServer_Pipe, // pipe name 
                PIPE_ACCESS_DUPLEX,       // read/write access 
                PIPE_TYPE_MESSAGE |       // message type pipe 
                PIPE_READMODE_MESSAGE |   // message-read mode 
                PIPE_WAIT,                // blocking mode 
                PIPE_UNLIMITED_INSTANCES, // max. instances  
                PIPEBUFSIZE,              // output buffer size 
                PIPEBUFSIZE,              // input buffer size 
                0,                        // client time-out 
                NULL);                    // default security attribute 

            _ASSERTE(hPipe != INVALID_HANDLE_VALUE);

            if (hPipe == INVALID_HANDLE_VALUE) 
            {
                //DisplayLastError(L"CreatePipe failed"); 
                hPipe = NULL;
                Sleep(50);
                continue;
            }

            // Wait for the client to connect; if it succeeds, 
            // the function returns a nonzero value. If the function
            // returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 

            fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : ((dwErr = GetLastError()) == ERROR_PIPE_CONNECTED); 

            // ������� �����������!
            if (WaitForSingleObject ( pRCon->mh_TermEvent, 0 ) == WAIT_OBJECT_0) {
                //FlushFileBuffers(hPipe); -- ��� �� �����, �� ������ �� ����������
                //DisconnectNamedPipe(hPipe); 
                ReleaseSemaphore(pRCon->mh_ServerSemaphore, 1, NULL);
                SafeCloseHandle(hPipe);
                return 0;
            }

            if (!fConnected)
                SafeCloseHandle(hPipe);
        }

        if (fConnected) {
            // ����� �������, ����� �� ������
            fConnected = FALSE;
            // ��������� ������ ���� ������� �����
            ReleaseSemaphore(pRCon->mh_ServerSemaphore, 1, NULL);
            
            /*{ // ��������� ����� ��������� ����. ���� instance ����� ������ ����� ��������� �������.
                DWORD dwServerTID = 0;
                HANDLE hThread = CreateThread(NULL, 0, ServerThread, (LPVOID)pRCon, 0, &dwServerTID);
                _ASSERTE(hThread!=NULL);
                SafeCloseHandle(hThread);
            }*/

            //ServerThreadCommandArg* pArg = (ServerThreadCommandArg*)calloc(sizeof(ServerThreadCommandArg),1);
            //pArg->pRCon = pRCon;
            //pArg->hPipe = hPipe;
            pRCon->ServerThreadCommand ( hPipe ); // ��� ������������� - ���������� � ���� ��������� ����
            //DWORD dwCommandTID = 0;
            //HANDLE hCommandThread = CreateThread(NULL, 0, ServerThreadCommand, (LPVOID)pArg, 0, &dwCommandTID);
            //_ASSERTE(hCommandThread!=NULL);
            //SafeCloseHandle(hCommandThread);
        }

        FlushFileBuffers(hPipe); 
        //DisconnectNamedPipe(hPipe); 
        SafeCloseHandle(hPipe);
    } // ������� � �������� ������ instance �����
    while (WaitForSingleObject ( pRCon->mh_TermEvent, 0 ) != WAIT_OBJECT_0);

    return 0; 
}

// ��� ������� ���� �� ���������!
void CRealConsole::ServerThreadCommand(HANDLE hPipe)
{
    CESERVER_REQ in={{0}}, *pIn=NULL;
    DWORD cbRead = 0, cbWritten = 0, dwErr = 0;
    BOOL fSuccess = FALSE;

    // Send a message to the pipe server and read the response. 
    fSuccess = ReadFile( 
        hPipe,            // pipe handle 
        &in,              // buffer to receive reply
        sizeof(in),       // size of read buffer
        &cbRead,          // bytes read
        NULL);            // not overlapped 

    if (!fSuccess && ((dwErr = GetLastError()) != ERROR_MORE_DATA)) 
    {
        _ASSERTE("ReadFile(pipe) failed"==NULL);
        //CloseHandle(hPipe);
        return;
    }
    if (in.hdr.nVersion != CESERVER_REQ_VER) {
        gConEmu.ShowOldCmdVersion(in.hdr.nCmd, in.hdr.nVersion);
        return;
    }
    _ASSERTE(in.hdr.nSize>=sizeof(CESERVER_REQ_HDR) && cbRead>=sizeof(CESERVER_REQ_HDR));
    if (cbRead < sizeof(CESERVER_REQ_HDR) || /*in.hdr.nSize < cbRead ||*/ in.hdr.nVersion != CESERVER_REQ_VER) {
        //CloseHandle(hPipe);
        return;
    }

    if (in.hdr.nSize <= cbRead) {
        pIn = &in; // ��������� ������ �� ���������
    } else {
        int nAllSize = in.hdr.nSize;
        pIn = (CESERVER_REQ*)calloc(nAllSize,1);
        _ASSERTE(pIn!=NULL);
        memmove(pIn, &in, cbRead);
        _ASSERTE(pIn->hdr.nVersion==CESERVER_REQ_VER);

        LPBYTE ptrData = ((LPBYTE)pIn)+cbRead;
        nAllSize -= cbRead;

        while(nAllSize>0)
        { 
            //_tprintf(TEXT("%s\n"), chReadBuf);

            // Break if TransactNamedPipe or ReadFile is successful
            if(fSuccess)
                break;

            // Read from the pipe if there is more data in the message.
            fSuccess = ReadFile( 
                hPipe,      // pipe handle 
                ptrData,    // buffer to receive reply 
                nAllSize,   // size of buffer 
                &cbRead,    // number of bytes read 
                NULL);      // not overlapped 

            // Exit if an error other than ERROR_MORE_DATA occurs.
            if( !fSuccess && ((dwErr = GetLastError()) != ERROR_MORE_DATA)) 
                break;
            ptrData += cbRead;
            nAllSize -= cbRead;
        }

        TODO("����� ���������� ASSERT, ���� ������� ���� ������� � �������� ������");
        _ASSERTE(nAllSize==0);
        if (nAllSize>0) {
            //CloseHandle(hPipe);
            return; // ������� ������� �� ��� ������
        }
    }

    
    UINT nDataSize = pIn->hdr.nSize - sizeof(CESERVER_REQ_HDR);

    // ��� ������ �� ����� ��������, ������������ ������� � ���������� (���� �����) ���������
    if (pIn->hdr.nCmd == CECMD_GETFULLINFO || pIn->hdr.nCmd == CECMD_GETSHORTINFO) {
		#ifdef _DEBUG
		wchar_t szDbg[255]; wsprintf(szDbg, L"GUI recieved %s, PktID=%i\n", 
			(pIn->hdr.nCmd == CECMD_GETFULLINFO) ? L"CECMD_GETFULLINFO" : L"CECMD_GETSHORTINFO",
			pIn->ConInfo.inf.nPacketId);
        DEBUGSTRCMD(szDbg);
		#endif
        //ApplyConsoleInfo(pIn);
        if (((LPVOID)&in)==((LPVOID)pIn)) {
            // ��� ������������� ������ - ���������� (in)
            _ASSERTE(in.hdr.nSize>0);
            // ��� ��� ��������� ����� ������� ����� ������, ������� ��������� PopPacket
            pIn = (CESERVER_REQ*)calloc(in.hdr.nSize,1);
            memmove(pIn, &in, in.hdr.nSize);
        }
        PushPacket(pIn);
        pIn = NULL;

    } else if (pIn->hdr.nCmd == CECMD_CMDSTARTSTOP) {
        //
		DWORD nStarted = pIn->StartStop.nStarted;
        #ifdef _DEBUG
		HWND  hWnd     = (HWND)pIn->StartStop.hWnd;
        #endif
        DWORD nPID     = pIn->StartStop.dwPID;
        DEBUGSTRCMD(L"GUI recieved CECMD_CMDSTARTSTOP\n");
		DWORD nSubSystem = pIn->StartStop.nSubSystem;

		_ASSERTE(sizeof(CESERVER_REQ_STARTSTOPRET) <= sizeof(CESERVER_REQ_STARTSTOP));
		pIn->hdr.nSize = sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_REQ_STARTSTOPRET);
        
        if (nStarted == 0 || nStarted == 2) {
            // ����� �������� ���������
            pIn->StartStopRet.bWasBufferHeight = isBufferHeight(); // ����� comspec ����, ��� ����� ����� ����� ���������
			pIn->StartStopRet.hWnd = ghWnd;
			pIn->StartStopRet.dwPID = GetCurrentProcessId();

            AllowSetForegroundWindow(nPID);
            
            COORD cr16bit = {80,con.m_sbi.dwSize.Y};
			if (nSubSystem == 255) {
				if (gSet.ntvdmHeight && cr16bit.Y >= (int)gSet.ntvdmHeight) cr16bit.Y = gSet.ntvdmHeight;
				else if (cr16bit.Y>=50) cr16bit.Y = 50;
				else if (cr16bit.Y>=43) cr16bit.Y = 43;
				else if (cr16bit.Y>=28) cr16bit.Y = 28;
				else if (cr16bit.Y>=25) cr16bit.Y = 25;
			}

            // ComSpec started
            if (nStarted == 2) {
				// ��������������� � TRUE ���� ����� �������� 16������ ����������
				if (nSubSystem == 255) {
					DEBUGSTRCMD(L"16 bit application STARTED, aquired from CECMD_CMDSTARTSTOP\n");
					mn_ProgramStatus |= CES_NTVDM;
					mb_IgnoreCmdStop = TRUE;
					
					SetConsoleSize(cr16bit, CECMD_CMDSTARTED);
					
					pIn->StartStopRet.nBufferHeight = 0;
					pIn->StartStopRet.nWidth = cr16bit.X;
					pIn->StartStopRet.nHeight = cr16bit.Y;
				} else {
					// �� ���� ��� ����� ��������
					mb_IgnoreCmdStop = FALSE;

					pIn->StartStopRet.nBufferHeight = gSet.DefaultBufferHeight;
					pIn->StartStopRet.nWidth = con.m_sbi.dwSize.X;
					pIn->StartStopRet.nHeight = con.m_sbi.dwSize.Y;

					if (gSet.DefaultBufferHeight && !isBufferHeight()) {
						WARNING("��� �������� ����� �� ������������� ����� ������� ����� ������� �� ������� ConEmuC");
						con.m_sbi.dwSize.Y = gSet.DefaultBufferHeight;
						mb_BuferModeChangeLocked = TRUE;
						SetBufferHeightMode(TRUE, TRUE); // ����� ��������, ����� ������� ����������� ������������
						SetConsoleSize(con.m_sbi.dwSize, CECMD_CMDSTARTED);
						mb_BuferModeChangeLocked = FALSE;
					}
				}
            } else if (nStarted == 0) {
            	// Server
            	if (nSubSystem == 255) {
					pIn->StartStopRet.nBufferHeight = 0;
					pIn->StartStopRet.nWidth = cr16bit.X;
					pIn->StartStopRet.nHeight = cr16bit.Y;
            	} else {
            		if (gSet.DefaultBufferHeight)
						pIn->StartStopRet.nBufferHeight = gSet.DefaultBufferHeight;
					else
						pIn->StartStopRet.nBufferHeight = 0;
					pIn->StartStopRet.nWidth = con.m_sbi.dwSize.X;
					pIn->StartStopRet.nHeight = con.m_sbi.dwSize.Y;
            	}

				con.bBufferHeight = (pIn->StartStopRet.nBufferHeight != 0);
				con.nChange2TextWidth = pIn->StartStopRet.nWidth;
				con.nChange2TextHeight = pIn->StartStopRet.nHeight;
            }

            // 23.06.2009 Maks - ������ ����. ������ �������� � ApplyConsoleInfo
            //Process Add(nPID);

        } else {
            // 23.06.2009 Maks - ������ ����. ������ �������� � ApplyConsoleInfo
            //Process Delete(nPID);

			// ComSpec stopped
            if (nStarted == 3) {
				BOOL lbNeedResizeWnd = FALSE;
				COORD crNewSize = {TextWidth(),TextHeight()};
				int nNewWidth=0, nNewHeight=0; BOOL bBufferHeight = FALSE;
				if ((mn_ProgramStatus & CES_NTVDM) == 0
					&& !(gSet.isFullScreen || IsZoomed(ghWnd)))
				{
					if (GetConWindowSize(pIn->StartStop.sbi, nNewWidth, nNewHeight, &bBufferHeight)) {
						if (crNewSize.X != nNewWidth || crNewSize.Y != nNewHeight) {
							//gConEmu.SyncWindowToConsole(); - ��� ������������ ������. �� ������ ��� �� ������� ����, �� ������ - ������ pVCon ����� ���� ��� �� �������
							lbNeedResizeWnd = TRUE;
							crNewSize.X = nNewWidth;
							crNewSize.Y = nNewHeight;
							pIn->StartStopRet.bWasBufferHeight = FALSE;
						}
					}
				}

				if (mb_IgnoreCmdStop || (mn_ProgramStatus & CES_NTVDM) == CES_NTVDM) {
					// ����� ������������ ������ � WinXP � ����
					// ���� �������� 16������ ����������, ������ ����������� ������ ������� ������ ����� 80x25
					// ��� �� ������������� ��������� ������� ��� ������ �� 16���. ������� ����� ������������
					// ��� ������ ����. ��� ������� OnWinEvent.
					//SetBufferHeightMode(FALSE, TRUE);
					//PRAGMA_ERROR("���� �� ������� CECMD_CMDFINISHED �� ��������� WinEvents");
					mb_IgnoreCmdStop = FALSE; // �������� ����� �������, ��� ������ ������ �� �����
					
    				DEBUGSTRCMD(L"16 bit application TERMINATED (aquired from CECMD_CMDFINISHED)\n");
                    mn_ProgramStatus &= ~CES_NTVDM;
    				SyncConsole2Window(); // ����� ������ �� 16bit ������ ������ �� ����������� ������� �� ������� GUI
				} //else {
				// ������������ ������ ����� ��������� ConEmuC
				mb_BuferModeChangeLocked = TRUE;
				con.m_sbi.dwSize.Y = crNewSize.Y;
				SetBufferHeightMode(FALSE, TRUE); // ����� ���������, ����� ������� ����������� ������������
				SetConsoleSize(crNewSize, CECMD_CMDFINISHED);
				if (lbNeedResizeWnd) {
					RECT rcCon = MakeRect(nNewWidth, nNewHeight);
					RECT rcNew = gConEmu.CalcRect(CER_MAIN, rcCon, CER_CONSOLE);
					RECT rcWnd; GetWindowRect(ghWnd, &rcWnd);
					MOVEWINDOW ( ghWnd, rcWnd.left, rcWnd.top, rcNew.right, rcNew.bottom, 1);
				}
				mb_BuferModeChangeLocked = FALSE;
				//}
            }
        }
        
        // ����������
        fSuccess = WriteFile( 
            hPipe,        // handle to pipe 
            pIn,         // buffer to write from 
            pIn->hdr.nSize,  // number of bytes to write 
            &cbWritten,   // number of bytes written 
            NULL);        // not overlapped I/O 

    } else if (pIn->hdr.nCmd == CECMD_GETGUIHWND) {
        DEBUGSTRCMD(L"GUI recieved CECMD_GETGUIHWND\n");
        CESERVER_REQ *pRet = NULL;
        int nSize = sizeof(CESERVER_REQ_HDR) + 2*sizeof(DWORD);
        pRet = (CESERVER_REQ*)calloc(nSize, 1);
        pRet->hdr.nSize = nSize;
        pRet->hdr.nCmd = pIn->hdr.nCmd;
        pRet->hdr.nSrcThreadId = GetCurrentThreadId();
        pRet->hdr.nVersion = CESERVER_REQ_VER;
        ((DWORD*)pRet->Data)[0] = (DWORD)ghWnd;
        ((DWORD*)pRet->Data)[1] = (DWORD)ghWndDC;
        // ����������
        fSuccess = WriteFile( 
            hPipe,        // handle to pipe 
            pRet,         // buffer to write from 
            pRet->hdr.nSize,  // number of bytes to write 
            &cbWritten,   // number of bytes written 
            NULL);        // not overlapped I/O 
        free(pRet);
            
    } else if (pIn->hdr.nCmd == CECMD_TABSCHANGED) {
        DEBUGSTRCMD(L"GUI recieved CECMD_TABSCHANGED\n");
        if (nDataSize == 0) {
            // ��� �����������
            SetTabs(NULL, 1);
        } else {
            _ASSERTE(nDataSize>=4);
            int nTabCount = (nDataSize-4) / sizeof(ConEmuTab);
            _ASSERTE((nTabCount*sizeof(ConEmuTab))==(nDataSize-4));
            SetTabs((ConEmuTab*)(pIn->Data+4), nTabCount);
        }

    } else if (pIn->hdr.nCmd == CECMD_GETOUTPUTFILE) {
        DEBUGSTRCMD(L"GUI recieved CECMD_GETOUTPUTFILE\n");
        _ASSERTE(nDataSize>=4);
        BOOL lbUnicode = pIn->OutputFile.bUnicode;

        CESERVER_REQ *pRet = NULL;
        int nSize = sizeof(CESERVER_REQ_HDR) + sizeof(CESERVER_REQ_OUTPUTFILE);
        pRet = (CESERVER_REQ*)calloc(nSize, 1);
        pRet->hdr.nSize = nSize;
        pRet->hdr.nCmd = pIn->hdr.nCmd;
        pRet->hdr.nSrcThreadId = GetCurrentThreadId();
        pRet->hdr.nVersion = CESERVER_REQ_VER;

        pRet->OutputFile.bUnicode = lbUnicode;
        pRet->OutputFile.szFilePathName[0] = 0; // ���������� PrepareOutputFile

        if (!PrepareOutputFile(lbUnicode, pRet->OutputFile.szFilePathName)) {
            pRet->OutputFile.szFilePathName[0] = 0;
        }

        // ����������
        fSuccess = WriteFile( 
            hPipe,        // handle to pipe 
            pRet,         // buffer to write from 
            pRet->hdr.nSize,  // number of bytes to write 
            &cbWritten,   // number of bytes written 
            NULL);        // not overlapped I/O 
        free(pRet);

    } else if (pIn->hdr.nCmd == CECMD_LANGCHANGE) {
        DEBUGSTRCMD(L"GUI recieved CECMD_LANGCHANGE\n");
        _ASSERTE(nDataSize>=4);
        DWORD dwNewKeybLayout = *(DWORD*)pIn->Data;
        if ((gSet.isMonitorConsoleLang & 1) == 1) {
            if (con.dwKeybLayout != dwNewKeybLayout) {
                con.dwKeybLayout = dwNewKeybLayout;
                if (isActive())
                    gConEmu.SwitchKeyboardLayout(dwNewKeybLayout);
            }
        }

    } else if (pIn->hdr.nCmd == CECMD_TABSCMD) {
        // 0: ��������/�������� ����, 1: ������� �� ���������, 2: ������� �� ����������, 3: commit switch
        DEBUGSTRCMD(L"GUI recieved CECMD_TABSCMD\n");
        _ASSERTE(nDataSize>=1);
        DWORD nTabCmd = pIn->Data[0];
        gConEmu.TabCommand(nTabCmd);

    } else if (pIn->hdr.nCmd == CECMD_RESOURCES) {
        DEBUGSTRCMD(L"GUI recieved CECMD_TABSCMD\n");
        _ASSERTE(nDataSize>=6);
        mb_PluginDetected = TRUE; // ��������, ��� � ���� ���� ������ (���� ��� ����� ���� ������)
        mn_FarPID_PluginDetected = *((DWORD*)pIn->Data);
        // 23.06.2009 Maks - ������ ����. ������ �������� � ApplyConsoleInfo
        //Process Add(mn_FarPID_PluginDetected); // �� ������ ������, ����� �� ��� �� � ����� ������?
        wchar_t* pszRes = (wchar_t*)(pIn->Data+4), *pszNext;
        if (*pszRes) {
            EnableComSpec(mn_FarPID_PluginDetected, TRUE);

            pszNext = pszRes + lstrlen(pszRes)+1;
            if (lstrlen(pszRes)>=30) pszRes[30] = 0;
            lstrcpy(ms_EditorRus, pszRes); lstrcat(ms_EditorRus, L" ");
            pszRes = pszNext;

            if (*pszRes) {
                pszNext = pszRes + lstrlen(pszRes)+1;
                if (lstrlen(pszRes)>=30) pszRes[30] = 0;
                lstrcpy(ms_ViewerRus, pszRes); lstrcat(ms_ViewerRus, L" ");
                pszRes = pszNext;

                if (*pszRes) {
                    pszNext = pszRes + lstrlen(pszRes)+1;
                    if (lstrlen(pszRes)>=31) pszRes[31] = 0;
                    lstrcpy(ms_TempPanelRus, pszRes);
                    pszRes = pszNext;
                }
            }
        }
    }

    // ���������� ������
    if (pIn && (LPVOID)pIn != (LPVOID)&in) {
        free(pIn); pIn = NULL;
    }

    //CloseHandle(hPipe);
    return;
}

//#define COPYBUFFERS(v,s) { 
//    nVarSize = s; 
//    if ((lpCur+nVarSize)>lpEnd) { 
//        _ASSERT(FALSE); 
//        return; 
//    } 
//    memmove(&(v), lpCur, nVarSize); 
//    lpCur += nVarSize; 
//}
//#define COPYBUFFER(v) { 
//    COPYBUFFERS(v, sizeof(v)); 
//}

WARNING("��� ����������� ������� � ����������� �������� ����� ������������ Semaphore nor CriticalSection");
void CRealConsole::ApplyConsoleInfo(CESERVER_REQ* pInfo)
{
    _ASSERTE(this!=NULL);
    if (this==NULL) return;

    #ifdef _DEBUG
	int nNeedVer = CESERVER_REQ_VER;
	int nThisVer = pInfo->hdr.nVersion;
	BOOL lbVerOk = nThisVer==nNeedVer;
	_ASSERTE(lbVerOk);
	_ASSERTE(pInfo->hdr.nCmd<100);
    if (!con.bBufferHeight && !mb_BuferModeChangeLocked && pInfo->ConInfo.inf.sbi.srWindow.Top != 0) {
        _ASSERTE(pInfo->ConInfo.inf.sbi.srWindow.Top == 0);
    }
    #endif

	// ���������� ����� ������ ��������
	if (mn_LastProcessedPkt > pInfo->ConInfo.inf.nPacketId)
		return;
    // �� ������ ������ ����� �������� �� ������, ������������� ���������
	// ��� ��� ����� ���� (mn_LastProcessedPkt == pInfo->ConInfo.inf.nPacketId)
	// ���� ����� ������ �� RetrieveConsoleInfo
    mn_LastProcessedPkt = pInfo->ConInfo.inf.nPacketId;

    #ifdef _DEBUG
    wchar_t szDbg[100];
    wsprintfW(szDbg, L"ApplyConsoleInfo(PacketID=%u)\n", ((DWORD*)pInfo->Data)[1]);
    DEBUGSTRPKT(szDbg);
    #endif

    LogPacket(pInfo);

    _ASSERTE(pInfo->hdr.nSize>=(sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_CONINFO_HDR)+2*sizeof(DWORD)));
    if (pInfo->hdr.nSize < (sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_CONINFO_HDR)+2*sizeof(DWORD)))
        return; // Broken packet!

    BOOL bBufRecreated = FALSE;
    // 1
    HWND hWnd = pInfo->ConInfo.inf.hConWnd;
    _ASSERTE(hWnd!=NULL);
    if (hConWnd != hWnd) {
        SetHwnd ( hWnd );
    }
    // 3
    // ����� � ��� �������� �������� �������, ���� ����������
    ProcessUpdate(pInfo->ConInfo.inf.nProcesses, countof(pInfo->ConInfo.inf.nProcesses));

    // ������ ����� ������� ������ - �������� ��������� ���������� ������
    MSectionLock sc;

    // 4
    DWORD dwCiSize = pInfo->ConInfo.inf.dwCiSize;
    if (dwCiSize != 0) {
        _ASSERTE(dwCiSize == sizeof(con.m_ci));
        memmove(&con.m_ci, &pInfo->ConInfo.inf.ci, dwCiSize);
    }
    // 5, 6, 7
    con.m_dwConsoleCP = pInfo->ConInfo.inf.dwConsoleCP;
    con.m_dwConsoleOutputCP = pInfo->ConInfo.inf.dwConsoleOutputCP;
    con.m_dwConsoleMode = pInfo->ConInfo.inf.dwConsoleMode;
    // 8
    DWORD dwSbiSize = pInfo->ConInfo.inf.dwSbiSize;
    int nNewWidth = 0, nNewHeight = 0;
    if (dwSbiSize != 0) {
        MCHKHEAP
        _ASSERTE(dwSbiSize == sizeof(con.m_sbi));
        memmove(&con.m_sbi, &pInfo->ConInfo.inf.sbi, dwSbiSize);
        if (GetConWindowSize(con.m_sbi, nNewWidth, nNewHeight)) {
            if (con.bBufferHeight != (nNewHeight < con.m_sbi.dwSize.Y))
                SetBufferHeightMode(nNewHeight < con.m_sbi.dwSize.Y);
            //  TODO("�������� ���������? ��� ��� ����?");
            if (nNewWidth != con.nTextWidth || nNewHeight != con.nTextHeight) {
                bBufRecreated = TRUE; // ����� �������, ����� �������������
                sc.Lock(&csCON, TRUE);
                WARNING("����� �� ���������������?");
                InitBuffers(nNewWidth*nNewHeight*2);
            }
        }
        if (gSet.AutoScroll)
            con.nTopVisibleLine = con.m_sbi.srWindow.Top;
        MCHKHEAP
    }
    // 9
    DWORD dwCharChanged = pInfo->ConInfo.RgnInfo.dwRgnInfoSize;
    //BOOL  lbDataRecv = FALSE;
    if (dwCharChanged != 0) {
		#ifdef _DEBUG
        _ASSERTE(dwCharChanged >= sizeof(CESERVER_CHAR));
		#endif

        // ���� ����� ��� ���������� (bBufRecreated) ��������� ������ ����� �������� ������������ - ����� ������ ������!
		if (bBufRecreated && con.pConChar && con.pConAttr) {
			DEBUGSTRPKT(L"### Buffer was recreated, via consize changing. Relative packet will be ignored ###\n");
			RetrieveConsoleInfo(0);
		} else
        if (!bBufRecreated && con.pConChar && con.pConAttr) {
            MSectionLock sc2; sc2.Lock(&csCON);

            DEBUGSTRPKT(L"                   *** Relative dump received!\n");
            MCHKHEAP
            CESERVER_CHAR* pch = &pInfo->ConInfo.RgnInfo.RgnInfo;
            //calloc(dwCharChanged,1);
            //COPYBUFFERS(*pch,dwCharChanged);
            _ASSERTE(pch->hdr.nSize==dwCharChanged);
            int nLineLen = pch->hdr.cr2.X - pch->hdr.cr1.X + 1;
            _ASSERTE(nLineLen>0);
            int nLineCount = pch->hdr.cr2.Y - pch->hdr.cr1.Y + 1;
            _ASSERTE(nLineCount>0);
            MCHKHEAP
            _ASSERTE((nLineLen*nLineCount*4+sizeof(CESERVER_CHAR_HDR))==dwCharChanged);
            //COPYBUFFERS(pch->data,dwCharChanged-2*sizeof(COORD));
            //_ASSERTE(!isBufferHeight());
            wchar_t* pszLine = (wchar_t*)(pch->data);
            WORD*    pnLine  = ((WORD*)pch->data)+(nLineLen*nLineCount);
            MCHKHEAP
            // ���������� ������ ���������� � ����� ���������� - ������� ����� �������
            for (int y = pch->hdr.cr1.Y; y <= pch->hdr.cr2.Y; y++) {
                int nLine = y - con.nTopVisibleLine;
                if (nLine < 0 || nLine >= con.nTextHeight)
                    continue; // ��� ������ �� �������� � ������� �������
                int nIdx = pch->hdr.cr1.X + nLine * con.m_sbi.dwSize.X;
                memmove(con.pConChar+nIdx, pszLine, nLineLen*2);
                    pszLine += nLineLen;
                MCHKHEAP
                memmove(con.pConAttr+nIdx, pnLine, nLineLen*2);
                    pnLine += nLineLen;
                MCHKHEAP
            }
			FindPanels();
        } else {
            DEBUGSTRPKT(L"                   *** Expected full dump, but Relative received!\n");
        }
    } else {
        // 10
        DWORD OneBufferSize = pInfo->ConInfo.FullData.dwOneBufferSize;
        if (OneBufferSize != 0) {
            MSectionLock sc2; sc2.Lock(&csCON);

            DEBUGSTRPKT(L"                   *** FULL dump received!\n");
            MCHKHEAP
            if (InitBuffers(OneBufferSize)) {
                MCHKHEAP
                LPBYTE lpCur = (LPBYTE)pInfo->ConInfo.FullData.Data;
                memmove(con.pConChar, lpCur, OneBufferSize); lpCur += OneBufferSize;
                memmove(con.pConAttr, lpCur, OneBufferSize); lpCur += OneBufferSize;
                MCHKHEAP

				FindPanels();
            }
        }
    }

    TODO("�� ����� ������� ������� ����� ������������ - ������ �� �� ��� �����...");
    //_ASSERTE(*con.pConChar!=ucBoxDblVert);
    
    con.nChange2TextWidth = -1;
    con.nChange2TextHeight = -1;

    mb_DataChanged = TRUE;

#ifdef _DEBUG
    wchar_t szCursorInfo[60];
    wsprintfW(szCursorInfo, L"Cursor (X=%i, Y=%i, Vis:%i, H:%i)\n", 
        con.m_sbi.dwCursorPosition.X, con.m_sbi.dwCursorPosition.Y,
        con.m_ci.bVisible, con.m_ci.dwSize);
    DEBUGSTRPKT(szCursorInfo);
    // ������ ��� ������ ���� ���������, � ��� �� ������ ���� ����
    if (con.pConChar) {
        BOOL lbDataValid = TRUE; uint n = 0;
        _ASSERTE(con.nTextWidth == con.m_sbi.dwSize.X);
        uint TextLen = con.nTextWidth * con.nTextHeight;
        while (n<TextLen) {
            if (con.pConChar[n] == 0) {
                lbDataValid = FALSE; break;
            } else if (con.pConChar[n] != L' ') {
                // 0 - ����� ���� ������ ��� �������. ����� ������ ����� �������, ���� �� ����, ���� �� ������
                if (con.pConAttr[n] == 0) {
                    lbDataValid = FALSE; break;
                }
            }
            n++;
        }
    }
    //_ASSERTE(lbDataValid);
#endif

    sc.Unlock();
}

int CRealConsole::GetProcesses(ConProcess** ppPrc)
{
    if (mn_InRecreate) {
        DWORD dwCurTick = GetTickCount();
        DWORD dwDelta = dwCurTick - mn_InRecreate;
        if (dwDelta > CON_RECREATE_TIMEOUT) {
            mn_InRecreate = 0;
        } else if (ppPrc == NULL) {
            if (mn_InRecreate && !mb_ProcessRestarted && mh_ConEmuC) {
                DWORD dwExitCode = 0;
                if (GetExitCodeProcess(mh_ConEmuC, &dwExitCode) && dwExitCode != STILL_ACTIVE) {
                    RecreateProcessStart();
                }
            }
            return 1; // ����� �� ����� Recreate GUI �� �����������
        }
    }


    // ���� ����� ������ ������ ���������� ���������
    if (ppPrc == NULL || mn_ProcessCount == 0) {
        if (ppPrc) *ppPrc = NULL;
        return mn_ProcessCount;
    }

    MSectionLock SPRC; SPRC.Lock(&csPRC);
    
    int dwProcCount = m_Processes.size();

    if (dwProcCount > 0) {
        *ppPrc = (ConProcess*)calloc(dwProcCount, sizeof(ConProcess));
        _ASSERTE((*ppPrc)!=NULL);
        
        for (int i=0; i<dwProcCount; i++) {
            (*ppPrc)[i] = m_Processes[i];
        }
        
    } else {
        *ppPrc = NULL;
    }
    
    SPRC.Unlock();
    return dwProcCount;
}

DWORD CRealConsole::GetProgramStatus()
{
    if (!this)
        return 0;
    return mn_ProgramStatus;
}

DWORD CRealConsole::GetFarStatus()
{
    if (!this)
        return 0;
    if ((mn_ProgramStatus & CES_FARACTIVE) == 0)
        return 0;
    return mn_FarStatus;
}

DWORD CRealConsole::GetServerPID()
{
    if (!this)
        return 0;
    return mn_ConEmuC_PID;
}

DWORD CRealConsole::GetFarPID()
{
    if (!this)
        return 0;

    if ((mn_ProgramStatus & CES_FARACTIVE) == 0)
        return 0;

    return mn_FarPID;
}

// �������� ������ ���������� ��������
void CRealConsole::ProcessUpdateFlags(BOOL abProcessChanged)
{
    //Warning: ������ ���������� ������ �� ProcessAdd/ProcessDelete, �.�. ��� ������ �� ���������

    bool bIsFar=false, bIsTelnet=false, bIsCmd=false;
    DWORD dwPID = 0;
    
    // ������� 16bit ���������� ������ �� WinEvent. ����� �� ��������� ������ ��� ����������,
    // �.�. ������� ntvdm.exe �� �����������, � �������� � ������.
    bool bIsNtvdm = (mn_ProgramStatus & CES_NTVDM) == CES_NTVDM;

    std::vector<ConProcess>::reverse_iterator iter = m_Processes.rbegin();
    while (iter!=m_Processes.rend()) {
        // �������� ������� ConEmuC �� ���������!
        if (iter->ProcessID != mn_ConEmuC_PID) {
            if (!bIsFar && iter->IsFar) bIsFar = true;
            if (!bIsTelnet && iter->IsTelnet) bIsTelnet = true;
            //if (!bIsNtvdm && iter->IsNtvdm) bIsNtvdm = true;
            if (!bIsCmd && iter->IsCmd) bIsCmd = true;
            // 
            if (!dwPID && iter->IsFar)  dwPID = iter->ProcessID;
        }
        iter++;
    }

    TODO("������, �������� cmd.exe ����� ���� ������� � � '����'? �������� �� Update");
    if (bIsCmd && bIsFar) { // ���� � ������� ������� cmd.exe - ������ (������ �����?) ��� ��������� �������
        bIsFar = false; dwPID = 0;
    }
        
    mn_ProgramStatus = 0;
    if (bIsFar) mn_ProgramStatus |= CES_FARACTIVE;
    if (bIsTelnet) mn_ProgramStatus |= CES_TELNETACTIVE;
    if (bIsNtvdm)
        mn_ProgramStatus |= CES_NTVDM;

    mn_ProcessCount = m_Processes.size();
    if (mn_FarPID != dwPID)
        AllowSetForegroundWindow(dwPID);
    mn_FarPID = dwPID;

    if (mn_ProcessCount == 0) {
        if (mn_InRecreate == 0) {
            StopSignal();
        } else if (mn_InRecreate == 1) {
            mn_InRecreate = 2;
        }
    }

    // �������� ������ ��������� � ���� �������� � ��������
    if (abProcessChanged) {
        gConEmu.UpdateProcessDisplay(abProcessChanged);
        TabBar.Refresh(mn_ProgramStatus & CES_FARACTIVE);
    }
}

void CRealConsole::ProcessUpdate(DWORD *apPID, UINT anCount)
{
    MSectionLock SPRC; SPRC.Lock(&csPRC);

    BOOL lbRecreateOk = FALSE;
    if (mn_InRecreate && mn_ProcessCount == 0) {
        // ��� ���� ���-�� ������, ������ �� �������� �����, ������ ������� �����������
        lbRecreateOk = TRUE;
    }
    
    UINT i = 0;
    std::vector<ConProcess>::iterator iter;
    //BOOL bAlive = FALSE;
    BOOL bProcessChanged = FALSE, bProcessNew = FALSE, bProcessDel = FALSE;
    
    // ��������� ��������� �� ��� ��������, ����� ��� ��� ������ 
    iter = m_Processes.begin();
    while (iter != m_Processes.end()) { iter->inConsole = false; iter ++; }
    
    // ���������, ����� �������� ��� ���� � ����� ������
    iter=m_Processes.begin();
    while (iter!=m_Processes.end()) {
        for (i = 0; i < anCount; i++) {
            if (apPID[i] && apPID[i] == iter->ProcessID) {
                iter->inConsole = true;
                apPID[i] = 0; // ��� ��������� ��� �� �����, �� � ��� �����
                break;
            }    
        }
        iter++;
    }
    
    // ���������, ���� �� ���������
    for (i = 0; i < anCount; i++) {
        if (apPID[i]) {
            bProcessNew = TRUE; break;
        }
    }
    iter = m_Processes.begin();
    while (iter != m_Processes.end()) {
        if (iter->inConsole == false) {
            bProcessDel = TRUE; break;
        }
        iter ++;
    }


    

    // ������ ����� �������� ����� �������
    if (bProcessNew || bProcessDel)
    {
        ConProcess cp;
        HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
        _ASSERTE(h!=INVALID_HANDLE_VALUE);
        if (h==INVALID_HANDLE_VALUE) {
            DWORD dwErr = GetLastError();
            wchar_t szError[255];
            wsprintf(szError, L"Can't create process snapshoot, ErrCode=0x%08X", dwErr);
            gConEmu.DnDstep(szError);
            
        } else {
            //Snapshoot ������, �������

            // ����� ����������� ������ - ��������� ��������� �� ��� ��������, ����� ��� ��� ������ 
            iter = m_Processes.begin();
            while (iter != m_Processes.end()) { iter->Alive = false; iter ++; }

            PROCESSENTRY32 p; memset(&p, 0, sizeof(p)); p.dwSize = sizeof(p);
            if (Process32First(h, &p)) 
            {
                do {
                    // ���� �� addPID - �������� � m_Processes
                    if (bProcessNew) {
                        for (i = 0; i < anCount; i++) {
                            if (apPID[i] && apPID[i] == p.th32ProcessID) {
                                if (!bProcessChanged) bProcessChanged = TRUE;
                                memset(&cp, 0, sizeof(cp));
                                cp.ProcessID = apPID[i]; cp.ParentPID = p.th32ParentProcessID;
                                ProcessCheckName(cp, p.szExeFile); //far, telnet, cmd, conemuc, � ��.
                                cp.Alive = true;
                                cp.inConsole = true;

                                SPRC.RelockExclusive(300); // �������������, ���� ��� ��� �� �������
                                m_Processes.push_back(cp);
                            }
                        }
                    }
                    
                    // ���������� ����������� �������� - ��������� ������ Alive
                    // ��������� ��� ��� ��� ���������, ������� ����� ��� ������� �� �������
                    iter = m_Processes.begin();
                    while (iter != m_Processes.end())
                    {
                        if (iter->ProcessID == p.th32ProcessID) {
                            iter->Alive = true;
                            if (!iter->NameChecked)
                            {
                                // ��������, ��� �������� ������ (������������ ��� ��������)
                                if (!bProcessChanged) bProcessChanged = TRUE;
                                //far, telnet, cmd, conemuc, � ��.
                                ProcessCheckName(*iter, p.szExeFile);
                                // ��������� ��������
                                iter->ParentPID = p.th32ParentProcessID;
                            }
                        }
                        iter ++;
                    }
                    
                // �������� �������
                } while (Process32Next(h, &p));
            }
            
            // ������� shapshoot
            SafeCloseHandle(h);
        }
    }
    
    // ������ ��������, ������� ��� ���
    iter = m_Processes.begin();
    while (iter != m_Processes.end())
    {
        if (!iter->Alive || !iter->inConsole) {
            if (!bProcessChanged) bProcessChanged = TRUE;

            SPRC.RelockExclusive(300); // ���� ��� ���� ������������ - ������ ������ FALSE  
            iter = m_Processes.erase(iter);
        } else {
            iter ++;
        }
    }
    
    // �������� ������ ���������� ��������, �������� PID FAR'�, ��������� ���������� ��������� � �������
    ProcessUpdateFlags(bProcessChanged);

    // 
    if (lbRecreateOk)
        mn_InRecreate = 0;
}

//void CRealConsole::ProcessAdd(DWORD addPID)
//{
//    MSectionLock SPRC; SPRC.Lock(&csPRC);
//
//    BOOL lbRecreateOk = FALSE;
//    if (mn_InRecreate && mn_ProcessCount == 0) {
//        lbRecreateOk = TRUE;
//    }
//    
//    std::vector<ConProcess>::iterator iter;
//    BOOL bAlive = FALSE;
//    BOOL bProcessChanged = FALSE;
//    
//    // ����� �� ��� ���� � ������?
//    iter=m_Processes.begin();
//    while (iter!=m_Processes.end()) {
//        if (addPID == iter->ProcessID) {
//            addPID = 0;
//            break;
//        }
//        iter++;
//    }
//
//    
//
//    // ������ ����� �������� ����� �������
//    ConProcess cp;
//    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
//    _ASSERTE(h!=INVALID_HANDLE_VALUE);
//    if (h==INVALID_HANDLE_VALUE) {
//        DWORD dwErr = GetLastError();
//      if (addPID) {
//          // ������ ��������, ��� �� ������� ����� Snapshoot
//          memset(&cp, 0, sizeof(cp));
//          cp.ProcessID = addPID; cp.RetryName = true;
//          wsprintf(cp.Name, L"Can't create snaphoot. ErrCode=0x%08X", dwErr);
//
//          SPRC.RelockExclusive(300);
//          m_Processes.push_back(cp);
//          SPRC.Unlock();
//      }
//        
//    } else {
//        //Snapshoot ������, �������
//
//        // ����� ����������� ������ - ��������� ��������� �� ��� ��������, ����� ��� ��� ������ 
//        iter = m_Processes.begin();
//        while (iter != m_Processes.end()) { iter->Alive = false; iter ++; }
//
//        PROCESSENTRY32 p; memset(&p, 0, sizeof(p)); p.dwSize = sizeof(p);
//        if (Process32First(h, &p)) 
//        {
//            do {
//                // ���� �� addPID - �������� � m_Processes
//                if (addPID && addPID == p.th32ProcessID) {
//                    if (!bProcessChanged) bProcessChanged = TRUE;
//                    memset(&cp, 0, sizeof(cp));
//                    cp.ProcessID = addPID; cp.ParentPID = p.th32ParentProcessID;
//                    ProcessCheckName(cp, p.szExeFile); //far, telnet, cmd, conemuc, � ��.
//                    cp.Alive = true;
//
//                  SPRC.RelockExclusive(300);
//                    m_Processes.push_back(cp);
//                    break;
//                }
//                
//                // ���������� ����������� �������� - ��������� ������ Alive
//                // ��������� ��� ��� ��� ���������, ������� ����� ��� ������� �� �������
//                iter = m_Processes.begin();
//                while (iter != m_Processes.end())
//                {
//                    if (iter->ProcessID == p.th32ProcessID) {
//                        iter->Alive = true;
//                        if (!iter->NameChecked)
//                        {
//                            // ��������, ��� �������� ������ (������������ ��� ��������)
//                            if (!bProcessChanged) bProcessChanged = TRUE;
//                            //far, telnet, cmd, conemuc, � ��.
//                            ProcessCheckName(*iter, p.szExeFile);
//                            // ��������� ��������
//                            iter->ParentPID = p.th32ParentProcessID;
//                        }
//                    }
//                    iter ++;
//                }
//                
//            // �������� �������
//            } while (Process32Next(h, &p));
//        }
//        
//        // ������ ��������, ������� ��� ���
//        iter = m_Processes.begin();
//        while (iter != m_Processes.end())
//        {
//            if (!iter->Alive) {
//                if (!bProcessChanged) bProcessChanged = TRUE;
//              SPRC.RelockExclusive(300);
//                iter = m_Processes.erase(iter);
//            } else {
//                iter ++;
//            }
//        }
//        
//        // ������� shapshoot
//        SafeCloseHandle(h);
//    }
//    
//    // �������� ������ ���������� ��������, �������� PID FAR'�, ��������� ���������� ��������� � �������
//    ProcessUpdateFlags(bProcessChanged);
//
//    // 
//    if (lbRecreateOk)
//        mn_InRecreate = 0;
//}

//void CRealConsole::ProcessDelete(DWORD addPID)
//{
//    MSectionLock SPRC; SPRC.Lock(&csPRC, TRUE);
//
//    BOOL bProcessChanged = FALSE;
//    std::vector<ConProcess>::iterator iter=m_Processes.begin();
//    while (iter!=m_Processes.end()) {
//        if (addPID == iter->ProcessID) {
//            m_Processes.erase(iter);
//            bProcessChanged = TRUE;
//            break;
//        }
//        iter++;
//    }
//
//    // �������� ������ ���������� ��������, �������� PID FAR'�, ��������� ���������� ��������� � �������
//    ProcessUpdateFlags(bProcessChanged);
//
//  SPRC.Unlock();
//    
//    if (mn_InRecreate && mn_ProcessCount == 0 && !mb_ProcessRestarted) {
//        RecreateProcessStart();
//    }
//}

void CRealConsole::ProcessCheckName(struct ConProcess &ConPrc, LPWSTR asFullFileName)
{
    wchar_t* pszSlash = _tcsrchr(asFullFileName, _T('\\'));
    if (pszSlash) pszSlash++; else pszSlash=asFullFileName;
    int nLen = _tcslen(pszSlash);
    if (nLen>=63) pszSlash[63]=0;
    lstrcpyW(ConPrc.Name, pszSlash);

    ConPrc.IsFar = lstrcmpi(ConPrc.Name, _T("far.exe"))==0;

    ConPrc.IsNtvdm = lstrcmpi(ConPrc.Name, _T("ntvdm.exe"))==0;

    ConPrc.IsTelnet = lstrcmpi(ConPrc.Name, _T("telnet.exe"))==0;
    
    TODO("��� ������� �� ������������, � �� ��������� �������� conemuc, �� �������� ������� ��� FAR, ��� ������� �������� ������, ����� GUI ���������� � ���� �������");
    ConPrc.IsCmd = lstrcmpi(ConPrc.Name, _T("cmd.exe"))==0 || lstrcmpi(ConPrc.Name, _T("conemuc.exe"))==0;

    ConPrc.NameChecked = true;
}


BOOL CRealConsole::RetrieveConsoleInfo(UINT anWaitSize)
{
    TODO("!!! WinEvent ����� ��������� � ConEmu. �������� ������ �������� �� ��������� ������� ����� ������������ ��� ������� ConEmuC");

    TODO("!!! ���������� ������� ������, ��� ������ ����� CECMD_GETFULLINFO/CECMD_GETSHORTINFO");

    //BOOL lbRc = FALSE;
    HANDLE hPipe = NULL; 
    CESERVER_REQ_HDR in = {0};
	CESERVER_REQ *pOut = NULL;
    BYTE cbReadBuf[512];
    BOOL fSuccess;
    DWORD cbRead, dwMode, dwErr;


    // Try to open a named pipe; wait for it, if necessary. 
    while (1) 
    { 
      hPipe = CreateFile( 
         ms_ConEmuC_Pipe,// pipe name 
         GENERIC_READ |  // read and write access 
         GENERIC_WRITE, 
         0,              // no sharing 
         NULL,           // default security attributes
         OPEN_EXISTING,  // opens existing pipe 
         0,              // default attributes 
         NULL);          // no template file 

      // Break if the pipe handle is valid. 
      if (hPipe != INVALID_HANDLE_VALUE) 
         break; 

      // Exit if an error other than ERROR_PIPE_BUSY occurs. 
      dwErr = GetLastError();
      if (dwErr != ERROR_PIPE_BUSY) 
      {
        TODO("���������, ���� �������� ���� � ����� ������, �� ������ ���� ��� mh_ConEmuC");
        dwErr = WaitForSingleObject(mh_ConEmuC, 100);
        if (dwErr == WAIT_OBJECT_0)
            return FALSE;
        continue;
        //DisplayLastError(L"Could not open pipe", dwErr);
        //return 0;
      }

      // All pipe instances are busy, so wait for 1 second.
      if (!WaitNamedPipe(ms_ConEmuC_Pipe, 1000) ) 
      {
        dwErr = WaitForSingleObject(mh_ConEmuC, 100);
        if (dwErr == WAIT_OBJECT_0) {
            DEBUGSTRPKT(L" - FAILED!\n");
            return FALSE;
        }
        //DisplayLastError(L"WaitNamedPipe failed"); 
        //return 0;
      }
    } 

    // The pipe connected; change to message-read mode. 
    dwMode = PIPE_READMODE_MESSAGE; 
    fSuccess = SetNamedPipeHandleState( 
      hPipe,    // pipe handle 
      &dwMode,  // new pipe mode 
      NULL,     // don't set maximum bytes 
      NULL);    // don't set maximum time 
    if (!fSuccess) 
    {
      DEBUGSTRPKT(L" - FAILED!\n");
      DisplayLastError(L"SetNamedPipeHandleState failed");
      CloseHandle(hPipe);
      return 0;
    }

    in.nSize = sizeof(CESERVER_REQ_HDR);
    in.nVersion = CESERVER_REQ_VER;
    in.nCmd  = CECMD_GETFULLINFO;
    in.nSrcThreadId = GetCurrentThreadId();

    // Send a message to the pipe server and read the response. 
    fSuccess = TransactNamedPipe( 
      hPipe,                  // pipe handle 
      &in,                    // message to server
      in.nSize,               // message length 
      cbReadBuf,              // buffer to receive reply
      sizeof(cbReadBuf),      // size of read buffer
      &cbRead,                // bytes read
      NULL);                  // not overlapped 

    if (!fSuccess && (GetLastError() != ERROR_MORE_DATA)) 
    {
      DEBUGSTRPKT(L" - FAILED!\n");
      DisplayLastError(L"TransactNamedPipe failed"); 
      CloseHandle(hPipe);
      return 0;
    }

    // ���� �� ������� ������, ������ ��������� �� �����
    pOut = (CESERVER_REQ*)cbReadBuf;
    if (pOut->hdr.nVersion != CESERVER_REQ_VER) {
      gConEmu.ShowOldCmdVersion(pOut->hdr.nCmd, pOut->hdr.nVersion);
      CloseHandle(hPipe);
      return 0;
    }

    int nAllSize = *((DWORD*)cbReadBuf);
    if (nAllSize == 0) {
       DEBUGSTRPKT(L" - FAILED!\n");
       DisplayLastError(L"Empty data recieved from server", 0);
       CloseHandle(hPipe);
       return 0;
    }
    
  	if (nAllSize > (int)sizeof(cbReadBuf))
  	{
      pOut = (CESERVER_REQ*)calloc(nAllSize,1);
      _ASSERTE(pOut!=NULL);
      memmove(pOut, cbReadBuf, cbRead);
      _ASSERTE(pOut->hdr.nVersion==CESERVER_REQ_VER);

      LPBYTE ptrData = ((LPBYTE)pOut)+cbRead;
      nAllSize -= cbRead;

      while(nAllSize>0)
      { 
        //_tprintf(TEXT("%s\n"), chReadBuf);

        // Break if TransactNamedPipe or ReadFile is successful
        if(fSuccess)
           break;

        // Read from the pipe if there is more data in the message.
        fSuccess = ReadFile( 
           hPipe,      // pipe handle 
           ptrData,    // buffer to receive reply 
           nAllSize,   // size of buffer 
           &cbRead,    // number of bytes read 
           NULL);      // not overlapped 

        // Exit if an error other than ERROR_MORE_DATA occurs.
        if( !fSuccess && (GetLastError() != ERROR_MORE_DATA)) 
           break;
        ptrData += cbRead;
        nAllSize -= cbRead;
      }

      TODO("����� ���������� ASSERT, ���� ������� ���� ������� � �������� ������");
      _ASSERTE(nAllSize==0);
  }

    CloseHandle(hPipe);

	BOOL lbRc = FALSE;

    // Warning. � ��������� ������� ��� ����� ���� ������ ��������� �� �����, � �� ���������� ������!
	if (pOut) {
		// ���������� ���������, ����� ����� ��� ��������� ��������� - �� �������� ���������� ����������
		mn_LastProcessedPkt = pOut->ConInfo.inf.nPacketId;

		#ifdef _DEBUG
		wchar_t szDbg[128]; wsprintf(szDbg, L"RetrieveConsoleInfo received PktID=%i, ConHeight=%i\n",
			pOut->ConInfo.inf.nPacketId, pOut->ConInfo.inf.sbi.dwSize.Y);
		DEBUGSTRPKT(szDbg);
		#endif
		if (GetCurrentThreadId() == mn_MonitorThreadID) {
			DEBUGSTRPKT(L"RetrieveConsoleInfo is applying packet\n");
			ApplyConsoleInfo ( pOut );
			lbRc = TRUE;
		} else {
			// ����������� MonitorThread -- �� �����. ��� ������� PushPacket
			//SetEvent(mh_MonitorThreadEvent);
			_ASSERTE((LPVOID)pOut != (LPVOID)cbReadBuf);
			if ((LPVOID)pOut != (LPVOID)cbReadBuf) {
				BOOL lbSizeOk = (anWaitSize == 0) || (pOut->ConInfo.inf.sbi.dwSize.Y == (int)anWaitSize);
				if (lbSizeOk) {
					PushPacket(pOut);
					pOut = NULL;
					lbRc = TRUE;
				} else {
					DEBUGSTRPKT(L"RetrieveConsoleInfo failed, incorrect console height\n");
				}
			} else {
				DEBUGSTRPKT(L"RetrieveConsoleInfo failed, (LPVOID)pOut == (LPVOID)cbReadBuf\n");
			}
		}
	}

    // ���������� ������
  // Warning. � ��������� ������� ��� ����� ���� ������ ��������� �� �����, � �� ���������� ������!
  if (pOut && (LPVOID)pOut != (LPVOID)cbReadBuf) {
      free(pOut);
  }
  pOut = NULL;


    return lbRc;
}

BOOL CRealConsole::GetConWindowSize(const CONSOLE_SCREEN_BUFFER_INFO& sbi, int& nNewWidth, int& nNewHeight, BOOL* pbBufferHeight/*=NULL*/)
{
    // ������� ���������� ������ ����, �� ���� ����� ����� ���� ������
    WARNING("���������, ����� ��� ��� ���������� GetConWindowSize ����� �������� ����� BufferHeight?");

    nNewWidth  = sbi.dwSize.X; // sbi.srWindow.Right - sbi.srWindow.Left + 1;
    BOOL lbBufferHeight = con.bBufferHeight;
    // �������� ������� ���������
    if (!lbBufferHeight) {
        if (sbi.dwSize.Y > sbi.dwMaximumWindowSize.Y)
        {
            lbBufferHeight = TRUE; // ����������� ��������� ���������
        }
    }
    if (lbBufferHeight) {
        if (sbi.srWindow.Top == 0
            && sbi.dwSize.Y == (sbi.srWindow.Bottom + 1)
            )
        {
            lbBufferHeight = FALSE; // ����������� ���������� ���������
        }
    }

    // ������ ���������� �������
    if (!lbBufferHeight) {
        nNewHeight =  sbi.dwSize.Y;
    } else {
        // ��� ����� ������ �� ����� ����� �������
        if ((sbi.srWindow.Bottom - sbi.srWindow.Top + 1) < MIN_CON_HEIGHT)
            nNewHeight = con.nTextHeight;
        else
            nNewHeight = sbi.srWindow.Bottom - sbi.srWindow.Top + 1;
    }
    WARNING("����� ����� ��������� ���������, ���� nNewHeight ������ - �������� ����� BufferHeight");


    _ASSERTE(nNewWidth>=MIN_CON_WIDTH && nNewHeight>=MIN_CON_HEIGHT);

    if (!nNewWidth || !nNewHeight) {
        Assert(nNewWidth && nNewHeight);
        return FALSE;
    }

    //if (nNewWidth < sbi.dwSize.X)
    //    nNewWidth = sbi.dwSize.X;

    return TRUE;
}

BOOL CRealConsole::InitBuffers(DWORD OneBufferSize)
{
    // ��� ������� ������ ���������� ������ � MonitorThread.
    // ����� ���������� ������ �� �����������
    #ifdef _DEBUG
    DWORD dwCurThId = GetCurrentThreadId();
    #endif
    _ASSERTE(mn_MonitorThreadID==0 || dwCurThId==mn_MonitorThreadID);

    BOOL lbRc = FALSE;
    int nNewWidth = 0, nNewHeight = 0;

    if (!GetConWindowSize(con.m_sbi, nNewWidth, nNewHeight))
        return FALSE;

    if (OneBufferSize) {
        if ((nNewWidth * nNewHeight * sizeof(*con.pConChar)) != OneBufferSize) {
            // ��� ����� ��������� �� ����� �������
            nNewWidth = nNewWidth;
            //_ASSERTE((nNewWidth * nNewHeight * sizeof(*con.pConChar)) == OneBufferSize);
        }
        if ((nNewWidth * nNewHeight * sizeof(*con.pConChar)) != OneBufferSize)
            return FALSE;
    }

    // ���� ��������� ��������� ��� ������� (��������) ������
    if (!con.pConChar || (con.nTextWidth*con.nTextHeight) < (nNewWidth*nNewHeight))
    {
        MSectionLock sc; sc.Lock(&csCON, TRUE);

        //try {
        if (con.pConChar)
            { Free(con.pConChar); con.pConChar = NULL; }
        if (con.pConAttr)
            { Free(con.pConAttr); con.pConAttr = NULL; }

        MCHKHEAP

        con.pConChar = (TCHAR*)Alloc((nNewWidth * nNewHeight * 2), sizeof(*con.pConChar));
        con.pConAttr = (WORD*)Alloc((nNewWidth * nNewHeight * 2), sizeof(*con.pConAttr));
        //} catch(...) {
        //    con.pConChar = NULL;
        //    con.pConAttr = NULL;
        //}

        sc.Unlock();

        _ASSERTE(con.pConChar!=NULL);
        _ASSERTE(con.pConAttr!=NULL);

        MCHKHEAP

        lbRc = con.pConChar!=NULL && con.pConAttr!=NULL;
    } else if (con.nTextWidth!=nNewWidth || con.nTextHeight!=nNewHeight) {
        MCHKHEAP
        MSectionLock sc; sc.Lock(&csCON);
        memset(con.pConChar, 0, (nNewWidth * nNewHeight * 2) * sizeof(*con.pConChar));
        memset(con.pConAttr, 0, (nNewWidth * nNewHeight * 2) * sizeof(*con.pConAttr));
        sc.Unlock();
        MCHKHEAP

        lbRc = TRUE;
    } else {
        lbRc = TRUE;
    }
    MCHKHEAP

    con.nTextWidth = nNewWidth;
    con.nTextHeight = nNewHeight;

	FindPanels(TRUE);

    //InitDC(false,true);

    return lbRc;
}

void CRealConsole::ShowConsole(int nMode) // -1 Toggle, 0 - Hide, 1 - Show
{
    if (this == NULL) return;
    if (!hConWnd) return;
    
    if (nMode == -1) {
        nMode = IsWindowVisible(hConWnd) ? 0 : 1;
    }
    
    if (nMode == 1)
    {
        isShowConsole = true;
        //ShowWindow(hConWnd, SW_SHOWNORMAL);
        //if (setParent) SetParent(hConWnd, 0);
        RECT rcCon, rcWnd; GetWindowRect(hConWnd, &rcCon); GetWindowRect(ghWnd, &rcWnd);
        //if (!IsDebuggerPresent())
        /* */ SetWindowPos(hConWnd, HWND_TOPMOST, 
            rcWnd.right-rcCon.right+rcCon.left, rcWnd.bottom-rcCon.bottom+rcCon.top,
            0,0, SWP_NOSIZE|SWP_SHOWWINDOW);
        EnableWindow(hConWnd, true);
        SetFocus(ghWnd);
        //if (setParent) SetParent(hConWnd, 0);
    }
    else
    {
        isShowConsole = false;
        //if (!gSet.isConVisible)
        ShowWindow(hConWnd, SW_HIDE);
        //if (setParent) SetParent(hConWnd, setParent2 ? ghWnd : ghWndDC);
        //if (!gSet.isConVisible)
        //EnableWindow(hConWnd, false); -- �������� �� �����
        SetFocus(ghWnd);
    }
}

void CRealConsole::SetHwnd(HWND ahConWnd)
{
    hConWnd = ahConWnd;
    //if (mb_Detached && ahConWnd) // �� ����������, � �� ���� ����� �� ������!
    //  mb_Detached = FALSE; // ����� ������, �� ��� ������������
        
    mb_ProcessRestarted = FALSE; // ������� ��������

    if (ms_VConServer_Pipe[0] == 0) {
        // �������� ���������� ��� ����������, ����� �� ������� ���������
        wsprintfW(ms_VConServer_Pipe, CEGUIATTACHED, (DWORD)hConWnd);
        mh_GuiAttached = CreateEvent(NULL, TRUE, FALSE, ms_VConServer_Pipe);
        _ASSERTE(mh_GuiAttached!=NULL);

        // ��������� ��������� ����
        wsprintf(ms_VConServer_Pipe, CEGUIPIPENAME, L".", (DWORD)hConWnd); //��� mn_ConEmuC_PID
        mh_ServerSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
        for (int i=0; i<MAX_SERVER_THREADS; i++) {
            mn_ServerThreadsId[i] = 0;
            mh_ServerThreads[i] = CreateThread(NULL, 0, ServerThread, (LPVOID)this, 0, &mn_ServerThreadsId[i]);
            _ASSERTE(mh_ServerThreads[i]!=NULL);
        }

        // ����� ConEmuC ����, ��� �� ������
        SetEvent(mh_GuiAttached);
    }

    if (gSet.isConVisible)
        ShowConsole(1); // ���������� ����������� ���� ���� AlwaysOnTop

    if ((gSet.isMonitorConsoleLang & 2) == 2) // ���� Layout �� ��� �������
        SwitchKeyboardLayout(gConEmu.GetActiveKeyboardLayout());

    if (isActive()) {
        ghConWnd = hConWnd;
        // ����� ����� ���� ����� ����� ���� �� ������ �������
        SetWindowLongPtr(ghWnd, GWLP_USERDATA, (LONG_PTR)hConWnd);
    }
}

void CRealConsole::OnFocus(BOOL abFocused)
{
    if (!this) return;

    if ((mn_Focused == -1) ||
        ((mn_Focused == 0) && abFocused) ||
        ((mn_Focused == 1) && !abFocused))
    {
#ifdef _DEBUG
        if (abFocused) {
            DEBUGSTRINPUT(L"--Get focus\n")
        } else {
            DEBUGSTRINPUT(L"--Loose focus\n")
        }
#endif
        INPUT_RECORD r = {FOCUS_EVENT};
        r.Event.FocusEvent.bSetFocus = abFocused;
        PostConsoleEvent(&r);

        mn_Focused = abFocused ? 1 : 0;
    }
}

void CRealConsole::CreateLogFiles()
{
    if (!m_UseLogs || mh_LogInput) return; // ���

    DWORD dwErr = 0;
    wchar_t szFile[MAX_PATH+64], *pszDot;
    _ASSERTE(gConEmu.ms_ConEmuExe[0]);
    lstrcpyW(szFile, gConEmu.ms_ConEmuExe);
    if ((pszDot = wcsrchr(szFile, L'\\')) == NULL) {
        DisplayLastError(L"wcsrchr failed!");
        return; // ������
    }
    *pszDot = 0;

    mpsz_LogPackets = (wchar_t*)calloc(pszDot - szFile + 64, 2);
    lstrcpyW(mpsz_LogPackets, szFile);
    wsprintf(mpsz_LogPackets+(pszDot-szFile), L"\\ConEmu-recv-%i-%%i.con", mn_ConEmuC_PID); // ConEmu-recv-<ConEmuC_PID>-<index>.con

    wsprintfW(pszDot, L"\\ConEmu-input-%i.log", mn_ConEmuC_PID);
    

    mh_LogInput = CreateFileW ( szFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (mh_LogInput == INVALID_HANDLE_VALUE) {
        mh_LogInput = NULL;
        dwErr = GetLastError();
        wprintf(L"CreateFile failed! ErrCode=0x%08X\n%s\n", dwErr, szFile);
        return;
    }

    mpsz_LogInputFile = wcsdup(szFile);
    // OK, ��� �������
}

void CRealConsole::LogString(LPCSTR asText)
{
    if (!this) return;
    if (!asText) return;
    DWORD dwLen = strlen(asText);
    if (dwLen)
        WriteFile(mh_LogInput, asText, dwLen, &dwLen, 0);
    WriteFile(mh_LogInput, "\r\n", 2, &dwLen, 0);
    FlushFileBuffers(mh_LogInput);
}

void CRealConsole::LogInput(INPUT_RECORD* pRec)
{
    if (!mh_LogInput) return;

    char szInfo[192] = {0};

    SYSTEMTIME st; GetLocalTime(&st);
    wsprintfA(szInfo, "%i:%02i:%02i.%03i ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    char *pszAdd = szInfo+strlen(szInfo);

    switch (pRec->EventType) {
    /*case FOCUS_EVENT: wsprintfA(pszAdd, "FOCUS_EVENT(%i)\r\n", pRec->Event.FocusEvent.bSetFocus);
        break;*/
    case MOUSE_EVENT: wsprintfA(pszAdd, "MOUSE_EVENT\r\n");
        {
            wsprintfA(pszAdd, "Mouse: {%ix%i} Btns:{", pRec->Event.MouseEvent.dwMousePosition.X, pRec->Event.MouseEvent.dwMousePosition.Y);
            pszAdd += strlen(pszAdd);
            if (pRec->Event.MouseEvent.dwButtonState & 1) strcat(pszAdd, "L");
            if (pRec->Event.MouseEvent.dwButtonState & 2) strcat(pszAdd, "R");
            if (pRec->Event.MouseEvent.dwButtonState & 4) strcat(pszAdd, "M1");
            if (pRec->Event.MouseEvent.dwButtonState & 8) strcat(pszAdd, "M2");
            if (pRec->Event.MouseEvent.dwButtonState & 0x10) strcat(pszAdd, "M3");
            strcat(pszAdd, "} "); pszAdd += strlen(pszAdd);
            wsprintfA(pszAdd, "KeyState: 0x%08X ", pRec->Event.MouseEvent.dwControlKeyState);
            if (pRec->Event.MouseEvent.dwEventFlags & 0x01) strcat(pszAdd, "|MOUSE_MOVED");
            if (pRec->Event.MouseEvent.dwEventFlags & 0x02) strcat(pszAdd, "|DOUBLE_CLICK");
            if (pRec->Event.MouseEvent.dwEventFlags & 0x04) strcat(pszAdd, "|MOUSE_WHEELED");
            if (pRec->Event.MouseEvent.dwEventFlags & 0x08) strcat(pszAdd, "|MOUSE_HWHEELED");
            strcat(pszAdd, "\r\n");
            
        } break;
    case KEY_EVENT:
        {
            char chAcp; WideCharToMultiByte(CP_ACP, 0, &pRec->Event.KeyEvent.uChar.UnicodeChar, 1, &chAcp, 1, 0,0);
            /* */ wsprintfA(pszAdd, "%c %s count=%i, VK=%i, SC=%i, CH=%i, State=0x%08x %s\r\n",
                chAcp ? chAcp : ' ',
                pRec->Event.KeyEvent.bKeyDown ? "Down," : "Up,  ",
                pRec->Event.KeyEvent.wRepeatCount,
                pRec->Event.KeyEvent.wVirtualKeyCode,
                pRec->Event.KeyEvent.wVirtualScanCode,
                pRec->Event.KeyEvent.uChar.UnicodeChar,
                pRec->Event.KeyEvent.dwControlKeyState,
                (pRec->Event.KeyEvent.dwControlKeyState & ENHANCED_KEY) ?
                "<Enhanced>" : "");
        } break;
    }

    if (*pszAdd) {
        DWORD dwLen = 0;
        WriteFile(mh_LogInput, szInfo, strlen(szInfo), &dwLen, 0);
        FlushFileBuffers(mh_LogInput);
    }
}

void CRealConsole::CloseLogFiles()
{
    SafeCloseHandle(mh_LogInput);
    if (mpsz_LogInputFile) {
        DeleteFile(mpsz_LogInputFile);
        free(mpsz_LogInputFile); mpsz_LogInputFile = NULL;
    }
    if (mpsz_LogPackets) {
        wchar_t szMask[MAX_PATH*2]; wcscpy(szMask, mpsz_LogPackets);
        wchar_t *psz = wcsrchr(szMask, L'%');
        if (psz) {
            wcscpy(psz, L"*.*");
            psz = wcsrchr(szMask, L'\\'); 
            if (psz) {
                psz++;
                WIN32_FIND_DATA fnd;
                HANDLE hFind = FindFirstFile(szMask, &fnd);
                if (hFind != INVALID_HANDLE_VALUE) {
                    do {
                        if ((fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0) {
                            wcscpy(psz, fnd.cFileName);
                            DeleteFile(szMask);
                        }
                    } while (FindNextFile(hFind, &fnd));
                    FindClose(hFind);
                }
            }
        }
        free(mpsz_LogPackets); mpsz_LogPackets = NULL;
    }
}

void CRealConsole::LogPacket(CESERVER_REQ* pInfo)
{
    if (!mpsz_LogPackets || m_UseLogs!=2) return;

    wchar_t szPath[MAX_PATH];
    wsprintf(szPath, mpsz_LogPackets, ++mn_LogPackets);

    HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD dwSize = 0;
        WriteFile(hFile, pInfo, pInfo->hdr.nSize, &dwSize, 0);
        CloseHandle(hFile);
    }
}



// ������� � ������� ������ �� ��������
BOOL CRealConsole::RecreateProcess(RConStartArgs *args)
{
    if (!this)
        return false;
        
    _ASSERTE(hConWnd && mh_ConEmuC);

    if (!hConWnd || !mh_ConEmuC) {
        Box(_T("Console was not created (CRealConsole::SetConsoleSize)"));
        return false; // ������� ���� �� �������?
    }
    if (mn_InRecreate) {
        Box(_T("Console already in recreate..."));
        return false;
    }
    
    if (args->pszSpecialCmd && *args->pszSpecialCmd) {
        if (m_Args.pszSpecialCmd) Free(m_Args.pszSpecialCmd);
        int nLen = lstrlenW(args->pszSpecialCmd);
        m_Args.pszSpecialCmd = (wchar_t*)Alloc(nLen+1,2);
        if (!m_Args.pszSpecialCmd) {
            Box(_T("Can't allocate memory..."));
            return FALSE;
        }
        lstrcpyW(m_Args.pszSpecialCmd, args->pszSpecialCmd);
    }

	m_Args.bRunAsAdministrator = args->bRunAsAdministrator;

    //DWORD nWait = 0;

    mb_ProcessRestarted = FALSE;
    mn_InRecreate = GetTickCount();
    if (!mn_InRecreate) {
        DisplayLastError(L"GetTickCount failed");
        return false;
    }

    CloseConsole();

    return true;

    // ���-�� ��� �� �����... ����� �������� ����������
#ifdef xxxxxx
    bool lbRc = false;
    BOOL fSuccess = FALSE;
    DWORD dwRead = 0;
    CESERVER_REQ In = {0};
    HANDLE hPipe = NULL;
    DWORD dwErr = 0;

    In.nSize = sizeof(In);
    In.nVersion = CESERVER_REQ_VER;
    In.nCmd = CECMD_RECREATE;


    //fSuccess = CallNamedPipe(ms_ConEmuC_Pipe, pIn, pIn->hdr.nSize, pOut, pOut->hdr.nSize, &dwRead, 500);

    // Try to open a named pipe; wait for it, if necessary. 
    while (1) 
    { 
      hPipe = CreateFile( ms_ConEmuC_Pipe, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
      // Break if the pipe handle is valid. 
      if (hPipe != INVALID_HANDLE_VALUE) 
         break; 
      // Exit if an error other than ERROR_PIPE_BUSY occurs. 
      dwErr = GetLastError();
      if (dwErr != ERROR_PIPE_BUSY) 
      {
        TODO("���������, ���� �������� ���� � ����� ������, �� ������ ���� ��� mh_ConEmuC");
        dwErr = WaitForSingleObject(mh_ConEmuC, 100);
        if (dwErr = WAIT_OBJECT_0)
            return FALSE;
        continue;
      }

      // All pipe instances are busy, so wait for 1 second.
      if (!WaitNamedPipe(ms_ConEmuC_Pipe, 300) ) 
      {
        dwErr = WaitForSingleObject(mh_ConEmuC, 100);
        if (dwErr = WAIT_OBJECT_0) {
            DEBUGSTRPROC(L" - FAILED!\n");
            return false;
        }
      }
    } 

    // The pipe connected; change to message-read mode. 
    DWORD dwMode = PIPE_READMODE_MESSAGE; 
    fSuccess = SetNamedPipeHandleState( 
      hPipe,    // pipe handle 
      &dwMode,  // new pipe mode 
      NULL,     // don't set maximum bytes 
      NULL);    // don't set maximum time 
    if (!fSuccess) 
    {
      DEBUGSTRPROC(L" - FAILED!\n");
      DisplayLastError(L"SetNamedPipeHandleState failed");
      CloseHandle(hPipe);
      return false;
    }
    
    mn_InRecreate = 1;

    // Send a message to the pipe server and read the response. 
    DWORD cbWritten=0;
    lbRc = WriteFile( hPipe, &In, In.nSize, &cbWritten, NULL);
    
    SafeCloseHandle ( hPipe );

    return lbRc;
#endif
}

// � ��������� �� ������
BOOL CRealConsole::RecreateProcessStart()
{
    bool lbRc = false;
    if (mn_InRecreate && mn_ProcessCount == 0 && !mb_ProcessRestarted) {
        mb_ProcessRestarted = TRUE;
        if ((mn_ProgramStatus & CES_NTVDM) == CES_NTVDM) {
        	mn_ProgramStatus = 0; mb_IgnoreCmdStop = FALSE;
        	// ��� ������������ ������������ 16������ �����, ����� ������������
        	if (!PreInit())
        		return FALSE;
        } else {
        	mn_ProgramStatus = 0; mb_IgnoreCmdStop = FALSE;
        }
        
        StopThread(TRUE/*abRecreate*/);
        
        ResetEvent(mh_TermEvent);
        hConWnd = NULL;
        ms_VConServer_Pipe[0] = 0;
        SafeCloseHandle(mh_ServerSemaphore);
        SafeCloseHandle(mh_GuiAttached);

        if (!StartProcess()) {
            mn_InRecreate = 0;
            mb_ProcessRestarted = FALSE;
            
        } else {
            ResetEvent(mh_TermEvent);
            lbRc = StartMonitorThread();
        }
    }
    return lbRc;
}

// nWidth � nHeight ��� �������, ������� ����� �������� VCon (��� ����� ��� �� ������������ �� ���������?
void CRealConsole::GetData(wchar_t* pChar, WORD* pAttr, int nWidth, int nHeight)
{
    DWORD cbDstBufSize = nWidth * nHeight * 2;
    DWORD cwDstBufSize = nWidth * nHeight;

    _ASSERT(nWidth != 0 && nHeight != 0);
    if (nWidth == 0 || nHeight == 0)
        return;
    
    MSectionLock csData; csData.Lock(&csCON);
    HEAPVAL

    wchar_t wSetChar = L' ';
    WORD    wSetAttr = 7;
    #ifdef _DEBUG
    wSetChar = (wchar_t)8776; wSetAttr = 12;
    #endif

    if (!con.pConChar || !con.pConAttr) {
        wmemset(pChar, wSetChar, cwDstBufSize);
        wmemset((wchar_t*)pAttr, wSetAttr, cbDstBufSize);
    } else if (nWidth == con.nTextWidth && nHeight == con.nTextHeight) {
        TODO("�� ����� ������� ������� ����� ������������ - ������ �� �� ��� �����...");
        //_ASSERTE(*con.pConChar!=ucBoxDblVert);
        memmove(pChar, con.pConChar, cbDstBufSize);
        memmove(pAttr, con.pConAttr, cbDstBufSize);
    } else {
        TODO("�� ����� ������� ������� ����� ������������ - ������ �� �� ��� �����...");
        //_ASSERTE(*con.pConChar!=ucBoxDblVert);

        int nYMax = min(nHeight,con.nTextHeight);
        wchar_t *pszDst = pChar, *pszSrc = con.pConChar;
        WORD    *pnDst = pAttr, *pnSrc = con.pConAttr;
        DWORD cbDstLineSize = nWidth * 2;
        DWORD cnSrcLineLen = con.nTextWidth;
        DWORD cbSrcLineSize = cnSrcLineLen * 2;
        _ASSERTE(con.nTextWidth == con.m_sbi.dwSize.X);
        DWORD cbLineSize = min(cbDstLineSize,cbSrcLineSize);
        int nCharsLeft = max(0, (nWidth-con.nTextWidth));
        int nY;

        for (nY = 0; nY < nYMax; nY++) {
            memmove(pszDst, pszSrc, cbLineSize);
            if (nCharsLeft > 0)
                wmemset(pszDst+cnSrcLineLen, wSetChar, nCharsLeft);
            pszDst += nWidth; pszSrc += con.nTextWidth;

            memmove(pnDst, pnSrc, cbLineSize);
            if (nCharsLeft > 0)
                wmemset((wchar_t*)pnDst+cnSrcLineLen, wSetAttr, nCharsLeft);
            pnDst += nWidth; pnSrc += con.nTextWidth;
        }
        for (nY = nYMax; nY < nHeight; nY++) {
            wmemset(pszDst, wSetChar, nWidth);
            pszDst += nWidth;

            wmemset((wchar_t*)pnDst, wSetAttr, nWidth);
            pnDst += nWidth;
        }
    }
    HEAPVAL
    csData.Unlock();
}

void CRealConsole::OnActivate(int nNewNum, int nOldNum)
{
    if (!this)
        return;

    _ASSERTE(isActive());

    // ����� ����� ���� ����� ����� ���� �� ������ �������
    SetWindowLongPtr(ghWnd, GWLP_USERDATA, (LONG_PTR)hConWnd);
    ghConWnd = hConWnd;

    //if (mh_MonitorThread) SetThreadPriority(mh_MonitorThread, THREAD_PRIORITY_ABOVE_NORMAL);

    if ((gSet.isMonitorConsoleLang & 2) == 2) // ���� Layout �� ��� �������
        SwitchKeyboardLayout(gConEmu.GetActiveKeyboardLayout());
    else if (con.dwKeybLayout && (gSet.isMonitorConsoleLang & 1) == 1) // ������� �� Layout'�� � �������
        gConEmu.SwitchKeyboardLayout(con.dwKeybLayout);

    WARNING("�� �������� ���������� ���������");
    gConEmu.UpdateTitle(Title);

    UpdateScrollInfo();

    TabBar.OnConsoleActivated(nNewNum+1/*, isBufferHeight()*/);
    TabBar.Update();

    gConEmu.OnBufferHeight(con.bBufferHeight);

    gConEmu.UpdateProcessDisplay(TRUE);

    HWND hPic = isPictureView();
    if (hPic && mb_PicViewWasHidden) {
        if (!IsWindowVisible(hPic))
            ShowWindow(hPic, SW_SHOWNA);
    }
    mb_PicViewWasHidden = FALSE;
}

void CRealConsole::OnDeactivate(int nNewNum)
{
    if (!this) return;

    HWND hPic = isPictureView();
    if (hPic && IsWindowVisible(hPic)) {
        mb_PicViewWasHidden = TRUE;
        ShowWindow(hPic, SW_HIDE);
    }

    //if (mh_MonitorThread) SetThreadPriority(mh_MonitorThread, THREAD_PRIORITY_NORMAL);
}

// �� ����������� CONSOLE_SCREEN_BUFFER_INFO ����������, �������� �� ���������
BOOL CRealConsole::BufferHeightTurnedOn(CONSOLE_SCREEN_BUFFER_INFO* psbi)
{
    BOOL lbTurnedOn = FALSE;
    TODO("!!! ���������������");

    if (psbi->dwSize.Y == (psbi->srWindow.Bottom - psbi->srWindow.Top + 1)) {
        // ������ ���� == ������ ������, 
        lbTurnedOn = FALSE;
    } else if (con.m_sbi.dwSize.Y < (con.m_sbi.srWindow.Bottom-con.m_sbi.srWindow.Top+10)) {
        // ������ ���� �������� ����� ������ ������
        lbTurnedOn = FALSE;
    } else if (con.m_sbi.dwSize.Y>(con.m_sbi.srWindow.Bottom-con.m_sbi.srWindow.Top+10)) {
        // ������ ������ '�������' ������ ������ ����
        lbTurnedOn = TRUE;
    }

    // ������, ���� ������ ������� ������ ��� ����������� � GUI ���� - ����� �������� BufferHeight
    if (!lbTurnedOn) {
        //TODO: ������, ���� ������ ������� ������ ��� ����������� � GUI ���� - ����� �������� BufferHeight
    }

    return lbTurnedOn;
}

void CRealConsole::UpdateScrollInfo()
{
    if (!isActive())
        return;

    if (!gConEmu.isMainThread()) {
        
        return;
    }

    TODO("���-�� ���������� ����� ������ ���-��... � SetScrollInfo ����� �� ��������� � m_Back");

    int nCurPos = 0;
    BOOL lbScrollRc = FALSE;
    SCROLLINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;

    // ���� ����� "BufferHeight" ������� - �������� �� ����������� ���� ������� ��������� ������ ���������
    if (con.bBufferHeight) {
        lbScrollRc = GetScrollInfo(hConWnd, SB_VERT, &si);
    } else {
        // ���������� ��������� ���, ����� ������ �� ������������ (��� �� 0)
    }

    TODO("����� ��� ������������� '�������' ������ ���������");
    nCurPos = SetScrollInfo(gConEmu.m_Back.mh_WndScroll, SB_VERT, &si, true);
}

// �� con.m_sbi ���������, �������� �� ���������
BOOL CRealConsole::CheckBufferSize()
{
    bool lbForceUpdate = false;

    if (!this)
        return false;
    if (mb_BuferModeChangeLocked)
        return false;


    //if (con.m_sbi.dwSize.X>(con.m_sbi.srWindow.Right-con.m_sbi.srWindow.Left+1)) {
    //  DEBUGLOGFILE("Wrong screen buffer width\n");
    //  // ������ ������� ������-�� ����������� �� �����������
    //  WARNING("���� ����� ��� �����");
    //  //MOVEWINDOW(hConWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1);
    //} else {

    // BufferHeight ����� �������� � �� �������, �� ����� ������ ����...
    BOOL lbTurnedOn = BufferHeightTurnedOn(&con.m_sbi);

    if ( !lbTurnedOn && con.bBufferHeight )
    {
        // ����� ���� ���������� ��������� ��������� ����� ��������������?
        // TODO: ��������� ���������!!!
        SetBufferHeightMode(FALSE);

        UpdateScrollInfo();

        lbForceUpdate = true;
    } 
    else if ( lbTurnedOn && !con.bBufferHeight )
    {
        SetBufferHeightMode(TRUE);

        UpdateScrollInfo();

        lbForceUpdate = true;
    }

    //TODO: � ���� ������ ������ ����� ��������� �� ����� ���������� ���������?

    //if ((BufferHeight == 0) && (con.m_sbi.dwSize.Y>(con.m_sbi.srWindow.Bottom-con.m_sbi.srWindow.Top+1))) {
    //  TODO("��� ����� ���� ���������� ��������� ��������� ����� ��������������!")
    //      DEBUGLOGFILE("Wrong screen buffer height\n");
    //  // ������ ������� ������-�� ����������� �� ���������
    //  WARNING("���� ����� ��� �����");
    //  //MOVEWINDOW(hConWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1);
    //}

    //TODO: ����� �� ��������� � ConEmuC, ���� ����� �����
    //// ��� ������ �� FAR -> CMD � BufferHeight - ����� QuickEdit ������
    //DWORD mode = 0;
    //BOOL lb = FALSE;
    //if (BufferHeight) {
    //  //TODO: ������, ��� ��� BufferHeight ��� ���������� ���������?
    //  //lb = GetConsoleMode(hConIn(), &mode);
    //  mode = GetConsoleMode();

    //  if (con.m_sbi.dwSize.Y>(con.m_sbi.srWindow.Bottom-con.m_sbi.srWindow.Top+1)) {
    //      // ����� ������ ������ ����
    //      mode |= ENABLE_QUICK_EDIT_MODE|ENABLE_INSERT_MODE|ENABLE_EXTENDED_FLAGS;
    //  } else {
    //      // ����� ����� ������ ���� (������ ��� ����������)
    //      mode &= ~(ENABLE_QUICK_EDIT_MODE|ENABLE_INSERT_MODE);
    //      mode |= ENABLE_EXTENDED_FLAGS;
    //  }

    //  TODO("SetConsoleMode");
    //  //lb = SetConsoleMode(hConIn(), mode);
    //}

    return lbForceUpdate;
}

//// ������ �������: HIWORD==newBufferHeight, LOWORD=newBufferWidth
//LRESULT CRealConsole::OnConEmuCmd(BOOL abStarted, DWORD anConEmuC_PID)
//{
//  WORD newBufferHeight = TextHeight();
//  WORD newBufferWidth  = TextWidth();
//
//  if (abStarted) {
//      Process Add(anConEmuC_PID);
//
//      BufferHeight = TextHeight(); TODO("����� ������� �������� ��������� ������...");
//  } else {
//      Process Delete(anConEmuC_PID);
//
//      if ((mn_ProgramStatus & CES_CMDACTIVE) == 0)
//          BufferHeight = 0;
//  }
//
//  return MAKELONG(newBufferWidth,newBufferHeight);
//}


void CRealConsole::SetTabs(ConEmuTab* tabs, int tabsCount)
{
    ConEmuTab* lpNewTabs = NULL;
    
    int nActiveTab = 0;
    if (tabs && tabsCount) {
        int nNewSize = sizeof(ConEmuTab)*tabsCount;
        lpNewTabs = (ConEmuTab*)Alloc(nNewSize, 1);
        if (!lpNewTabs)
            return;
        memmove ( lpNewTabs, tabs, nNewSize );

        // ����� ������ "Panels" ����� �� ��� � ��������� �������. �������� "edit - doc1.doc"
        if (tabsCount > 1 && lpNewTabs[0].Type == 1/*WTYPE_PANELS*/ && lpNewTabs[0].Current)
            lpNewTabs[0].Name[0] = 0;

        if (tabsCount == 1)
            lpNewTabs[0].Current = 1;
        
        for (int i=(tabsCount-1); i>=0; i--) {
            if (lpNewTabs[i].Current) {
                nActiveTab = i; break;
            }
        }
        #ifdef _DEBUG
        for (int i=1; i<tabsCount; i++) {
            if (lpNewTabs[i].Name[0] == 0) {
                _ASSERTE(lpNewTabs[i].Name[0]!=0);
                //wcscpy(lpNewTabs[i].Name, L"ConEmu");
            }
        }
        #endif

    } else if (tabsCount == 1 && tabs == NULL) {
        lpNewTabs = (ConEmuTab*)Alloc(sizeof(ConEmuTab),1);
        if (!lpNewTabs)
            return;
        lpNewTabs->Pos = 0;
        lpNewTabs->Current = 1;
        lpNewTabs->Type = 1;
        //wcscpy(lpNewTabs->Name, L"ConEmu");
    }
    
    mn_tabsCount = 0; Free(mp_tabs); mn_ActiveTab = nActiveTab;
    mp_tabs = lpNewTabs; mn_tabsCount = tabsCount;

    if (mp_tabs && mn_tabsCount) {
        CheckPanelTitle();
        CheckFarStates();
    }
    
    // ����������� TabBar...
    if (gConEmu.isValid(mp_VCon)) // �� ����� �������� ������� ��� ��� �� ��������� � ������...
        TabBar.Update();
}

// ���� ������ ���� ��� - pTab �� ��������!!!
BOOL CRealConsole::GetTab(int tabIdx, /*OUT*/ ConEmuTab* pTab)
{
    if (!this)
        return FALSE;
        
	if (!mp_tabs || tabIdx<0 || tabIdx>=mn_tabsCount) {
		// �� ������ ������, ���� ���� ���� �� ����������������, � ������ ������ -
		// ������ ������ ��������� �������
		if (tabIdx == 0) {
			pTab->Pos = 0; pTab->Current = 1; pTab->Type = 1; pTab->Modified = 0;
            int nMaxLen = max(countof(Title) , countof(pTab->Name));
            lstrcpyn(pTab->Name, Title, nMaxLen);
			return TRUE;
		}
        return FALSE;
	}
    
    memmove(pTab, mp_tabs+tabIdx, sizeof(ConEmuTab));
    if (mn_tabsCount == 1 && pTab->Type == 1) {
        if (Title[0]) { // ���� ������ ������������ - ����� ���������� ��������� �������
            int nMaxLen = max(countof(Title) , countof(pTab->Name));
            lstrcpyn(pTab->Name, Title, nMaxLen);
        }
    }
    if (pTab->Name[0] == 0) {
        if (ms_PanelTitle[0]) { // ������ ����� - ��� Panels?
            int nMaxLen = max(countof(ms_PanelTitle) , countof(pTab->Name));
            lstrcpyn(pTab->Name, ms_PanelTitle, nMaxLen);
        } else if (mn_tabsCount == 1 && Title[0]) { // ���� ������ ������������ - ����� ���������� ��������� �������
            int nMaxLen = max(countof(Title) , countof(pTab->Name));
            lstrcpyn(pTab->Name, Title, nMaxLen);
        }
    }
    wchar_t* pszAmp = pTab->Name;
    int nCurLen = lstrlenW(pTab->Name), nMaxLen = countof(pTab->Name)-1;
    while ((pszAmp = wcschr(pszAmp, L'&')) != NULL) {
        if (nCurLen >= (nMaxLen - 2)) {
            *pszAmp = L'_';
            pszAmp ++;
        } else {
            wmemmove(pszAmp+1, pszAmp, nCurLen-(pszAmp-pTab->Name)-1);
            nCurLen ++;
            pszAmp += 2;
        }
    }
    return TRUE;
}

int CRealConsole::GetTabCount()
{
    if (!this)
        return 0;
        
    return max(mn_tabsCount,1);
}

void CRealConsole::CheckPanelTitle()
{
    if (mp_tabs && mn_tabsCount) {
        if ((mn_ActiveTab >= 0 && mn_ActiveTab < mn_tabsCount) || mn_tabsCount == 1) {
            WCHAR szPanelTitle[CONEMUTABMAX];
            if (!GetWindowText(hConWnd, szPanelTitle, countof(ms_PanelTitle)-1))
                ms_PanelTitle[0] = 0;
            else if (szPanelTitle[0] == L'{' || szPanelTitle[0] == L'(')
                lstrcpy(ms_PanelTitle, szPanelTitle);
        }
    }
}

DWORD CRealConsole::CanActivateFarWindow(int anWndIndex)
{
    if (!this)
        return 0;
        
    DWORD dwPID = GetFarPID();
    if (!dwPID) {
        return -1; // ������� ������������ ��� ������� �� �������� (���� ���)
        //if (anWndIndex == mn_ActiveTab)
        //return 0;
    }

    // ���� ���� ����: ucBoxDblDownRight ucBoxDblHorz ucBoxDblHorz ucBoxDblHorz (������ ��� ������ ������ �������) - �������
    // ���� ���� ������� (� ��������� ������� {n%}) - �������
    // ���� ����� ������ - ������� (������ ���������� ��� ������)
    
    if (anWndIndex<0 || anWndIndex>=mn_tabsCount)
        return 0;

    // ������� ����� ����������. �� ����, � ��� ������ ������ ���� ���������� ����� �������� ����.
    if (mn_ActiveTab == anWndIndex)
        return (DWORD)-1; // ������ ���� ��� ��������, ����� �� ���������...

    if (isPictureView())
        return 0; // ��� ������� PictureView ������������� �� ������ ��� ���� ������� �� ���������

    if (!GetWindowText(hConWnd, TitleCmp, countof(TitleCmp)-2))
        TitleCmp[0] = 0;
    // ����������� � FAR: "{33%}..."
    //2009-06-02: PPCBrowser ���������� ����������� ���: "(33% 00:02:20)..."
    if ((TitleCmp[0] == L'{' || TitleCmp[0] == L'(')
        && isDigit(TitleCmp[1]) &&
        ((TitleCmp[2] == L'%' /*&& TitleCmp[3] == L'}'*/) ||
         (isDigit(TitleCmp[2]) && TitleCmp[3] == L'%' /*&& TitleCmp[4] == L'}'*/) ||
         (isDigit(TitleCmp[2]) && isDigit(TitleCmp[3]) && TitleCmp[4] == L'%' /*&& TitleCmp[5] == L'}'*/))
       )
    {
        // ���� �����������
        return 0;
    }
    if (!con.pConChar || !con.nTextWidth || con.nTextHeight<2)
        return 0; // ������� �� ����������������, ������ ������
    BOOL lbMenuActive = FALSE;
    if (mp_tabs && mn_ActiveTab >= 0 && mn_ActiveTab < mn_tabsCount)
    {   // ���� ����� ���� ������ � �������
        if (mp_tabs[mn_ActiveTab].Type == 1/*WTYPE_PANELS*/)
        {
            MSectionLock cs; cs.Lock(&csCON);
            if (con.pConChar) {
                if (con.pConChar[0] == L'R') {
                    // ������ �������. �������� �������� �������������?
                    lbMenuActive = TRUE;
                }
                else if (con.pConChar[0] == L' ' && con.pConChar[con.nTextWidth] == ucBoxDblVert) {
                    lbMenuActive = TRUE;
                } else if (con.pConChar[0] == L' ' && (con.pConChar[con.nTextWidth] == ucBoxDblDownRight || 
                    (con.pConChar[con.nTextWidth] == '[' 
                     && (con.pConChar[con.nTextWidth+1] >= L'0' && con.pConChar[con.nTextWidth+1] <= L'9'))))
                {
                    // ������ ���� ������ �����. ���������, ������� �� ����.
                    for (int x=1; !lbMenuActive && x<con.nTextWidth; x++) {
                        if (con.pConAttr[x] != con.pConAttr[0]) // ���������� ���� - �� ��������������
                            lbMenuActive = TRUE;
                    }
                } else {
                    // ���� ������ ���� ������ �� �����, � ������ ���������
                    wchar_t* pszLine = con.pConChar + con.nTextWidth;
                    for (int x=1; !lbMenuActive && x<(con.nTextWidth-10); x++)
                    {
                        if (pszLine[x] == ucBoxDblDownRight && pszLine[x+1] == ucBoxDblHorz)
                            lbMenuActive = TRUE;
                    }
                }
            }
            cs.Unlock();
        }
    }
    
    // ���� ������ ���� ������������ ������:
    //  0-������ ���������� � "  " ��� � "R   " ���� ���� ������ �������
    //  1-� ������ ucBoxDblDownRight ucBoxDblHorz ucBoxDblHorz ��� "[0+1]" ucBoxDblHorz ucBoxDblHorz
    //  2-� ������ ucBoxDblVert
    // ������� ��������� ���� ���������� �� ���������� ������ � ������ ������.
    // ���������� ���� ������������ ������ ����� ������ - � �������� �������������� ������ � ��������� �����
    
    if (lbMenuActive)
        return 0;
        
    return dwPID;
}

BOOL CRealConsole::ActivateFarWindow(int anWndIndex)
{
    if (!this)
        return FALSE;
    

    DWORD dwPID = CanActivateFarWindow(anWndIndex);
    if (!dwPID)
        return FALSE;
    else if (dwPID == (DWORD)-1)
        return TRUE; // ������ ���� ��� ��������, ����� �� ���������...


    BOOL lbRc = FALSE;

    CConEmuPipe pipe(dwPID, 100);
    if (pipe.Init(_T("CRealConsole::ActivateFarWindow")))
    {
        //DWORD cbWritten = 0;
        if (pipe.Execute(CMD_SETWINDOW, &anWndIndex, 4))
        {
            DWORD cbBytesRead=0;
            DWORD tabCount = 0;
            ConEmuTab* tabs = NULL;
            if (pipe.Read(&tabCount, sizeof(DWORD), &cbBytesRead)) {
                tabs = (ConEmuTab*)pipe.GetPtr(&cbBytesRead);
                _ASSERTE(cbBytesRead==(tabCount*sizeof(ConEmuTab)));
                if (cbBytesRead == (tabCount*sizeof(ConEmuTab))) {
                    SetTabs(tabs, tabCount);
                    lbRc = TRUE;
                }
            }

            pipe.Close();
        }
    }

    return lbRc;
}

BOOL CRealConsole::IsConsoleThread()
{
    if (!this) return FALSE;

    DWORD dwCurThreadId = GetCurrentThreadId();
    return dwCurThreadId == mn_MonitorThreadID;
}

void CRealConsole::SetForceRead()
{
    SetEvent(mh_MonitorThreadEvent);
}

/*
DWORD CRealConsole::WaitEndUpdate(DWORD dwTimeout)
{
    DWORD dwWait = WaitForSingleObject(mh_EndUpdateEvent, dwTimeout);
    return dwWait;
}
*/

bool CRealConsole::isPackets()
{
    CESERVER_REQ** iter = m_PacketQueue;
    CESERVER_REQ** end = iter + countof(m_PacketQueue);
    
    while (iter != end) {
        if ( *iter != NULL )
            return true;
        iter ++; // ������ �����
    }

    return false;
}

// �������� ��������� ����� � ������. ������ ������� �� �����
void CRealConsole::PushPacket(CESERVER_REQ* pPkt)
{
//#ifdef _DEBUG
//    DWORD dwChSize = 0;
//    CESERVER_CHAR_HDR ch = {0};
//    memmove(&dwChSize, pPkt->Data+86, dwChSize);
//    if (dwChSize) {
//        memmove(&ch, pPkt->Data+90, sizeof(ch));
//        _ASSERTE(ch.nSize == dwChSize);
//        _ASSERTE(ch.cr1.X<=ch.cr2.X && ch.cr1.Y<=ch.cr2.Y);
//    }
//#endif

	#ifdef _DEBUG
	wchar_t szDbg[128]; wsprintf(szDbg, L"Pushing packet PktID=%i, ConHeight=%i\n",
		pPkt->ConInfo.inf.nPacketId, pPkt->ConInfo.inf.sbi.dwSize.Y);
	DEBUGSTRPKT(szDbg);
	#endif

    int i;
    DWORD dwTID = GetCurrentThreadId();
    
    CESERVER_REQ **pQueue = NULL;
    for (i=0; !pQueue && i < MAX_SERVER_THREADS; i++) {
        if (mn_ServerThreadsId[i] == dwTID)
            pQueue = m_PacketQueue + (i*MAX_THREAD_PACKETS);
    }
    // ���� ��� �� ��������� ���� - ��������� � ����������������� �������
    if (!pQueue) pQueue = m_PacketQueue + (MAX_SERVER_THREADS*MAX_THREAD_PACKETS);
    
    for (i=0; i < MAX_THREAD_PACKETS; i++, pQueue++)
    {
        if (*pQueue == NULL) {
            *pQueue = pPkt;
            pQueue = NULL;
            break;
        }
    }
    
    // ���� �� NULL - ������ ������� �����������!
    _ASSERTE(pQueue == NULL);

    //MSectionLock cs; cs.Lock(&csPKT);
    //m_Packets.push_back(pPkt);
    //cs.Unlock();
    //MCHKHEAP
    SetEvent(mh_PacketArrived);
}

// � ��� ����� ����� ������� ����� � ���������� ��
// ���������� ������� ������ ���������� ��������� (free)
CESERVER_REQ* CRealConsole::PopPacket()
{
	if (!isPackets()) {
		DEBUGSTRPKT(L"Popping packet failed, queue is empty\n");
        return NULL;
	}

    MCHKHEAP

    CESERVER_REQ* pRet = NULL, *pCmp = NULL;

    //MSectionLock cs; cs.Lock(&csPKT, TRUE);

    //std::vector<CESERVER_REQ*>::iterator iter;
    CESERVER_REQ** iter = m_PacketQueue;
    CESERVER_REQ** end = iter + countof(m_PacketQueue);
    
    DWORD dwMinID = 0;

	// �� ����� ���������� ���� ������� mn_LastProcessedPkt ����� ����������, 
	// ������� �� ��������� ������ ���������� �������� ����� �� ������ �������
	DWORD nLastProcessedPkt = mn_LastProcessedPkt;
    
    while (iter != end) {
        pCmp = *iter;
        if (pCmp == NULL) {
            iter ++; // ������ �����
            continue;
        }

        if ( pCmp->ConInfo.inf.nPacketId < nLastProcessedPkt ) {
            // ����� ��� ������, � ������� �� (���� �� � ��� ��� ���� �� ������)
            TODO("���������, ��� ���������� � �������� ��������/��������...");
			#ifdef _DEBUG
			wchar_t szDbg[128]; wsprintf(szDbg, L"!!! *** Obsolete packet found (%i < %i) *** !!!\n", 
				pCmp->ConInfo.inf.nPacketId, nLastProcessedPkt);
            DEBUGSTRPKT(szDbg);
			#endif
            *iter = NULL;
            iter ++;
            free(pCmp);
            MCHKHEAP
            continue;
        } 
        
        if ( dwMinID == 0 || pCmp->ConInfo.inf.nPacketId <= dwMinID ) {
            // ���� ����������� �� ����������
            dwMinID = pCmp->ConInfo.inf.nPacketId;
            pRet = pCmp;
        }
        iter ++;
    }

    if (pRet) {
        nLastProcessedPkt = dwMinID;

        iter = m_PacketQueue;
        while (iter != end) {
            pCmp = *iter;
            if (pCmp == NULL) {
                iter ++;
                continue;
            }
            
            if ( pCmp->ConInfo.inf.nPacketId <= nLastProcessedPkt ) {
                if (pRet!=pCmp)
                    free(pCmp);
                MCHKHEAP
                *iter = NULL;
            }
            iter ++;
        }

		if (mn_LastProcessedPkt < nLastProcessedPkt)
			mn_LastProcessedPkt = nLastProcessedPkt;
    }

	#ifdef _DEBUG
	wchar_t szDbg[128]; 
	if (pRet)
		wsprintf(szDbg, L"Popping packet PktID=%i, ConHeight=%i\n",
			pRet->ConInfo.inf.nPacketId, pRet->ConInfo.inf.sbi.dwSize.Y);
	else
		wcscpy(szDbg, L"Popping packet failed, queue is empty\n");
	DEBUGSTRPKT(szDbg);
	#endif

    //cs.Unlock();
    MCHKHEAP
    return pRet;
}

void CRealConsole::SetBufferHeightMode(BOOL abBufferHeight, BOOL abLock/*=FALSE*/)
{
    if (mb_BuferModeChangeLocked) {
        if (!abLock) {
            //_ASSERT(FALSE);
            return;
        }
    }

    con.bBufferHeight = abBufferHeight;
    if (isActive())
        gConEmu.OnBufferHeight(abBufferHeight);
}

HANDLE CRealConsole::PrepareOutputFileCreate(wchar_t* pszFilePathName)
{
    wchar_t szTemp[200];
    HANDLE hFile = NULL;
    if (GetTempPath(200, szTemp)) {
        if (GetTempFileName(szTemp, L"CEM", 0, pszFilePathName)) {
            hFile = CreateFile(pszFilePathName, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
            if (hFile == INVALID_HANDLE_VALUE) {
                pszFilePathName[0] = 0;
                hFile = NULL;
            }
        }
    }
    return hFile;
}

BOOL CRealConsole::PrepareOutputFile(BOOL abUnicodeText, wchar_t* pszFilePathName)
{
    BOOL lbRc = FALSE;
    HANDLE hPipe = NULL; 
    CESERVER_REQ_HDR in = {0};
    CESERVER_REQ *pOut = NULL;
    BYTE cbReadBuf[512];
    BOOL fSuccess;
    DWORD cbRead, dwMode, dwErr;

    // Try to open a named pipe; wait for it, if necessary. 
    while (1) 
    { 
        hPipe = CreateFile( 
            ms_ConEmuC_Pipe,// pipe name 
            GENERIC_READ |  // read and write access 
            GENERIC_WRITE, 
            0,              // no sharing 
            NULL,           // default security attributes
            OPEN_EXISTING,  // opens existing pipe 
            0,              // default attributes 
            NULL);          // no template file 

        // Break if the pipe handle is valid. 
        if (hPipe != INVALID_HANDLE_VALUE) 
            break; 

        // Exit if an error other than ERROR_PIPE_BUSY occurs. 
        dwErr = GetLastError();
        if (dwErr != ERROR_PIPE_BUSY) 
        {
            TODO("���������, ���� �������� ���� � ����� ������, �� ������ ���� ��� mh_ConEmuC");
            dwErr = WaitForSingleObject(mh_ConEmuC, 100);
            if (dwErr == WAIT_OBJECT_0)
                return FALSE;
            continue;
            //DisplayLastError(L"Could not open pipe", dwErr);
            //return 0;
        }

        // All pipe instances are busy, so wait for 1 second.
        if (!WaitNamedPipe(ms_ConEmuC_Pipe, 1000) ) 
        {
            dwErr = WaitForSingleObject(mh_ConEmuC, 100);
            if (dwErr == WAIT_OBJECT_0) {
                DEBUGSTRCMD(L" - FAILED!\n");
                return FALSE;
            }
            //DisplayLastError(L"WaitNamedPipe failed"); 
            //return 0;
        }
    } 

    // The pipe connected; change to message-read mode. 
    dwMode = PIPE_READMODE_MESSAGE; 
    fSuccess = SetNamedPipeHandleState( 
        hPipe,    // pipe handle 
        &dwMode,  // new pipe mode 
        NULL,     // don't set maximum bytes 
        NULL);    // don't set maximum time 
    if (!fSuccess) 
    {
        DEBUGSTRCMD(L" - FAILED!\n");
        DisplayLastError(L"SetNamedPipeHandleState failed");
        CloseHandle(hPipe);
        return 0;
    }

    in.nSize = sizeof(CESERVER_REQ_HDR);
    in.nVersion = CESERVER_REQ_VER;
    in.nCmd  = CECMD_GETOUTPUT;
    in.nSrcThreadId = GetCurrentThreadId();

    // Send a message to the pipe server and read the response. 
    fSuccess = TransactNamedPipe( 
        hPipe,                  // pipe handle 
        &in,                    // message to server
        in.nSize,               // message length 
        cbReadBuf,              // buffer to receive reply
        sizeof(cbReadBuf),      // size of read buffer
        &cbRead,                // bytes read
        NULL);                  // not overlapped 

    if (!fSuccess && (GetLastError() != ERROR_MORE_DATA)) 
    {
        DEBUGSTRCMD(L" - FAILED!\n");
        //DisplayLastError(L"TransactNamedPipe failed"); 
        CloseHandle(hPipe);
        HANDLE hFile = PrepareOutputFileCreate(pszFilePathName);
        if (hFile)
            CloseHandle(hFile);
        return (hFile!=NULL);
    }

    // ���� �� ������� ������, ������ ��������� �� �����
    pOut = (CESERVER_REQ*)cbReadBuf;
    if (pOut->hdr.nVersion != CESERVER_REQ_VER) {
        gConEmu.ShowOldCmdVersion(pOut->hdr.nCmd, pOut->hdr.nVersion);
        CloseHandle(hPipe);
        return 0;
    }

    int nAllSize = *((DWORD*)cbReadBuf);
    if (nAllSize==0) {
        DEBUGSTRCMD(L" - FAILED!\n");
        DisplayLastError(L"Empty data recieved from server", 0);
        CloseHandle(hPipe);
        return 0;
    }
    if (nAllSize > (int)sizeof(cbReadBuf))
    {
        pOut = (CESERVER_REQ*)calloc(nAllSize,1);
        _ASSERTE(pOut!=NULL);
        memmove(pOut, cbReadBuf, cbRead);
        _ASSERTE(pOut->hdr.nVersion==CESERVER_REQ_VER);

        LPBYTE ptrData = ((LPBYTE)pOut)+cbRead;
        nAllSize -= cbRead;

        while(nAllSize>0)
        { 
            //_tprintf(TEXT("%s\n"), chReadBuf);

            // Break if TransactNamedPipe or ReadFile is successful
            if(fSuccess)
                break;

            // Read from the pipe if there is more data in the message.
            fSuccess = ReadFile( 
                hPipe,      // pipe handle 
                ptrData,    // buffer to receive reply 
                nAllSize,   // size of buffer 
                &cbRead,    // number of bytes read 
                NULL);      // not overlapped 

            // Exit if an error other than ERROR_MORE_DATA occurs.
            if( !fSuccess && (GetLastError() != ERROR_MORE_DATA)) 
                break;
            ptrData += cbRead;
            nAllSize -= cbRead;
        }

        TODO("����� ���������� ASSERT, ���� ������� ���� ������� � �������� ������");
        _ASSERTE(nAllSize==0);
    }

    CloseHandle(hPipe);

    // ������ pOut �������� ������ ������ : CESERVER_CONSAVE

    HANDLE hFile = PrepareOutputFileCreate(pszFilePathName);
    lbRc = (hFile != NULL);

    if (pOut->hdr.nVersion == CESERVER_REQ_VER) {
        CESERVER_CONSAVE* pSave = (CESERVER_CONSAVE*)pOut;
        UINT nWidth = pSave->hdr.sbi.dwSize.X;
        UINT nHeight = pSave->hdr.sbi.dwSize.Y;
        wchar_t* pwszCur = pSave->Data;
        wchar_t* pwszEnd = (wchar_t*)(((LPBYTE)pOut)+pOut->hdr.nSize);

        if (pOut->hdr.nVersion == CESERVER_REQ_VER && nWidth && nHeight && (pwszCur < pwszEnd)) {
            DWORD dwWritten;
            char *pszAnsi = NULL;
            wchar_t* pwszRn = NULL;
            if (!abUnicodeText) {
                pszAnsi = (char*)calloc(nWidth+1,1);
            } else {
                WORD dwTag = 0xFEFF;
                WriteFile(hFile, &dwTag, 2, &dwWritten, 0);
            }

            BOOL lbHeader = TRUE;

            for (UINT y = 0; y < nHeight && pwszCur < pwszEnd; y++)
            {
                UINT nCurLen = 0;
                pwszRn = pwszCur + nWidth - 1;
                while (pwszRn >= pwszCur && *pwszRn == L' ') {
                    *pwszRn = 0; pwszRn --;
                }
                nCurLen = pwszRn - pwszCur + 1;
                if (nCurLen > 0 || !lbHeader) { // ������ N ����� ���� ��� ������ - �� ����������
                    if (lbHeader) {
                        lbHeader = FALSE;
                    } else if (nCurLen == 0) {
                        // ���� ���� ����� ��� - ������ ������ �� ������
                        pwszRn = pwszCur + nWidth;
                        while (pwszRn < pwszEnd && *pwszRn == L' ') pwszRn ++;
                        if (pwszRn >= pwszEnd) break; // ����������� ����� ������ ���
                    }
                    if (abUnicodeText) {
                        if (nCurLen>0)
                            WriteFile(hFile, pwszCur, nCurLen*2, &dwWritten, 0);
                        WriteFile(hFile, L"\r\n", 4, &dwWritten, 0);
                    } else {
                        nCurLen = WideCharToMultiByte(CP_ACP, 0, pwszCur, nCurLen, pszAnsi, nWidth, 0,0);
                        if (nCurLen>0)
                            WriteFile(hFile, pszAnsi, nCurLen, &dwWritten, 0);
                        WriteFile(hFile, "\r\n", 2, &dwWritten, 0);
                    }
                }
                pwszCur += nWidth;
            }
        }
    }

    if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    if (pOut && (LPVOID)pOut != (LPVOID)cbReadBuf)
        free(pOut);
    return lbRc;
}

void CRealConsole::SwitchKeyboardLayout(DWORD dwNewKeyboardLayout)
{
    if (!this) return;
    if (ms_ConEmuC_Pipe[0] == 0) return;
    if (!hConWnd) return;

    // � FAR ��� XLat �������� ���:
    //PostMessageW(hFarWnd,WM_INPUTLANGCHANGEREQUEST, INPUTLANGCHANGE_FORWARD, 0);
    PostMessageW(hConWnd, WM_INPUTLANGCHANGEREQUEST, 0, dwNewKeyboardLayout);
    

    //   int nInSize = sizeof(CESERVER_REQ_HDR)+sizeof(DWORD);
    //   CESERVER_REQ *pIn = (CESERVER_REQ*)calloc(nInSize,1);
    //CESERVER_REQ *pOut = (CESERVER_REQ*)calloc(nInSize,1);
    //DWORD dwRead = 0;
    //BOOL fSuccess = FALSE;

    //if (pIn) {
    //  pIn->hdr.nCmd = CECMD_LANGCHANGE;
    //  pIn->hdr.nSize = nInSize;
    //  pIn->hdr.nSrcThreadId = GetCurrentThreadId();
    //  pIn->hdr.nVersion = CESERVER_REQ_VER;
    //  memmove(pIn->Data, &dwNewKeyboardLayout, sizeof(dwNewKeyboardLayout));

    //  
    //  fSuccess = CallNamedPipe(ms_ConEmuC_Pipe, pIn, pIn->hdr.nSize, pOut, nInSize, &dwRead, 200);
    //}

    //SafeFree(pIn);
    //SafeFree(pOut);
}

void CRealConsole::Paste()
{
    if (!this) return;
    if (!hConWnd) return;
    
#ifndef RCON_INTERNAL_PASTE
    // ����� ���
    PostMessage(hConWnd, WM_COMMAND, SC_PASTE_SECRET, 0);
#endif

#ifdef RCON_INTERNAL_PASTE
    // � ����� � ����� ��� ������
    if (!OpenClipboard(NULL)) {
        MBox(_T("Can't open PC clipboard"));
        return;
    }
    HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);
    if (!hglb) {
        CloseClipboard();
        MBox(_T("Can't get CF_UNICODETEXT"));
        return;
    }
    wchar_t* lptstr = (wchar_t*)GlobalLock(hglb);
    if (!lptstr) {
        CloseClipboard();
        MBox(_T("Can't lock CF_UNICODETEXT"));
        return;
    }
    // ������ ���������� �����
    size_t nLen = lstrlenW(lptstr);
    if (nLen>0) {
        if (nLen>127) {
            TCHAR szMsg[128]; wsprintf(szMsg, L"Clipboard length is %i chars!\nContinue?", nLen);
            if (MessageBox(ghWnd, szMsg, gConEmu.GetTitle(), MB_OKCANCEL) != IDOK) {
                GlobalUnlock(hglb);
                CloseClipboard();
                return;
            }
        }
        
        INPUT_RECORD r = {KEY_EVENT};
        
        // ��������� � ������� ��� ������� ��: lptstr
        while (*lptstr)
        {
            r.Event.KeyEvent.bKeyDown = TRUE;
            r.Event.KeyEvent.uChar.UnicodeChar = *lptstr;
            r.Event.KeyEvent.wRepeatCount = 1; //TODO("0-15 ? Specifies the repeat count for the current message. The value is the number of times the keystroke is autorepeated as a result of the user holding down the key. If the keystroke is held long enough, multiple messages are sent. However, the repeat count is not cumulative.");
            r.Event.KeyEvent.wVirtualKeyCode = 0;

            //r.Event.KeyEvent.wVirtualScanCode = ((DWORD)lParam & 0xFF0000) >> 16; // 16-23 - Specifies the scan code. The value depends on the OEM.
            // 24 - Specifies whether the key is an extended key, such as the right-hand ALT and CTRL keys that appear on an enhanced 101- or 102-key keyboard. The value is 1 if it is an extended key; otherwise, it is 0.
            // 29 - Specifies the context code. The value is 1 if the ALT key is held down while the key is pressed; otherwise, the value is 0.
            // 30 - Specifies the previous key state. The value is 1 if the key is down before the message is sent, or it is 0 if the key is up.
            // 31 - Specifies the transition state. The value is 1 if the key is being released, or it is 0 if the key is being pressed.
            //r.Event.KeyEvent.dwControlKeyState = 0;
            
            PostConsoleEvent(&r);

            // � ����� �������� ����������
            r.Event.KeyEvent.bKeyDown = FALSE;
            PostConsoleEvent(&r);
        }
    }
    GlobalUnlock(hglb);
    CloseClipboard();
#endif
}

void CRealConsole::CloseConsole()
{
    if (!this) return;
    _ASSERTE(!mb_ProcessRestarted);
	if (hConWnd) {
        PostMessage(hConWnd, WM_CLOSE, 0, 0);
	} else {
		m_Args.bDetached = FALSE;
		if (mp_VCon)
			gConEmu.OnVConTerminated(mp_VCon);
	}
}

uint CRealConsole::TextWidth()
{ 
    _ASSERTE(this!=NULL);
    if (!this) return MIN_CON_WIDTH;
    if (con.nChange2TextWidth!=-1 && con.nChange2TextWidth!=0)
        return con.nChange2TextWidth;
    return con.nTextWidth;
}

uint CRealConsole::TextHeight()
{ 
    _ASSERTE(this!=NULL);
    if (!this) return MIN_CON_HEIGHT;
    uint nRet = 0;
    if (con.nChange2TextHeight!=-1 && con.nChange2TextHeight!=0)
        nRet = con.nChange2TextHeight;
    else
        nRet = con.nTextHeight;
    if (nRet > 200) {
        _ASSERTE(nRet<=200);
    }
    return nRet;
}

bool CRealConsole::isActive()
{
    if (!this) return false;
    if (!mp_VCon) return false;
    return gConEmu.isActive(mp_VCon);
}

void CRealConsole::CheckFarStates()
{
	DWORD nLastState = mn_FarStatus;

	if (GetFarPID() == 0) {
		mn_FarStatus = 0;
	} else {
		mn_FarStatus &= ~CES_FARFLAGS;

		if (mp_tabs && mn_ActiveTab >= 0 && mn_ActiveTab < mn_tabsCount) {
			if (mp_tabs[mn_ActiveTab].Type != 1) {
				if (mp_tabs[mn_ActiveTab].Type == 2)
					mn_FarStatus |= CES_VIEWER;
				else if (mp_tabs[mn_ActiveTab].Type == 3)
					mn_FarStatus |= CES_EDITOR;
			}
		}

		if ((mn_FarStatus & CES_FARFLAGS) == 0) {
			if (wcsncmp(Title, ms_Editor, lstrlen(ms_Editor))==0 || wcsncmp(Title, ms_EditorRus, lstrlen(ms_EditorRus))==0)
				mn_FarStatus |= CES_EDITOR;
			else if (wcsncmp(Title, ms_Viewer, lstrlen(ms_Viewer))==0 || wcsncmp(Title, ms_ViewerRus, lstrlen(ms_ViewerRus))==0)
				mn_FarStatus |= CES_VIEWER;
			else if (isFilePanel(true))
				mn_FarStatus |= CES_FILEPANEL;
		}

		// ����� � ���, ����� �� �������� ������ CES_MAYBEPANEL ���� � ������� ������ ������
		// ������ ������������ ������ � ����������/�������
		if ((mn_FarStatus & (CES_EDITOR | CES_VIEWER)) != 0)
			mn_FarStatus &= ~(CES_MAYBEPANEL|CES_WASPROGRESS|CES_OPER_ERROR); // ��� ������������ � ��������/������ - �������� CES_MAYBEPANEL

		if ((mn_FarStatus & CES_FILEPANEL) == CES_FILEPANEL) {
			mn_FarStatus |= CES_MAYBEPANEL; // ������ � ������ - �������� ������
			mn_FarStatus &= ~(CES_WASPROGRESS|CES_OPER_ERROR); // ������ ������ ������� ����������� �� ����
		}

		if (mn_Progress >= 0 && mn_Progress <= 100) {
			mn_PreWarningProgress = mn_Progress;
			if ((mn_FarStatus & CES_MAYBEPANEL) == CES_MAYBEPANEL)
				mn_FarStatus |= CES_WASPROGRESS; // �������� ������, ��� �������� ���
			mn_FarStatus &= ~CES_OPER_ERROR;
		} else if ((mn_FarStatus & (CES_WASPROGRESS|CES_MAYBEPANEL)) == (CES_WASPROGRESS|CES_MAYBEPANEL)) {
			mn_FarStatus |= CES_OPER_ERROR;
		}
	}

	if ((mn_FarStatus & CES_WASPROGRESS) == 0 && mn_Progress == -1)
		mn_PreWarningProgress = -1;

	if (mn_FarStatus != nLastState)
		gConEmu.UpdateProcessDisplay(FALSE);
}

void CRealConsole::OnTitleChanged()
{
    if (!this) return;

    wcscpy(Title, TitleCmp);

    // ��������� ��������� ��������
    short nLastProgress = mn_Progress;
    if (Title[0] == L'{' || Title[0] == L'(' || Title[0] == L'[') {
        if (isDigit(Title[1])) {
            if (isDigit(Title[2]) && isDigit(Title[3]) 
                && (Title[4] == L'%' || (Title[4] == L'.' && isDigit(Title[5]) && Title[6] == L'%'))
                )
            {
                // �� ���� ������ 100% ���� �� ������ :)
                mn_Progress = 100*(Title[1] - L'0') + 10*(Title[2] - L'0') + (Title[3] - L'0');
            } else if (isDigit(Title[2]) 
                && (Title[3] == L'%' || (Title[3] == L'.' && isDigit(Title[4]) && Title[5] == L'%') )
                )
            {
                // 10 .. 99 %
                mn_Progress = 10*(Title[1] - L'0') + (Title[2] - L'0');
            } else if (Title[2] == L'%' || (Title[2] == L'.' && isDigit(Title[3]) && Title[4] == L'%'))
            {
                // 0 .. 9 %
                mn_Progress = (Title[1] - L'0');
            } else {
                // ��������� ���
                mn_Progress = -1;
            }
            _ASSERTE(mn_Progress<=100);
            if (mn_Progress > 100)
                mn_Progress = 100;
        } else {
            mn_Progress = -1;
        }
    } else {
        mn_Progress = -1;
    }

	CheckFarStates();

    // ����� ����� ������������ �� ��������� ��������� �� ����,
    // ��� �� ������, ��� ������������� ��������...
    TODO("������ ����������� � ��� ������� ���������� ���������!");
    if (Title[0] == L'{' || Title[0] == L'(')
        CheckPanelTitle();

    if (gConEmu.isActive(mp_VCon)) {
        // ��� �������� ������� - ��������� ���������. �������� ��������� ��� ��
        gConEmu.UpdateTitle(TitleCmp);
    } else if (nLastProgress != mn_Progress) {
        // ��� �� �������� ������� - ��������� ������� ����, ��� � ��� ��������� ��������
        gConEmu.UpdateProgress(TRUE/*abUpdateTitle*/);
    }
    TabBar.Update(); // ������� ��������� ��������?
}

bool CRealConsole::isFilePanel(bool abPluginAllowed/*=false*/)
{
    if (!this) return false;

    if (Title[0] == 0) return false;

    if (abPluginAllowed) {
        if (isEditor() || isViewer())
            return false;
    }

    // ����� ��� DragDrop
    if (_tcsncmp(Title, ms_TempPanel, _tcslen(ms_TempPanel)) == 0 || _tcsncmp(Title, ms_TempPanelRus, _tcslen(ms_TempPanelRus)) == 0)
        return true;

    if ((abPluginAllowed && Title[0]==_T('{')) ||
        (_tcsncmp(Title, _T("{\\\\"), 3)==0) ||
        (Title[0] == _T('{') && isDriveLetter(Title[1]) && Title[2] == _T(':') && Title[3] == _T('\\')))
    {
        TCHAR *Br = _tcsrchr(Title, _T('}'));
        if (Br && _tcscmp(Br, _T("} - Far"))==0)
            return true;
    }
    //TCHAR *BrF = _tcschr(Title, '{'), *BrS = _tcschr(Title, '}'), *Slash = _tcschr(Title, '\\');
    //if (BrF && BrS && Slash && BrF == Title && (Slash == Title+1 || Slash == Title+3))
    //    return true;
    return false;
}

bool CRealConsole::isEditor()
{
    if (!this) return false;
    return GetFarStatus() & CES_EDITOR;
}

bool CRealConsole::isViewer()
{
    if (!this) return false;
    return GetFarStatus() & CES_VIEWER;
}

bool CRealConsole::isNtvdm()
{
    if (!this) return false;
    if (mn_ProgramStatus & CES_NTVDM) {
        // ������� 16bit ���������� ������ �� WinEvent. ����� �� ��������� ������ ��� ����������,
        // �.�. ������� ntvdm.exe �� �����������, � �������� � ������.
        return true;
        //if (mn_ProgramStatus & CES_FARFLAGS) {
        //  //mn_ActiveStatus &= ~CES_NTVDM;
        //} else if (isFilePanel()) {
        //  //mn_ActiveStatus &= ~CES_NTVDM;
        //} else {
        //  return true;
        //}
    }
    return false;
}

LPCWSTR CRealConsole::GetCmd()
{
    if (m_Args.pszSpecialCmd)
        return m_Args.pszSpecialCmd;
    else
        return gSet.GetCmd();
}

short CRealConsole::GetProgress(BOOL *rpbError)
{
    if (!this) return -1;
	if (mn_Progress >= 0)
		return mn_Progress;
	if (mn_PreWarningProgress >= 0) {
		*rpbError = TRUE;
		return mn_PreWarningProgress;
	}
	return -1;
}

void CRealConsole::EnableComSpec(DWORD anFarPID, BOOL abSwitch)
{
    if (!this) return;
    if (!anFarPID) return;

    //int nLen = /*ComSpec\0*/ 8 + /*ComSpecC\0*/ 9 + 20 +  2*lstrlenW(gConEmu.ms_ConEmuExe);
    wchar_t szCMD[MAX_PATH+1];
    wchar_t szData[MAX_PATH*4];

    // ���� ���������� ComSpecC - ������ ������������� ����������� ComSpec
    if (!GetEnvironmentVariable(L"ComSpecC", szCMD, MAX_PATH) || szCMD[0] == 0)
        if (!GetEnvironmentVariable(L"ComSpec", szCMD, MAX_PATH) || szCMD[0] == 0)
            szCMD[0] = 0;
    if (szCMD[0] != 0) {
        // ������ ���� ��� (��������) �� conemuc.exe
        wchar_t* pwszCopy = wcsrchr(szCMD, L'\\'); if (!pwszCopy) pwszCopy = szCMD;
        #if !defined(__GNUC__)
        #pragma warning( push )
        #pragma warning(disable : 6400)
        #endif
        if (lstrcmpiW(pwszCopy, L"ConEmuC")==0 || lstrcmpiW(pwszCopy, L"ConEmuC.exe")==0)
            szCMD[0] = 0;
        #if !defined(__GNUC__)
        #pragma warning( pop )
        #endif
    }

    // ComSpec/ComSpecC �� ���������, ���������� cmd.exe
    if (szCMD[0] == 0) {
        wchar_t* psFilePart = NULL;
        if (!SearchPathW(NULL, L"cmd.exe", NULL, MAX_PATH, szCMD, &psFilePart))
        {
            DisplayLastError(L"Can't find cmd.exe!\n", 0);
            return;
        }
    }

    BOOL lbNeedQuot = (wcschr(gConEmu.ms_ConEmuExe, L' ') != NULL);
    wchar_t* pszName = szData;
    lstrcpy(pszName, L"ComSpec");
    wchar_t* pszValue = pszName + lstrlenW(pszName) + 1;

	if (abSwitch) {
		if (lbNeedQuot) *(pszValue++) = L'"';
		lstrcpy(pszValue, gConEmu.ms_ConEmuExe);
		wchar_t* pszSlash = wcsrchr(pszValue, L'\\');
		_ASSERTE(pszSlash!=NULL);
		lstrcpy(pszSlash, L"\\ConEmuC.exe");
		if (lbNeedQuot) lstrcat(pszSlash, L"\"");

		lbNeedQuot = (szCMD[0] != L'"') && (wcschr(szCMD, L' ') != NULL);
		pszName = pszValue + lstrlenW(pszValue) + 1;
		lstrcpy(pszName, L"ComSpecC");
		pszValue = pszName + lstrlenW(pszName) + 1;
	}
    if (lbNeedQuot) *(pszValue++) = L'"';
    lstrcpy(pszValue, szCMD);
    if (lbNeedQuot) lstrcat(pszValue, L"\"");

    pszName = pszValue + lstrlenW(pszValue) + 1;
    *(pszName++) = 0;
    *(pszName++) = 0;

    // ��������� � �������
    CConEmuPipe pipe(anFarPID, 300);
    if (pipe.Init(L"SetEnvVars", TRUE))
        pipe.Execute(CMD_SETENVVAR, szData, 2*(pszName - szData));
}

// ��������� ���� ��� PictureView ������ ����� ������������� �������������, ��� ���
// ������������ �� ���� ��� ����������� "���������" - ������
HWND CRealConsole::isPictureView()
{
    if (!this) return NULL;

    if (hPictureView && (!IsWindow(hPictureView) || !isFar())) {
        hPictureView = NULL; mb_PicViewWasHidden = FALSE;
        gConEmu.InvalidateAll();
    }

    if (!hPictureView) {
        hPictureView = FindWindowEx(ghWnd, NULL, L"FarPictureViewControlClass", NULL);
        if (!hPictureView)
            hPictureView = FindWindowEx(ghWndDC, NULL, L"FarPictureViewControlClass", NULL);
        if (!hPictureView) { // FullScreen?
            hPictureView = FindWindowEx(NULL, NULL, L"FarPictureViewControlClass", NULL);
        }
        if (hPictureView) {
            // ��������� �� �������������� ����
            DWORD dwPID, dwTID;
            dwTID = GetWindowThreadProcessId ( hPictureView, &dwPID );
            if (dwPID != mn_FarPID) {
                hPictureView = NULL; mb_PicViewWasHidden = FALSE;
            }
        }
    }

    
    if (hPictureView) {
        if (mb_PicViewWasHidden) {
            // ������ ���� ������ ��� ������������ �� ������ �������, �� �������� ��� �������
        } else if (!IsWindowVisible(hPictureView)) {
            // ���� �������� Help (F1) - ������ PictureView �������� (����� ��������), ������� ��� �������� ���
            hPictureView = NULL; mb_PicViewWasHidden = FALSE;
        }
    }

    if (mb_PicViewWasHidden && !hPictureView) mb_PicViewWasHidden = FALSE;

    return hPictureView;
}

HWND CRealConsole::ConWnd()
{
    if (!this) return NULL;
    return hConWnd;
}

TODO("���� ������ ������� ���� (��� ����� ������ �� �������) �� ����� ������� ������/����� ������");
void CRealConsole::FindPanels(BOOL abResetOnly/* = FALSE */)
{
    mr_LeftPanel = mr_RightPanel = MakeRect(-1,-1);
    if (isFar() && con.nTextHeight >= MIN_CON_HEIGHT && con.nTextWidth >= MIN_CON_WIDTH)
    {
        uint nY = 0;
        if (con.pConChar[0] == L' ')
            nY ++; // ������ �����, ������ ������ - ����
        else if (con.pConChar[0] == L'R' && con.pConChar[1] == L' ')
            nY ++; // ������ �����, ������ ������ - ����, ��� ���������� ������ �������
            
        uint nIdx = nY*con.nTextWidth;
        
        if (( (con.pConChar[nIdx] == L'[' && (con.pConChar[nIdx+1]>=L'0' && con.pConChar[nIdx+1]<=L'9')) // ������� ��������� ����������/��������
              || (con.pConChar[nIdx] == 0x2554 && con.pConChar[nIdx+1] == 0x2550) // ���.���� ���, ������ �����
            ) && con.pConChar[nIdx+con.nTextWidth] == 0x2551)
        {
            LPCWSTR pszCenter = con.pConChar + nIdx;
            LPCWSTR pszLine = con.pConChar + nIdx;
            uint nCenter = 0;
            while ( (pszCenter = wcsstr(pszCenter+1, L"\x2557\x2554")) != NULL ) {
                nCenter = pszCenter - pszLine;
                if (con.pConChar[nIdx+con.nTextWidth+nCenter] == 0x2551 && con.pConChar[nIdx+con.nTextWidth+nCenter+1] == 0x2551) {
                    break; // �����
                }
            }
            
            uint nBottom = con.nTextHeight - 1;
            while (nBottom > 4) {
                if (con.pConChar[con.nTextWidth*nBottom] == 0x255A && con.pConChar[con.nTextWidth*(nBottom-1)] == 0x2551)
                    break;
                nBottom --;
            }
            
            if (pszCenter && nBottom > 4) {
                mr_LeftPanel.left = 1;
                mr_LeftPanel.top = nY + 2;
                mr_LeftPanel.right = nCenter - 1;
                mr_LeftPanel.bottom = nBottom - 3;
                
                mr_RightPanel.left = nCenter + 3;
                mr_RightPanel.top = nY + 2;
                mr_RightPanel.right = con.nTextWidth - 2;
                mr_RightPanel.bottom = mr_LeftPanel.bottom;
            }
        }
    }
}

int CRealConsole::CoordInPanel(COORD cr)
{
	if (!this)
		return 0;

	if (CoordInRect(cr, mr_LeftPanel))
		return 1;
	if (CoordInRect(cr, mr_RightPanel))
		return 2;
	return 0;
}

void CRealConsole::GetPanelRect(BOOL abRight, RECT* prc)
{
	if (!this) {
		*prc = MakeRect(-1,-1);
		return;
	}

	if (abRight)
		*prc = mr_RightPanel;
	else
		*prc = mr_LeftPanel;
}

// � ���������� ���� �� ���������� �������� ��� ��������� ����������
// �� ������� ���� � ���� �� ����� ��������� ������.
// � ��������� ���������� ����������� ����� ���� ���������� ������ "Run as administrator"
// ����� ���� ��������������� ��������...
// ���� ������ ����� ��� ����������. � ����� ������� �� ����� ��������� ��������� ��� ������� ����������
bool CRealConsole::isAdministrator()
{
	if (!this) return false;
	return m_Args.bRunAsAdministrator;
}
