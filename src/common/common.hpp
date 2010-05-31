
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


#ifndef _COMMON_HEADER_HPP_
#define _COMMON_HEADER_HPP_

#include <windows.h>
#include <wchar.h>
#if !defined(__GNUC__)
#include <crtdbg.h>
#else
#define _ASSERTE(f)
#endif

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

    //#define CONSOLE_APPLICATION_16BIT 1
    
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

#ifdef _WIN64
	#ifndef WIN64
		WARNING("WIN64 was not defined");
		#define WIN64
	#endif
#endif

#ifdef _DEBUG
	#define USE_SEH
#endif

#ifdef USE_SEH
	#if defined(_MSC_VER)
		#pragma message ("Compiling USING exception handler")
	#endif
	
	#define SAFETRY   __try
	#define SAFECATCH __except(EXCEPTION_EXECUTE_HANDLER)
#else
	#if defined(_MSC_VER)
		#pragma message ("Compiling NOT using exception handler")
	#endif

	#define SAFETRY   if (true)
	#define SAFECATCH else
#endif	


#define isPressed(inp) ((GetKeyState(inp) & 0x8000) == 0x8000)
#define countof(a) (sizeof((a))/(sizeof(*(a))))
#define ZeroStruct(s) memset(&(s), 0, sizeof(s))

#ifdef _DEBUG
extern wchar_t gszDbgModLabel[6];
#define CHEKCDBGMODLABEL if (gszDbgModLabel[0]==0) { \
	wchar_t szFile[MAX_PATH]; GetModuleFileName(NULL, szFile, MAX_PATH); \
	wchar_t* pszName = wcsrchr(szFile, L'\\'); \
	if (_wcsicmp(pszName, L"\\conemu.exe")==0) lstrcpyW(gszDbgModLabel, L"gui"); \
	else if (_wcsicmp(pszName, L"\\conemuc.exe")==0) lstrcpyW(gszDbgModLabel, L"srv"); \
	else lstrcpyW(gszDbgModLabel, L"dll"); \
}
#ifdef SHOWDEBUGSTR
	#define DEBUGSTR(s) { MCHKHEAP; CHEKCDBGMODLABEL; SYSTEMTIME st; GetLocalTime(&st); wchar_t szDEBUGSTRTime[40]; wsprintf(szDEBUGSTRTime, L"%i:%02i:%02i.%03i(%s.%i) ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, gszDbgModLabel, GetCurrentThreadId()); OutputDebugString(szDEBUGSTRTime); OutputDebugString(s); }
#else
	#ifndef DEBUGSTR
		#define DEBUGSTR(s)
	#endif
#endif
#else
	#ifndef DEBUGSTR
		#define DEBUGSTR(s)
	#endif
#endif


#define CES_NTVDM 0x10
#define CEC_INITTITLE       L"ConEmu"
//#define CE_CURSORUPDATE     L"ConEmuCursorUpdate%u" // ConEmuC_PID - ��������� ������ (������ ��� ���������). ��������� ������� ����������� GUI

#define CESERVERPIPENAME    L"\\\\%s\\pipe\\ConEmuSrv%u"      // ConEmuC_PID
#define CESERVERINPUTNAME   L"\\\\%s\\pipe\\ConEmuSrvInput%u" // ConEmuC_PID
#define CESERVERQUERYNAME   L"\\\\%s\\pipe\\ConEmuSrvQuery%u" // ConEmuC_PID
#define CESERVERWRITENAME   L"\\\\%s\\pipe\\ConEmuSrvWrite%u" // ConEmuC_PID
#define CESERVERREADNAME    L"\\\\%s\\pipe\\ConEmuSrvRead%u"  // ConEmuC_PID
#define CEGUIPIPENAME       L"\\\\%s\\pipe\\ConEmuGui%u"      // GetConsoleWindow() // ����������, ����� ������ ��� �������� � GUI
#define CEPLUGINPIPENAME    L"\\\\%s\\pipe\\ConEmuPlugin%u"   // Far_PID
//
//#define MAXCONMAPCELLS      (600*400)
#define CECONMAPNAME        L"ConEmuFileMapping.%08X"
#define CECONMAPNAME_A      "ConEmuFileMapping.%08X"
#define CEFARMAPNAME        L"ConEmuFarMapping.%08X"
#define CEDATAREADYEVENT    L"ConEmuSrvDataReady.%u"
#define CECONVIEWSETNAME    L"ConEmuViewSetMapping.%u"
#define CEFARALIVEEVENT     L"ConEmuFarAliveEvent.%u"
//#define CECONMAPNAMESIZE    (sizeof(CESERVER_REQ_CONINFO)+(MAXCONMAPCELLS*sizeof(CHAR_INFO)))
//#define CEGUIATTACHED       L"ConEmuGuiAttached.%u"
#define CEGUIRCONSTARTED    L"ConEmuGuiRConStarted.%u"
#define CEGUI_ALIVE_EVENT   L"ConEmuGuiStarted"
#define CEKEYEVENT_CTRL     L"ConEmuCtrlPressed.%u"
#define CEKEYEVENT_SHIFT    L"ConEmuShiftPressed.%u"


