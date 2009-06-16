
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT 0x0500
//#endif

//#define CONEMUC_DLL_MODE

#define CSECTION_NON_RAISE

#include <Windows.h>
#include <WinCon.h>
#include <stdio.h>
#include <Shlwapi.h>
#include <Tlhelp32.h>
#include <vector>
#include "..\common\common.hpp"
extern "C" {
#include "..\common\ConEmuCheck.h"
}

WARNING("����������� ����� ������� ������� SetForegroundWindow �� GUI ����, ���� � ������ �������");
WARNING("����������� �������� ��� � ��� ������������� ��������");

WARNING("��� ������� ��� ComSpec �������� ������: {crNewSize.X>=MIN_CON_WIDTH && crNewSize.Y>=MIN_CON_HEIGHT}");
//E:\Source\FARUnicode\trunk\unicode_far\Debug.32.vc\ConEmuC.exe /c tools\gawk.exe -f .\scripts\gendate.awk


#ifdef _DEBUG
//  �����������������, ����� ����� ����� ������� �������� (conemuc.exe) �������� MessageBox, ����� ����������� ����������
  //#define SHOW_STARTED_MSGBOX
#endif

#ifdef _DEBUG
wchar_t gszDbgModLabel[6] = {0};
#endif


WARNING("!!!! ���� ����� ��� ��������� ������� ���������� ������� ���");
// � ��������� ��� � RefreshThread. ���� �� �� 0 - � ������ ������ (100��?)
// �� ������������� ���������� ������� � �������� ��� � 0.

#ifdef _DEBUG
CRITICAL_SECTION gcsHeap;
//#define MCHKHEAP { EnterCriticalSection(&gcsHeap); int MDEBUG_CHK=_CrtCheckMemory(); _ASSERTE(MDEBUG_CHK); LeaveCriticalSection(&gcsHeap); }
#define MCHKHEAP HeapValidate(ghHeap, 0, NULL);
//#define HEAP_LOGGING
#define DEBUGLOG(s) OutputDebugString(s)
#else
#define MCHKHEAP
#define DEBUGLOG(s)
#endif

#ifndef _DEBUG
// �������� �����
#define FORCE_REDRAW_FIX
#define RELATIVE_TRANSMIT_DISABLE
#else
// ���������� �����
//#define FORCE_REDRAW_FIX
#define PRINTCMDLINE
#endif


#define MIN_FORCEREFRESH_INTERVAL 100
#define MAX_FORCEREFRESH_INTERVAL 1000

#if !defined(CONSOLE_APPLICATION_16BIT)
#define CONSOLE_APPLICATION_16BIT       0x0000
#endif


WARNING("�������� ���-�� ����� ����������� ������������� ������ ����������� �������, � �� ������ �� �������");

WARNING("����� ������ ����� ������������ �������� ������� GUI ���� (���� ��� ����). ���� ����� ���� ������� �� far, � CMD.exe");

WARNING("���� GUI ����, ��� �� ���������� �� �������� - �������� ���������� ���� � �������� ���������� ����� ������");

WARNING("� ��������� ������� �� ����������� �� EVENT_CONSOLE_UPDATE_SIMPLE �� EVENT_CONSOLE_UPDATE_REGION");
// ������. ��������� cmd.exe. �������� �����-�� ����� � ��������� ������ � �������� 'Esc'
// ��� Esc ������� ������� ������ �� ���������, � ����� � ������� ���������!

#if defined(__GNUC__)
    //#include "assert.h"
    #ifndef _ASSERTE
    #define _ASSERTE(x)
    #endif
    #ifndef _ASSERT
    #define _ASSERT(x)
    #endif
#else
    #include <crtdbg.h>
#endif

#ifndef EVENT_CONSOLE_CARET
#define EVENT_CONSOLE_CARET             0x4001
#define EVENT_CONSOLE_UPDATE_REGION     0x4002
#define EVENT_CONSOLE_UPDATE_SIMPLE     0x4003
#define EVENT_CONSOLE_UPDATE_SCROLL     0x4004
#define EVENT_CONSOLE_LAYOUT            0x4005
#define EVENT_CONSOLE_START_APPLICATION 0x4006
#define EVENT_CONSOLE_END_APPLICATION   0x4007
#endif

#define SafeCloseHandle(h) { if ((h)!=NULL) { HANDLE hh = (h); (h) = NULL; if (hh!=INVALID_HANDLE_VALUE) CloseHandle(hh); } }


#ifndef CONEMUC_DLL_MODE
DWORD WINAPI InstanceThread(LPVOID);
DWORD WINAPI ServerThread(LPVOID lpvParam);
DWORD WINAPI InputThread(LPVOID lpvParam);
BOOL GetAnswerToRequest(CESERVER_REQ& in, CESERVER_REQ** out); 
DWORD WINAPI WinEventThread(LPVOID lpvParam);
void WINAPI WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
void CheckCursorPos();
void SendConsoleChanges(CESERVER_REQ* pOut);
CESERVER_REQ* CreateConsoleInfo(CESERVER_CHAR* pRgnOnly, BOOL bCharAttrBuff);
BOOL ReloadConsoleInfo(CESERVER_CHAR* pChangedRgn=NULL); // ���������� TRUE � ������ ���������
BOOL ReloadFullConsoleInfo(/*CESERVER_CHAR* pCharOnly=NULL*/); // � ��� ����� ������������ ����������
DWORD WINAPI RefreshThread(LPVOID lpvParam); // ����, �������������� ���������� �������
BOOL ReadConsoleData(CESERVER_CHAR* pCheck = NULL); //((LPRECT)1) ��� �������� LPRECT
void SetConsoleFontSizeTo(HWND inConWnd, int inSizeY, int inSizeX, wchar_t *asFontName);
int ServerInit(); // ������� ����������� ������� � ����
void ServerDone(int aiRc);
int ComspecInit();
void ComspecDone(int aiRc);
BOOL SetConsoleSize(USHORT BufferHeight, COORD crNewSize, SMALL_RECT rNewRect, LPCSTR asLabel = NULL);
void CreateLogSizeFile();
void LogSize(COORD* pcrSize, LPCSTR pszLabel);
BOOL WINAPI HandlerRoutine(DWORD dwCtrlType);
int GetProcessCount(DWORD **rpdwPID);
SHORT CorrectTopVisible(int nY);
void CorrectVisibleRect(CONSOLE_SCREEN_BUFFER_INFO* pSbi);
WARNING("������ GetConsoleScreenBufferInfo ����� ������������ MyGetConsoleScreenBufferInfo!");
BOOL MyGetConsoleScreenBufferInfo(HANDLE ahConOut, PCONSOLE_SCREEN_BUFFER_INFO apsc);
void EnlargeRegion(CESERVER_CHAR_HDR& rgn, const COORD crNew);
void CmdOutputStore();
void CmdOutputRestore();
LPVOID Alloc(size_t nCount, size_t nSize);
void Free(LPVOID ptr);
void CheckConEmuHwnd();
typedef BOOL (__stdcall *PGETCONSOLEKEYBOARDLAYOUTNAME)(wchar_t*);
PGETCONSOLEKEYBOARDLAYOUTNAME pfnGetConsoleKeyboardLayoutName = NULL; //(PGETCONSOLEKEYBOARDLAYOUTNAME)GetProcAddress (hKernel, "GetConsoleKeyboardLayoutNameW");
void CheckKeyboardLayout();

#else

PHANDLER_ROUTINE HandlerRoutine = NULL;

#endif

int ParseCommandLine(LPCWSTR asCmdLine, wchar_t** psNewCmd); // ������ ���������� ��������� ������
int NextArg(LPCWSTR &asCmdLine, wchar_t* rsArg/*[MAX_PATH+1]*/);
void Help();
void ExitWaitForKey(WORD vkKey, LPCWSTR asConfirm, BOOL abNewLine);


/*  Global  */
DWORD   gnSelfPID = 0;
HANDLE  ghConIn = NULL, ghConOut = NULL;
HWND    ghConWnd = NULL;
HWND    ghConEmuWnd = NULL; // Root! window
HANDLE  ghExitEvent = NULL;
HANDLE  ghFinilizeEvent = NULL;
BOOL    gbAlwaysConfirmExit = FALSE;
BOOL    gbAttachMode = FALSE;
DWORD   gdwMainThreadId = 0;
//int       gnBufferHeight = 0;
wchar_t* gpszRunCmd = NULL;
HANDLE  ghCtrlCEvent = NULL, ghCtrlBreakEvent = NULL;
HANDLE ghHeap = NULL; //HeapCreate(HEAP_GENERATE_EXCEPTIONS, nMinHeapSize, 0);
#ifdef _DEBUG
size_t gnHeapUsed = 0, gnHeapMax = 0;
#endif

enum tag_RunMode {
    RM_UNDEFINED = 0,
    RM_SERVER,
    RM_COMSPEC
} gnRunMode = RM_UNDEFINED;


struct tag_Srv {
    DWORD dwProcessGroup;
    //
    HANDLE hServerThread;   DWORD dwServerThreadId;
    HANDLE hRefreshThread;  DWORD dwRefreshThread;
    HANDLE hWinEventThread; DWORD dwWinEventThread;
    HANDLE hInputThread;    DWORD dwInputThreadId;
    //
    CRITICAL_SECTION csProc;
    CRITICAL_SECTION csConBuf;
    // ������ ��������� ��� �����, ����� ����������, ����� ������� ��� �� �����.
    // ��������, ��������� FAR, �� �������� Update, FAR �����������...
    std::vector<DWORD> nProcesses;
    //
    wchar_t szPipename[MAX_PATH], szInputname[MAX_PATH], szGuiPipeName[MAX_PATH];
    //
    HANDLE hConEmuGuiAttached;
    HWINEVENTHOOK hWinHook;
    //BOOL bContentsChanged; // ������ ������ ���������� ������ ���� ������
    wchar_t* psChars;
    WORD* pnAttrs;
        DWORD nBufCharCount;  // ������������ ������ (����� ���������� ������)
		DWORD nOneBufferSize; // ������ ��� ������� � GUI (������� ������)
    WORD* ptrLineCmp;
        DWORD nLineCmpSize;
    DWORD dwSelRc; CONSOLE_SELECTION_INFO sel; // GetConsoleSelectionInfo
    DWORD dwCiRc; CONSOLE_CURSOR_INFO ci; // GetConsoleCursorInfo
    DWORD dwConsoleCP, dwConsoleOutputCP, dwConsoleMode;
    DWORD dwSbiRc; CONSOLE_SCREEN_BUFFER_INFO sbi; // MyGetConsoleScreenBufferInfo
    //USHORT nUsedHeight; // ������, ������������ � GUI - ������ ���� ���������� gcrBufferSize.Y
    SHORT nTopVisibleLine; // ��������� � GUI ����� ���� �������������. ���� -1 - ��� ����������, ���������� ������� ��������
	SHORT nVisibleHeight;  // �� ����, ������ ���� ����� (gcrBufferSize.Y). ��� ��������������� ���������� ����� psChars & pnAttrs
    DWORD nMainTimerElapse;
    BOOL  bConsoleActive;
    HANDLE hRefreshEvent; // ServerMode, ���������� �������, � ���� ���� ��������� - �������� � GUI
    //HANDLE hChangingSize; // FALSE �� ����� ����� ������� �������
	CRITICAL_SECTION csChangeSize; DWORD ncsTChangeSize;
    BOOL  bNeedFullReload;  // ����� ������ ���� �������
    BOOL  bForceFullSend; // ���������� �������� ������ ���������� �������, � �� ������ ����������
	BOOL  bRequestPostFullReload; // �� ����� ������ ��������� ������ - ����� ��������� ��������� ����!
    //DWORD nLastUpdateTick; // ��� FORCE_REDRAW_FIX
	DWORD nLastPacketID; // �� ������ ��� �������� � GUI
	// ���� �������� ������ ���� ������... (�� ���������� ��� �����)
	//BOOL bCharChangedSet; 
	CESERVER_CHAR CharChanged; CRITICAL_SECTION csChar;
	
	// ����� ��� ������� � �������
	DWORD nChangedBufferSize;
	CESERVER_CHAR *pChangedBuffer;

	// ����������� Output ���������� cmd...
	//
	// Keyboard layout name
	wchar_t szKeybLayout[KL_NAMELENGTH+1];

	// Optional console font (may be specified in registry)
	wchar_t szConsoleFont[LF_FACESIZE];
	//wchar_t szConsoleFontFile[MAX_PATH]; -- �� ��������
	SHORT nConFontWidth, nConFontHeight;
} srv = {0};


#pragma pack(push, 1)
CESERVER_CONSAVE* gpStoredOutput = NULL;
#pragma pack(pop)

struct tag_Cmd {
    DWORD dwProcessGroup;
    DWORD dwFarPID;
    BOOL  bK;
    BOOL  bNonGuiMode; // ���� ������� �� � �������, ����������� � GUI. ����� ���� ��-�� ����, ��� �������� ��� COMSPEC
    CONSOLE_SCREEN_BUFFER_INFO sbi;
    BOOL  bNewConsole;
} cmd = {0};

COORD gcrBufferSize = {80,25};
BOOL  gbParmBufferSize = FALSE;
SHORT gnBufferHeight = 0;

HANDLE ghLogSize = NULL;
wchar_t* wpszLogSizeFile = NULL;


BOOL gbInRecreateRoot = FALSE;

//#define CES_NTVDM 0x10 -- common.hpp
DWORD dwActiveFlags = 0;

#define CERR_GETCOMMANDLINE 100
#define CERR_CARGUMENT 101
#define CERR_CMDEXENOTFOUND 102
#define CERR_NOTENOUGHMEM1 103
#define CERR_CREATESERVERTHREAD 104
#define CERR_CREATEPROCESS 105
#define CERR_WINEVENTTHREAD 106
#define CERR_CONINFAILED 107
#define CERR_GETCONSOLEWINDOW 108
#define CERR_EXITEVENT 109
#define CERR_GLOBALUPDATE 110
#define CERR_WINHOOKNOTCREATED 111
#define CERR_CREATEINPUTTHREAD 112
#define CERR_CONOUTFAILED 113
#define CERR_PROCESSTIMEOUT 114
#define CERR_REFRESHEVENT 115
#define CERR_CREATEREFRESHTHREAD 116
#define CERR_CMDLINE 117
#define CERR_HELPREQUESTED 118
#define CERR_ATTACHFAILED 119
#define CERR_CMDLINEEMPTY 120
#define CERR_RUNNEWCONSOLE 121


int main()
{
    TODO("����� ��� ������� �������� �������, �������������� �������� 80x25 � ��������� ������� �����");

#ifdef _DEBUG
	InitializeCriticalSection(&gcsHeap);
#endif

    int iRc = 100;
    //wchar_t sComSpec[MAX_PATH];
    //wchar_t* psFilePart;
    //wchar_t* psCmdLine = GetCommandLineW();
    //size_t nCmdLine = lstrlenW(psCmdLine);
    //wchar_t* psNewCmd = NULL;
    //HANDLE hWait[2]={NULL,NULL};
    //BOOL bViaCmdExe = TRUE;
    PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
    STARTUPINFOW si; memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
    DWORD dwErr = 0, nWait = 0;
    BOOL lbRc = FALSE;
    DWORD mode = 0;
    //BOOL lb = FALSE;

	ghHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 200000, 0);

    // ����� ����������� ����
    ghConWnd = GetConsoleWindow();
    _ASSERTE(ghConWnd!=NULL);
    if (!ghConWnd) {
        dwErr = GetLastError();
        wprintf(L"ghConWnd==NULL, ErrCode=0x%08X\n", dwErr); 
        iRc = CERR_GETCONSOLEWINDOW; goto wrap;
    }
    // PID
    gnSelfPID = GetCurrentProcessId();
    gdwMainThreadId = GetCurrentThreadId();
    
#ifdef SHOW_STARTED_MSGBOX
    if (!IsDebuggerPresent()) MessageBox(GetConsoleWindow(),GetCommandLineW(),L"ComEmuC Loaded",0);
#endif

	#ifdef PRINTCMDLINE
	wprintf(L"ConEmuC: %s\n", GetCommandLineW());
	#endif
    
    if ((iRc = ParseCommandLine(GetCommandLineW(), &gpszRunCmd)) != 0)
        goto wrap;
    //#ifdef _DEBUG
    //CreateLogSizeFile();
    //#endif
    
    /* ***************************** */
    /* *** "�����" ������������� *** */
    /* ***************************** */
    
    
    // ������� ������������ ��� ���� �������
    ghExitEvent = CreateEvent(NULL, TRUE/*������������ � ���������� �����, manual*/, FALSE, NULL);
    if (!ghExitEvent) {
        dwErr = GetLastError();
        wprintf(L"CreateEvent() failed, ErrCode=0x%08X\n", dwErr); 
        iRc = CERR_EXITEVENT; goto wrap;
    }
    ResetEvent(ghExitEvent);
    ghFinilizeEvent = CreateEvent(NULL, TRUE/*������������ � ���������� �����, manual*/, FALSE, NULL);
    if (!ghFinilizeEvent) {
        dwErr = GetLastError();
        wprintf(L"CreateEvent() failed, ErrCode=0x%08X\n", dwErr); 
        iRc = CERR_EXITEVENT; goto wrap;
    }
    ResetEvent(ghFinilizeEvent);

    // �����������
    ghConIn  = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
                0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (ghConIn == INVALID_HANDLE_VALUE) {
        dwErr = GetLastError();
        wprintf(L"CreateFile(CONIN$) failed, ErrCode=0x%08X\n", dwErr); 
        iRc = CERR_CONINFAILED; goto wrap;
    }
    // �����������
    ghConOut = CreateFile(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
                0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (ghConOut == INVALID_HANDLE_VALUE) {
        dwErr = GetLastError();
        wprintf(L"CreateFile(CONOUT$) failed, ErrCode=0x%08X\n", dwErr); 
        iRc = CERR_CONOUTFAILED; goto wrap;
    }
    
    //2009-05-30 ��������� ��� ����� ?
    //SetHandleInformation(GetStdHandle(STD_INPUT_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    //SetHandleInformation(GetStdHandle(STD_OUTPUT_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    //SetHandleInformation(GetStdHandle(STD_ERROR_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    
    mode = 0;
    /*lb = GetConsoleMode(ghConIn, &mode);
    if (!(mode & ENABLE_MOUSE_INPUT)) {
        mode |= ENABLE_MOUSE_INPUT;
        lb = SetConsoleMode(ghConIn, mode);
    }*/

    // �����������, ����� �� CtrlC �� ��������
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)HandlerRoutine, true);
    //SetConsoleMode(ghConIn, 0);

    /* ******************************** */
    /* *** "��������" ������������� *** */
    /* ******************************** */
    if (gnRunMode == RM_SERVER) {
        if ((iRc = ServerInit()) != 0)
            goto wrap;
    } else {
        if ((iRc = ComspecInit()) != 0)
            goto wrap;
    }

    
    
    
    /* ********************************* */
    /* *** ������ ��������� �������� *** */
    /* ********************************* */
    
    // CREATE_NEW_PROCESS_GROUP - ����, ��������� �������� Ctrl-C
    lbRc = CreateProcessW(NULL, gpszRunCmd, NULL,NULL, TRUE, 
            NORMAL_PRIORITY_CLASS/*|CREATE_NEW_PROCESS_GROUP*/, 
            NULL, NULL, &si, &pi);
    dwErr = GetLastError();
    if (!lbRc)
    {
        wprintf (L"Can't create process, ErrCode=0x%08X! Command to be executed:\n%s\n", dwErr, gpszRunCmd);
        iRc = CERR_CREATEPROCESS; goto wrap;
    }
    //delete psNewCmd; psNewCmd = NULL;
	AllowSetForegroundWindow(pi.dwProcessId);


    
    /* ************************ */
    /* *** �������� ������� *** */
    /* ************************ */
    
    if (gnRunMode == RM_SERVER) {
        srv.dwProcessGroup = pi.dwProcessId;

        if (pi.hProcess) SafeCloseHandle(pi.hProcess); 
        if (pi.hThread) SafeCloseHandle(pi.hThread);

        if (srv.hConEmuGuiAttached) {
            if (WaitForSingleObject(srv.hConEmuGuiAttached, 1000) == WAIT_OBJECT_0) {
                // GUI ���� �����
                wsprintf(srv.szGuiPipeName, CEGUIPIPENAME, L".", (DWORD)ghConWnd); // ��� gnSelfPID
            }
        }
    
        // ����, ���� � ������� �� ��������� ��������� (����� ������)
        TODO("���������, ����� �� ��� ����������, ��� CreateProcess ������, � � ������� �� �� ����������? �����, ���� ������� GUI");
        nWait = WaitForSingleObject(ghFinilizeEvent, 6*1000); //������ �������� �������� ����� ��������� ���������
        if (nWait != WAIT_OBJECT_0) { // ���� �������
            EnterCriticalSection(&srv.csProc);
            iRc = srv.nProcesses.size();
            LeaveCriticalSection(&srv.csProc);
            // � ��������� � ������� ��� ��� ���
            if (iRc == 0) {
                wprintf (L"Process was not attached to console. Is it GUI?\nCommand to be executed:\n%s\n", gpszRunCmd);
                iRc = CERR_PROCESSTIMEOUT; goto wrap;
            }
        }
    } else {
        // � ������ ComSpec ��� ���������� ���������� ������ ��������� ��������

        wchar_t szEvtName[128];

        wsprintf(szEvtName, CESIGNAL_C, pi.dwProcessId);
        ghCtrlCEvent = CreateEvent(NULL, FALSE, FALSE, szEvtName);
        wsprintf(szEvtName, CESIGNAL_BREAK, pi.dwProcessId);
        ghCtrlBreakEvent = CreateEvent(NULL, FALSE, FALSE, szEvtName);
    }

    /* *************************** */
    /* *** �������� ���������� *** */
    /* *************************** */
