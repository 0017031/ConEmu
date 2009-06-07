#pragma once
#include <windows.h>

#include "usetodo.hpp"

//#define CONEMUPIPE      L"\\\\.\\pipe\\ConEmuPipe%u"
//#define CONEMUEVENTIN   L"ConEmuInEvent%u"
//#define CONEMUEVENTOUT  L"ConEmuOutEvent%u"
//#define CONEMUEVENTPIPE L"ConEmuPipeEvent%u"

#define MIN_CON_WIDTH 28
#define MIN_CON_HEIGHT 7
#define GUI_ATTACH_TIMEOUT 5000

// with line number
#if !defined(_MSC_VER)

    #define TODO(s)
    #define WARNING(s)
    #define PRAGMA_ERROR(s)

    #define CONSOLE_APPLICATION_16BIT 1
    
    typedef struct _CONSOLE_SELECTION_INFO {
        DWORD dwFlags;
        COORD dwSelectionAnchor;
        SMALL_RECT srSelection;
    } CONSOLE_SELECTION_INFO, *PCONSOLE_SELECTION_INFO;

    #ifndef max
    #define max(a,b)            (((a) > (b)) ? (a) : (b))
    #endif

    #ifndef min
    #define min(a,b)            (((a) < (b)) ? (a) : (b))
    #endif

    #define _ASSERT(f)
    #define _ASSERTE(f)
    
#else

    #define STRING2(x) #x
    #define STRING(x) STRING2(x)
    #define FILE_LINE __FILE__ "(" STRING(__LINE__) "): "
    #ifdef HIDE_TODO
    #define TODO(s) 
    #define WARNING(s) 
    #else
    #define TODO(s) __pragma(message (FILE_LINE "TODO: " s))
    #define WARNING(s) __pragma(message (FILE_LINE "warning: " s))
    #endif
    #define PRAGMA_ERROR(s) __pragma(message (FILE_LINE "error: " s))

    #ifdef _DEBUG
    #include <crtdbg.h>
    #endif

#endif

#define CES_NTVDM 0x10
#define CEC_INITTITLE       L"ConEmu"
//#define CE_CURSORUPDATE     L"ConEmuCursorUpdate%u" // ConEmuC_PID - ��������� ������ (������ ��� ���������). ��������� ������� ����������� GUI

#define CESERVERPIPENAME    L"\\\\%s\\pipe\\ConEmuSrv%u"      // ConEmuC_PID
#define CESERVERINPUTNAME   L"\\\\%s\\pipe\\ConEmuSrvInput%u" // ConEmuC_PID
#define CEGUIPIPENAME       L"\\\\%s\\pipe\\ConEmuGui%u"      // GetConsoleWindow() // ����������, ����� ������ ��� �������� � GUI
#define CEPLUGINPIPENAME    L"\\\\%s\\pipe\\ConEmuPlugin%u"   // Far_PID
#define CEGUIATTACHED       L"ConEmuGuiAttached.%u"
#define CESIGNAL_C          L"ConEmuC_C_Signal.%u"
#define CESIGNAL_BREAK      L"ConEmuC_Break_Signal.%u"
#define CECMD_GETSHORTINFO  1
#define CECMD_GETFULLINFO   2
#define CECMD_SETSIZE       3
#define CECMD_CMDSTARTSTOP  4 // 0 - ServerStart, 1 - ServerStop, 2 - ComspecStart, 3 - ComspecStop
#define CECMD_GETGUIHWND    5
#define CECMD_RECREATE      6
#define CECMD_TABSCHANGED   7
#define CECMD_CMDSTARTED    8 // == CECMD_SETSIZE + ������������ ���������� ������� (���������� comspec)
#define CECMD_CMDFINISHED   9 // == CECMD_SETSIZE + ��������� ���������� ������� (���������� comspec)
#define CECMD_GETOUTPUTFILE 10 // �������� ����� ��������� ���������� ��������� �� ��������� ���� � ������� ��� ���
#define CECMD_GETOUTPUT     11

#define CESERVER_REQ_VER    4

#define PIPEBUFSIZE 4096

#pragma pack(push, 1)


typedef struct tag_CESERVER_REQ_HDR {
	DWORD   nSize;
	DWORD   nCmd;
	DWORD   nVersion;
	DWORD   nSrcThreadId;
} CESERVER_REQ_HDR;


typedef struct tag_CESERVER_CHAR_HDR {
	int   nSize;    // ������ ��������� ������������. ���� 0 - ������ ������������� is NULL
	COORD cr1, cr2; // WARNING: ��� ���������� ���������� (��� ����� ���������), � �� ��������.
} CESERVER_CHAR_HDR;

typedef struct tag_CESERVER_CHAR {
	CESERVER_CHAR_HDR hdr; // ������������� �����
	WORD  data[2];  // variable(!) length
} CESERVER_CHAR;