//#define CONEMUMSG_ATTACH L"ConEmuMain::Attach"            // wParam == hConWnd, lParam == ConEmuC_PID
//WARNING("CONEMUMSG_SRVSTARTED ����� ���������� � ������� ����� ��� GUI");
//#define CONEMUMSG_SRVSTARTED L"ConEmuMain::SrvStarted"    // wParam == hConWnd, lParam == ConEmuC_PID
//#define CONEMUMSG_SETFOREGROUND L"ConEmuMain::SetForeground"            // wParam == hConWnd, lParam == ConEmuC_PID
#define CONEMUMSG_FLASHWINDOW L"ConEmuMain::FlashWindow"
//#define CONEMUCMDSTARTED L"ConEmuMain::CmdStarted"    // wParam == hConWnd, lParam == ConEmuC_PID (as ComSpec)
//#define CONEMUCMDSTOPPED L"ConEmuMain::CmdTerminated" // wParam == hConWnd, lParam == ConEmuC_PID (as ComSpec)
#define CONEMUMSG_LLKEYHOOK L"ConEmuMain::LLKeyHook"    // wParam == hConWnd, lParam == ConEmuC_PID
#define CONEMUMSG_PNLVIEWFADE L"ConEmuTh::Fade"
#define CONEMUMSG_PNLVIEWSETTINGS L"ConEmuTh::Settings"

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


//#define CESIGNAL_C          L"ConEmuC_C_Signal.%u"
//#define CESIGNAL_BREAK      L"ConEmuC_Break_Signal.%u"
//#define CECMD_CONSOLEINFO   1
#define CECMD_CONSOLEDATA   2
//#define CECMD_SETSIZE       3
#define CECMD_CMDSTARTSTOP  4 // 0 - ServerStart, 1 - ServerStop, 2 - ComspecStart, 3 - ComspecStop
#define CECMD_GETGUIHWND    5
//#define CECMD_RECREATE      6
#define CECMD_TABSCHANGED   7
#define CECMD_CMDSTARTED    8 // == CECMD_SETSIZE + ������������ ���������� ������� (���������� comspec)
#define CECMD_CMDFINISHED   9 // == CECMD_SETSIZE + ��������� ���������� ������� (���������� comspec)
#define CECMD_GETOUTPUTFILE 10 // �������� ����� ��������� ���������� ��������� �� ��������� ���� � ������� ��� ���
#define CECMD_GETOUTPUT     11
#define CECMD_LANGCHANGE    12
#define CECMD_NEWCMD        13 // ��������� � ���� ���������� ����� ������� � ���������� �������� (������������ ��� SingleInstance)
#define CECMD_TABSCMD       14 // 0: ��������/�������� ����, 1: ������� �� ���������, 2: ������� �� ����������, 3: commit switch
#define CECMD_RESOURCES     15 // ���������� �������� ��� ������������� (��������� ��������)
#define CECMD_GETNEWCONPARM 16 // ���.��������� ��� �������� ����� ������� (�����, ������,...)
#define CECMD_SETSIZESYNC   17 // ���������, ���� (�� �������), ���� FAR ���������� ��������� ������� (�� ���� ����������)
#define CECMD_ATTACH2GUI    18 // ��������� ����������� ������� (�����������) ������� � GUI. ��� ����������
#define CECMD_FARLOADED     19 // ���������� �������� � ������
#define CECMD_SHOWCONSOLE   20 // � Win7 ������ ������ �������� ���� �������, ���������� � ������ ��������������
#define CECMD_POSTCONMSG    21 // � Win7 ������ ������ �������� ��������� ���� �������, ���������� � ������ ��������������
//#define CECMD_REQUESTCONSOLEINFO 22 // ���� CECMD_REQUESTFULLINFO
#define CECMD_SETFOREGROUND 23
#define CECMD_FLASHWINDOW   24
#define CECMD_SETCONSOLECP  25
#define CECMD_SAVEALIASES   26
#define CECMD_GETALIASES    27
#define CECMD_SETSIZENOSYNC 28 // ����� CECMD_SETSIZE. ���������� �� �������.
#define CECMD_SETDONTCLOSE  29
#define CECMD_REGPANELVIEW  30
#define CECMD_ONACTIVATION  31 // ��� ��������� ������ ConsoleInfo->bConsoleActive
#define CECMD_SETWINDOWPOS  32 // CESERVER_REQ_SETWINDOWPOS.
#define CECMD_SETWINDOWRGN  33 // CESERVER_REQ_SETWINDOWRGN.