wait:    
    if (gnRunMode == RM_SERVER) {
        // �� ������� ���� ���� ������� � ������� ����������. ���� ���� � ������� �� ��������� ������ ����� ���
        WaitForSingleObject(ghFinilizeEvent, INFINITE);
	} else {
        HANDLE hEvents[3];
        hEvents[0] = pi.hProcess;
        hEvents[1] = ghCtrlCEvent;
        hEvents[2] = ghCtrlBreakEvent;
        //WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD dwWait = 0;
        BOOL lbGenRc = FALSE;
        while ((dwWait = WaitForMultipleObjects(3, hEvents, FALSE, INFINITE)) != WAIT_OBJECT_0)
        {
            if (dwWait == (WAIT_OBJECT_0+1) || dwWait == (WAIT_OBJECT_0+2)) {
                DWORD dwEvent = (dwWait == (WAIT_OBJECT_0+1)) ? CTRL_C_EVENT : CTRL_BREAK_EVENT;
                /*DWORD dwMode = 0;
                HANDLE hConIn  = CreateFile(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
                    0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
                GetConsoleMode(hConIn, &dwMode);
                SetConsoleMode(hConIn, dwMode);
                CloseHandle(hConIn);*/

                lbGenRc = GenerateConsoleCtrlEvent(dwEvent, pi.dwProcessId);
            }
        }
        // ����� ������� ������
        if (pi.hProcess) SafeCloseHandle(pi.hProcess); 
        if (pi.hThread) SafeCloseHandle(pi.hThread);
	}
    
    
    
    /* ************************* */
    /* *** ���������� ������ *** */
    /* ************************* */
    
    iRc = 0;
wrap:
    // 
    if ((iRc!=0 && iRc!=CERR_RUNNEWCONSOLE) || gbAlwaysConfirmExit) {
        ExitWaitForKey(VK_RETURN, L"\n\nPress Enter to close console, or wait...", TRUE);
        if (iRc == CERR_PROCESSTIMEOUT) {
            EnterCriticalSection(&srv.csProc);
            int nCount = srv.nProcesses.size();
            LeaveCriticalSection(&srv.csProc);
	        if (nCount > 0) {
		        // ������� ���� ����������!
		        goto wait;
	        }
        }
    }

    // �� ������ ������ - �������� �������
    if (ghExitEvent) SetEvent(ghExitEvent);
    
    
    /* ***************************** */
    /* *** "��������" ���������� *** */
    /* ***************************** */
    
    if (gnRunMode == RM_SERVER) {
        ServerDone(iRc);
        //MessageBox(0,L"Server done...",L"ComEmuC",0);
    } else {
        ComspecDone(iRc);
        //MessageBox(0,L"Comspec done...",L"ComEmuC",0);
    }

    
    /* ************************** */
    /* *** "�����" ���������� *** */
    /* ************************** */
    
    if (ghConIn && ghConIn!=INVALID_HANDLE_VALUE) {
        SafeCloseHandle(ghConIn);
    }
    if (ghConOut && ghConOut!=INVALID_HANDLE_VALUE) {
        SafeCloseHandle(ghConOut);
    }

    SafeCloseHandle(ghLogSize);
    if (wpszLogSizeFile) {
        DeleteFile(wpszLogSizeFile);
        Free(wpszLogSizeFile); wpszLogSizeFile = NULL;
    }
    
    if (gpszRunCmd) { delete gpszRunCmd; gpszRunCmd = NULL; }

	if (ghHeap) {
		HeapDestroy(ghHeap);
		ghHeap = NULL;
	}

    return iRc;
}

void Help()
{
    wprintf(
        L"ConEmuC. Copyright (c) 2009, Maximus5\n"
        L"This is a console part of ConEmu product.\n"
        L"Usage: ComEmuC [switches] /C <command line, passed to %%COMSPEC%%>\n"
        L"   or: ComEmuC [switches] /CMD <program with arguments, far.exe for example>\n"
        L"   or: ComEmuC /?\n"
        L"Switches:\n"
        L"        /CONFIRM  - confirm closing console on program termination\n"
        L"        /ATTACH   - auto attach to ConEmu GUI\n"
        L"        /B{W|H|Z} - define buffer width, height, window height\n"
        L"        /LOG      - create (debug) log file\n"
    );
}

// ������ ���������� ��������� ������
int ParseCommandLine(LPCWSTR asCmdLine, wchar_t** psNewCmd)
{
    int iRc = 0;
    wchar_t szArg[MAX_PATH+1] = {0};
    wchar_t szComSpec[MAX_PATH+1] = {0};
    LPCWSTR pwszCopy = NULL;
    wchar_t* psFilePart = NULL;
    BOOL bViaCmdExe = TRUE;
    size_t nCmdLine = 0;
    LPCWSTR pwszStartCmdLine = asCmdLine;
    
    if (!asCmdLine || !*asCmdLine)
    {
        DWORD dwErr = GetLastError();
        wprintf (L"GetCommandLineW failed! ErrCode=0x%08X\n", dwErr);
        return CERR_GETCOMMANDLINE;
    }

    gnRunMode = RM_UNDEFINED;
    
    
    while ((iRc = NextArg(asCmdLine, szArg)) == 0)
    {
        if (wcscmp(szArg, L"/?")==0 || wcscmp(szArg, L"-?")==0 || wcscmp(szArg, L"/h")==0 || wcscmp(szArg, L"-h")==0) {
            Help();
            return CERR_HELPREQUESTED;
        } else 
        
        if (wcscmp(szArg, L"/CONFIRM")==0) {
            gbAlwaysConfirmExit = TRUE;
        } else

        if (wcscmp(szArg, L"/ATTACH")==0) {
            gbAttachMode = TRUE;
            gnRunMode = RM_SERVER;
        } else

        if (wcsncmp(szArg, L"/B", 2)==0) {
            if (wcsncmp(szArg, L"/BW=", 4)==0) {
                gcrBufferSize.X = _wtoi(szArg+4); gbParmBufferSize = TRUE;
            } else if (wcsncmp(szArg, L"/BH=", 4)==0) {
                gcrBufferSize.Y = _wtoi(szArg+4); gbParmBufferSize = TRUE;
            } else if (wcsncmp(szArg, L"/BZ=", 4)==0) {
                gnBufferHeight = _wtoi(szArg+4); gbParmBufferSize = TRUE;
            }
        } else

		if (wcsncmp(szArg, L"/F", 2)==0) {
			if (wcsncmp(szArg, L"/FN=", 4)==0) {
				lstrcpynW(srv.szConsoleFont, szArg+4, 32);
			} else if (wcsncmp(szArg, L"/FW=", 4)==0) {
				srv.nConFontWidth = _wtoi(szArg+4);
			} else if (wcsncmp(szArg, L"/FH=", 4)==0) {
				srv.nConFontHeight = _wtoi(szArg+4);
			//} else if (wcsncmp(szArg, L"/FF=", 4)==0) {
			//	lstrcpynW(srv.szConsoleFontFile, szArg+4, MAX_PATH);
			}
		} else
        
        if (wcscmp(szArg, L"/LOG")==0) {
            CreateLogSizeFile();
        } else

        // ����� ���� ���������� - ���� ��, ��� ���������� � CreateProcess!
        if (wcscmp(szArg, L"/C")==0 || wcscmp(szArg, L"/c")==0 || wcscmp(szArg, L"/K")==0 || wcscmp(szArg, L"/k")==0) {
            gnRunMode = RM_COMSPEC;
            cmd.bK = (wcscmp(szArg, L"/K")==0 || wcscmp(szArg, L"/k")==0);
            break; // asCmdLine ��� ��������� �� ����������� ���������
        } else if (wcscmp(szArg, L"/CMD")==0 || wcscmp(szArg, L"/cmd")==0) {
            gnRunMode = RM_SERVER;
            break; // asCmdLine ��� ��������� �� ����������� ���������
        }
    }
    
    if (iRc != 0) {
        if (iRc == CERR_CMDLINEEMPTY) {
            Help();
            wprintf (L"\n\nParsing command line failed (/C argument not found):\n%s\n", GetCommandLineW());
        } else {
            wprintf (L"Parsing command line failed:\n%s\n", asCmdLine);
        }
        return iRc;
    }
    if (gnRunMode == RM_UNDEFINED) {
        wprintf (L"Parsing command line failed (/C argument not found):\n%s\n", GetCommandLineW());
        return CERR_CARGUMENT;
    }

    if (gnRunMode == RM_COMSPEC) {
    
		// ����� ������� ������� ����� �������?
		int nArgLen = lstrlenA(" -new_console");
		pwszCopy = (wchar_t*)wcsstr(asCmdLine, L" -new_console");
		// ���� ����� -new_console ���� ������, ��� ��� ������ ����� ������
		if (pwszCopy && 
			(pwszCopy[nArgLen]==L' ' || pwszCopy[nArgLen]==0
			 || (pwszCopy[nArgLen]==L'"' || pwszCopy[nArgLen+1]==0)))
		{
			// ����� ������������
			cmd.bNewConsole = TRUE;
			//
			int nNewLen = wcslen(pwszStartCmdLine) + 100;
			wchar_t* pszNewCmd = new wchar_t[nNewLen];
			if (!pszNewCmd) {
		        wprintf (L"Can't allocate %i wchars!\n", nNewLen);
		        return CERR_NOTENOUGHMEM1;
			}
			// ������� ����������� ���, ��� ���� �� /c
			const wchar_t* pszC = asCmdLine;
			while (*pszC != L'/') pszC --;
			nNewLen = pszC - pwszStartCmdLine;
			_ASSERTE(nNewLen>0);
			wcsncpy(pszNewCmd, pwszStartCmdLine, nNewLen);
			pszNewCmd[nNewLen] = 0;
			// �������� ������ ��������
			if (!gbAttachMode)
				wcscat(pszNewCmd, L" /ATTACH ");
			if (!gbAlwaysConfirmExit)
				wcscat(pszNewCmd, L" /CONFIRM ");
			// ������������ ����� �������
			// "cmd" ������ ��� ���� �� ������� �������� ������� � ������, ������� �� �� �����
			// cmd /c ""c:\program files\arc\7z.exe" -?"   // �� ��� � ������ ����� ���� ��������...
			// cmd /c "dir c:\"
			// � ��.
			wcscat(pszNewCmd, L" /CMD cmd /C ");
			nNewLen = pwszCopy - asCmdLine;
			psFilePart = pszNewCmd + lstrlenW(pszNewCmd);
			wcsncpy(psFilePart, asCmdLine, nNewLen); psFilePart += nNewLen;
			pwszCopy += nArgLen;
			if (*pwszCopy) wcscpy(psFilePart, pwszCopy);
			//MessageBox(NULL, pszNewCmd, L"CmdLine", 0);
			//return 200;
			// ����� �����������
			*psNewCmd = pszNewCmd;
			return 0;
		}
    
        pwszCopy = asCmdLine;
        if ((iRc = NextArg(pwszCopy, szArg)) != 0) {
            wprintf (L"Parsing command line failed:\n%s\n", asCmdLine);
            return iRc;
        }
        pwszCopy = wcsrchr(szArg, L'\\'); if (!pwszCopy) pwszCopy = szArg;
    
        #pragma warning( push )
        #pragma warning(disable : 6400)
        if (lstrcmpiW(pwszCopy, L"cmd")==0 || lstrcmpiW(pwszCopy, L"cmd.exe")==0) {
            bViaCmdExe = FALSE; // ��� ������ ��������� ���������, cmd.exe � ������ ��������� �� �����
        }
        #pragma warning( pop )
    } else {
        bViaCmdExe = FALSE; // ��������� ����������� ��������� ��� ConEmuC (��������� �����)
    }
    
    nCmdLine = lstrlenW(asCmdLine);

    if (!bViaCmdExe) {
        nCmdLine += 1; // ������ ����� ��� 0
    } else {
        // ���� ���������� ComSpecC - ������ ConEmuC ������������� ����������� ComSpec
        if (!GetEnvironmentVariable(L"ComSpecC", szComSpec, MAX_PATH) || szComSpec[0] == 0)
            if (!GetEnvironmentVariable(L"ComSpec", szComSpec, MAX_PATH) || szComSpec[0] == 0)
                szComSpec[0] = 0;
        if (szComSpec[0] != 0) {
            // ������ ���� ��� (��������) �� conemuc.exe
            pwszCopy = wcsrchr(szComSpec, L'\\'); if (!pwszCopy) pwszCopy = szComSpec;
            #pragma warning( push )
            #pragma warning(disable : 6400)
            if (lstrcmpiW(pwszCopy, L"ConEmuC")==0 || lstrcmpiW(pwszCopy, L"ConEmuC.exe")==0)
                szComSpec[0] = 0;
            #pragma warning( pop )
        }
        
        // ComSpec/ComSpecC �� ���������, ���������� cmd.exe
        if (szComSpec[0] == 0) {
            if (!SearchPathW(NULL, L"cmd.exe", NULL, MAX_PATH, szComSpec, &psFilePart))
            {
                wprintf (L"Can't find cmd.exe!\n");
                return CERR_CMDEXENOTFOUND;
            }
        }

        nCmdLine += lstrlenW(szComSpec)+10; // "/C" � �������
    }

    *psNewCmd = new wchar_t[nCmdLine];
    if (!(*psNewCmd))
    {
        wprintf (L"Can't allocate %i wchars!\n", nCmdLine);
        return CERR_NOTENOUGHMEM1;
    }
    
	lstrcpyW( *psNewCmd, asCmdLine );
    
    // ������ ��������� �������
    if (*asCmdLine == L'"') {
	    if (asCmdLine[1]) {
		    wchar_t *pszTitle = *psNewCmd;
		    wchar_t *pszEndQ = pszTitle + lstrlenW(pszTitle) - 1;
		    if (pszEndQ > (pszTitle+1) && *pszEndQ == L'"') {
			    *pszEndQ = 0; pszTitle ++;
		    } else {
			    pszEndQ = NULL;
		    }
		    SetWindowText(ghConWnd, pszTitle);
		    if (pszEndQ) *pszEndQ = L'"';
		}
    } else if (*asCmdLine) {
	    SetWindowText(ghConWnd, asCmdLine);
    }
    
    if (bViaCmdExe)
    {
        if (wcschr(szComSpec, L' ')) {
            (*psNewCmd)[0] = L'"';
            lstrcpyW( (*psNewCmd)+1, szComSpec );
            lstrcatW( (*psNewCmd), cmd.bK ? L"\" /K " : L"\" /C " );
        } else {
            lstrcpyW( (*psNewCmd), szComSpec );
            lstrcatW( (*psNewCmd), cmd.bK ? L" /K " : L" /C " );
        }
        lstrcatW( (*psNewCmd), asCmdLine );
    }
    
    return 0;
}

int NextArg(LPCWSTR &asCmdLine, wchar_t* rsArg/*[MAX_PATH+1]*/)
{
    LPCWSTR psCmdLine = asCmdLine, pch = NULL;
    wchar_t ch = *psCmdLine;
    int nArgLen = 0;
    
    while (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n') ch = *(++psCmdLine);
    if (ch == 0) return CERR_CMDLINEEMPTY;

    // �������� ���������� � "
    if (ch == L'"') {
        psCmdLine++;
        pch = wcschr(psCmdLine, L'"');
        if (!pch) return CERR_CMDLINE;
        while (pch[1] == L'"') {
            pch += 2;
            pch = wcschr(pch, L'"');
            if (!pch) return CERR_CMDLINE;
        }
        // ������ � pch ������ �� ��������� "
    } else {
        // �� ����� ������ ��� �� ������� �������
        //pch = wcschr(psCmdLine, L' ');
        // 09.06.2009 Maks - ��������� ��: cmd /c" echo Y "
        pch = psCmdLine;
        while (*pch && *pch!=L' ' && *pch!=L'"') pch++;
        //if (!pch) pch = psCmdLine + wcslen(psCmdLine); // �� ����� ������
    }
    
    nArgLen = pch - psCmdLine;
    if (nArgLen > MAX_PATH) return CERR_CMDLINE;

    // ������� ��������
    memcpy(rsArg, psCmdLine, nArgLen*sizeof(wchar_t));
    rsArg[nArgLen] = 0;

    psCmdLine = pch;
    
    // Finalize
    ch = *psCmdLine; // ����� ��������� �� ����������� �������
    if (ch == L'"') ch = *(++psCmdLine);
    while (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n') ch = *(++psCmdLine);
    asCmdLine = psCmdLine;
    
    return 0;
}

void ExitWaitForKey(WORD vkKey, LPCWSTR asConfirm, BOOL abNewLine)
{
    // ����� ������ ���� ��������� �����
    BOOL lbNeedVisible = FALSE;
    if (!ghConWnd) ghConWnd = GetConsoleWindow();
    if (ghConWnd) { // ���� ������� ���� ������
        WARNING("���� GUI ��� - �������� �� ������� SendMessageTimeout - ���������� ������� �� �����. �� ������� ����������");
        if (!IsWindowVisible(ghConWnd)) {
            lbNeedVisible = TRUE;
            // ��������� "�����������" 80x25, ��� ��, ��� ���� �������� � ���.������
            SMALL_RECT rcNil = {0}; SetConsoleSize(0, gcrBufferSize, rcNil, ":Exiting");
            //SetConsoleFontSizeTo(ghConWnd, 8, 12); // ��������� ����� ��������
            ShowWindow(ghConWnd, SW_SHOWNORMAL); // � ������� ������
        }
    }

    // ������� ��������� �����
    INPUT_RECORD r = {0}; DWORD dwCount = 0;
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));

    //
    wprintf(asConfirm);

    //if (lbNeedVisible)
    // ���� ������ ����� ���� ������ - ������ GUI ����� ����, � ����� ������������ �� �����
    //while (PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwCount)) {
    //    if (dwCount)
    //        ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwCount);
    //    else
    //        Sleep(100);
    //    if (lbNeedVisible && !IsWindowVisible(ghConWnd)) {
    //        ShowWindow(ghConWnd, SW_SHOWNORMAL); // � ������� ������
    //    }
    while (ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwCount)) {
		if (gnRunMode == RM_SERVER) {
			EnterCriticalSection(&srv.csProc);
			int nCount = srv.nProcesses.size();
			LeaveCriticalSection(&srv.csProc);
			if (nCount > 0) {
				// ! ������� ���� ����������, ����������� �� �����. ������� ������� � �����!
				WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, 1, &dwCount);
				break;
			}
		}
    
        if (r.EventType == KEY_EVENT && r.Event.KeyEvent.bKeyDown && r.Event.KeyEvent.wVirtualKeyCode == vkKey)
            break;
    }
    //MessageBox(0,L"Debug message...............1",L"ComEmuC",0);
    //int nCh = _getch();
    if (abNewLine)
        wprintf(L"\n");
}


#ifndef CONEMUC_DLL_MODE