typedef struct tag_CESERVER_CONSAVE_HDR {
	CESERVER_REQ_HDR hdr; // ����������� ������ ����� �������� � ������
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	DWORD cbMaxOneBufferSize;
} CESERVER_CONSAVE_HDR;

typedef struct tag_CESERVER_CONSAVE {
	CESERVER_CONSAVE_HDR hdr;
	wchar_t Data[1];
} CESERVER_CONSAVE;



typedef struct tag_CESERVER_REQ_RGNINFO {
	DWORD dwRgnInfoSize;
	CESERVER_CHAR RgnInfo;
} CESERVER_REQ_RGNINFO;

typedef struct tag_CESERVER_REQ_FULLCONDATA {
	DWORD dwRgnInfoSize_MustBe0; // must be 0
	DWORD dwOneBufferSize; // may be 0
	wchar_t Data[300]; // Variable length!!!
} CESERVER_REQ_FULLCONDATA;

typedef struct tag_CESERVER_REQ_CONINFO {
	/* 1*/HWND hConWnd;
	/* 2*/DWORD nPacketId;
	/* 3*/DWORD nProcessCount; // Will be 0 in current version
	// Next Fields are valid ONLY if (nProcessCount == 0)
	/* 4*/DWORD dwSelSize; // To kill
	      CONSOLE_SELECTION_INFO si;
    /* 5*/DWORD dwCiSize;
	      CONSOLE_CURSOR_INFO ci;
    /* 6*/DWORD dwConsoleCP;
	/* 7*/DWORD dwConsoleOutputCP;
	/* 8*/DWORD dwConsoleMode;
	/* 9*/DWORD dwSbiSize;
	      CONSOLE_SCREEN_BUFFER_INFO sbi;
    union {
	/*10*/DWORD dwRgnInfoSize;
	      CESERVER_REQ_RGNINFO RgnInfo;
    /*11*/CESERVER_REQ_FULLCONDATA FullData;
	};
} CESERVER_REQ_CONINFO;

typedef struct tag_CESERVER_REQ_SETSIZE {
	USHORT nBufferHeight; // 0 ��� ������ ������ (����� � ����������)
	COORD  size;
	SHORT  nSendTopLine;  // -1 ��� 0based ����� ������ ��������������� � GUI (������ ��� ������ � ����������)
	SMALL_RECT rcWindow;  // ���������� ������� ������� ��� ������ � ����������
} CESERVER_REQ_SETSIZE;

typedef struct tag_CESERVER_REQ_OUTPUTFILE {
	BOOL  bUnicode;
	WCHAR szFilePathName[MAX_PATH+1];
} CESERVER_REQ_OUTPUTFILE;

typedef struct tag_CESERVER_REQ {
    CESERVER_REQ_HDR hdr;
	union {
		BYTE    Data[1]; // variable(!) length
		CESERVER_REQ_CONINFO ConInfo; // Informational only! Some fields ARE VARIABLE LENGTH
		CESERVER_REQ_SETSIZE SetSize;
		CONSOLE_SCREEN_BUFFER_INFO SetSizeRet;
		CESERVER_REQ_OUTPUTFILE OutputFile;
	};
} CESERVER_REQ;


#pragma pack(pop)

#define CONEMUMSG_ATTACH L"ConEmuMain::Attach"        // wParam == hConWnd, lParam == ConEmuC_PID
//#define CONEMUCMDSTARTED L"ConEmuMain::CmdStarted"    // wParam == hConWnd, lParam == ConEmuC_PID (as ComSpec)
//#define CONEMUCMDSTOPPED L"ConEmuMain::CmdTerminated" // wParam == hConWnd, lParam == ConEmuC_PID (as ComSpec)

//#define CONEMUMAPPING    L"ConEmuPluginData%u"
//#define CONEMUDRAGFROM   L"ConEmuDragFrom%u"
//#define CONEMUDRAGTO     L"ConEmuDragTo%u"
//#define CONEMUREQTABS    L"ConEmuReqTabs%u"
//#define CONEMUSETWINDOW  L"ConEmuSetWindow%u"
//#define CONEMUPOSTMACRO  L"ConEmuPostMacro%u"
//#define CONEMUDEFFONT    L"ConEmuDefFont%u"
//#define CONEMULANGCHANGE L"ConEmuLangChange%u"
//#define CONEMUEXIT       L"ConEmuExit%u"
//#define CONEMUALIVE      L"ConEmuAlive%u"
//#define CONEMUREADY      L"ConEmuReady%u"
#define CONEMUTABCHANGED L"ConEmuTabsChanged"
#define CMD_DRAGFROM     0
#define CMD_DRAGTO       1
#define CMD_REQTABS      2
#define CMD_SETWINDOW    3
#define CMD_POSTMACRO    4
#define CMD_DEFFONT      5
#define CMD_LANGCHANGE   6
// +2
#define MAXCMDCOUNT      8
#define CMD_EXIT         MAXCMDCOUNT-1