// ������ ����������
#define CESERVER_REQ_VER    43

#define PIPEBUFSIZE 4096
#define DATAPIPEBUFSIZE 40000


// ������� FAR �������
#define CMD_DRAGFROM     0
#define CMD_DRAGTO       1
#define CMD_REQTABS      2
#define CMD_SETWINDOW    3
#define CMD_POSTMACRO    4 // ���� ������ ������ ������� '@' � ����� ���� �� ������ - ������ ����������� � DisabledOutput
//#define CMD_DEFFONT    5
#define CMD_LANGCHANGE   6
#define CMD_SETENVVAR    7 // ���������� ���������� ��������� (FAR plugin)
//#define CMD_SETSIZE    8
#define CMD_EMENU        9
#define CMD_LEFTCLKSYNC  10
#define CMD_REDRAWFAR    11
#define CMD_FARPOST      12
#define CMD_CHKRESOURCES 13
#define CMD_QUITFAR      14 // ������� ���������� ������� (����?)
// +2
#define MAXCMDCOUNT      16
#define CMD_EXIT         MAXCMDCOUNT-1



//#pragma pack(push, 1)
#include <pshpack1.h>

#define u64 unsigned __int64
typedef struct tag_HWND2 {
	DWORD u;
	operator HWND() const {
		return (HWND)u;
	};
	operator DWORD() const {
		return (DWORD)u;
	};
	struct tag_HWND2& operator=(HWND h) {
		u = (DWORD)h;
		return *this;
	};
} HWND2;


typedef struct tag_ThumbColor {
	union {
		struct {
			unsigned int   ColorRGB : 24;
			unsigned int   ColorIdx : 5; // 5 bits, to support value '16' - automatic use of panel color
			unsigned int   UseIndex : 1; // TRUE - Use ColorRef, FALSE - ColorIdx
		};
		DWORD RawColor;
	};
} ThumbColor;

typedef struct tag_ThumbSizes {
	// ������� ��������� ��� ������
	int nImgSize; // Thumbs: 96, Tiles: 48
	// ����� ���������/������ ������&���� ������������ ������� ���� �����
	int nSpaceX1, nSpaceY1; // Thumbs: 1x1, Tiles: 4x4
	// ������ "�������" ������&���� ����� ���������/������. ����� �������� �����.
	int nSpaceX2, nSpaceY2; // Thumbs: 5x25, Tiles: 172x4
	// ���������� ����� ����������/������� � �������
	int nLabelSpacing; // Thumbs: 0, Tiles: 4
	// ������ ������ �� ����� �������������� �����
	int nLabelPadding; // Thumbs: 0, Tiles: 1
	// �����
	wchar_t sFontName[36]; // Tahoma
	int nFontHeight; // 14
} ThumbSizes;


typedef enum {
	pvm_None = 0,
	pvm_Thumbnails = 1,
	pvm_Tiles = 2,
	// ��������� ����� (���� �� �����) ������ 4! (��� bitmask)
} PanelViewMode;