int ComspecInit()
{
    TODO("���������� ��� ������������� ��������, � ���� ��� FAR - ��������� ��� (��� ����������� � ����� �������)");
    TODO("������ �������� �� GUI, ���� ��� ����, ����� - �� ���������");
    TODO("GUI ����� ��������������� ������ � ������ ������ ���������");

	// ������ ������ ������ ��� GUI, ����� ��������� ConEmuC!
	#ifdef SHOW_STARTED_MSGBOX
	MessageBox(GetConsoleWindow(), L"ConEmuC (comspec mode) is about to START", L"ConEmuC.ComSpec", 0);
	#endif


    int nNewBufferHeight = 0;
	COORD crNewSize = {0,0};
    SMALL_RECT rNewWindow = cmd.sbi.srWindow;
	BOOL lbSbiRc = FALSE;
	

	// ��� �������� � �� �����, ������ ��� ����������...
	lbSbiRc = MyGetConsoleScreenBufferInfo(ghConOut, &cmd.sbi);
	
	
	if (cmd.bNewConsole) {
	    PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
	    STARTUPINFOW si; memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW|STARTF_USECOUNTCHARS;
        si.dwXCountChars = cmd.sbi.dwSize.X;
		si.dwYCountChars = cmd.sbi.dwSize.Y;
        si.wShowWindow = SW_HIDE;
	
	    // CREATE_NEW_PROCESS_GROUP - ����, ��������� �������� Ctrl-C
	    BOOL lbRc = CreateProcessW(NULL, gpszRunCmd, NULL,NULL, TRUE, 
	            NORMAL_PRIORITY_CLASS|CREATE_NEW_CONSOLE, 
	            NULL, NULL, &si, &pi);
	    DWORD dwErr = GetLastError();
	    if (!lbRc)
	    {
	        wprintf (L"Can't create process, ErrCode=0x%08X! Command to be executed:\n%s\n", dwErr, gpszRunCmd);
	        return CERR_CREATEPROCESS;
	    }
	    //delete psNewCmd; psNewCmd = NULL;
		AllowSetForegroundWindow(pi.dwProcessId);
		wprintf(L"New console created. PID=%i. Exiting...\n", pi.dwProcessId);
		SafeCloseHandle(pi.hProcess); SafeCloseHandle(pi.hThread);
		gbAlwaysConfirmExit = FALSE;
		return CERR_RUNNEWCONSOLE;
	}
	

	crNewSize = cmd.sbi.dwSize;
	_ASSERTE(crNewSize.X>=MIN_CON_WIDTH && crNewSize.Y>=MIN_CON_HEIGHT);
    
    CESERVER_REQ *pIn = NULL, *pOut = NULL;
    int nSize = sizeof(CESERVER_REQ_HDR)+3*sizeof(DWORD);
    pIn = (CESERVER_REQ*)Alloc(nSize,1);
    if (pIn) {
        pIn->hdr.nCmd = CECMD_CMDSTARTSTOP;
		pIn->hdr.nSrcThreadId = GetCurrentThreadId();
        pIn->hdr.nSize = nSize;
        pIn->hdr.nVersion = CESERVER_REQ_VER;
        ((DWORD*)(pIn->Data))[0] = 2; // Cmd ����� �����
        ((DWORD*)(pIn->Data))[1] = (DWORD)ghConWnd;
        ((DWORD*)(pIn->Data))[2] = gnSelfPID;

        pOut = ExecuteGuiCmd(ghConWnd, pIn);
        if (pOut) {
			BOOL  bAlreadyBufferHeight = *(DWORD*)(pOut->Data);
			#ifdef _DEBUG
			HWND  hGuiWnd = (HWND)*(DWORD*)(pOut->Data+4);
			#endif
			DWORD nGuiPID = *(DWORD*)(pOut->Data+8);

			AllowSetForegroundWindow(nGuiPID);

			// ����� ��� ����������, ��� ���� COMSPEC ������� �� �������.
			if (bAlreadyBufferHeight)
				cmd.bNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������ - ��������� ������ ��������

            //nNewBufferHeight = ((DWORD*)(pOut->Data))[0];
            //crNewSize.X = (SHORT)((DWORD*)(pOut->Data))[1];
            //crNewSize.Y = (SHORT)((DWORD*)(pOut->Data))[2];
            TODO("���� �� ������� ��� COMSPEC - �� � GUI �������� ��������� ����� �� ������");
            //if (rNewWindow.Right >= crNewSize.X) // ������ ��� �������� �� ���� ������ ���������
            //    rNewWindow.Right = crNewSize.X-1;
            ExecuteFreeResult(pOut); pOut = NULL;

            gnBufferHeight = nNewBufferHeight;
        } else {
            cmd.bNonGuiMode = TRUE; // �� �������� ExecuteGuiCmd ��� ������. ��� �� ���� �������
        }
        Free(pIn); pIn = NULL;
    }

	//	if (crNewSize.X && crNewSize.Y) {
	//       if (!cmd.bNonGuiMode)
	//       {
	//           if (gnBufferHeight > nNewBufferHeight)
	//               nNewBufferHeight = gnBufferHeight;
	//           //SMALL_RECT rc = {0}; 
	//           //COORD crNew = {cmd.sbi.dwSize.X,cmd.sbi.dwSize.Y};
	//           SetConsoleSize(nNewBufferHeight, crNewSize, rNewWindow, "ComspecInit");
	//       }
	//   }
    return 0;
}

void ComspecDone(int aiRc)
{
    //WARNING("������� � GUI CONEMUCMDSTOPPED");

    //TODO("��������� ������ ����� ���� (���� �������� - FAR) ��� ������� ��������. ������ ������ ������� � ��������� ���������� ������� � ������ ����� ������� ���������� � ConEmuC!");

	BOOL lbRc1 = FALSE, lbRc2 = FALSE;
	CONSOLE_SCREEN_BUFFER_INFO sbi1 = {{0,0}}, sbi2 = {{0,0}};
	// ��� ����� ��������, � �� ����������������� ����������!
	if (!cmd.bNonGuiMode) // ���� GUI �� ������ ����� ������ ������� ������ ������ - ��� ����� ������� ���!
		lbRc1 = GetConsoleScreenBufferInfo(ghConOut, &sbi1);


	//PRAGMA_ERROR("������ ������ ���������� ��� GUI, ����� ��������� ConEmuC!");
	#ifdef SHOW_STARTED_MSGBOX
	MessageBox(GetConsoleWindow(), L"ConEmuC (comspec mode) is about to TERMINATE", L"ConEmuC.ComSpec", 0);
	#endif
    
    if (!cmd.bNonGuiMode)
    {
        //// ������� ������ ������ (������ � ������)
        //if (cmd.sbi.dwSize.X && cmd.sbi.dwSize.Y) {
        //    SMALL_RECT rc = {0};
        //    SetConsoleSize(0, cmd.sbi.dwSize, rc, "ComspecDone");
        //}

        CESERVER_REQ *pIn = NULL, *pOut = NULL;
        int nSize = sizeof(CESERVER_REQ_HDR)+3*sizeof(DWORD);
        pIn = (CESERVER_REQ*)Alloc(nSize,1);
        if (pIn) {
            pIn->hdr.nCmd = CECMD_CMDSTARTSTOP;
			pIn->hdr.nSrcThreadId = GetCurrentThreadId();
            pIn->hdr.nSize = nSize;
            pIn->hdr.nVersion = CESERVER_REQ_VER;
            ((DWORD*)(pIn->Data))[0] = 3; // Cmd ����� ��������
            ((DWORD*)(pIn->Data))[1] = (DWORD)ghConWnd;
            ((DWORD*)(pIn->Data))[2] = gnSelfPID;

            pOut = ExecuteGuiCmd(ghConWnd, pIn);
            if (pOut) {
                ExecuteFreeResult(pOut);
            }
        }

		lbRc2 = GetConsoleScreenBufferInfo(ghConOut, &sbi2);
		if (lbRc1 && lbRc2 && sbi2.dwSize.Y == sbi1.dwSize.Y) {
			// GUI �� ���� ������� ������ ������... 
			// ��� �����, �.�. ��� ������ ������ �� ������ � ����� ������ ������� �� N ������ �����...
			if (sbi2.dwSize.Y != cmd.sbi.dwSize.Y) {
			    SMALL_RECT rc = {0};
				sbi2.dwSize.Y = cmd.sbi.dwSize.Y;
			    SetConsoleSize(0, sbi2.dwSize, rc, "ComspecDone.Force");
			}
		}
    }

    SafeCloseHandle(ghCtrlCEvent);
    SafeCloseHandle(ghCtrlBreakEvent);
}

WARNING("�������� LogInput(INPUT_RECORD* pRec) �� ��� ����� ������� 'ConEmuC-input-%i.log'");
void CreateLogSizeFile()
{
    if (ghLogSize) return; // ���
    
    DWORD dwErr = 0;
    wchar_t szFile[MAX_PATH+64], *pszDot;
    if (!GetModuleFileName(NULL, szFile, MAX_PATH)) {
        dwErr = GetLastError();
        wprintf(L"GetModuleFileName failed! ErrCode=0x%08X\n", dwErr);
        return; // �� �������
    }
    if ((pszDot = wcsrchr(szFile, L'.')) == NULL) {
        wprintf(L"wcsrchr failed!\n%s\n", szFile);
        return; // ������
    }
    wsprintfW(pszDot, L"-size-%i.log", gnSelfPID);
    
    ghLogSize = CreateFileW ( szFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (ghLogSize == INVALID_HANDLE_VALUE) {
        ghLogSize = NULL;
        dwErr = GetLastError();
        wprintf(L"CreateFile failed! ErrCode=0x%08X\n%s\n", dwErr, szFile);
        return;
    }
    
    wpszLogSizeFile = _wcsdup(szFile);
    // OK, ��� �������
    LPCSTR pszCmdLine = GetCommandLineA();
    if (pszCmdLine) {
        WriteFile(ghLogSize, pszCmdLine, strlen(pszCmdLine), &dwErr, 0);
        WriteFile(ghLogSize, "\r\n", 2, &dwErr, 0);
    }
    LogSize(NULL, "Startup");
}

void LogSize(COORD* pcrSize, LPCSTR pszLabel)
{
    if (!ghLogSize) return;
    
    CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}};
    // � �������� ��� �������� �������� ��������
    GetConsoleScreenBufferInfo(ghConOut ? ghConOut : GetStdHandle(STD_OUTPUT_HANDLE), &lsbi);
    
    char szInfo[192] = {0};
    LPCSTR pszThread = "<unknown thread>";
    
    DWORD dwId = GetCurrentThreadId();
    if (dwId == gdwMainThreadId)
            pszThread = "MainThread";
            else
    if (dwId == srv.dwServerThreadId)
            pszThread = "ServerThread";
            else
    if (dwId == srv.dwRefreshThread)
            pszThread = "RefreshThread";
            else
    if (dwId == srv.dwWinEventThread)
            pszThread = "WinEventThread";
            else
    if (dwId == srv.dwInputThreadId)
            pszThread = "InputThread";
            
    /*HDESK hDesk = GetThreadDesktop ( GetCurrentThreadId() );
    HDESK hInp = OpenInputDesktop ( 0, FALSE, GENERIC_READ );*/
    
            
    SYSTEMTIME st; GetLocalTime(&st);
    if (pcrSize) {
        sprintf(szInfo, "%i:%02i:%02i CurSize={%ix%i} ChangeTo={%ix%i} %s %s\r\n",
            st.wHour, st.wMinute, st.wSecond,
            lsbi.dwSize.X, lsbi.dwSize.Y, pcrSize->X, pcrSize->Y, pszThread, (pszLabel ? pszLabel : ""));
    } else {
        sprintf(szInfo, "%i:%02i:%02i CurSize={%ix%i} %s %s\r\n",
            st.wHour, st.wMinute, st.wSecond,
            lsbi.dwSize.X, lsbi.dwSize.Y, pszThread, (pszLabel ? pszLabel : ""));
    }
    
    //if (hInp) CloseDesktop ( hInp );
    
    DWORD dwLen = 0;
    WriteFile(ghLogSize, szInfo, strlen(szInfo), &dwLen, 0);
    FlushFileBuffers(ghLogSize);
}

// ������� ����������� ������� � ����
int ServerInit()
{
    int iRc = 0;
    DWORD dwErr = 0;
    HANDLE hWait[2] = {NULL,NULL};
    wchar_t szComSpec[MAX_PATH+1], szSelf[MAX_PATH+1];
    HMODULE hKernel = GetModuleHandleW (L"kernel32.dll");
    
    
    
    if (hKernel) pfnGetConsoleKeyboardLayoutName = (PGETCONSOLEKEYBOARDLAYOUTNAME)GetProcAddress (hKernel, "GetConsoleKeyboardLayoutNameW");

	if (!gbAttachMode) {
		CheckConEmuHwnd();
	}

	InitializeCriticalSection(&srv.csChangeSize);

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
    if (GetModuleFileName(NULL, szSelf, MAX_PATH)) {
        SetEnvironmentVariable(L"ComSpec", szSelf);
    }

    //srv.bContentsChanged = TRUE;
    srv.nMainTimerElapse = 10;
    srv.bConsoleActive = TRUE; TODO("������������ ���������� ������� Activate/Deactivate");
    srv.bNeedFullReload = FALSE; srv.bForceFullSend = TRUE;
    srv.nTopVisibleLine = -1; // ���������� ��������� �� ��������

    
    InitializeCriticalSection(&srv.csConBuf);
    InitializeCriticalSection(&srv.csProc);
    InitializeCriticalSection(&srv.csChar);

    // �������� ���������� ��� ����������, ����� �� ������� ���������
    wsprintfW(srv.szPipename, CEGUIATTACHED, (DWORD)ghConWnd);
    srv.hConEmuGuiAttached = CreateEvent(NULL, TRUE, FALSE, srv.szPipename);
    _ASSERTE(srv.hConEmuGuiAttached!=NULL);
    if (srv.hConEmuGuiAttached) ResetEvent(srv.hConEmuGuiAttached);
    
    // ������������� ���� ������
    wsprintfW(srv.szPipename, CESERVERPIPENAME, L".", gnSelfPID);
    wsprintfW(srv.szInputname, CESERVERINPUTNAME, L".", gnSelfPID);

    // ������ ������ � Lucida. ����������� ��� ���������� ������.
	if (srv.szConsoleFont[0] == 0) lstrcpyW(srv.szConsoleFont, L"Lucida Console");
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
	//if (srv.szConsoleFontFile[0])
	//	AddFontResourceEx(srv.szConsoleFontFile, FR_PRIVATE, NULL);
    if (ghLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.before");
    SetConsoleFontSizeTo(ghConWnd, srv.nConFontHeight, srv.nConFontWidth, srv.szConsoleFont);
    if (ghLogSize) LogSize(NULL, ":SetConsoleFontSizeTo.after");
    if (gbParmBufferSize && gcrBufferSize.X && gcrBufferSize.Y) {
        SMALL_RECT rc = {0};
        SetConsoleSize(gnBufferHeight, gcrBufferSize, rc, ":ServerInit.SetFromArg"); // ����� ����������? ���� ����� ��� �������
    }

    if (IsIconic(ghConWnd)) { // ������ ����� ����������!
        WINDOWPLACEMENT wplCon = {sizeof(wplCon)};
        GetWindowPlacement(ghConWnd, &wplCon);
        wplCon.showCmd = SW_RESTORE;
        SetWindowPlacement(ghConWnd, &wplCon);
    }


    // ����� �������� ������� ��������� �������
    ReloadConsoleInfo();

    //
    srv.hRefreshEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    if (!srv.hRefreshEvent) {
        dwErr = GetLastError();
        wprintf(L"CreateEvent(hRefreshEvent) failed, ErrCode=0x%08X\n", dwErr); 
        iRc = CERR_REFRESHEVENT; goto wrap;
    }

    //srv.hChangingSize = CreateEvent(NULL,TRUE,FALSE,NULL);
    //if (!srv.hChangingSize) {
    //    dwErr = GetLastError();
    //    wprintf(L"CreateEvent(hChangingSize) failed, ErrCode=0x%08X\n", dwErr); 
    //    iRc = CERR_REFRESHEVENT; goto wrap;
    //}
    //SetEvent(srv.hChangingSize);

    
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
        wprintf(L"CreateThread(RefreshThread) failed, ErrCode=0x%08X\n", dwErr); 
        iRc = CERR_CREATEREFRESHTHREAD; goto wrap;
    }
    
    
    // The client thread that calls SetWinEventHook must have a message loop in order to receive events.");
    hWait[0] = CreateEvent(NULL,FALSE,FALSE,NULL);
    _ASSERTE(hWait[0]!=NULL);
    srv.hWinEventThread = CreateThread( 
        NULL,              // no security attribute 
        0,                 // default stack size 
        WinEventThread,      // thread proc
        hWait[0],              // thread parameter 
        0,                 // not suspended 
        &srv.dwWinEventThread);      // returns thread ID 
    if (srv.hWinEventThread == NULL) 
    {
        dwErr = GetLastError();
        wprintf(L"CreateThread(WinEventThread) failed, ErrCode=0x%08X\n", dwErr); 
        SafeCloseHandle(hWait[0]);
        hWait[0]=NULL; hWait[1]=NULL;
        iRc = CERR_WINEVENTTHREAD; goto wrap;
    }
    hWait[1] = srv.hWinEventThread;
    dwErr = WaitForMultipleObjects(2, hWait, FALSE, 10000);
    SafeCloseHandle(hWait[0]);
    hWait[0]=NULL; hWait[1]=NULL;
    if (!srv.hWinHook) {
        _ASSERT(dwErr == WAIT_TIMEOUT);
        if (dwErr == WAIT_TIMEOUT) { // �� ���� ����� ���� �� ������
            #pragma warning( push )
            #pragma warning( disable : 6258 )
            TerminateThread(srv.hWinEventThread,100);
            #pragma warning( pop )
            SafeCloseHandle(srv.hWinEventThread);
        }
        // ������ �� ����� ��� ��������, ���� ��� ���������, ������� ����������
        SafeCloseHandle(srv.hWinEventThread);
        iRc = CERR_WINHOOKNOTCREATED; goto wrap;
    }

    // ��������� ���� ��������� ������  
    srv.hServerThread = CreateThread( 
        NULL,              // no security attribute 
        0,                 // default stack size 
        ServerThread,      // thread proc
        NULL,              // thread parameter 
        0,                 // not suspended 
        &srv.dwServerThreadId);      // returns thread ID 

    if (srv.hServerThread == NULL) 
    {
        dwErr = GetLastError();
        wprintf(L"CreateThread(ServerThread) failed, ErrCode=0x%08X\n", dwErr); 
        iRc = CERR_CREATESERVERTHREAD; goto wrap;
    }

    // ��������� ���� ��������� ������� (����������, ����, � ��.)
    srv.hInputThread = CreateThread( 
        NULL,              // no security attribute 
        0,                 // default stack size 
        InputThread,      // thread proc
        NULL,              // thread parameter 
        0,                 // not suspended 
        &srv.dwInputThreadId);      // returns thread ID 

    if (srv.hInputThread == NULL) 
    {
        dwErr = GetLastError();
        wprintf(L"CreateThread(InputThread) failed, ErrCode=0x%08X\n", dwErr); 
        iRc = CERR_CREATEINPUTTHREAD; goto wrap;
    }
	//SetThreadPriority(srv.hInputThread, THREAD_PRIORITY_ABOVE_NORMAL);

    if (gbAttachMode) {
        HWND hGui = NULL, hDcWnd = NULL;
        UINT nMsg = RegisterWindowMessage(CONEMUMSG_ATTACH);
        DWORD dwStart = GetTickCount(), dwDelta = 0, dwCur = 0;
        // ���� � ������� ���� �� ��������� (GUI ��� ��� �� �����������) ������� ���
        while (!hDcWnd && dwDelta <= 5000) {
            while ((hGui = FindWindowEx(NULL, hGui, VirtualConsoleClassMain, NULL)) != NULL) {
                hDcWnd = (HWND)SendMessage(hGui, nMsg, (WPARAM)ghConWnd, (LPARAM)gnSelfPID);
                if (hDcWnd != NULL) {
					ghConEmuWnd = hGui;
                    break;
                }
            }
            if (hDcWnd) break;

            Sleep(500);
            dwCur = GetTickCount(); dwDelta = dwCur - dwStart;
        }
        if (!hDcWnd) {
            wprintf(L"Available ConEmu GUI window not found!\n");
            iRc = CERR_ATTACHFAILED; goto wrap;
        }
    }

	CheckConEmuHwnd();

wrap:
    return iRc;
}

// ��������� ��� ���� � ������� �����������
void ServerDone(int aiRc)
{
    // ��������� ����������� � �������
    if (srv.dwWinEventThread && srv.hWinEventThread) {
        PostThreadMessage(srv.dwWinEventThread, WM_QUIT, 0, 0);
        // �������� ��������, ���� ���� ���� ����������
        if (WaitForSingleObject(srv.hWinEventThread, 500) != WAIT_OBJECT_0) {
            #pragma warning( push )
            #pragma warning( disable : 6258 )
            TerminateThread ( srv.hWinEventThread, 100 ); // ��� ��������� �� �����...
            #pragma warning( pop )
        }
        SafeCloseHandle(srv.hWinEventThread);
    }
    if (srv.hInputThread) {
        #pragma warning( push )
        #pragma warning( disable : 6258 )
        TerminateThread ( srv.hInputThread, 100 ); TODO("������� ���������� ����������");
        #pragma warning( pop )
        SafeCloseHandle(srv.hInputThread);
    }

    if (srv.hServerThread) {
        #pragma warning( push )
        #pragma warning( disable : 6258 )
        TerminateThread ( srv.hServerThread, 100 ); TODO("������� ���������� ����������");
        #pragma warning( pop )
        SafeCloseHandle(srv.hServerThread);
    }
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
    //if (srv.hChangingSize) {
    //    SafeCloseHandle(srv.hChangingSize);
    //}
    if (srv.hWinHook) {
        UnhookWinEvent(srv.hWinHook); srv.hWinHook = NULL;
    }
    
	if (gpStoredOutput) { Free(gpStoredOutput); gpStoredOutput = NULL; }
    if (srv.psChars) { Free(srv.psChars); srv.psChars = NULL; }
    if (srv.pnAttrs) { Free(srv.pnAttrs); srv.pnAttrs = NULL; }
    if (srv.ptrLineCmp) { Free(srv.ptrLineCmp); srv.ptrLineCmp = NULL; }
    DeleteCriticalSection(&srv.csConBuf);
    DeleteCriticalSection(&srv.csProc);
    DeleteCriticalSection(&srv.csChar);
	DeleteCriticalSection(&srv.csChangeSize);

	//if (srv.szConsoleFontFile[0])
	//	RemoveFontResourceEx(srv.szConsoleFontFile, FR_PRIVATE, NULL);
}