//#define GWL_TABINDEX     0
//#define GWL_LANGCHANGE   4

#ifdef _DEBUG
    #define CONEMUALIVETIMEOUT INFINITE
    #define CONEMUREADYTIMEOUT INFINITE
    #define CONEMUFARTIMEOUT   120000 // ������� �������, ���� ��� ���������� �� ����� �������
#else
    #define CONEMUALIVETIMEOUT 1000  // ������� ������� ���� �������
    #define CONEMUREADYTIMEOUT 10000 // � �� ���������� ������� - 10s max
    #define CONEMUFARTIMEOUT   10000 // ������� �������, ���� ��� ���������� �� ����� �������
#endif

#define CONEMUTABMAX 0x400
struct ConEmuTab
{
    int  Pos;
    int  Current;
    int  Type;
    int  Modified;
    wchar_t Name[CONEMUTABMAX];
//  int  Modified;
//  int isEditor;
};

struct ForwardedPanelInfo
{
    RECT ActiveRect;
    RECT PassiveRect;
    int ActivePathShift; // ����� � ���� ��������� � ������
    int PassivePathShift; // ����� � ���� ��������� � ������
    WCHAR* pszActivePath/*[MAX_PATH+1]*/;
    WCHAR* pszPassivePath/*[MAX_PATH+1]*/;
};

struct FarVersion {
    union {
        DWORD dwVer;
        struct {
            WORD dwVerMinor;
            WORD dwVerMajor;
        };
    };
    DWORD dwBuild;
};

struct ForwardedFileInfo
{
    WCHAR Path[MAX_PATH+1];
};

/*enum PipeCmd
{
    SetTabs=0,
    DragFrom,
    DragTo
};*/

// ConEmu.dll ������������ ��������� �������
//HWND WINAPI GetFarHWND();
//void WINAPI _export GetFarVersion ( FarVersion* pfv );

//------------------------------------------------------------------------
///| Section |////////////////////////////////////////////////////////////
//------------------------------------------------------------------------
#ifdef __cplusplus
class CSection
{
protected:
	CRITICAL_SECTION* mp_cs;
	DWORD* mp_TID;
public:
	void Leave()
	{
		if (mp_cs) {
			*mp_TID = 0;
			mp_TID = NULL;
			//OutputDebugString(_T("LeaveCriticalSection\n"));
			LeaveCriticalSection(mp_cs);
			#ifdef _DEBUG
			#ifndef CSECTION_NON_RAISE
			_ASSERTE(mp_cs->LockCount==-1);
			#endif
			#endif
			mp_cs = NULL;
		}
	}
	void Enter(CRITICAL_SECTION* pcs, DWORD* pTID)
	{
		Leave(); // ���� ����

		mp_TID = pTID;
		DWORD dwTID = GetCurrentThreadId();
		if (dwTID == *pTID)
			return; // � ���� ���� ��� �������������

		mp_cs = pcs;
		if (mp_cs) {
			//OutputDebugString(_T("TryEnterCriticalSection\n"));
			#ifdef _DEBUG
			DWORD dwTryLockSectionStart = GetTickCount(), dwCurrentTick;
			#endif
			if (!TryEnterCriticalSection(mp_cs)) {
				Sleep(50);
				while (!TryEnterCriticalSection(mp_cs)) {
					Sleep(50);
					OutputDebugString(L"TryEnterCriticalSection failed!!!\n");
					#ifdef _DEBUG
					dwCurrentTick = GetTickCount();
					if ((dwCurrentTick - dwTryLockSectionStart) > 3000) {
						#ifndef CSECTION_NON_RAISE
						_ASSERTE((dwCurrentTick - dwTryLockSectionStart) <= 3000);
						#endif
						dwTryLockSectionStart = GetTickCount();
					}
					#endif
				}
			}
			//EnterCriticalSection(mp_cs);
			*mp_TID = dwTID;
		}
	}
	bool isLocked()
	{
		if (mp_cs)
			return true;
		// ��� ���� ������������ �� ������ ���� � ���� �� �������
		if (mp_TID) {
			DWORD dwTID = GetCurrentThreadId();
			if (*mp_TID == dwTID)
				return true;
		}
		return false;
	}
	CSection (CRITICAL_SECTION* pcs, DWORD* pTID) : mp_cs(NULL), mp_TID(NULL)
	{
		if (pcs) Enter(pcs, pTID);
	}
	~CSection()
	{
		Leave();
	}
};
#endif