typedef struct tag_PanelViewSettings {
	DWORD cbSize; // Struct size, �� ������ ������

	/* ����� � ����� */
	ThumbColor crBackground; // ��� ���������: RGB ��� Index
	
	int nPreviewFrame; // 1 (����� ����� ������ ���������
	ThumbColor crPreviewFrame; // RGB ��� Index
	
	int nSelectFrame; // 1 (����� ������ �������� ��������)
	ThumbColor crSelectFrame; // RGB ��� Index

	/* ������������ */
	DWORD nFontQuality;

	/* ������ ������������� ������� */
	ThumbSizes Thumbs;
	ThumbSizes Tiles;

    /* �������� ����� � GUI */
    struct {
    	wchar_t sFontName[32];
    	DWORD nFontHeight, nFontWidth, nFontCellWidth;
    	DWORD nFontQuality, nFontCharSet;
    	BOOL Bold, Italic;
    	wchar_t sBorderFontName[32];
        DWORD nBorderFontWidth;
    } MainFont;
	
	// ������ ��������� ��������
	BYTE  bLoadPreviews; // bitmask of PanelViewMode {1=Thumbs, 2=Tiles}
	bool  bLoadFolders;  // true - load infolder previews (only for Thumbs)
	DWORD nLoadTimeout;  // 15 sec

	DWORD nMaxZoom; // 600%
	bool  bUsePicView2; // true
	bool  bRestoreOnStartup;


	// ����� ������ ����� �����!	
	COLORREF crPalette[16], crFadePalette[16];
	

	//// ���� �� ������������
	//DWORD nCacheFolderType; // ����/���������/temp/� �.�.
	//wchar_t sCacheFolder[MAX_PATH];
} PanelViewSettings;