void CheckConEmuHwnd()
{
	//HWND hWndFore = GetForegroundWindow();
	//HWND hWndFocus = GetFocus();
	DWORD dwGuiThreadId = 0, dwGuiProcessId = 0;

	if (ghConEmuWnd == NULL) {
		CESERVER_REQ *pIn = NULL, *pOut = NULL;
		int nSize = sizeof(CESERVER_REQ_HDR)+3*sizeof(DWORD);
		pIn = (CESERVER_REQ*)Alloc(nSize,1);
		if (pIn) {
			pIn->hdr.nCmd = CECMD_CMDSTARTSTOP;
			pIn->hdr.nSrcThreadId = GetCurrentThreadId();
			pIn->hdr.nSize = nSize;
			pIn->hdr.nVersion = CESERVER_REQ_VER;
			((DWORD*)(pIn->Data))[0] = 0; // Server ����� �����
			((DWORD*)(pIn->Data))[1] = (DWORD)ghConWnd;
			((DWORD*)(pIn->Data))[2] = gnSelfPID;

			pOut = ExecuteGuiCmd(ghConWnd, pIn);
			if (pOut) {
				#ifdef _DEBUG
				BOOL  bAlreadyBufferHeight = *(DWORD*)(pOut->Data);
				#endif
				HWND  hGuiWnd = (HWND)*(DWORD*)(pOut->Data+4);
				#ifdef _DEBUG
				DWORD nGuiPID = *(DWORD*)(pOut->Data+8);
				#endif

				ghConEmuWnd = hGuiWnd;

				ExecuteFreeResult(pOut);
			}
			Free(pIn);
		}
	}
	// GUI ����� ��� "������" � �������� ��� � ���������, ��� ��� ������� � ����� Snapshoot
	if (ghConEmuWnd == NULL) {
		DWORD dwGuiPID = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
		if (hSnap != INVALID_HANDLE_VALUE) {
			PROCESSENTRY32 prc = {sizeof(PROCESSENTRY32)};
			if (Process32First(hSnap, &prc)) {
				do {
					if (prc.th32ProcessID == gnSelfPID) {
						dwGuiPID = prc.th32ParentProcessID;
						break;
					}
				} while (Process32Next(hSnap, &prc));
			}
			CloseHandle(hSnap);
		}
		if (dwGuiPID) {
			HWND hGui = NULL;
			while ((hGui = FindWindowEx(NULL, hGui, VirtualConsoleClassMain, NULL)) != NULL) {
				dwGuiThreadId = GetWindowThreadProcessId(hGui, &dwGuiProcessId);
				if (dwGuiProcessId == dwGuiPID) {
					ghConEmuWnd = hGui;
					break;
				}
			}
		}
	}
	if (ghConEmuWnd == NULL) { // ���� �� ������ �� �������...
		ghConEmuWnd = GetConEmuHWND(TRUE/*abRoot*/);
	}
	if (ghConEmuWnd) {
		dwGuiThreadId = GetWindowThreadProcessId(ghConEmuWnd, &dwGuiProcessId);

		AllowSetForegroundWindow(dwGuiProcessId);

		//if (hWndFore == ghConWnd || hWndFocus == ghConWnd)
		//if (hWndFore != ghConEmuWnd)

		SetForegroundWindow(ghConWnd);

	} else {
		_ASSERTE(ghConEmuWnd!=NULL);
	}
}



DWORD WINAPI ServerThread(LPVOID lpvParam) 
{ 
   BOOL fConnected = FALSE;
   DWORD dwInstanceThreadId = 0, dwErr = 0;
   HANDLE hPipe = NULL, hInstanceThread = NULL;
   
 
// The main loop creates an instance of the named pipe and 
// then waits for a client to connect to it. When the client 
// connects, a thread is created to handle communications 
// with that client, and the loop is repeated. 
 
   for (;;) 
   { 
	  MCHKHEAP
      hPipe = CreateNamedPipe( 
          srv.szPipename,               // pipe name 
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
          dwErr = GetLastError();
          wprintf(L"CreatePipe failed, ErrCode=0x%08X\n", dwErr); 
          Sleep(50);
          //return 99;
          continue;
      }
 
      // Wait for the client to connect; if it succeeds, 
      // the function returns a nonzero value. If the function
      // returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 
 
      fConnected = ConnectNamedPipe(hPipe, NULL) ? 
         TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 
 
	  MCHKHEAP
      if (fConnected) 
      { 
      // Create a thread for this client. 
         hInstanceThread = CreateThread( 
            NULL,              // no security attribute 
            0,                 // default stack size 
            InstanceThread,    // thread proc
            (LPVOID) hPipe,    // thread parameter 
            0,                 // not suspended 
            &dwInstanceThreadId);      // returns thread ID 

         if (hInstanceThread == NULL) 
         {
            dwErr = GetLastError();
            wprintf(L"CreateThread(Instance) failed, ErrCode=0x%08X\n", dwErr);
            Sleep(50);
            //return 0;
            continue;
         }
         else {
             SafeCloseHandle(hInstanceThread); 
         }
       } 
      else {
        // The client could not connect, so close the pipe. 
         SafeCloseHandle(hPipe); 
      }
	  MCHKHEAP
   } 
   return 1; 
} 

DWORD WINAPI InputThread(LPVOID lpvParam) 
{ 
   BOOL fConnected, fSuccess; 
   //DWORD srv.dwServerThreadId;
   HANDLE hPipe = NULL; 
   DWORD dwErr = 0;
   
 
// The main loop creates an instance of the named pipe and 
// then waits for a client to connect to it. When the client 
// connects, a thread is created to handle communications 
// with that client, and the loop is repeated. 
 
   for (;;) 
   { 
      MCHKHEAP
      hPipe = CreateNamedPipe( 
          srv.szInputname,              // pipe name 
          PIPE_ACCESS_INBOUND,      // goes from client to server only
          PIPE_TYPE_MESSAGE |       // message type pipe 
          PIPE_READMODE_MESSAGE |   // message-read mode 
          PIPE_WAIT,                // blocking mode 
          PIPE_UNLIMITED_INSTANCES, // max. instances  
          sizeof(INPUT_RECORD),     // output buffer size 
          sizeof(INPUT_RECORD),     // input buffer size 
          0,                        // client time-out
          NULL);                    // default security attribute 

      _ASSERTE(hPipe != INVALID_HANDLE_VALUE);
      
      if (hPipe == INVALID_HANDLE_VALUE) 
      {
          dwErr = GetLastError();
          wprintf(L"CreatePipe failed, ErrCode=0x%08X\n", dwErr);
          Sleep(50);
          //return 99;
          continue;
      }
 
      // Wait for the client to connect; if it succeeds, 
      // the function returns a nonzero value. If the function
      // returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 
 
      fConnected = ConnectNamedPipe(hPipe, NULL) ? 
         TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 
 
	  MCHKHEAP
      if (fConnected) 
      { 
          //TODO:
          DWORD cbBytesRead, cbWritten;
          INPUT_RECORD iRec; memset(&iRec,0,sizeof(iRec));
          while ((fSuccess = ReadFile( 
             hPipe,        // handle to pipe 
             &iRec,        // buffer to receive data 
             sizeof(iRec), // size of buffer 
             &cbBytesRead, // number of bytes read 
             NULL)) != FALSE)        // not overlapped I/O 
          {
              // ������������� ����������� ���������� ����
              if (iRec.EventType == 0xFFFF) {
                  SafeCloseHandle(hPipe);
                  break;
              }
			  MCHKHEAP
              if (iRec.EventType) {
                  // ��������� ENABLE_PROCESSED_INPUT � GetConsoleMode
                  #define ALL_MODIFIERS (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED|SHIFT_PRESSED)
                  #define CTRL_MODIFIERS (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)

                  if (iRec.EventType == KEY_EVENT && iRec.Event.KeyEvent.bKeyDown &&
                      (iRec.Event.KeyEvent.wVirtualKeyCode == 'C' || iRec.Event.KeyEvent.wVirtualKeyCode == VK_CANCEL)
                     )
                  {
                        BOOL lbRc = FALSE;
                        DWORD dwEvent = (iRec.Event.KeyEvent.wVirtualKeyCode == 'C') ? CTRL_C_EVENT : CTRL_BREAK_EVENT;
                      //&& (srv.dwConsoleMode & ENABLE_PROCESSED_INPUT)

					//The SetConsoleMode function can disable the ENABLE_PROCESSED_INPUT mode for a console's input buffer, 
					//so CTRL+C is reported as keyboard input rather than as a signal. 
					// CTRL+BREAK is always treated as a signal
					if (
						(iRec.Event.KeyEvent.dwControlKeyState & CTRL_MODIFIERS) &&
						((iRec.Event.KeyEvent.dwControlKeyState & ALL_MODIFIERS) 
						== (iRec.Event.KeyEvent.dwControlKeyState & CTRL_MODIFIERS))
						)
					{


                        // ����� ��������, ������� �� ��������� ������� � ������ CREATE_NEW_PROCESS_GROUP
                        // ����� � ��������������� ������� (WinXP SP3) ������ �����, � ��� ���������
                        // �� Ctrl-Break, �� ������� ���������� Ctrl-C
                        lbRc = GenerateConsoleCtrlEvent(dwEvent, 0);

                    }
                  }
              
                  if (iRec.EventType) {
					#ifdef _DEBUG
					  if (iRec.EventType == KEY_EVENT && iRec.Event.KeyEvent.bKeyDown &&
						  iRec.Event.KeyEvent.wVirtualKeyCode == VK_F11)
					  {
						  DEBUGSTR(L"  ---  F11 recieved\n");
					  }
					#endif

                      fSuccess = WriteConsoleInput(ghConIn, &iRec, 1, &cbWritten);
                      _ASSERTE(fSuccess && cbWritten==1);
                  }
				  MCHKHEAP
              }
              // next
              memset(&iRec,0,sizeof(iRec));
			  MCHKHEAP
          }
      } 
      else 
        // The client could not connect, so close the pipe. 
         SafeCloseHandle(hPipe);
   } 
   MCHKHEAP
   return 1; 
} 
 
DWORD WINAPI InstanceThread(LPVOID lpvParam) 
{ 
    CESERVER_REQ in={0}, *pIn=NULL, *pOut=NULL;
    DWORD cbBytesRead, cbWritten, dwErr = 0;
    BOOL fSuccess; 
    HANDLE hPipe; 

    // The thread's parameter is a handle to a pipe instance. 
    hPipe = (HANDLE) lpvParam; 

	MCHKHEAP
 
    // Read client requests from the pipe. 
    memset(&in, 0, sizeof(in));
    fSuccess = ReadFile(
        hPipe,        // handle to pipe 
        &in,          // buffer to receive data 
        sizeof(in),   // size of buffer 
        &cbBytesRead, // number of bytes read 
        NULL);        // not overlapped I/O 

    if ((!fSuccess && ((dwErr = GetLastError()) != ERROR_MORE_DATA)) ||
            cbBytesRead < sizeof(CESERVER_REQ_HDR) || in.hdr.nSize < sizeof(CESERVER_REQ_HDR))
    {
        goto wrap;
    }

    if (in.hdr.nSize > cbBytesRead)
    {
        DWORD cbNextRead = 0;
        pIn = (CESERVER_REQ*)Alloc(in.hdr.nSize, 1);
        if (!pIn)
            goto wrap;
        *pIn = in;
        fSuccess = ReadFile(
            hPipe,        // handle to pipe 
            ((LPBYTE)pIn)+cbBytesRead,  // buffer to receive data 
            in.hdr.nSize - cbBytesRead,   // size of buffer 
            &cbNextRead, // number of bytes read 
            NULL);        // not overlapped I/O 
        if (fSuccess)
            cbBytesRead += cbNextRead;
    }

    if (!GetAnswerToRequest(pIn ? *pIn : in, &pOut) || pOut==NULL) {
        goto wrap;
    }

	MCHKHEAP
    // Write the reply to the pipe. 
    fSuccess = WriteFile( 
        hPipe,        // handle to pipe 
        pOut,         // buffer to write from 
        pOut->hdr.nSize,  // number of bytes to write 
        &cbWritten,   // number of bytes written 
        NULL);        // not overlapped I/O 

    // ���������� ������
	if ((LPVOID)pOut != (LPVOID)gpStoredOutput) // ���� ��� �� ����������� �����
		Free(pOut);

	MCHKHEAP
    //if (!fSuccess || pOut->hdr.nSize != cbWritten) break; 

// Flush the pipe to allow the client to read the pipe's contents 
// before disconnecting. Then disconnect the pipe, and close the 
// handle to this pipe instance. 

    FlushFileBuffers(hPipe); 
    DisconnectNamedPipe(hPipe);
wrap:
    SafeCloseHandle(hPipe); 

    return 1;
}

// �� ��������� ������ �������� �� �������� �������� - �� ����������� �� ����������
// ����������� �� �������... ��������, ��� ���������� �� ������ ����� ��������
// � ��� ��������� �����.
BOOL ReadConsoleData(CESERVER_CHAR* pCheck /*= NULL*/)
{
    WARNING("���� ������� prcDataChanged(!=0 && !=1) - ��������� �� ������� ������ ���, � ������ ���������");
    WARNING("��� ����������� ������� - ����� �������, ��� ����� � ������ ���� ��������� � ������ �������");
    WARNING("� ������ ���� � ���� ������ ��� �� ������� - ������ �������� � ������ �������� �����");
    WARNING("���� ������� prcChanged - � ���� ��������� ������� �������� �������������, ��� ����������� ��������� � GUI");
    BOOL lbRc = TRUE, lbChanged = FALSE;
    //DWORD cbDataSize = 0; // Size in bytes of ONE buffer
    //srv.bContentsChanged = FALSE;
    EnterCriticalSection(&srv.csConBuf);
    //RECT rcReadRect = {0};
	// ���� �� ����� ������ ������� �������� ������ - ������� ������
	// ����� ������ ���������� ����� ������ (�������� �� � ��� ������, � ������� �������)
	CSection cs(&srv.csChangeSize, &srv.ncsTChangeSize);

	BOOL lbFirstLoop = TRUE;

Loop1:

    USHORT TextWidth=0, TextHeight=0;
    DWORD TextLen=0;
    COORD coord;

	MCHKHEAP
    
    TextWidth = srv.sbi.dwSize.X;
    // ��� ������ ���� ���������� � CorrectVisibleRect
    //if (gnBufferHeight == 0) {
    //  // ������ ��� ��� �� ������ ������������ �� ��������� ������ BufferHeight
    //  if (srv.sbi.dwMaximumWindowSize.Y < srv.sbi.dwSize.Y)
    //      gnBufferHeight = srv.sbi.dwSize.Y; // ��� ���������� �������� �����
    //}
    if (gnBufferHeight == 0) {
        TextHeight = srv.sbi.dwSize.Y; //srv.sbi.srWindow.Bottom - srv.sbi.srWindow.Top + 1;
    } else {
        //��� ������ BufferHeight ����� ������� �� �������!
        TextHeight = gcrBufferSize.Y;
    }
	srv.nVisibleHeight = TextHeight;

	// ����� �������� gnBufferHeight �� ����������?
	_ASSERTE(TextHeight<=150);
	// ������ ���� � ������� ������ ���� ����� ������ � GUI, ����� �� ����� ������ ������ � �������� ��� ���������
	_ASSERTE(TextHeight==(srv.sbi.srWindow.Bottom-srv.sbi.srWindow.Top+1));
	
    TextLen = TextWidth * TextHeight;
	// ���� ��� ������ ���� ��������� ����� ��������� ������� ����� ������, ������ ��� �������
    if (TextLen > srv.nBufCharCount) {
        lbChanged = TRUE;
        Free(srv.psChars);
        srv.psChars = (wchar_t*)Alloc(TextLen*2,sizeof(wchar_t));
        _ASSERTE(srv.psChars!=NULL);
        Free(srv.pnAttrs);
        srv.pnAttrs = (WORD*)Alloc(TextLen*2,sizeof(WORD));
        _ASSERTE(srv.pnAttrs!=NULL);
        if (srv.psChars && srv.pnAttrs) {
            srv.nBufCharCount = TextLen;
        } else {
            srv.nBufCharCount = 0; // ������ ��������� ������
            lbRc = FALSE;
        }
    }
	srv.nOneBufferSize = TextLen*2; // ������ � ������!

	MCHKHEAP

	// ���� ��� ������ ����� ������ (������������ ��� ��������� ����� �������) ��������� ������� ����� ������
    if (TextWidth > srv.nLineCmpSize) {
        Free(srv.ptrLineCmp);
        srv.ptrLineCmp = (WORD*)Alloc(TextWidth*2,sizeof(WORD));
        _ASSERTE(srv.ptrLineCmp!=NULL);
        if (srv.ptrLineCmp) {
            srv.nLineCmpSize = TextWidth*2;
        } else {
            srv.nLineCmpSize = 0;
        }
    }

	MCHKHEAP

    coord.X = 0;
	coord.Y = (srv.nTopVisibleLine == -1) ? srv.sbi.srWindow.Top : srv.nTopVisibleLine;

    //TODO: ������������ ���������� ������ �� srv.bContentsChanged
    // ����� ������� - �������� srv.bContentsChanged � FALSE
    // ����� ������� ����������, 
    // ��� ������������� - ������� ����� CriticalSection
    
	// ������ ������� ������ ���� ���������
    if (srv.psChars && srv.pnAttrs && (srv.bForceFullSend || srv.bNeedFullReload || pCheck != NULL))
	{
        //dwAllSize += TextWidth*TextHeight*4;
        
        // Get attributes (first) and text (second)
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

		DWORD nbActuallyRead;

		TODO("���� ������� �������� ������������� pCheck - ������ ����� ����� ��������� ������ ���");
		if (!srv.bForceFullSend && pCheck && pCheck->hdr.nSize)
		{
			// ������ ������ �������������, ������������ � pCheck->hdr.
			WARNING("���������� � pCheck->hdr ������ ���� �����������, � �� ��������");
			// ������ ������ ������� (� GUI) �����! ( {0,coord.Y} - {srv.sbi.dwSize.X-1,coord.Y+TextHeight-1} )
			// ��������������� ���������� �������������. ���� ������ �� �������� - ������ (pCheck->hdr.nSize=0)

			int nTop = coord.Y;
			int nLines = pCheck->hdr.cr2.Y - pCheck->hdr.cr1.Y + 1;

			// ���� �� ������ �� ����� �������...
			if (pCheck->hdr.cr1.Y < nTop)
				pCheck->hdr.cr1.Y = nTop;
			if (pCheck->hdr.cr2.Y >= (nTop+srv.nVisibleHeight)) {
				srv.bRequestPostFullReload = TRUE;

				if (lbFirstLoop) {
					lbFirstLoop = FALSE;
					srv.bForceFullSend = TRUE;
					if (pCheck)
						memset(pCheck, 0, sizeof(CESERVER_CHAR_HDR));
					goto Loop1;
				}
				pCheck->hdr.cr2.Y = (nTop+srv.nVisibleHeight-1);
			}
			_ASSERTE(pCheck->hdr.cr1.Y>=0 && pCheck->hdr.cr1.Y<srv.sbi.dwSize.Y);
			_ASSERTE(pCheck->hdr.cr2.Y>=0 && pCheck->hdr.cr2.Y<srv.sbi.dwSize.Y);
			//_ASSERTE(pCheck->hdr.cr2.Y<=(nTop+srv.nVisibleHeight));

			coord.X = 0; coord.Y = pCheck->hdr.cr1.Y;
			int nShift = TextWidth * (pCheck->hdr.cr1.Y - nTop);
			wchar_t* ConCharNow = srv.psChars + nShift;
			wchar_t* ConCharEnd = srv.psChars + srv.nOneBufferSize;
			WORD* ConAttrNow = srv.pnAttrs + nShift;
			int nRectLen = TextWidth * nLines;
			int y;

			MCHKHEAP

			// ���� �� ������ �� ����� �������...
			if (((DWORD)(nShift+nRectLen)) > TextLen) {
				srv.bRequestPostFullReload = TRUE;

				if (lbFirstLoop) {
					lbFirstLoop = FALSE;
					srv.bForceFullSend = TRUE;
					if (pCheck)
						memset(pCheck, 0, sizeof(CESERVER_CHAR_HDR));
					goto Loop1;
				}

				// ���������� ���������� ������ ��������� ��������
				TextWidth = srv.sbi.dwSize.X;
				TextHeight = (gnBufferHeight == 0) ? srv.sbi.dwSize.Y : gcrBufferSize.Y;
				srv.nVisibleHeight = TextHeight;
				TextLen = TextWidth * TextHeight;
				if (TextLen > srv.nBufCharCount) {
					TextLen = srv.nBufCharCount;
					TextHeight = (USHORT)(TextLen / TextWidth);
				}
				pCheck->hdr.cr1.X = min(pCheck->hdr.cr1.X,(TextWidth-1));
				pCheck->hdr.cr2.X = min(pCheck->hdr.cr2.X,(TextWidth-1));
				pCheck->hdr.cr1.Y = min(pCheck->hdr.cr1.Y,(TextHeight-1));
				pCheck->hdr.cr2.Y = min(pCheck->hdr.cr2.Y,(TextHeight-1));

				nShift = TextWidth * (pCheck->hdr.cr1.Y - nTop);
				ConCharNow = srv.psChars + nShift;
				ConCharEnd = srv.psChars + srv.nOneBufferSize;
				ConAttrNow = srv.pnAttrs + nShift;
				nRectLen = TextWidth * nLines;
			}
			_ASSERTE(((DWORD)(nShift+nRectLen))<=TextLen);

			if (!ReadConsoleOutputAttribute(ghConOut, ConAttrNow, nRectLen, coord, &nbActuallyRead)
				|| !ReadConsoleOutputCharacter(ghConOut, ConCharNow, nRectLen, coord, &nbActuallyRead) 
				)
			{
				DEBUGSTR(L" !!! Can't read full console screen. Read line by line\n");
				for (y = 0; y < nLines; y++, coord.Y ++)
				{
					_ASSERTE(ConCharNow<ConCharEnd);
					if (ConCharNow>=ConCharEnd) break;
					ReadConsoleOutputAttribute(ghConOut, ConAttrNow, TextWidth, coord, &nbActuallyRead);
						ConAttrNow += TextWidth;
					ReadConsoleOutputCharacter(ghConOut, ConCharNow, TextWidth, coord, &nbActuallyRead);
						ConCharNow += TextWidth;
				}
			}
			TODO("�� ����� ������� ������� ����� ������������ - ������ �� �� ��� �����...");
			if (gnBufferHeight && *srv.psChars!=9553) {
				//_ASSERTE(*srv.psChars!=9553);
			}

			MCHKHEAP
			// ������ ��������� ����������� ��������������
			COORD cr1 = {TextWidth,TextHeight};
			COORD cr2 = {-1,-1};
			ConCharNow = srv.psChars + nShift;
			ConAttrNow = srv.pnAttrs + nShift;
			wchar_t* ConCharCmp = srv.psChars + nShift + srv.nBufCharCount; // ������ srv.nBufCharCount, � �� TextLen!
			WORD* ConAttrCmp = srv.pnAttrs + nShift + srv.nBufCharCount;
			int x, LineSize = TextWidth*2;
			for (y = 0; y < nLines; y++)
			{
				MCHKHEAP
				if (memcmp(ConCharCmp, ConCharNow, LineSize) != 0 
					|| memcmp(ConAttrCmp, ConAttrNow, LineSize) != 0)
				{
					if (cr1.X > 0 || cr2.X < (TextWidth-1))
					{
						int x1 = TextWidth, x2 = -1;
						for (x = 0; x < TextWidth; x++) {
							if (ConCharCmp[x] != ConCharNow[x] || ConAttrCmp[x] != ConAttrNow[x]) {
								_ASSERTE(x1>x);
								x1 = x;
								break;
							}
						}
						for (x = TextWidth-1; x >= 0; x--) {
							if (ConCharCmp[x] != ConCharNow[x] || ConAttrCmp[x] != ConAttrNow[x]) {
								_ASSERTE(x2 < x);
								x2 = x;
								break;
							}
						}
						if (x1>x2) {
							_ASSERTE(x1<=x2);
						}

						if (x1 < cr1.X) cr1.X = x1;
						if (x2 > cr2.X) cr2.X = x2;
					}

					if (y < cr1.Y) cr1.Y = y;
					if (y > cr2.Y) cr2.Y = y;
				}

				// Next line
				ConCharCmp += TextWidth;
				ConCharNow += TextWidth;
				ConAttrCmp += TextWidth;
				ConAttrNow += TextWidth;


				//// ����� ����
				//WARNING("���-�� ��� ������ ����������� ����������� ��������������!");
				//for (x = 0; x < cr1.X; x++) {
				//	if (ConCharCmp[x] != ConCharNow[x] || ConAttrCmp[x] != ConAttrNow[x]) {
				//		cr1.X = x;
				//		if (x > cr2.X)
				//			cr2.X = x;
				//		if (y < cr1.Y)
				//			cr1.Y = y;
				//		if (y > cr2.Y)
				//			cr2.Y = y;
				//		break;
				//	}
				//}
				//// ������ ����
				//for (x = cr2.X-1; x > cr1.X; x--) {
				//	if (ConCharCmp[x] != ConCharNow[x] || ConAttrCmp[x] != ConAttrNow[x]) {
				//		if (x < cr1.X)
				//			cr1.X = x;
				//		cr2.X = x;
				//		if (y < cr1.Y)
				//			cr1.Y = y;
				//		if (y > cr2.Y)
				//			cr2.Y = y;
				//		break;
				//	}
				//}
			}
			MCHKHEAP
			if (cr2.X != -1) {
				// �������� ����� ������, � ������� ������ �������
				// ������ � ��� ���������� ���������� ��������� �� � ������� �������
				cr1.Y += pCheck->hdr.cr1.Y; cr2.Y += pCheck->hdr.cr1.Y;
				_ASSERTE(cr1.Y>=0 && cr1.Y<srv.sbi.dwSize.Y);
				_ASSERTE(cr2.Y>=0 && cr2.Y<srv.sbi.dwSize.Y);

				lbChanged = TRUE;
				_ASSERTE(cr2.X>=cr1.X && cr2.Y>=cr1.Y);
				//cr1.Y += nTop; cr2.Y += nTop;
				if (cr1.X == 0 && cr1.Y == 0 && cr2.X == (TextWidth-1) && cr2.Y == (TextHeight-1)) {
					// ������� ���������� ��������� - �� ������� � ��������� ��������������
					srv.bForceFullSend = TRUE;
					memset(&pCheck->hdr, 0, sizeof(pCheck->hdr));
					goto Loop1;
				} else {
					pCheck->hdr.cr1 = cr1;
					pCheck->hdr.cr2 = cr2;
				}
			} else {
				memset(&pCheck->hdr, 0, sizeof(pCheck->hdr));
			}
			
		} else {
			MCHKHEAP
			// ������ ��� ������� � GUI �������
			if (!ReadConsoleOutputAttribute(ghConOut, srv.pnAttrs, TextLen, coord, &nbActuallyRead)
				|| !ReadConsoleOutputCharacter(ghConOut, srv.psChars, TextLen, coord, &nbActuallyRead)
				)
			{
				DEBUGSTR(L" !!! Can't read full console screen. Read line by line\n");
				WORD* ConAttrNow = srv.pnAttrs; wchar_t* ConCharNow = srv.psChars;
				for(int y = 0; y < (int)TextHeight; y++, coord.Y++)
				{
					MCHKHEAP
					ReadConsoleOutputAttribute(ghConOut, ConAttrNow, TextWidth, coord, &nbActuallyRead);
						ConAttrNow += TextWidth;
					ReadConsoleOutputCharacter(ghConOut, ConCharNow, TextWidth, coord, &nbActuallyRead);
						ConCharNow += TextWidth;
				}
			}
			TODO("�� ����� ������� ������� ����� ������������ - ������ �� �� ��� �����...");
			//_ASSERTE(*srv.psChars!=9553);
			if (srv.bForceFullSend)
				lbChanged = TRUE;

			if (!lbChanged)
			{
				MCHKHEAP
				// ��� ����� ������ srv.nBufCharCount, � �� TextLen, ����� �� ������������ ��� ������ ���
				if (memcmp(srv.psChars, srv.psChars+srv.nBufCharCount, TextLen*sizeof(wchar_t)))
					lbChanged = TRUE;
				else if (memcmp(srv.pnAttrs, srv.pnAttrs+srv.nBufCharCount, TextLen*sizeof(WORD)))
					lbChanged = TRUE;
				MCHKHEAP
			}
		}

		if (TextWidth != srv.sbi.dwSize.X)
			srv.bRequestPostFullReload = TRUE;

        //cbDataSize = TextLen * 2; // Size in bytes of ONE buffer


    } else {
        lbRc = FALSE;
    }

	MCHKHEAP
    //if (pbDataChanged)
    //    *pbDataChanged = lbChanged;

    LeaveCriticalSection(&srv.csConBuf);
    //return cbDataSize;
	return lbChanged;
}