typedef BOOL (WINAPI* PanelViewInputCallback)(HANDLE hInput, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead, BOOL* pbResult);
typedef BOOL (WINAPI* PanelViewOutputCallback)(HANDLE hOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
typedef struct tag_PanelViewText {
	// ���� ��������������, ������������ ������, � ��.
	#define PVI_TEXT_NOTUSED 0
	#define PVI_TEXT_LEFT    1
	#define PVI_TEXT_CENTER  2
	#define PVI_TEXT_RIGHT   4
	#define PVI_TEXT_SKIPSORTMODE 0x100 // ������ ��� ����� ������� (�������� ��������� ������� ���������� � ^)
	DWORD nFlags;
	// ������������ ������ ������� ���� - ������� ���������� ������
	DWORD bConAttr;
	// ���������� �����
	wchar_t sText[128];
} PanelViewText;
typedef struct tag_PanelViewInit {
	DWORD cbSize;
	BOOL  bRegister, bVisible;
	HWND2 hWnd;
	BOOL  bLeftPanel;
	// Flags
	#define PVI_COVER_NORMAL         0x000 // ��� ������, ����������� ���������������, � ������� ��� ������� �����
	#define PVI_COVER_SCROLL_CLEAR   0x001 // ��� ��������� GUI ������� ������ ��������� (������� �� ������������-�������)
	#define PVI_COVER_SCROLL_OVER    0x002 // PVI_COVER_NORMAL + ������ ����� (��� ������ ���������). ������ ������ ���������� ��������� ���
	#define PVI_COVER_COLTITLE_OVER  0x004 // PVI_COVER_NORMAL + ������ � ����������� ������� (���� ��� ����)
	#define PVI_COVER_FULL_OVER      0x008 // ��� ������� ������� ������ (�� ���� ������ - �����)
	#define PVI_COVER_FRAME_OVER     0x010 // ��� ������� ������ (������� �����)
	#define PVI_COVER_2PANEL_OVER    0x020 // ��� ������ (�������������)
	#define PVI_COVER_CONSOLE_OVER   0x040 // ������� �������
	DWORD nCoverFlags;
	// FAR settings
	DWORD nFarInterfaceSettings;
	DWORD nFarPanelSettings;
	// ���������� ���� ������ (�����, ������, ��� fullscreen)
	RECT  PanelRect;
	// ������������� ��������� ���������
	PanelViewText tPanelTitle, tColumnTitle, tInfoLine[3];
	// ��� ���������� ��������������, � ������� ������� ������������� View
	// ���������� ������������ � GUI. ������� - ����������.
	RECT  WorkRect;
	// Callbacks, ������������ ������ � ��������
	PanelViewInputCallback pfnPeekPreCall, pfnPeekPostCall, pfnReadPreCall, pfnReadPostCall;
	PanelViewOutputCallback pfnWriteCall;
	/* out */
	//PanelViewSettings ThSet;
	BOOL bFadeColors;
} PanelViewInit;



TODO("Restrict CONEMUTABMAX to 128 chars. Only filename, and may be ellipsed...");
#define CONEMUTABMAX 0x400
typedef struct tag_ConEmuTab {
	int  Pos;
	int  Current;
	int  Type; // (Panels=1, Viewer=2, Editor=3) | (Elevated=0x100)
	int  Modified;
	wchar_t Name[CONEMUTABMAX];
	//  int  Modified;
	//  int isEditor;
} ConEmuTab;

typedef struct tag_CESERVER_REQ_CONEMUTAB {
	DWORD nTabCount;
	BOOL  bMacroActive;
	BOOL  bMainThread;
	ConEmuTab tabs[1];
} CESERVER_REQ_CONEMUTAB;

typedef struct tag_CESERVER_REQ_CONEMUTAB_RET {
	BOOL  bNeedPostTabSend;
	BOOL  bNeedResize;
	COORD crNewSize;
} CESERVER_REQ_CONEMUTAB_RET;

typedef struct tag_ForwardedPanelInfo {
	RECT ActiveRect;
	RECT PassiveRect;
	int ActivePathShift; // ����� � ���� ��������� � ������
	int PassivePathShift; // ����� � ���� ��������� � ������
	union { //x64 ready
		WCHAR* pszActivePath/*[MAX_PATH+1]*/;
		u64 Reserved1;
	};
	union { //x64 ready
		WCHAR* pszPassivePath/*[MAX_PATH+1]*/;
		u64 Reserved2;
	};
} ForwardedPanelInfo;

typedef struct tag_FarVersion {
	union {
		DWORD dwVer;
		struct {
			WORD dwVerMinor;
			WORD dwVerMajor;
		};
	};
	DWORD dwBuild;
} FarVersion;

typedef struct tag_ForwardedFileInfo {
	WCHAR Path[MAX_PATH+1];
} ForwardedFileInfo;


typedef struct tag_CESERVER_REQ_HDR {
	DWORD   cbSize;
	DWORD   nCmd;
	DWORD   nVersion;
	DWORD   nSrcThreadId;
	DWORD   nSrcPID;
	DWORD   nCreateTick;
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

typedef struct tag_CEFAR_SHORT_PANEL_INFO {
	int   PanelType;
	int   Plugin;
	RECT  PanelRect;
	int   ItemsNumber;
	int   SelectedItemsNumber;
	int   CurrentItem;
	int   TopPanelItem;
	int   Visible;
	int   Focus;
	int   ViewMode;
	int   ShortNames;
	int   SortMode;
	DWORD Flags;
} CEFAR_SHORT_PANEL_INFO;

typedef struct tag_CEFAR_INFO {
	DWORD cbSize;
	DWORD nFarInfoIdx;
	FarVersion FarVer;
	DWORD nProtocolVersion; // == CESERVER_REQ_VER
	DWORD nFarPID, nFarTID;
	BYTE nFarColors[0x100]; // ������ ������ ����
	DWORD nFarInterfaceSettings;
	DWORD nFarPanelSettings;
	DWORD nFarConfirmationSettings;
	BOOL  bFarPanelAllowed; // FCTL_CHECKPANELSEXIST
	// ���������� ���������� � ������� �������������� �� �������
	BOOL bFarPanelInfoFilled;
	BOOL bFarLeftPanel, bFarRightPanel;   
	CEFAR_SHORT_PANEL_INFO FarLeftPanel, FarRightPanel; // FCTL_GETPANELSHORTINFO,...
	DWORD nFarConsoleMode;
	BOOL bBufferSupport; // FAR2 � ������ /w ?
	//DWORD nFarReadIdx;    // index, +1, ����� ��� � ��������� ��� ������ (Read|Peek)ConsoleInput ��� GetConsoleInputCount
	// ����� ���� ��������� �������, �� ������� � ��������� ������� ������������� GUI
	wchar_t sLngEdit[64]; // "edit"
	wchar_t sLngView[64]; // "view"
	wchar_t sLngTemp[64]; // "{Temporary panel"
	//wchar_t sLngName[64]; // "Name"
	wchar_t sReserved[MAX_PATH]; // ����� �������� GetMsg �� ����������� ��������� �� �����
} CEFAR_INFO;


typedef struct tag_CESERVER_REQ_CONINFO_HDR {
	DWORD cbSize;
	DWORD nLogLevel;
	COORD crMaxConSize;
	DWORD bDataReady;  // TRUE, ����� ���� ��� ������ ������� ���������� � ����� �������� ������
	HWND2 hConWnd;     // !! ��������� hConWnd � nGuiPID �� ������ !!
	DWORD nServerPID;  //
	DWORD nGuiPID;     // !! �� ��� ������������� PicViewWrapper   !!
	//
	DWORD bConsoleActive;
	DWORD nProtocolVersion; // == CESERVER_REQ_VER
	//
	DWORD nFarPID; // PID ���������� ��������� ����
} CESERVER_REQ_CONINFO_HDR;

typedef struct tag_CESERVER_REQ_CONINFO_INFO {
	CESERVER_REQ_HDR cmd;
	//DWORD cbSize;
	HWND2 hConWnd;
	//DWORD nGuiPID;
	//DWORD nCurDataMapIdx; // ������� ��� �������� MAP ����� � �������
	//DWORD nCurDataMaxSize; // ������������ ������ ������ nCurDataMapIdx
	DWORD nPacketId;
	//DWORD nFarReadTick; // GetTickCount(), ����� ��� � ��������� ��� �������� ������� �� �������
	//DWORD nFarUpdateTick0;// GetTickCount(), ��������������� � ������ ���������� ������� �� ���� (����� ��� ��������...)
	//DWORD nFarUpdateTick; // GetTickCount(), ����� ������� ���� ��������� � ��������� ��� �� ����
	//DWORD nFarReadIdx;    // index, +1, ����� ��� � ��������� ��� ������ (Read|Peek)ConsoleInput ��� GetConsoleInputCount
	DWORD nSrvUpdateTick; // GetTickCount(), ����� ������� ���� ������� � ��������� ��� � �������
	DWORD nReserved0; //DWORD nInputTID;
	DWORD nProcesses[20];
    DWORD dwCiSize;
	CONSOLE_CURSOR_INFO ci;
    DWORD dwConsoleCP;
	DWORD dwConsoleOutputCP;
	DWORD dwConsoleMode;
	DWORD dwSbiSize;
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	COORD crWindow; // ��� �������� - ������ ���� (�� ������) ��� ��������� ReadConsoleData
	COORD crMaxSize; // ������������ ������ ������� � �������� (��� �������� ���������� ������)
	DWORD nDataShift; // ��� �������� - ����� ������ ������ (data) �� ������ info
	DWORD nDataCount; // ��� �������� - ���������� ����� (data)
	//// ���������� � ������� FAR
	//DWORD nFarInfoIdx; // ������� �� ��������� CEFAR_INFO, �.�. �� ����� �������� � �������
	//CEFAR_INFO FarInfo;
} CESERVER_REQ_CONINFO_INFO;

//typedef struct tag_CESERVER_REQ_CONINFO_DATA {
//	CESERVER_REQ_HDR cmd;
//	COORD      crMaxSize;
//	CHAR_INFO  Buf[1];
//} CESERVER_REQ_CONINFO_DATA;

typedef struct tag_CESERVER_REQ_CONINFO_FULL {
	DWORD cbMaxSize;    // ������ ����� ������ CESERVER_REQ_CONINFO_FULL (������ ����� ����� ������ �������� ������)
	//DWORD cbActiveSize; // ������ �������� ������ CESERVER_REQ_CONINFO_FULL, � �� ����� ������ data
	//BOOL  bChanged;     // ���� ����, ��� ������ ���������� � ��������� �������� � GUI
	BOOL  bDataChanged; // ������������ � TRUE, ��� ���������� ����������� ������� (� �� ������ ��������� �������...)
	CESERVER_REQ_CONINFO_HDR  hdr;
	CESERVER_REQ_CONINFO_INFO info;
	//CESERVER_REQ_CONINFO_DATA data;
	CHAR_INFO  data[1];
} CESERVER_REQ_CONINFO_FULL;

//typedef struct tag_CESERVER_REQ_CONINFO {
//	CESERVER_REQ_CONINFO_HDR inf;
//    union {
//	/* 9*/DWORD dwRgnInfoSize;
//	      CESERVER_REQ_RGNINFO RgnInfo;
//    /*10*/CESERVER_REQ_FULLCONDATA FullData;
//	};
//} CESERVER_REQ_CONINFO;

typedef struct tag_CESERVER_REQ_SETSIZE {
	USHORT nBufferHeight; // 0 ��� ������ ������ (����� � ����������)
	COORD  size;
	SHORT  nSendTopLine;  // -1 ��� 0based ����� ������ ��������������� � GUI (������ ��� ������ � ����������)
	SMALL_RECT rcWindow;  // ���������� ������� ������� ��� ������ � ����������
	DWORD  dwFarPID;      // ���� �������� - ������ ������ ��� ����������� �� FAR'� � �������� ��� ������ ����� ������ ����� ���������
} CESERVER_REQ_SETSIZE;

typedef struct tag_CESERVER_REQ_OUTPUTFILE {
	BOOL  bUnicode;
	WCHAR szFilePathName[MAX_PATH+1];
} CESERVER_REQ_OUTPUTFILE;

typedef struct tag_CESERVER_REQ_RETSIZE {
	DWORD nNextPacketId;
	CONSOLE_SCREEN_BUFFER_INFO SetSizeRet;
} CESERVER_REQ_RETSIZE;

typedef struct tag_CESERVER_REQ_NEWCMD {
	wchar_t szCurDir[MAX_PATH];
	wchar_t szCommand[MAX_PATH]; // �� ����� ���� - variable_size !!!
} CESERVER_REQ_NEWCMD;

typedef struct tag_CESERVER_REQ_STARTSTOP {
	DWORD nStarted; // 0 - ServerStart, 1 - ServerStop, 2 - ComspecStart, 3 - ComspecStop
	HWND2 hWnd; // ��� �������� � GUI - �������, ��� �������� � ������� - GUI
	DWORD dwPID; //, dwInputTID;
	DWORD nSubSystem; // 255 ��� DOS ��������, 0x100 - ����� �� FAR �������
	BOOL  bRootIsCmdExe;
	BOOL  bUserIsAdmin;
	// � ��� �������� �� �������, ������ ���������� ��������� ������ �������� ������ ������
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	// Reserved
	DWORD nReserved0, nReserved1;
} CESERVER_REQ_STARTSTOP;

// _ASSERTE(sizeof(CESERVER_REQ_STARTSTOPRET) <= sizeof(CESERVER_REQ_STARTSTOP));
typedef struct tag_CESERVER_REQ_STARTSTOPRET {
	BOOL  bWasBufferHeight;
	HWND2 hWnd; // ��� �������� � ������� - GUI (������� ����)
	HWND2 hWndDC;
	DWORD dwPID; // ��� �������� � ������� - PID ConEmu.exe
	DWORD nBufferHeight, nWidth, nHeight;
	DWORD dwSrvPID;
	BOOL  bNeedLangChange;
	u64   NewConsoleLang;
} CESERVER_REQ_STARTSTOPRET;

typedef struct tag_CESERVER_REQ_POSTMSG {
	BOOL    bPost;
	HWND2   hWnd;
	UINT    nMsg;
	// ��������� �� ���������� x86 & x64
	u64     wParam, lParam;
} CESERVER_REQ_POSTMSG;

typedef struct tag_CESERVER_REQ_FLASHWINFO {
	BOOL  bSimple;
	HWND2 hWnd;
	BOOL  bInvert; // ������ ���� bSimple == TRUE
	DWORD dwFlags; // � ��� � �����, ���� bSimple == FALSE
	UINT  uCount;
	DWORD dwTimeout;
} CESERVER_REQ_FLASHWINFO;

// CMD_SETENVVAR - FAR plugin
typedef struct tag_FAR_REQ_SETENVVAR {
	BOOL    bFARuseASCIIsort;
	wchar_t szEnv[1]; // Variable length: <Name>\0<Value>\0<Name2>\0<Value2>\0\0
} FAR_REQ_SETENVVAR;

typedef struct tag_CESERVER_REQ_SETCONCP {
	BOOL    bSetOutputCP; // [IN], [Out]=result
	DWORD   nCP;          // [IN], [Out]=LastError
} CESERVER_REQ_SETCONCP;

typedef struct tag_CESERVER_REQ_SETWINDOWPOS {
	HWND2 hWnd;
	HWND2 hWndInsertAfter;
	int X;
	int Y;
	int cx;
	int cy;
	UINT uFlags;
} CESERVER_REQ_SETWINDOWPOS;

typedef struct tag_CESERVER_REQ_SETWINDOWRGN {
	HWND2 hWnd;
	int   nRectCount;  // ���� 0 - �������� WindowRgn, ����� - ���������.
	BOOL  bRedraw;
	RECT  rcRects[20]; // [0] - �������� ����, [
} CESERVER_REQ_SETWINDOWRGN;

typedef struct tag_CESERVER_REQ {
    CESERVER_REQ_HDR hdr;
	union {
		BYTE    Data[1]; // variable(!) length
		WORD    wData[1];
		DWORD   dwData[1];
		u64     qwData[1];
		CESERVER_REQ_CONINFO_HDR ConInfo;
		CESERVER_REQ_SETSIZE SetSize;
		CESERVER_REQ_RETSIZE SetSizeRet;
		CESERVER_REQ_OUTPUTFILE OutputFile;
		CESERVER_REQ_NEWCMD NewCmd;
		CESERVER_REQ_STARTSTOP StartStop;
		CESERVER_REQ_STARTSTOPRET StartStopRet;
		CESERVER_REQ_CONEMUTAB Tabs;
		CESERVER_REQ_CONEMUTAB_RET TabsRet;
		CESERVER_REQ_POSTMSG Msg;
		CESERVER_REQ_FLASHWINFO Flash;
		FAR_REQ_SETENVVAR SetEnvVar;
		CESERVER_REQ_SETCONCP SetConCP;
		CESERVER_REQ_SETWINDOWPOS SetWndPos;
		CESERVER_REQ_SETWINDOWRGN SetWndRgn;
		PanelViewInit PVI;
	};
} CESERVER_REQ;


//#pragma pack(pop)
#include <poppack.h>



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

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif



/*enum PipeCmd
{
    SetTabs=0,
    DragFrom,
    DragTo
};*/

// ConEmu.dll ������������ ��������� �������
//HWND WINAPI GetFarHWND();
//void WINAPI _export GetFarVersion ( FarVersion* pfv );

//#if defined(__GNUC__)
////typedef DWORD   HWINEVENTHOOK;
//#define WINEVENT_OUTOFCONTEXT   0x0000  // Events are ASYNC
//// User32.dll
//typedef HWINEVENTHOOK (WINAPI* FSetWinEventHook)(DWORD eventMin, DWORD eventMax, HMODULE hmodWinEventProc, WINEVENTPROC pfnWinEventProc, DWORD idProcess, DWORD idThread, DWORD dwFlags);
//typedef BOOL (WINAPI* FUnhookWinEvent)(HWINEVENTHOOK hWinEventHook);
//#endif


#ifdef __cplusplus


#define CERR_CMDLINEEMPTY 200
#define CERR_CMDLINE      201

#define MOUSE_EVENT_MOVE      (WM_APP+10)
#define MOUSE_EVENT_CLICK     (WM_APP+11)
#define MOUSE_EVENT_DBLCLICK  (WM_APP+12)
#define MOUSE_EVENT_WHEELED   (WM_APP+13)
#define MOUSE_EVENT_HWHEELED  (WM_APP+14)
#define MOUSE_EVENT_FIRST MOUSE_EVENT_MOVE
#define MOUSE_EVENT_LAST MOUSE_EVENT_HWHEELED

//#define INPUT_THREAD_ALIVE_MSG (WM_APP+100)

//#define MAX_INPUT_QUEUE_EMPTY_WAIT 1000


int NextArg(const wchar_t** asCmdLine, wchar_t* rsArg/*[MAX_PATH+1]*/, const wchar_t** rsArgStart=NULL);
BOOL PackInputRecord(const INPUT_RECORD* piRec, MSG* pMsg);
BOOL UnpackInputRecord(const MSG* piMsg, INPUT_RECORD* pRec);
SECURITY_ATTRIBUTES* NullSecurity();
void CommonShutdown();




#endif // __cplusplus



// This must be the end line
#endif // _COMMON_HEADER_HPP_