BOOL GetAnswerToRequest(CESERVER_REQ& in, CESERVER_REQ** out)
{
    BOOL lbRc = FALSE;

	MCHKHEAP

    switch (in.hdr.nCmd) {
        case CECMD_GETSHORTINFO:
        case CECMD_GETFULLINFO:
        {
            if (srv.szGuiPipeName[0] == 0) { // ��������� ���� � CVirtualConsole ��� ������ ���� �������
                wsprintf(srv.szGuiPipeName, CEGUIPIPENAME, L".", (DWORD)ghConWnd); // ��� gnSelfPID
            }

            _ASSERT(ghConOut && ghConOut!=INVALID_HANDLE_VALUE);
            if (ghConOut==NULL || ghConOut==INVALID_HANDLE_VALUE)
                return FALSE;

			if (in.hdr.nCmd == CECMD_GETFULLINFO)
				ReadConsoleData(NULL);

			MCHKHEAP

            // �� ������ �� GUI (GetAnswerToRequest)
            *out = CreateConsoleInfo(NULL, (in.hdr.nCmd == CECMD_GETFULLINFO));

			MCHKHEAP

            lbRc = TRUE;
        } break;
        case CECMD_SETSIZE:
		case CECMD_CMDSTARTED:
		case CECMD_CMDFINISHED:
        {
			MCHKHEAP
            int nOutSize = sizeof(CESERVER_REQ_HDR) + sizeof(CONSOLE_SCREEN_BUFFER_INFO) + sizeof(DWORD);
            *out = (CESERVER_REQ*)Alloc(nOutSize,1);
            if (*out == NULL) return FALSE;
            (*out)->hdr.nCmd = 0;
			(*out)->hdr.nSrcThreadId = GetCurrentThreadId();
            (*out)->hdr.nSize = nOutSize;
            (*out)->hdr.nVersion = CESERVER_REQ_VER;
			MCHKHEAP
            if (in.hdr.nSize >= (sizeof(CESERVER_REQ_HDR) + sizeof(USHORT)+sizeof(COORD)+sizeof(SHORT)+sizeof(SMALL_RECT))) {
                USHORT nBufferHeight = 0;
                COORD  crNewSize = {0,0};
                SMALL_RECT rNewRect = {0};
                SHORT  nNewTopVisible = -1;
                memmove(&nBufferHeight, in.Data, sizeof(USHORT));
                memmove(&crNewSize, in.Data+sizeof(USHORT), sizeof(COORD));
                memmove(&nNewTopVisible, in.Data+sizeof(USHORT)+sizeof(COORD), sizeof(SHORT));
                memmove(&rNewRect, in.Data+sizeof(USHORT)+sizeof(COORD)+sizeof(SHORT), sizeof(SMALL_RECT));

				MCHKHEAP

                (*out)->hdr.nCmd = in.hdr.nCmd;
				(*out)->hdr.nSrcThreadId = GetCurrentThreadId();

				#ifdef _DEBUG
				if (in.hdr.nCmd == CECMD_CMDFINISHED || in.hdr.nCmd == CECMD_CMDSTARTED) {
					DEBUGSTR((in.hdr.nCmd == CECMD_CMDFINISHED) ? 
						L"\n!!! CECMD_CMDFINISHED !!!\n\n" : L"\n!!! CECMD_CMDSTARTED !!!\n\n");
				}
				#endif

				if (in.hdr.nCmd == CECMD_CMDFINISHED) {
					// ��������� ������ ���� �������
					CmdOutputStore();
				}

				MCHKHEAP

                srv.nTopVisibleLine = nNewTopVisible;
                SetConsoleSize(nBufferHeight, crNewSize, rNewRect, ":CECMD_SETSIZE");

				MCHKHEAP

				if (in.hdr.nCmd == CECMD_CMDSTARTED) {
					// ������������ ����� ������� (������������ �����) ����� �������
					CmdOutputRestore();
				}
            }
			MCHKHEAP
			
			PCONSOLE_SCREEN_BUFFER_INFO psc = &((*out)->SetSizeRet.SetSizeRet);
			MyGetConsoleScreenBufferInfo(ghConOut, psc);
			
			DWORD nPacketId = ++srv.nLastPacketID;
			(*out)->SetSizeRet.nNextPacketId = nPacketId;
			
			srv.bForceFullSend = TRUE;
			SetEvent(srv.hRefreshEvent);

			MCHKHEAP

            lbRc = TRUE;
        } break;

		case CECMD_GETOUTPUT:
		{
			if (gpStoredOutput) {
				gpStoredOutput->hdr.hdr.nCmd = CECMD_GETOUTPUT;
				gpStoredOutput->hdr.hdr.nSize = 
					sizeof(CESERVER_CONSAVE_HDR)
					+ min((int)gpStoredOutput->hdr.cbMaxOneBufferSize, 
					      (gpStoredOutput->hdr.sbi.dwSize.X*gpStoredOutput->hdr.sbi.dwSize.Y*2));
				gpStoredOutput->hdr.hdr.nVersion = CESERVER_REQ_VER;
				gpStoredOutput->hdr.hdr.nSrcThreadId = GetCurrentThreadId();

				*out = (CESERVER_REQ*)gpStoredOutput;

				lbRc = TRUE;
			}
		} break;
        
    }
    
    if (gbInRecreateRoot) gbInRecreateRoot = FALSE;
    return lbRc;
}


DWORD WINAPI WinEventThread(LPVOID lpvParam)
{
    DWORD dwErr = 0;
    HANDLE hStartedEvent = (HANDLE)lpvParam;
    
    
    // "�����" ��� ���������� �������
    srv.hWinHook = SetWinEventHook(EVENT_CONSOLE_CARET,EVENT_CONSOLE_END_APPLICATION,
        NULL, (WINEVENTPROC)WinEventProc, 0,0, WINEVENT_OUTOFCONTEXT /*| WINEVENT_SKIPOWNPROCESS ?*/);
    dwErr = GetLastError();
    if (!srv.hWinHook) {
        dwErr = GetLastError();
        wprintf(L"SetWinEventHook failed, ErrCode=0x%08X\n", dwErr); 
        SetEvent(hStartedEvent);
        return 100;
    }
    SetEvent(hStartedEvent); hStartedEvent = NULL; // ����� �� ����� �� ���������

    MSG lpMsg;
    while (GetMessage(&lpMsg, NULL, 0, 0))
    {
		MCHKHEAP
        //if (lpMsg.message == WM_QUIT) { // GetMessage ���������� FALSE ��� ��������� ����� ���������
        //  lbQuit = TRUE; break;
        //}
        TranslateMessage(&lpMsg);
        DispatchMessage(&lpMsg);
		MCHKHEAP
    }
    
    // ������� ���
    if (srv.hWinHook) {
        UnhookWinEvent(srv.hWinHook); srv.hWinHook = NULL;
    }

	MCHKHEAP
    
    return 0;
}

DWORD WINAPI RefreshThread(LPVOID lpvParam)
{
    DWORD /*dwErr = 0,*/ nWait = 0;
    HANDLE hEvents[2] = {ghExitEvent, srv.hRefreshEvent};
    CONSOLE_CURSOR_INFO lci = {0}; // GetConsoleCursorInfo
    CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}}; // MyGetConsoleScreenBufferInfo
    DWORD nDelayRefresh = MAX_FORCEREFRESH_INTERVAL;
    DWORD nDelta = 0;
    DWORD nLastUpdateTick = GetTickCount();
    BOOL  lbEventualChange = FALSE;
    DWORD dwTimeout = 10; // �������� ������ ���������� �� ���� (��������, �������,...)
	BOOL  lbFirstForeground = TRUE;
	BOOL  lbLocalForced = FALSE;
    
    BOOL lbQuit = FALSE;

    while (!lbQuit)
    {
        nWait = WAIT_TIMEOUT;
		MCHKHEAP

        // ��������� ��������
        // !!! ����� ������� ������ ���� �����������, �� ����� ��� ������� ���������
        nWait = WaitForMultipleObjects ( 2, hEvents, FALSE, /*srv.bConsoleActive ? srv.nMainTimerElapse :*/ dwTimeout );
        if (nWait == WAIT_OBJECT_0)
            break; // ����������� ���������� ����
		#ifdef _DEBUG
		if (nWait == (WAIT_OBJECT_0+1)) {
			DEBUGSTR(L"*** hRefreshEvent was set, checking console...\n");
		}
		#endif

		if (ghConEmuWnd && GetForegroundWindow() == ghConWnd) {
			if (lbFirstForeground || !IsWindowVisible(ghConWnd)) {
				DEBUGSTR(L"...SetForegroundWindow(ghConEmuWnd);\n");
				SetForegroundWindow(ghConEmuWnd);
				lbFirstForeground = FALSE;
			}
		}
            
        // ��������, �������� ��� ������� - ��� ��������� �� ������� �� �������
        lbEventualChange = (nWait == (WAIT_OBJECT_0+1));

		// ���� ����� - �������� ������� ��������� � �������
	    if (pfnGetConsoleKeyboardLayoutName)
			CheckKeyboardLayout();

		
		// � ������� ��� ���� ��������� ������ ��������� ������������ �������
		if (srv.bRequestPostFullReload) {
			DEBUGSTR(L"...bRequestPostFullReload detected, full reload forces\n");
			srv.bNeedFullReload = TRUE;
			srv.bRequestPostFullReload = FALSE; // �����. FullReload ��� ���������
			nWait = (WAIT_OBJECT_0+1); // �������������
			memset(&srv.CharChanged.hdr, 0, sizeof(srv.CharChanged.hdr));
			lbEventualChange = TRUE; // ����� ��������� �������� ������ �� ����������
		}


		// �������� ������� ������� � ������� ������ - ��� ������� ��������� ��� ������ ������������ �������
		lbLocalForced = FALSE;
        if (nWait == WAIT_TIMEOUT) {
            // � ���������, ������������� ��������� ������� �� �������� (���� ������� �� � ������)
            if (MyGetConsoleScreenBufferInfo(ghConOut, &lsbi)) {
                if (memcmp(&srv.sbi.dwCursorPosition, &lsbi.dwCursorPosition, sizeof(lsbi.dwCursorPosition))) {
					DEBUGSTR(L"...dwCursorPosition changed\n");
					lbLocalForced = TRUE;
                    nWait = (WAIT_OBJECT_0+1);
                }
            }
            if ((nWait == WAIT_TIMEOUT) && GetConsoleCursorInfo(ghConOut, &lci)) {
                if (memcmp(&srv.ci, &lci, sizeof(srv.ci))) {
					DEBUGSTR(L"...CursorInfo changed\n");
					lbLocalForced = TRUE;
                    nWait = (WAIT_OBJECT_0+1);
                }
            }
            
			MCHKHEAP

            #ifdef FORCE_REDRAW_FIX
            if (nWait == WAIT_TIMEOUT) { // ���� ��������� �� ���������� - �������� ������� ����������
	            DWORD nCurTick = GetTickCount();
	            nDelta = nCurTick - nLastUpdateTick;
	            if (nDelta > nDelayRefresh) {
					DEBUGSTR(L"...FORCE_REDRAW_FIX triggered\n");
	                srv.bNeedFullReload = TRUE;
	                //srv.bForceFullSend = TRUE;
	                nLastUpdateTick = nCurTick; // ����� ������� ������� ����� tick
	                nWait = (WAIT_OBJECT_0+1);
	            }
            }
            #endif
        }

        if (nWait == (WAIT_OBJECT_0+1)) {
			#ifdef _DEBUG
			wchar_t szDbg[128], szRgn[64];
			if (srv.CharChanged.hdr.nSize) {
				wsprintf(szRgn, L"{L:%i, T:%i, R:%i, B:%i}", 
					srv.CharChanged.hdr.cr1.X, srv.CharChanged.hdr.cr1.Y,
					srv.CharChanged.hdr.cr2.X, srv.CharChanged.hdr.cr2.Y);
			} else {
				lstrcpyW(szRgn, L"No");
			}
			wsprintf(szDbg, L"...Reloading(Eventual:%i, FullReload:%i, FullSend:%i, Rgn:%s)\n",
				lbEventualChange, srv.bNeedFullReload, srv.bForceFullSend, szRgn);
			DEBUGLOG(szDbg);
			#endif
			MCHKHEAP
            // ����������, ���� �� ��������� � �������
			// � ���� ��� ���� - �������� �������� �������� � �����������
			// �� ���� ����� ���������� ����� ��������� srv.bRequestPostFullReload - 
			// ������ �� ����� ������ ��������� ������ � ����� ����� ��������� ������ ���� ������
			if (ReloadFullConsoleInfo()) {
				DEBUGLOG(L"+++Changes was sent\n");
				if (lbLocalForced)
					srv.bRequestPostFullReload = TRUE; // �� ������ ���� � ������� - ���������� ������ ��� ��� ����� ������� ��������
				nDelayRefresh = MIN_FORCEREFRESH_INTERVAL;
			} else {
				DEBUGLOG(L"---No changes in console\n");
			}
            // ���������, ����� � ��������� ��� ��������� ���������� �������
            nLastUpdateTick = GetTickCount();
			MCHKHEAP
			if (srv.bRequestPostFullReload) {
				DEBUGLOG(L"...bRequestPostFullReload was set\n");
				lbEventualChange = TRUE; // ���� ��������� � �������
				//srv.bRequestPostFullReload = FALSE;
				ResetEvent(srv.hRefreshEvent); // ����� ������� ������ ����������
			}
			DEBUGLOG(L"\n");
        }
        
        WARNING("Win2k ������ ����� ������� ������ �� �����, ��� ��� timeout ����� ������� ������ ����������� (10)");
        // ���� Refresh ������ �� ������� �� ������� - ���������� �������� � �����������
        if (lbEventualChange) {
	        nDelayRefresh = MIN_FORCEREFRESH_INTERVAL;
        } else {
			if (nDelayRefresh < MAX_FORCEREFRESH_INTERVAL) {
				#ifdef FORCE_REDRAW_FIX
				DEBUGLOG(L"...increasing delay refresh\n");
				#endif
		        nDelayRefresh = (DWORD)(nDelayRefresh * 1.3);
			}
        }
		MCHKHEAP
    }
    
    return 0;
}

//Minimum supported client Windows 2000 Professional 
//Minimum supported server Windows 2000 Server 
void WINAPI WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
    if (hwnd != ghConWnd) {
        _ASSERTE(hwnd); // �� ����, ��� ������ ���� ����� ����������� ����, ��������
        return;
    }

    //BOOL bNeedConAttrBuf = FALSE;
    CESERVER_CHAR ch = {{0,0}};
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
        wsprintfW(szDbg, L"EVENT_CONSOLE_START_APPLICATION(PID=%i%s)\n", idObject, (idChild == CONSOLE_APPLICATION_16BIT) ? L" 16bit" : L"");
        DEBUGSTR(szDbg);
        #endif

        if (((DWORD)idObject) != gnSelfPID) {
            EnterCriticalSection(&srv.csProc);
            srv.nProcesses.push_back(idObject);
            LeaveCriticalSection(&srv.csProc);

            if (idChild == CONSOLE_APPLICATION_16BIT) {
                //DWORD ntvdmPID = idObject;
                dwActiveFlags |= CES_NTVDM;
                SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
            }
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
        wsprintfW(szDbg, L"EVENT_CONSOLE_END_APPLICATION(PID=%i%s)\n", idObject, (idChild == CONSOLE_APPLICATION_16BIT) ? L" 16bit" : L"");
        DEBUGSTR(szDbg);
        #endif

        if (((DWORD)idObject) != gnSelfPID) {
            std::vector<DWORD>::iterator iter;
            EnterCriticalSection(&srv.csProc);
            for (iter=srv.nProcesses.begin(); iter!=srv.nProcesses.end(); iter++) {
                if (((DWORD)idObject) == *iter) {
                    srv.nProcesses.erase(iter);
                    if (idChild == CONSOLE_APPLICATION_16BIT) {
                        //DWORD ntvdmPID = idObject;
                        dwActiveFlags &= ~CES_NTVDM;
                        //TODO: �������� ����� ������� ������� NTVDM?
                        SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
                    }
                    // ��������� � ������� �� ��������?
                    if (srv.nProcesses.size() == 0 && !gbInRecreateRoot) {
                        LeaveCriticalSection(&srv.csProc);
                        SetEvent(ghFinilizeEvent);
                        return;
                    }
                    break;
                }
            }
            LeaveCriticalSection(&srv.csProc);
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

    case EVENT_CONSOLE_UPDATE_REGION: // 0x4002 
        {
        //More than one character has changed.
        //The idObject parameter is a COORD structure that specifies the start of the changed region. 
        //The idChild parameter is a COORD structure that specifies the end of the changed region.
            #ifdef _DEBUG
            COORD crStart, crEnd; memmove(&crStart, &idObject, sizeof(idObject)); memmove(&crEnd, &idChild, sizeof(idChild));
            wsprintfW(szDbg, L"EVENT_CONSOLE_UPDATE_REGION({%i, %i} - {%i, %i})\n", crStart.X,crStart.Y, crEnd.X,crEnd.Y);
            DEBUGSTR(szDbg);
            #endif
            //bNeedConAttrBuf = TRUE;
            //// ���������� ������, ��������� �������, � ��.
            //ReloadConsoleInfo();
            srv.bNeedFullReload = TRUE;
            SetEvent(srv.hRefreshEvent);
        }
        return; // ���������� �� ������� � ����

    case EVENT_CONSOLE_UPDATE_SCROLL: //0x4004
        {
        //The console has scrolled.
        //The idObject parameter is the horizontal distance the console has scrolled. 
        //The idChild parameter is the vertical distance the console has scrolled.
            #ifdef _DEBUG
            wsprintfW(szDbg, L"EVENT_CONSOLE_UPDATE_SCROLL(X=%i, Y=%i)\n", idObject, idChild);
            DEBUGSTR(szDbg);
            #endif
            //bNeedConAttrBuf = TRUE;
            //// ���������� ������, ��������� �������, � ��.
            //if (!ReloadConsoleInfo())
            //  return;
            srv.bNeedFullReload = TRUE;
            SetEvent(srv.hRefreshEvent);
        }
        return; // ���������� �� ������� � ����

    case EVENT_CONSOLE_UPDATE_SIMPLE: //0x4003
        {
        //A single character has changed.
        //The idObject parameter is a COORD structure that specifies the character that has changed.
        //Warning! � ������� ��  ���������� ��� ������!
        //The idChild parameter specifies the character in the low word and the character attributes in the high word.
            memmove(&ch.hdr.cr1, &idObject, sizeof(idObject));
            ch.hdr.cr2 = ch.hdr.cr1;
            ch.data[0] = (WCHAR)LOWORD(idChild); ch.data[1] = HIWORD(idChild);
            #ifdef _DEBUG
            wsprintfW(szDbg, L"EVENT_CONSOLE_UPDATE_SIMPLE({%i, %i} '%c'(\\x%04X) A=%i)\n", ch.hdr.cr1.X,ch.hdr.cr1.Y, ch.data[0], ch.data[0], ch.data[1]);
            DEBUGSTR(szDbg);
            #endif
            
            EnterCriticalSection(&srv.csChar);
            if (srv.CharChanged.hdr.nSize) {
	            // ���� ���������� ������� ��� ������ - ����������� ������
	            EnlargeRegion(srv.CharChanged.hdr, ch.hdr.cr1);
            } else {
	            srv.CharChanged = ch; // data - ������ �������������: ������ ����� ���������� �������
	            srv.CharChanged.hdr.nSize = sizeof(CESERVER_CHAR);
		        // srv.bNeedFullReload = TRUE; -- ����� ���������� �������� ������ ���������� ����
		    }
		    LeaveCriticalSection(&srv.csChar);
		    
		    // ������� ����
            SetEvent(srv.hRefreshEvent);
            
            
            //// ���������� ������, ��������� �������, � ��.
            //BOOL lbLayoutChanged = ReloadConsoleInfo(TRUE);
            //
            //SHORT nYCorrected = CorrectTopVisible(ch.crStart.Y);
            //
            //// ���� ������ �� ������ � ������������ � GUI ����� - ������ ������
            //if (nYCorrected < 0 || nYCorrected >= min(gcrBufferSize.Y,srv.sbi.dwSize.Y))
            //    return;
            //
            //int nIdx = ch.crStart.X + nYCorrected * srv.sbi.dwSize.X;
            //_ASSERTE(nIdx>=0 && (DWORD)nIdx<srv.nBufCharCount);
            //
            //if (!lbLayoutChanged) { // ���� Layout �� ���������
            //    // ���� Reload==FALSE, � ch �� ������ ����������� �������/�������� � srv.psChars/srv.pnAttrs - ����� �����");
            //    if (srv.psChars[nIdx] == (wchar_t)ch.data[0] && srv.pnAttrs[nIdx] == ch.data[1])
            //        return; // ������ �� ��������
            //}
            //
            //// ���������
            //srv.psChars[nIdx] = (wchar_t)ch.data[0];
            //// ����� �� ������������ ������ ��� ����� ������ - �� ������ ������������ == srv.nBufCharCount
            //srv.psChars[nIdx+srv.nBufCharCount] = (wchar_t)ch.data[0];
            //srv.pnAttrs[nIdx] = (WORD)ch.data[1];
            //srv.pnAttrs[nIdx+srv.nBufCharCount] = (WORD)ch.data[1];
            //
            //// � ���������
            //ReloadFullConsoleInfo(&ch);
        } return;

    case EVENT_CONSOLE_CARET: //0x4001
        {
        //Warning! WinXPSP3. ��� ������� �������� ������ ���� ������� � ������. 
        //         � � ConEmu ��� ������� �� � ������, ��� ��� ������ �� �����������.
        //The console caret has moved.
        //The idObject parameter is one or more of the following values:
        //      CONSOLE_CARET_SELECTION or CONSOLE_CARET_VISIBLE.
        //The idChild parameter is a COORD structure that specifies the cursor's current position.
            COORD crWhere; memmove(&crWhere, &idChild, sizeof(idChild));
            #ifdef _DEBUG
            wsprintfW(szDbg, L"EVENT_CONSOLE_CARET({%i, %i} Sel=%c, Vis=%c\n", crWhere.X,crWhere.Y, 
                ((idObject & CONSOLE_CARET_SELECTION)==CONSOLE_CARET_SELECTION) ? L'Y' : L'N',
                ((idObject & CONSOLE_CARET_VISIBLE)==CONSOLE_CARET_VISIBLE) ? L'Y' : L'N');
            DEBUGSTR(szDbg);
            #endif
			if (srv.sbi.dwCursorPosition.X != crWhere.X || srv.sbi.dwCursorPosition.Y != crWhere.Y
				|| srv.ci.bVisible != ((idObject & 2/*CONSOLE_CARET_VISIBLE*/)==2/*CONSOLE_CARET_VISIBLE*/))
			{
				SetEvent(srv.hRefreshEvent);
			}
            // ���������� ������, ��������� �������, � ��.
            //if (ReloadConsoleInfo())
            //  return;
        } return;

    case EVENT_CONSOLE_LAYOUT: //0x4005
        {
        //The console layout has changed.
        // ���������� ��� ��������� �������� ��� ��� �������������� ��� ������
            #ifdef _DEBUG
            DEBUGSTR(L"EVENT_CONSOLE_LAYOUT\n");
            #endif
            //bNeedConAttrBuf = TRUE;
            //// ���������� ������, ��������� �������, � ��.
            //if (!ReloadConsoleInfo())
            //  return;
            srv.bNeedFullReload = TRUE;
            SetEvent(srv.hRefreshEvent);
        }
        return; // ���������� �� ������� � ����
    }


    //// ���������� ����� ����� ������ ���� CVirtualConsole ��� �������� ��������� ����
    //if (srv.szGuiPipeName[0] == 0)
    //  return;

    //CESERVER_REQ* pOut = 
    //  Create ConsoleInfo (
    //      (event == EVENT_CONSOLE_UPDATE_SIMPLE) ? &ch : NULL,
    //      FALSE/*bNeedConAttrBuf*/
    //  );
    //_ASSERTE(pOut!=NULL);
    //if (!pOut)
    //  return;

    ////Warning! WinXPSP3. EVENT_CONSOLE_CARET �������� ������ ���� ������� � ������. 
    ////         � � ConEmu ��� ������� �� � ������, ��� ��� ������ �� �����������.
    //// �.�. ���������� ���������� ������� - ��������� GUI �����
    ////if (event != EVENT_CONSOLE_CARET) { // ���� ���������� ������ ��������� ������� - ������������ ���������� �� �����
    ////    srv.bContentsChanged = TRUE;
    ////}
    ////SetEvent(hGlblUpdateEvt);

    ////WARNING("��� ��������� ���������� � GUI ����� CEGUIPIPENAME");
    ////WARNING("���� ���� �������������� � ������ CVirtualConsole �� �����������");

    //SendConsoleChanges ( pOut );

    //Free ( pOut );
}

// ��������������� ������ nY ���, ����� �� ������ �������������� ������, ������������� � GUI
// ������������ ����� �������� ������� ����� ������� (��� ������� � GUI, ���� ������������� '�������������')
SHORT CorrectTopVisible(int nY)
{
    int nYCorrected = nY;
    if (srv.nTopVisibleLine != -1) {
        // ������� ������ � GUI �������������
        nYCorrected = nY - srv.nTopVisibleLine;
    } else if (srv.sbi.dwSize.Y <= gcrBufferSize.Y) {
        // ���� ������� ������ ������ �� ������ ������������ � GUI - ������ �� ��������
        nYCorrected = nY;
    } else {
        // ����� �� Y ������� (0-based) ������ ������� ������� ������
        nYCorrected = nY - srv.sbi.srWindow.Top;
    }
    return nYCorrected;
}

void SendConsoleChanges(CESERVER_REQ* pOut)
{
    //srv.szGuiPipeName ���������������� ������ ����� ����, ��� GUI ����� ����� ����������� ����
    if (srv.szGuiPipeName[0] == 0)
        return;

    HANDLE hPipe = NULL;
    DWORD dwErr = 0, dwMode = 0;
    BOOL fSuccess = FALSE;

    // Try to open a named pipe; wait for it, if necessary. 
    while (1) 
    { 
        hPipe = CreateFile( 
            srv.szGuiPipeName,  // pipe name 
            GENERIC_WRITE, 
            0,              // no sharing 
            NULL,           // default security attributes
            OPEN_EXISTING,  // opens existing pipe 
            0,              // default attributes 
            NULL);          // no template file 

        // Break if the pipe handle is valid. 
        if (hPipe != INVALID_HANDLE_VALUE) 
            break; // OK, �������

        // Exit if an error other than ERROR_PIPE_BUSY occurs. 
        dwErr = GetLastError();
        if (dwErr != ERROR_PIPE_BUSY) 
            return;

        // All pipe instances are busy, so wait for 100 ms.
        if (!WaitNamedPipe(srv.szGuiPipeName, 100) ) 
            return;
    }

    // The pipe connected; change to message-read mode. 
    dwMode = PIPE_READMODE_MESSAGE; 
    fSuccess = SetNamedPipeHandleState( 
        hPipe,    // pipe handle 
        &dwMode,  // new pipe mode 
        NULL,     // don't set maximum bytes 
        NULL);    // don't set maximum time 
    _ASSERT(fSuccess);
    if (!fSuccess) {
        CloseHandle(hPipe);
        return;
    }


    // ���������� ������ � ����
    DWORD dwWritten = 0;
    fSuccess = WriteFile ( hPipe, pOut, pOut->hdr.nSize, &dwWritten, NULL);
	if (!fSuccess || dwWritten != pOut->hdr.nSize) {
		// GUI ��� ���� ������!
		if (ghConEmuWnd && IsWindow(ghConEmuWnd)) {
			// ���� ����������� �� ������, � ��� ����� ��������� assert
			_ASSERTE(fSuccess && dwWritten == pOut->hdr.nSize);
		} else {
			ghConEmuWnd = NULL;
			TODO("�����-�� ��������... �������� ���������� ����, ��� ��� ���-��");
		}
	}
}

CESERVER_REQ* CreateConsoleInfo(CESERVER_CHAR* pRgnOnly, BOOL bCharAttrBuff)
{
	// �� ������ ��������� �����, ����� �� ������������
	DWORD nPacketId = ++srv.nLastPacketID;

    CESERVER_REQ* pOut = NULL;
    DWORD dwAllSize = sizeof(CESERVER_REQ_HDR);
	DWORD nSize; // temp, ��� ����������� ������� ����������� ����������
	#ifdef _DEBUG
	BOOL  lbDataSent = FALSE;
	#endif

    // 1
    HWND hWnd = NULL;
    dwAllSize += (nSize=sizeof(hWnd)); _ASSERTE(nSize==4);
    // 2
    // ��� ����� �������� �� ������, ����� GUI �������� �� '�������' ����� ������� �������
    dwAllSize += (nSize=sizeof(DWORD));
    // 3
    // �� ����� ������ ����������� ������� ����� ����������� ���������� ���������
    // ������� �������� ���������� - �������� ����� ������ � CriticalSection(srv.csProc);
    //EnterCriticalSection(&srv.csProc);
    //DWORD dwProcCount = srv.nProcesses.size()+20;
    //LeaveCriticalSection(&srv.csProc);
    //dwAllSize += sizeof(DWORD)*(dwProcCount+1); // PID ��������� + �� ����������
    dwAllSize += (nSize=sizeof(DWORD)); // ������ ��������� ����������� � GUI, ��� ��� ���� - ������ 0 (Reserved)
    // 4
    //DWORD srv.dwSelRc = 0; CONSOLE_SELECTION_INFO srv.sel = {0}; // GetConsoleSelectionInfo
    dwAllSize += sizeof(srv.dwSelRc)+((srv.dwSelRc==0) ? (nSize=sizeof(srv.sel)) : 0);
    // 5
    //DWORD srv.dwCiRc = 0; CONSOLE_CURSOR_INFO srv.ci = {0}; // GetConsoleCursorInfo
    dwAllSize += sizeof(srv.dwCiRc)+((srv.dwCiRc==0) ? (nSize=sizeof(srv.ci)) : 0);
    // 6, 7, 8
    //DWORD srv.dwConsoleCP=0, srv.dwConsoleOutputCP=0, srv.dwConsoleMode=0;
    dwAllSize += 3*sizeof(DWORD);
    // 9
    //DWORD srv.dwSbiRc = 0; CONSOLE_SCREEN_BUFFER_INFO srv.sbi = {{0,0}}; // MyGetConsoleScreenBufferInfo
    //if (!MyGetConsoleScreenBufferInfo(ghConOut, &srv.sbi)) { srv.dwSbiRc = GetLastError(); if (!srv.dwSbiRc) srv.dwSbiRc = -1; }
    dwAllSize += sizeof(srv.dwSbiRc)+((srv.dwSbiRc==0) ? (nSize=sizeof(srv.sbi)) : 0);
    // 10 -- ���� pRgnOnly->hdr.nSize == 0 - ������ ������ � ������� �� ��������
    dwAllSize += sizeof(DWORD) + (pRgnOnly ? pRgnOnly->hdr.nSize : 0);
    // 11
    DWORD OneBufferSize = 0; //srv.nOneBufferSize; -- ���� ������� ������ �� ��������! - 0
    dwAllSize += sizeof(DWORD);
    // ������������� �������� ������� ����� � ����������� �������������� �� �����
    if (pRgnOnly == NULL && bCharAttrBuff) {
		OneBufferSize = srv.nOneBufferSize;
        TODO("���������� ��� �������� ������ ������������� �������������� � BufferHeight");
        //if (bCharAttrBuff == 2 && OneBufferSize == (srv.nBufCharCount*2)) {
        //    _ASSERTE(srv.nBufCharCount>0);
        //    OneBufferSize = srv.nBufCharCount*2;
        //} else {
		WARNING("ReadConsoleData ��� ������ ���� ��������!");
        //OneBufferSize = ReadConsoleData(); // returns size in bytes of ONE buffer
        //}
        if (OneBufferSize > (200*100*2)) {
            _ASSERTE(OneBufferSize && OneBufferSize<=(200*100*2));
        }
        #ifdef _DEBUG
        if (gnBufferHeight == 0) {
			if (OneBufferSize != (srv.sbi.dwSize.X*srv.sbi.dwSize.Y*2)) {
				// ��� ����� ��������� �� ����� �������
				srv.bRequestPostFullReload = TRUE;
				OneBufferSize = OneBufferSize;
			}
            //_ASSERTE(OneBufferSize == (srv.sbi.dwSize.X*srv.sbi.dwSize.Y*2));
        }
        #endif
        if (OneBufferSize) {
            dwAllSize += OneBufferSize * 2;
        }
    }

    // ��������� ������ // ������ ��������� ��� �����!
    pOut = (CESERVER_REQ*)Alloc(dwAllSize/*+sizeof(CESERVER_REQ)*/, 1); // ������ ������� � ������
    _ASSERT(pOut!=NULL);
    if (pOut == NULL) {
        return FALSE;
    }

    // �������������
    pOut->hdr.nSize = dwAllSize;
    pOut->hdr.nCmd = bCharAttrBuff ? CECMD_GETFULLINFO : CECMD_GETSHORTINFO;
	pOut->hdr.nSrcThreadId = GetCurrentThreadId();
    pOut->hdr.nVersion = CESERVER_REQ_VER;

    // �������
    LPBYTE lpCur = (LPBYTE)(pOut->Data);

    // 1
    hWnd = GetConsoleWindow();
    _ASSERTE(hWnd == ghConWnd);
    *((DWORD*)lpCur) = (DWORD)hWnd;
    lpCur += sizeof(hWnd);

    // 2
    *((DWORD*)lpCur) = nPacketId;
    lpCur += sizeof(DWORD);

    // 3
    // �� ����� ������ ����������� ������� ����� ����������� ���������� ���������
    // ������� �������� ���������� - �������� ����� ������ � CriticalSection(srv.csProc);
    *((DWORD*)lpCur) = 0; lpCur += sizeof(DWORD);
    //EnterCriticalSection(&srv.csProc);
    //DWORD dwTestCount = srv.nProcesses.size();
    //_ASSERTE(dwTestCount<=dwProcCount);
    //if (dwTestCount < dwProcCount) dwProcCount = dwTestCount;
    //*((DWORD*)lpCur) = dwProcCount; lpCur += sizeof(DWORD);
    //for (DWORD n=0; n<dwProcCount; n++) {
    //  *((DWORD*)lpCur) = srv.nProcesses[n];
    //  lpCur += sizeof(DWORD);
    //}
    //LeaveCriticalSection(&srv.csProc);

    // 4
    nSize=sizeof(srv.sel); *((DWORD*)lpCur) = (srv.dwSelRc == 0) ? nSize : 0; lpCur += sizeof(DWORD);
    if (srv.dwSelRc == 0) {
        memmove(lpCur, &srv.sel, nSize); lpCur += nSize;
    }

    // 5
    nSize=sizeof(srv.ci); *((DWORD*)lpCur) = (srv.dwCiRc == 0) ? nSize : 0; lpCur += sizeof(DWORD);
    if (srv.dwCiRc == 0) {
        memmove(lpCur, &srv.ci, nSize); lpCur += nSize;
    }

    // 6
    *((DWORD*)lpCur) = srv.dwConsoleCP; lpCur += sizeof(DWORD);
    // 7
    *((DWORD*)lpCur) = srv.dwConsoleOutputCP; lpCur += sizeof(DWORD);
    // 8
    *((DWORD*)lpCur) = srv.dwConsoleMode; lpCur += sizeof(DWORD);

    // 9
    //if (!MyGetConsoleScreenBufferInfo(ghConOut, &srv.sbi)) { srv.dwSbiRc = GetLastError(); if (!srv.dwSbiRc) srv.dwSbiRc = -1; }
    nSize=sizeof(srv.sbi); *((DWORD*)lpCur) = (srv.dwSbiRc == 0) ? nSize : 0; lpCur += sizeof(DWORD);
    if (srv.dwSbiRc == 0) {
        memmove(lpCur, &srv.sbi, nSize); lpCur += nSize;
    }

    // 10
    *((DWORD*)lpCur) = pRgnOnly ? pRgnOnly->hdr.nSize : 0; lpCur += sizeof(DWORD);
    if (pRgnOnly && pRgnOnly->hdr.nSize != 0) {
		#ifdef _DEBUG
		wchar_t szDbg[128];
		wsprintf(szDbg, L"+++Sending region {L:%i, T:%i, R:%i, B:%i}, DataSize: %i bytes\n", 
					pRgnOnly->hdr.cr1.X, pRgnOnly->hdr.cr1.Y,
					pRgnOnly->hdr.cr2.X, pRgnOnly->hdr.cr2.Y,
					pRgnOnly->hdr.nSize);
		DEBUGLOG(szDbg);
		lbDataSent = TRUE;
		#endif
        memmove(lpCur, pRgnOnly, pRgnOnly->hdr.nSize); lpCur += (nSize=pRgnOnly->hdr.nSize);
    }

    // 11 - ����� ����� 0, ���� ����� � ������� �� �������
    *((DWORD*)lpCur) = OneBufferSize; lpCur += sizeof(DWORD);
    if (OneBufferSize && OneBufferSize!=(DWORD)-1) { // OneBufferSize==0, ���� pRgnOnly!=0
		#ifdef _DEBUG
		DEBUGLOG(L"---Sending full console data\n");
		TODO("�� ����� ������� ������� ����� ������������ - ������ �� �� ��� �����...");
		//_ASSERTE(*srv.psChars!=9553);
		lbDataSent = TRUE;
		#endif
        memmove(lpCur, srv.psChars, OneBufferSize); lpCur += OneBufferSize;
        memmove(lpCur, srv.pnAttrs, OneBufferSize); lpCur += OneBufferSize;
    }

	#ifdef _DEBUG
	if (!lbDataSent) {
		DEBUGLOG(L"---Sending only console information\n");
	}
	#endif
    
    if (ghLogSize) {
        static COORD scr_Last = {-1,-1};
        if (scr_Last.X != srv.sbi.dwSize.X || scr_Last.Y != srv.sbi.dwSize.Y) {
            LogSize(&srv.sbi.dwSize, ":CreateConsoleInfo(query only)");
            scr_Last = srv.sbi.dwSize;
        }
    }

    return pOut;
}

// pChangedRgn ����� ������, ���� �� �����, ��� �����-�� ������� ����� ��������
// � ���� ������ pChangedRgn ����� ���� ��������
BOOL ReloadConsoleInfo(CESERVER_CHAR* pChangedRgn/*=NULL*/)
//BOOL ReloadConsoleInfo(BOOL abSkipCursorCharCheck/*=FALSE*/)
{
    BOOL lbChanged = FALSE;
    //CONSOLE_SELECTION_INFO lsel = {0}; // GetConsoleSelectionInfo
    CONSOLE_CURSOR_INFO lci = {0}; // GetConsoleCursorInfo
    DWORD ldwConsoleCP=0, ldwConsoleOutputCP=0, ldwConsoleMode=0;
    CONSOLE_SCREEN_BUFFER_INFO lsbi = {{0,0}}; // MyGetConsoleScreenBufferInfo

	MCHKHEAP

    TODO("������-�� Selection ����� ������������ ���� � GUI, ��� ��� ��� ������ �������� �����, ��� ��������� ��������");
    srv.dwSelRc = 0; memset(&srv.sel, 0, sizeof(srv.sel));
    //if (!GetConsoleSelectionInfo(&lsel)) { srv.dwSelRc = GetLastError(); if (!srv.dwSelRc) srv.dwSelRc = -1; } else {
    //    srv.dwSelRc = 0;
    //    if (memcmp(&srv.sel, &lsel, sizeof(srv.sel))) {
    //        srv.sel = lsel;
    //        lbChanged = TRUE;
    //    }
    //}

	MCHKHEAP

    if (!GetConsoleCursorInfo(ghConOut, &lci)) { srv.dwCiRc = GetLastError(); if (!srv.dwCiRc) srv.dwCiRc = -1; } else {
        srv.dwCiRc = 0;
        if (memcmp(&srv.ci, &lci, sizeof(srv.ci))) {
            srv.ci = lci;
            lbChanged = TRUE;
        }
    }

    ldwConsoleCP = GetConsoleCP(); if (srv.dwConsoleCP!=ldwConsoleCP) { srv.dwConsoleCP = ldwConsoleCP; lbChanged = TRUE; }
    ldwConsoleOutputCP = GetConsoleOutputCP(); if (srv.dwConsoleOutputCP!=ldwConsoleOutputCP) { srv.dwConsoleOutputCP = ldwConsoleOutputCP; lbChanged = TRUE; }
    ldwConsoleMode=0; GetConsoleMode(ghConIn, &ldwConsoleMode); if (srv.dwConsoleMode!=ldwConsoleMode) { srv.dwConsoleMode = ldwConsoleMode; lbChanged = TRUE; }

	MCHKHEAP

    if (!MyGetConsoleScreenBufferInfo(ghConOut, &lsbi)) { srv.dwSbiRc = GetLastError(); if (!srv.dwSbiRc) srv.dwSbiRc = -1; } else {
        srv.dwSbiRc = 0;
        if (memcmp(&srv.sbi, &lsbi, sizeof(srv.sbi))) {
	        // ��������� � ����������� / �������� ���� (��������� ������ ����. ��� ������ � �������� ��������� �����)
	        
	        // ��������� ������ ������. ������������ ��� ��� ���������
	        if (srv.sbi.dwSize.X != lsbi.dwSize.X || srv.sbi.dwSize.Y != lsbi.dwSize.Y) {
		        srv.bForceFullSend = TRUE; // ���������� � ������� ���
	        } else
	        
	        // ���� ��������� ������� ������ (���������) ������ ���, ���� ��� ���������
	        TODO("��� ����������� ����� ���� �� ����������� ������������� ����� ��������� �����, �� ���� ���");
	        if (srv.sbi.srWindow.Top != lsbi.srWindow.Top) {
		        srv.bForceFullSend = TRUE; // ���������� � ������� ���
	        } else
	        
	        // ��� ����� ������� �������...
            if (srv.psChars && srv.pnAttrs /*&& !abSkipCursorCharCheck*/
                && !(srv.bForceFullSend || srv.bNeedFullReload) // ���� ������ � ��� ����� ���������� ������� - �� ���������
                && memcmp(&srv.sbi.dwCursorPosition, &lsbi.dwCursorPosition, sizeof(lsbi.dwCursorPosition))
                )
            {
                // � ��������� ������� �� ����������� �� EVENT_CONSOLE_UPDATE_SIMPLE �� EVENT_CONSOLE_UPDATE_REGION
                // ������. ��������� cmd.exe. �������� �����-�� ����� � ��������� ������ � �������� 'Esc'
                // ��� Esc ������� ������� ������ �� ���������, � ����� � ������� ���������!

                // ������, ���� ���� ��������� � ��������/��������� ������ �� ������� ��� ������,
				// ��� �� ������� ������ ������� - ���������� ������� ��������� - bForceFullSend=TRUE
                int nCount = min(lsbi.dwSize.X, (int)srv.nLineCmpSize);
                if (nCount && srv.ptrLineCmp) {
					MCHKHEAP
                    DWORD nbActuallyRead = 0;
                    COORD coord = {0,0};
                    DWORD nBufferShift = 0;
                    for (int i=0; i<=1; i++) {
                        if (i==0) {
                            // ������ �� ������� ��� ������
                            coord.Y = srv.sbi.dwCursorPosition.Y;
                            // � ������ ������ ������� ������...
                            nBufferShift = coord.Y - srv.sbi.srWindow.Top;
                        } else {
                            // ������ �� ������� ����� ������
                            if (coord.Y == lsbi.dwCursorPosition.Y) break; // ������ �� ��������, ������ �������
                            coord.Y = lsbi.dwCursorPosition.Y;
                            // � ������ ������ ������� ������...
                            nBufferShift = coord.Y - srv.sbi.srWindow.Top;
                        }
                        // �����������
                        if (nBufferShift < 0 || nBufferShift >= (USHORT)gcrBufferSize.Y)
                            continue; // ����� �� ������� � GUI �������
                        nBufferShift = nBufferShift*gcrBufferSize.X;

						MCHKHEAP

						if (ReadConsoleOutputAttribute(ghConOut, srv.ptrLineCmp, nCount, coord, &nbActuallyRead)) {
							if (memcmp(srv.ptrLineCmp, srv.pnAttrs+nBufferShift, nbActuallyRead*2)) {
								//srv.bForceFullSend = TRUE; break;
								if (pChangedRgn) {
									EnlargeRegion(pChangedRgn->hdr, coord);
									coord.X = gcrBufferSize.X -1;
									EnlargeRegion(pChangedRgn->hdr, coord);
								}
							}
						}

						MCHKHEAP

						coord.X = 0;
                        if (ReadConsoleOutputCharacter(ghConOut, (wchar_t*)srv.ptrLineCmp, nCount, coord, &nbActuallyRead)) {
                            if (memcmp(srv.ptrLineCmp, srv.psChars+nBufferShift, nbActuallyRead*2)) {
                                //srv.bForceFullSend = TRUE; break;
								if (pChangedRgn) {
	                                EnlargeRegion(pChangedRgn->hdr, coord);
									coord.X = gcrBufferSize.X -1;
									EnlargeRegion(pChangedRgn->hdr, coord);
								}
                            }
                        }
                    }
                }
            }
            if ((lsbi.srWindow.Bottom - lsbi.srWindow.Top)>lsbi.dwMaximumWindowSize.Y) {
                _ASSERTE((lsbi.srWindow.Bottom - lsbi.srWindow.Top)<lsbi.dwMaximumWindowSize.Y);
            }

			MCHKHEAP
            
            // ��������� � ����������� / �������� ����, ���������� ��
            srv.sbi = lsbi;
			// ��������� GUI'���� nTopVisibleLine, nVisibleHeight, � ��., ���� ����� �� ���������
			if (gnBufferHeight == 0) {
				_ASSERTE(srv.sbi.dwSize.Y<=200);
				// � ������ ��� ��������� - ������� ����� ������ ���� �����!
				srv.nVisibleHeight = srv.sbi.dwSize.Y;
				srv.nTopVisibleLine = -1; // �� � ������� ������� ����� ������ ���� ������
			} else {
				// � ���������� �������...
				if ((lsbi.srWindow.Bottom - lsbi.srWindow.Top + 1) >= MIN_CON_HEIGHT) {
					srv.nVisibleHeight = (lsbi.srWindow.Bottom - lsbi.srWindow.Top + 1);
				}
				if (srv.nTopVisibleLine != -1) {
					if ((srv.nTopVisibleLine + srv.nVisibleHeight) > srv.sbi.dwSize.Y)
						srv.nTopVisibleLine = max(0, (srv.sbi.dwSize.Y - srv.nVisibleHeight));
					if ((srv.nTopVisibleLine + srv.nVisibleHeight) > srv.sbi.dwSize.Y)
						srv.nVisibleHeight = srv.sbi.dwSize.Y - srv.nTopVisibleLine;
				}
			}
            lbChanged = TRUE;
        }
    }

    return lbChanged;
}

BOOL MyGetConsoleScreenBufferInfo(HANDLE ahConOut, PCONSOLE_SCREEN_BUFFER_INFO apsc)
{
    BOOL lbRc = FALSE;

	CSection cs(NULL,NULL);
	if (gnRunMode == RM_SERVER)
		cs.Enter(&srv.csChangeSize, &srv.ncsTChangeSize);

	if (gnRunMode == RM_SERVER) // ComSpec ���� ������ �� ������!
	{
		// ���� ���� �������� ����� ������������, ����� ���������� ���� ������ - ������ �������� �� �����
		if (IsZoomed(ghConWnd)) {
			SendMessage(ghConWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			Sleep(200);
			//lbRc = GetConsoleScreenBufferInfo(ahConOut, apsc);
			//SetConsoleSize(gnBufferHeight, gcrBufferSize, srv.sbi.srWindow);
			// ���� ����� �� ������� - ������ ������� ������ ���������
			RECT rcConPos;
			GetWindowRect(ghConWnd, &rcConPos);
			MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
			if (gnBufferHeight == 0)
			{
				//specified width and height cannot be less than the width and height of the console screen buffer's window
				lbRc = SetConsoleScreenBufferSize(ghConOut, gcrBufferSize);
			} else {
				// ������� ������ ��� BufferHeight
				COORD crHeight = {gcrBufferSize.X, gnBufferHeight};
				MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
				lbRc = SetConsoleScreenBufferSize(ghConOut, crHeight); // � �� crNewSize - ��� "�������" �������
			}
			SetConsoleWindowInfo(ghConOut, TRUE, &srv.sbi.srWindow);
		}
	}

    lbRc = GetConsoleScreenBufferInfo(ahConOut, apsc);
	_ASSERTE((apsc->srWindow.Bottom-apsc->srWindow.Top)<150);
    if (lbRc && gnRunMode == RM_SERVER) // ComSpec ���� ������ �� ������!
	{
				// ���������� � SetConsoleSize
				//     if (gnBufferHeight) {
				//// ���� �� ����� � ������ BufferHeight - ����� ����������������� ������ (����� ��� ���� �������?)
				//         if (gnBufferHeight <= (apsc->dwMaximumWindowSize.Y * 1.2))
				//             gnBufferHeight = max(300, (SHORT)(apsc->dwMaximumWindowSize.Y * 1.2));
				//     }

        // ���� ��������� ���� �� ������ - �� ����������� ������ ��, ����� ��� ������� FAR
        // ���������� ������ � ������� �������
		BOOL lbNeedCorrect = FALSE;
		if (apsc->srWindow.Left > 0) {
			lbNeedCorrect = TRUE; apsc->srWindow.Left = 0;
		}
		if ((apsc->srWindow.Right+1) < apsc->dwSize.X) {
			lbNeedCorrect = TRUE; apsc->srWindow.Right = (apsc->dwSize.X - 1);
		}
		BOOL lbBufferHeight = FALSE;
		if (apsc->dwSize.Y >= (apsc->dwMaximumWindowSize.Y * 1.2))
			lbBufferHeight = TRUE;

		if (!lbBufferHeight) {
			_ASSERTE((apsc->srWindow.Bottom-apsc->srWindow.Top)<200);

			if (apsc->srWindow.Top > 0) {
				lbNeedCorrect = TRUE; apsc->srWindow.Top = 0;
			}
			if ((apsc->srWindow.Bottom+1) < apsc->dwSize.Y) {
				lbNeedCorrect = TRUE; apsc->srWindow.Bottom = (apsc->dwSize.Y - 1);
			}
        }
		if (lbNeedCorrect) {
			lbRc = SetConsoleWindowInfo(ghConOut, TRUE, &apsc->srWindow);
			lbRc = GetConsoleScreenBufferInfo(ahConOut, apsc);
		}
        CorrectVisibleRect(apsc);
    }

	cs.Leave();

	#ifdef _DEBUG
	if ((apsc->srWindow.Bottom - apsc->srWindow.Top)>apsc->dwMaximumWindowSize.Y) {
		_ASSERTE((apsc->srWindow.Bottom - apsc->srWindow.Top)<apsc->dwMaximumWindowSize.Y);
	}
	#endif

    return lbRc;
}

void CorrectVisibleRect(CONSOLE_SCREEN_BUFFER_INFO* pSbi)
{
	_ASSERTE(gcrBufferSize.Y<200);

    // ���������� �������������� ���������
    pSbi->srWindow.Left = 0; pSbi->srWindow.Right = pSbi->dwSize.X - 1;
    if (gnBufferHeight == 0) {
        // ������ ��� ��� �� ������ ������������ �� ��������� ������ BufferHeight
        if (pSbi->dwMaximumWindowSize.Y < pSbi->dwSize.Y)
            gnBufferHeight = pSbi->dwSize.Y; // ��� ���������� �������� �����
    }
    // ���������� ������������ ��������� ��� �������� ������
    if (gnBufferHeight == 0) {
        pSbi->srWindow.Top = 0; pSbi->srWindow.Bottom = pSbi->dwSize.Y - 1;
    } else if (srv.nTopVisibleLine!=-1) {
        // � ��� '���������' ������ ������� ����� ���� �������������
        pSbi->srWindow.Top = srv.nTopVisibleLine;
        pSbi->srWindow.Bottom = min( (pSbi->dwSize.Y-1), (srv.nTopVisibleLine+gcrBufferSize.Y-1) );
    } else {
        // ������ ������������ ������ ������ �� ������������� � GUI �������
        TODO("������-�� ������ �� ��� ��������� ������� ���, ����� ������ ��� �����");
        pSbi->srWindow.Bottom = min( (pSbi->dwSize.Y-1), (pSbi->srWindow.Top+gcrBufferSize.Y-1) );
    }

	#ifdef _DEBUG
	if ((pSbi->srWindow.Bottom - pSbi->srWindow.Top)>pSbi->dwMaximumWindowSize.Y) {
		_ASSERTE((pSbi->srWindow.Bottom - pSbi->srWindow.Top)<pSbi->dwMaximumWindowSize.Y);
	}
	#endif
}

BOOL ReloadFullConsoleInfo(/*CESERVER_CHAR* pCharOnly/ *=NULL*/)
{
	#ifdef _DEBUG
	DWORD dwCurThId = GetCurrentThreadId();
	// ������ ���������� ������ � ������� ���� (RefreshThread)
	_ASSERTE(dwCurThId == srv.dwRefreshThread);
	#endif

	BOOL lbChangesWasSent = FALSE;
    CESERVER_CHAR* pCheck = NULL;
    BOOL lbInfChanged = FALSE, lbDataChanged = FALSE;
    //DWORD dwBufSize = 0;
    BOOL lbCharChangedSet = FALSE;
    BOOL bForceFullSend = FALSE;
    CESERVER_CHAR lCharChanged = {0}; // CRITICAL_SECTION csChar;

	MCHKHEAP
    
    EnterCriticalSection(&srv.csChar);
    if (srv.bNeedFullReload || srv.bForceFullSend) {
	    // ��� ����� ����� ���������� ��� �������
	    //srv.bCharChangedSet = FALSE;
	    memset(&srv.CharChanged, 0, sizeof(srv.CharChanged));
    } else {
	    lbCharChangedSet = srv.CharChanged.hdr.nSize != 0;
	    if (lbCharChangedSet) {
		    lCharChanged = srv.CharChanged;
		    //srv.bCharChangedSet = FALSE;
		    memset(&srv.CharChanged, 0, sizeof(srv.CharChanged));
		    pCheck = &lCharChanged;
	    }
	}
    LeaveCriticalSection(&srv.csChar);

	MCHKHEAP


	// ��������� ����������� �������: srv.sbi, srv.ci, srv.dwConsoleMode, srv.dwConsoleCP, srv.dwConsoleOutputCP
    // ������� ����� ���������� � TRUE srv.bForceFullSend.
	MCHKHEAP
    lbInfChanged = ReloadConsoleInfo(srv.bForceFullSend ? NULL : &lCharChanged); // lCharChanged ����� ���� ��� ������
	#ifdef _DEBUG
		if (lCharChanged.hdr.nSize) {
			_ASSERTE(lCharChanged.hdr.cr1.Y>=0 && lCharChanged.hdr.cr1.Y<srv.sbi.dwSize.Y);
			_ASSERTE(lCharChanged.hdr.cr2.Y>=0 && lCharChanged.hdr.cr2.Y<srv.sbi.dwSize.Y);
		}
	#endif
	MCHKHEAP
    if (srv.bForceFullSend)
	    pCheck = NULL; // ����������� ������ ������������� ����������� �������
    else if (!pCheck && lCharChanged.hdr.nSize)
	    pCheck = &lCharChanged; // ������ ReloadConsoleInfo ����������, ��� ����� �������� ������ ���������� �����

	MCHKHEAP

	// srv.bNeedFullReload  - ������������ ��� ������ ��� �� �������� �� �������
	// srv.bForceFullSend - ���������� ReloadConsoleInfo
    if (srv.bNeedFullReload || srv.bForceFullSend || pCheck) {
        
		if (pCheck == NULL && !srv.bForceFullSend) { // ��� ����� ���������� ���������� ���������� �������
			memset(&lCharChanged, 0, sizeof(lCharChanged));
			lCharChanged.hdr.cr1.Y = (srv.nTopVisibleLine == -1) ? srv.sbi.srWindow.Top : srv.nTopVisibleLine;
			lCharChanged.hdr.cr2.X = srv.sbi.dwSize.X - 1;
			lCharChanged.hdr.cr2.Y = lCharChanged.hdr.cr1.Y + srv.nVisibleHeight - 1;
			lCharChanged.hdr.nSize = sizeof(lCharChanged.hdr);
			// ������� ������� �� GUI ����� ��� �� ����������

			#ifdef _DEBUG
			if (lCharChanged.hdr.nSize) {
				_ASSERTE(lCharChanged.hdr.cr1.Y>=0 && lCharChanged.hdr.cr1.Y<srv.sbi.dwSize.Y);
				_ASSERTE(lCharChanged.hdr.cr2.Y>=0 && lCharChanged.hdr.cr2.Y<srv.sbi.dwSize.Y);
			}
			#endif
		}
		MCHKHEAP
		_ASSERTE(pCheck==NULL || pCheck==&lCharChanged);
		MCHKHEAP
		lbDataChanged = ReadConsoleData(srv.bForceFullSend ? NULL : &lCharChanged);
		MCHKHEAP
		#ifdef _DEBUG
			if (lCharChanged.hdr.nSize) {
				_ASSERTE(lCharChanged.hdr.cr1.Y>=0 && lCharChanged.hdr.cr1.Y<srv.sbi.dwSize.Y);
				_ASSERTE(lCharChanged.hdr.cr2.Y>=0 && lCharChanged.hdr.cr2.Y<srv.sbi.dwSize.Y);
			}
		#endif

		// ����� ���������� ������ ����� ReadConsoleData
		if (srv.bForceFullSend) {
			bForceFullSend = TRUE;
			pCheck = NULL; // �� ������� � ���������������
		} else if (lbDataChanged && !pCheck) {
			pCheck = &lCharChanged; // ���� �� ��������� ������� ������� �������������� - ������ ������ ���������� ������
		}
		srv.bNeedFullReload = FALSE;
		srv.bForceFullSend = FALSE;

        
        //if (lbDataChanged && pCheck != NULL) {
        //    pCharOnly = pCheck;
        //    lbDataChanged = FALSE; // ��������� ����������� ����� pCharOnly
        //} else 
        if (!lbDataChanged && (bForceFullSend || pCheck))
        {
            // ��� ���� ����� ����������, �� ������������� ���������� lbDataChanged
            // ��� ������, ��� �� �����-�� ������� ������ � ����� �������� ������������ ������... ���-�� ���, ���� �� �����
            lbDataChanged = TRUE;
            //DEBUGSTR(L"!!! Forced full console data send\n");
        }
    }

    if (lbInfChanged || lbDataChanged || pCheck) {
        //TODO("����� ����������� ������� ������������ ������������� � ���������� ������ ���");
        
        // ��� �� RefreshThread
        //CESERVER_REQ* pOut = CreateConsoleInfo(lbDataChanged ? NULL : pCharOnly, lbDataChanged ? 2 : 0); -- 2009.06.05 ���� ���
        CESERVER_CHAR *pChangedBuffer = NULL;
        #ifdef RELATIVE_TRANSMIT_DISABLE
        bForceFullSend = TRUE;
        #endif
		// ���� ��������� � ������� ������� ��� ��� - (pCheck->hdr.nSize == 0)
        if (!bForceFullSend && pCheck && pCheck->hdr.nSize) {
	        // ����� ������������ pChangedBuffer �� �������������� �� pCheck, ���� �� bForceFullSend !!!
	        // ����� �� �� ������ ���������� ����� ����� ������ �� ����� ������ ���� �������
	        //bForceFullSend = TRUE;
			MCHKHEAP
			_ASSERTE(pCheck->hdr.cr1.X<=pCheck->hdr.cr2.X && pCheck->hdr.cr1.Y<=pCheck->hdr.cr2.Y);
			// ���������� Y ��� ��������������� �� ������� �������
			int nSize = sizeof(CESERVER_CHAR_HDR) // ��������� + ���������� ����� * ������ * (������ + �������)
				+ (pCheck->hdr.cr2.Y - pCheck->hdr.cr1.Y + 1)
				* (pCheck->hdr.cr2.X - pCheck->hdr.cr1.X + 1)
				* (sizeof(WORD)+sizeof(wchar_t));
			if (nSize > (int)srv.nChangedBufferSize) {
				Free(srv.pChangedBuffer);
				srv.pChangedBuffer = (CESERVER_CHAR*)Alloc(nSize, 1);
				srv.nChangedBufferSize = (srv.pChangedBuffer != NULL) ? nSize : 0;
			}
			pChangedBuffer = srv.pChangedBuffer;
			if (pChangedBuffer) {
				pChangedBuffer->hdr.nSize = nSize;
				// ����� � ��� ���������� � �� ������� ����������, �� ���������� ������ � ������� �������
				pChangedBuffer->hdr.cr1 = pCheck->hdr.cr1;
				pChangedBuffer->hdr.cr2 = pCheck->hdr.cr2;

				// ������� ������������� ����������
				int nTop = (srv.nTopVisibleLine == -1) ? srv.sbi.srWindow.Top : srv.nTopVisibleLine;
				int nRelativeY1 = pChangedBuffer->hdr.cr1.Y - nTop;
				int nRelativeY2 = pChangedBuffer->hdr.cr2.Y - nTop;
				_ASSERTE(nRelativeY1>=0 && nRelativeY2>=nRelativeY1 && nRelativeY2<(nTop+srv.nVisibleHeight));
					
				// ����� ��������� pChangedBuffer->data
				int nLineLen = pChangedBuffer->hdr.cr2.X - pChangedBuffer->hdr.cr1.X + 1;
				_ASSERTE(nLineLen>0);
				int nLineSize = nLineLen * 2;
				int nLineCount = pChangedBuffer->hdr.cr2.Y - pChangedBuffer->hdr.cr1.Y + 1;
				_ASSERTE(nLineCount>0);
				_ASSERTE((nLineSize*nLineCount*2+sizeof(CESERVER_CHAR_HDR))==nSize);
				// ��������� �� ���������� ������
				wchar_t* pszSend = (wchar_t*)(pChangedBuffer->data);
				WORD*    pnSend  = ((WORD*)pChangedBuffer->data)+(nLineLen*nLineCount);
				// ��������� �� ��������� �� ������� ������
				wchar_t* pszCon = srv.psChars + pChangedBuffer->hdr.cr1.X + nRelativeY1 * srv.sbi.dwSize.X;
				WORD* pnCon = srv.pnAttrs + pChangedBuffer->hdr.cr1.X + nRelativeY1 * srv.sbi.dwSize.X;
				MCHKHEAP
				// ������ �������
				int nConLineLen = srv.sbi.dwSize.X;
				//int nConLineSize = nConLineLen * 2; // wchar_t / WORD
				// �������� �� ������� (��� ���������� ��� �� ������ - �������� � �����������)
				for (int y = nRelativeY1; y <= nRelativeY2; y++) {
					#ifdef _DEBUG
						_ASSERTE(!IsBadWritePtr(pszSend, nLineSize));
						_ASSERTE(!IsBadWritePtr(pnSend, nLineSize));
						_ASSERTE(!IsBadReadPtr(pszCon, nLineSize));
						_ASSERTE(!IsBadReadPtr(pnCon, nLineSize));
					#endif
					memmove(pszSend, pszCon, nLineSize);
						pszSend += nLineLen; pszCon += nConLineLen;
					MCHKHEAP
					memmove(pnSend, pnCon, nLineSize);
						pnSend += nLineLen; pnCon += nConLineLen;
					MCHKHEAP
				}

				// ��� ���� ��������� ������ ������ � ������� �������
				//COORD    coord = pChangedBuffer->hdr.cr1;
				//DWORD    nbActuallyRead = 0;
				//for (coord.Y = pChangedBuffer->hdr.cr1.Y; coord.Y <= pChangedBuffer->hdr.cr2.Y; coord.Y++) {
				//	ReadConsoleOutputCharacter(ghConOut, pszLine, nLineLen, coord, &nbActuallyRead); 
				//		pszLine += nLineLen;
				//	ReadConsoleOutputAttribute(ghConOut, pnLine, nLineLen, coord, &nbActuallyRead);
				//		pnLine += nLineLen;
				//}
			}
		} else {
			pCheck = NULL;
		}

		MCHKHEAP
		
		#ifdef RELATIVE_TRANSMIT_DISABLE
		bForceFullSend = TRUE;
		// ������ ��� pChangedBuffer �� �����������, � ������������ ��������
		if (pChangedBuffer) 
			pChangedBuffer = NULL;
		#endif
        
		// ������������ ��� ���������� ���������� ���� �� ������
		_ASSERTE((pChangedBuffer!=NULL)!=bForceFullSend || !bForceFullSend);
		// CreateConsoleInfo �� ��������� ������ ��� ����������� ������ � pChangedBuffer
		// ��� ������ �������� ������ ��� ������� � �������� � ��� �� ��� ��� ����
        CESERVER_REQ* pOut = CreateConsoleInfo(pChangedBuffer, bForceFullSend);

		if (pChangedBuffer) {
			if (pChangedBuffer->hdr.nSize>((int)sizeof(CESERVER_CHAR_HDR)+4)) {
				srv.bRequestPostFullReload = TRUE; // ���� ���������� ������ ����� ����� - ���������� ������ ��� ��� ����� ������� ��������
			}
		}

		MCHKHEAP

		// ������ ��� pChangedBuffer �� �����������, � ������������ ��������
		//if (pChangedBuffer) { Free(pChangedBuffer); pChangedBuffer = NULL; }

		MCHKHEAP

        _ASSERTE(pOut!=NULL); 
		if (!pOut) {
			srv.bForceFullSend = TRUE;
			return TRUE; // ��������� ����, �� ������-�� �� ������ �� ���������
		}
        
		MCHKHEAP

        // !!! �������� ���������� � GUI
        SendConsoleChanges(pOut);
		lbChangesWasSent = TRUE;

		MCHKHEAP
        
        Free(pOut);
        
        // ��������� ��������� ������������
        if (lbDataChanged && srv.nBufCharCount) {
            MCHKHEAP
            memmove(srv.psChars+srv.nBufCharCount, srv.psChars, srv.nBufCharCount*sizeof(wchar_t));
            memmove(srv.pnAttrs+srv.nBufCharCount, srv.pnAttrs, srv.nBufCharCount*sizeof(WORD));
            MCHKHEAP
        }
    }
    
    //
    //srv.nLastUpdateTick = GetTickCount();

    //if (pCheck) Free(pCheck); -- ��� ������ �� ��������� ����������
	return lbChangesWasSent;
}

// BufferHeight  - ������ ������ (0 - ��� ���������)
// crNewSize     - ������ ���� (������ ���� == ������ ������)
// rNewRect      - ��� (BufferHeight!=0) ���������� new upper-left and lower-right corners of the window
BOOL SetConsoleSize(USHORT BufferHeight, COORD crNewSize, SMALL_RECT rNewRect, LPCSTR asLabel)
{
    _ASSERTE(ghConWnd);
    if (!ghConWnd) return FALSE;

	CSection cs(NULL,NULL);
	if (gnRunMode == RM_SERVER)
		cs.Enter(&srv.csChangeSize, &srv.ncsTChangeSize);
    
    if (ghLogSize) LogSize(&crNewSize, asLabel);

    _ASSERTE(crNewSize.X>=MIN_CON_WIDTH && crNewSize.Y>=MIN_CON_HEIGHT);

    // �������� ������������ �������
    if (crNewSize.X</*4*/MIN_CON_WIDTH)
        crNewSize.X = /*4*/MIN_CON_WIDTH;
    if (crNewSize.Y</*3*/MIN_CON_HEIGHT)
        crNewSize.Y = /*3*/MIN_CON_HEIGHT;

    BOOL lbNeedChange = TRUE;
    CONSOLE_SCREEN_BUFFER_INFO csbi = {{0,0}};
    if (MyGetConsoleScreenBufferInfo(ghConOut, &csbi)) {
        lbNeedChange = (csbi.dwSize.X != crNewSize.X) || (csbi.dwSize.Y != crNewSize.Y);
    }

	// ������ ��� ����� MyGetConsoleScreenBufferInfo, �.�. ��������� ��������� ������� ���� 
	// ��� ������ ������������ �� gnBufferHeight
    gnBufferHeight = BufferHeight;
    gcrBufferSize = crNewSize;
	_ASSERTE(gcrBufferSize.Y<200);

    if (gnBufferHeight) {
		// � ������ BufferHeight - ������ ������ ���� ������ ����������� ������� ���� �������
		// ����� �� ���������� ��� ��������� "�������� �� ��� �����"...
        if (gnBufferHeight <= (csbi.dwMaximumWindowSize.Y * 1.2))
            gnBufferHeight = max(300, (SHORT)(csbi.dwMaximumWindowSize.Y * 1.2));
    }

    RECT rcConPos = {0};
    GetWindowRect(ghConWnd, &rcConPos);

    BOOL lbRc = TRUE;
    //DWORD nWait = 0;

    //if (srv.hChangingSize) {
    //    nWait = WaitForSingleObject(srv.hChangingSize, 300);
    //    _ASSERTE(nWait == WAIT_OBJECT_0);
    //    ResetEvent(srv.hChangingSize);
    //}

    // case: simple mode
    if (BufferHeight == 0)
    {
        if (lbNeedChange) {
            DWORD dwErr = 0;
            // ���� ����� �� ������� - ������ ������� ������ ���������
            MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
            //specified width and height cannot be less than the width and height of the console screen buffer's window
            lbRc = SetConsoleScreenBufferSize(ghConOut, crNewSize);
            if (!lbRc) dwErr = GetLastError();
            //TODO: � ���� ������ ������ ���� ������� �� ������� ������?
            //WARNING("�������� ��� �����");
            //MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1);
			rNewRect.Left = 0; rNewRect.Top = 0;
			rNewRect.Right = crNewSize.X - 1;
			rNewRect.Bottom = crNewSize.Y - 1;
			SetConsoleWindowInfo(ghConOut, TRUE, &rNewRect);
        }

    } else {
        // ������� ������ ��� BufferHeight
        COORD crHeight = {crNewSize.X, BufferHeight};

        GetWindowRect(ghConWnd, &rcConPos);
        MoveWindow(ghConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
        lbRc = SetConsoleScreenBufferSize(ghConOut, crHeight); // � �� crNewSize - ��� "�������" �������
        //������ ���������� ������ �� ������!
        //RECT rcCurConPos = {0};
        //GetWindowRect(ghConWnd, &rcCurConPos); //X-Y �����, �� ������ - ������
        //MoveWindow(ghConWnd, rcCurConPos.left, rcCurConPos.top, GetSystemMetrics(SM_CXSCREEN), rcConPos.bottom-rcConPos.top, 1);

        rNewRect.Left = 0;
        rNewRect.Right = crHeight.X-1;
        rNewRect.Bottom = min( (crHeight.Y-1), (rNewRect.Top+gcrBufferSize.Y-1) );
		_ASSERTE(rNewRect.Bottom<200);
        SetConsoleWindowInfo(ghConOut, TRUE, &rNewRect);
    }

    //if (srv.hChangingSize) { // �� ����� ������� ConEmuC
    //    SetEvent(srv.hChangingSize);
    //}
    
    if (gnRunMode == RM_SERVER) {
	    srv.bForceFullSend = TRUE;
	    SetEvent(srv.hRefreshEvent);
    }

	cs.Leave();

    return lbRc;
}

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
    /*SafeCloseHandle(ghLogSize);
    if (wpszLogSizeFile) {
        DeleteFile(wpszLogSizeFile);
        Free(wpszLogSizeFile); wpszLogSizeFile = NULL;
    }*/

    return TRUE;
}

int GetProcessCount(DWORD **rpdwPID)
{
    //DWORD dwErr = 0; BOOL lbRc = FALSE;
    DWORD *pdwPID = NULL; int nCount = 0, i;
    EnterCriticalSection(&srv.csProc);
    nCount = srv.nProcesses.size();
    if (nCount > 0 && rpdwPID) {
        pdwPID = (DWORD*)Alloc(nCount, sizeof(DWORD));
        _ASSERTE(pdwPID!=NULL);
        if (pdwPID) {
            std::vector<DWORD>::iterator iter = srv.nProcesses.begin();
            i = 0;
            while (iter != srv.nProcesses.end()) {
                pdwPID[i++] = *iter;
                iter ++;
            }
        }
    }
    LeaveCriticalSection(&srv.csProc);
    if (rpdwPID)
        *rpdwPID = pdwPID;
    return nCount;
}

// ���� crNew ������� �� ������� rgn - ��������� ���
void EnlargeRegion(CESERVER_CHAR_HDR& rgn, const COORD crNew)
{
	//if (rgn.crStart.X == -1 && rgn.crStart.Y == -1 && rgn.crEnd.X == 0 && rgn.crEnd.Y == 0)
	if (rgn.nSize == 0)
	{   // �������, ��� ������ �� ���������
		rgn.nSize = sizeof(CESERVER_CHAR_HDR);
		rgn.cr1 = crNew; rgn.cr1 = crNew;
	} else
	
    // ���� ���������� ������� ��� ������ - ����������� ������
    if (crNew.X != rgn.cr1.X || crNew.Y != rgn.cr1.Y) {
        if ( (rgn.cr1.Y <= crNew.Y) && (crNew.Y <= rgn.cr2.Y) ) {
            // Y �������� � ��� ���������� ������. X - ������ �������������
        } else {
            if (crNew.Y < rgn.cr1.Y)
	            rgn.cr1.Y = crNew.Y;
	        else if (crNew.Y > rgn.cr2.Y)
		        rgn.cr2.Y = crNew.Y;
        }
        if (crNew.X < rgn.cr1.X)
            rgn.cr1.X = crNew.X;
        else if (crNew.X > rgn.cr2.X)
	        rgn.cr2.X = crNew.X;
    }
}

void CheckKeyboardLayout()
{
    if (pfnGetConsoleKeyboardLayoutName) {
        wchar_t szCurKeybLayout[KL_NAMELENGTH+1];
		// ���������� ������ � ���� "00000419"
        if (pfnGetConsoleKeyboardLayoutName(szCurKeybLayout)) {
	        if (lstrcmpW(szCurKeybLayout, srv.szKeybLayout)) {
		        // ��������
		        lstrcpyW(srv.szKeybLayout, szCurKeybLayout);
				// ������� � GUI
				wchar_t *pszEnd = szCurKeybLayout+8;
				DWORD dwLayout = wcstol(szCurKeybLayout, &pszEnd, 16);
				CESERVER_REQ* pIn = (CESERVER_REQ*)Alloc(sizeof(CESERVER_REQ_HDR)+4,1);
				if (pIn) {
					pIn->hdr.nSize = sizeof(CESERVER_REQ_HDR)+4;
					pIn->hdr.nCmd = CECMD_LANGCHANGE;
					pIn->hdr.nSrcThreadId = GetCurrentThreadId();
					pIn->hdr.nVersion = CESERVER_REQ_VER;
					memmove(pIn->Data, &dwLayout, 4);

					CESERVER_REQ* pOut = NULL;
					pOut = ExecuteGuiCmd(ghConWnd, pIn);
					if (pOut) ExecuteFreeResult(pOut);
					Free(pIn);
				}
	        }
        }
    }
}

// ��������� ������ ���� ������� � gpStoredOutput
void CmdOutputStore()
{
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
	memmove(&gpStoredOutput->hdr.sbi, &lsbi, sizeof(lsbi));
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

LPVOID Alloc(size_t nCount, size_t nSize)
{
	#ifdef _DEBUG
	//HeapValidate(ghHeap, 0, NULL);
	#endif

	size_t nWhole = nCount * nSize;
	LPVOID ptr = HeapAlloc ( ghHeap, HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY, nWhole );

	#ifdef HEAP_LOGGING
	wchar_t szDbg[64];
	wsprintfW(szDbg, L"%i: ALLOCATED   0x%08X..0x%08X   (%i bytes)\n", GetCurrentThreadId(), (DWORD)ptr, ((DWORD)ptr)+nWhole, nWhole);
	DEBUGSTR(szDbg);
	#endif

	#ifdef _DEBUG
	HeapValidate(ghHeap, 0, NULL);
	if (ptr) {
		gnHeapUsed += nWhole;
		if (gnHeapMax < gnHeapUsed)
			gnHeapMax = gnHeapUsed;
	}
	#endif

	return ptr;
}

void Free(LPVOID ptr)
{
	if (ptr && ghHeap) {
		#ifdef _DEBUG
		//HeapValidate(ghHeap, 0, NULL);
		size_t nMemSize = HeapSize(ghHeap, 0, ptr);
		#endif
		#ifdef HEAP_LOGGING
		wchar_t szDbg[64];
		wsprintfW(szDbg, L"%i: FREE BLOCK  0x%08X..0x%08X   (%i bytes)\n", GetCurrentThreadId(), (DWORD)ptr, ((DWORD)ptr)+nMemSize, nMemSize);
		DEBUGSTR(szDbg);
		#endif

		HeapFree ( ghHeap, 0, ptr );

		#ifdef _DEBUG
		HeapValidate(ghHeap, 0, NULL);
		if (gnHeapUsed > nMemSize)
			gnHeapUsed -= nMemSize;
		#endif
	}
}

#endif
