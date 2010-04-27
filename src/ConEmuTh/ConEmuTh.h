
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


#pragma once

#include "../common/common.hpp"
#include "ConEmuTh_Lang.h"

#define SafeCloseHandle(h) { if ((h)!=NULL) { HANDLE hh = (h); (h) = NULL; if (hh!=INVALID_HANDLE_VALUE) CloseHandle(hh); } }
#ifdef _DEBUG
	#define OUTPUTDEBUGSTRING(m) OutputDebugString(m)
	#define SHOWDBGINFO(x) OutputDebugStringW(x)
	//#include <crtdbg.h>
#else
	#define OUTPUTDEBUGSTRING(m)
	#define SHOWDBGINFO(x)
	//#define _ASSERT(x)
	//#define _ASSERTE(x)
#endif


#define ISALPHA(c) ((((c) >= (BYTE)'c') && ((c) <= (BYTE)'z')) || (((c) >= (BYTE)'C') && ((c) <= (BYTE)'Z')))
#define isPressed(inp) ((GetKeyState(inp) & 0x8000) == 0x8000)
#define sizeofarray(array) (sizeof(array)/sizeof(*array))

// X - �������, Y - �������
#define FAR_X_VER 995
#define FAR_Y_VER 995
#define FUNC_X(fn) fn##995
#define FUNC_Y(fn) fn##995



#define EVENT_TYPE_REDRAW 250
#define TH_ENVVAR_NAME L"FarThumbnails"
#define TH_ENVVAR_ACTIVE L"Active"
#define TH_ENVVAR_SCROLL L"Scrolling"



//typedef struct tag_FarVersion {
//	union {
//		DWORD dwVer;
//		struct {
//			WORD dwVerMinor;
//			WORD dwVerMajor;
//		};
//	};
//	DWORD dwBuild;
//} FarVersion;

struct CEFAR_FIND_DATA
{
	DWORD    dwFileAttributes;
	//FILETIME ftCreationTime;
	//FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	union {
		struct {
			DWORD nFileSizeHigh;
			DWORD nFileSizeLow;
		};
		unsigned __int64 nFileSize;
	};
	const wchar_t *lpwszFileName;
	const wchar_t *lpwszFileNamePart;
	const wchar_t *lpwszFileExt;
	//const wchar_t *lpwszAlternateFileName;
};

struct CePluginPanelItem
{
	DWORD			cbSize;
	struct CEFAR_FIND_DATA FindData;
	BOOL            bVirtualItem;
	DWORD_PTR       UserData;
	BOOL            bPreviewLoaded; // �������� �� ��� ��������� ���������
	const wchar_t*  pszFullName; // ��� ��������� ��������� - ������ �� ��������� �����
	const wchar_t*  pszDescription; // ������ �� ������ � ���� CePluginPanelItem
	DWORD           Flags;
	DWORD           NumberOfLinks;
	//wchar_t      *Description;
	//wchar_t      *Owner;
	//wchar_t     **CustomColumnData;
	//int           CustomColumnNumber;
	//DWORD_PTR     UserData;
	//DWORD         CRC32;
	//DWORD_PTR     Reserved[2];
};

enum CEPANELINFOFLAGS {
	CEPFLAGS_SHOWHIDDEN         = 0x00000001,
	CEPFLAGS_HIGHLIGHT          = 0x00000002,
	CEPFLAGS_REVERSESORTORDER   = 0x00000004,
	CEPFLAGS_USESORTGROUPS      = 0x00000008,
	CEPFLAGS_SELECTEDFIRST      = 0x00000010,
	CEPFLAGS_REALNAMES          = 0x00000020,
	CEPFLAGS_NUMERICSORT        = 0x00000040,
	CEPFLAGS_PANELLEFT          = 0x00000080,
};

typedef struct tag_CeFullPanelInfo
{
	DWORD cbSize;
	HWND  hView;
	HANDLE hPanel;
	PanelViewMode PVM;
	ThumbSizes Spaces;
	wchar_t sFontName[32]; // Tahoma
	int nFontHeight; // 14
	//
	int nWholeW, nWholeH; // ���������������� ��� ������ Paint
	int nXCountFull; // ��� �����, ����� ������ �� �����������
	int nXCount; // � ��� ����������� ����������� ����� ����� ������
	int nYCountFull; // ��� �����, ����� ������ �� �����������
	int nYCount; // � ��� ����������� ����������� ������� ����� ������
	//
	DWORD nFarInterfaceSettings;
	DWORD nFarPanelSettings;
	BOOL  bLeftPanel, bPlugin;
	RECT  PanelRect;
	int ItemsNumber;
	int CurrentItem;
	int TopPanelItem;
	int OurTopPanelItem; // �� ����� �� ��������� � ���������, ����� CurrentItem ��� ���� �����
	BOOL IsFilePanel;
	int PanelMode; // 0..9 - ������� ����� ������.
	BOOL Visible;
	BOOL Focus;
	DWORD Flags; // CEPANELINFOFLAGS
	// ************************
	int nMaxFarColors;
	BYTE *nFarColors; // ������ ������ ����
	// ************************
	int nMaxPanelDir;
	wchar_t* pszPanelDir;
	// ************************
	int nMaxItemsNumber;
	CePluginPanelItem** ppItems;
	// ************************
	int nFarTmpBuf;    // ��������� ����� ��� ���������
	LPVOID pFarTmpBuf; // ���������� �� �������� ������
	

	int RegisterPanelView();
	int UnregisterPanelView();
	HWND CreateView();
	void FreeInfo();
	
	// ��� "����������" ������� ���������� �� �������� ����, ��� ����� ������� FAR Api
	void DisplayReloadPanel();
	
	static LRESULT CALLBACK DisplayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI DisplayThread(LPVOID lpvParam);
	void Paint(HWND hwnd, PAINTSTRUCT& ps, RECT& rc);
	BOOL PaintItem(HDC hdc, int x, int y, CePluginPanelItem* pItem, BOOL abCurrentItem, 
			   COLORREF *nBackColor, COLORREF *nForeColor, HBRUSH *hBack,
			   BOOL abAllowPreview, HBRUSH hBackBrush);
	int DrawItemText(HDC hdc, LPRECT prcText, LPRECT prcMaxText, CePluginPanelItem* pItem, LPCWSTR pszComments, HBRUSH hBr);
	BOOL OnSettingsChanged(BOOL bInvalidate);
			   
	/*{
		if (ppItems) {
			for (int i=0; i<ItemsNumber; i++)
				if (ppItems[i]) free(ppItems[i]);
			free(ppItems);
			ppItems = NULL;
		}
		nMaxItemsNumber = 0;
		if (pszPanelDir) {
			free(pszPanelDir);
			pszPanelDir = NULL;
		}
		nMaxPanelDir = 0;
		if (nFarColors) {
			free(nFarColors);
			nFarColors = NULL;
		}
		nMaxFarColors = 0;
		if (pFarTmpBuf) {
			free(pFarTmpBuf);
			pFarTmpBuf = NULL;
		}
		nFarTmpBuf = 0;
	}*/
} CeFullPanelInfo;





//struct ThumbnailSettings
//{
//	TODO("nWidth & nHeight - deprecated");
//	// �������� �������� �� WholeWidth() & WholeHeight()
//	int nWidth, nHeight; // 98x98
//
//	int nThumbSize; // 96
//	int nIconSize; // 48
//	DWORD crBackground; // 0xFFFFFF (RGB) ��� 0xFF000000 (Index)
//
//	int nThumbFrame; // 1 (����� ����� ������ ���������
//	DWORD crThumbFrame; // 0xFFFFFF (RGB) ��� 0xFF000000 (Index)
//	int nSelectFrame; // 1 (����� ������ �������� ��������)
//	DWORD crSelectFrame; // 0xFFFFFF (RGB) ��� 0xFF000000 (Index)
//	int nHSpacing, nVSpacing; // 5, 25 - ���������� ����� ����� �������
//
//	TODO("����� �� ����� - �������� ������ nSelectFrame? ��� ����� �����?");
//	int nHPadding, nVPadding; // 1, 1 - ���������������� ������
//
//	int nFontHeight; // 14
//	wchar_t sFontName[32]; // Tahoma
//	BOOL bLoadPreviews, bLoadFolders;
//	int nLoadTimeout; // 15 sec
//
//	int nMaxZoom;
//	BOOL bUsePicView2;
//
//	wchar_t sCacheFolder[MAX_PATH];
//
//
//	void Load() {
//		TODO("nWidth & nHeight - deprecated");
//		nWidth = nHeight = 98;
//
//		nThumbSize = 96; // ����� ������� ����� 96. ����� ����� ���� 500% �� 16�16 ���������
//		nIconSize = 32;
//
//		nThumbFrame = 1;
//		nSelectFrame = 1;
//
//		nHSpacing = 5; nVSpacing = 25;
//		nHPadding = 1; nVPadding = 1;
//
//		nFontHeight = 14;
//		lstrcpy(sFontName, L"Tahoma");
//
//		bLoadPreviews = TRUE;
//		bLoadFolders = TRUE;
//		nLoadTimeout = 15;
//		nMaxZoom = 500; // �� �� ������ ������� ��������� :)
//		bUsePicView2 = TRUE;
//        sCacheFolder[0] = 0;
//	};
//};
extern PanelViewSettings gThSet;
extern BOOL gbCancelAll;



extern HWND ghConEmuRoot;
extern HMODULE ghPluginModule; // ConEmuTh.dll - ��� ������
extern DWORD gnSelfPID, gnMainThreadId;
//extern int lastModifiedStateW;
//extern bool gbHandleOneRedraw; //, gbHandleOneRedrawCh;
//extern WCHAR gszDir1[CONEMUTABMAX], gszDir2[CONEMUTABMAX], 
extern WCHAR gszRootKey[MAX_PATH*2];
//extern int maxTabCount, lastWindowCount;
//extern CESERVER_REQ* tabs; //(ConEmuTab*) calloc(maxTabCount, sizeof(ConEmuTab));
//extern CESERVER_REQ* gpCmdRet;
//extern HWND ConEmuHwnd;
//extern HWND FarHwnd;
extern FarVersion gFarVersion;
//extern int lastModifiedStateW;
////extern HANDLE hEventCmd[MAXCMDCOUNT];
//extern HANDLE hThread;
////extern WCHAR gcPlugKey;
////WARNING("������, �������� ghConIn �� GetStdHandle()"); // ����� � Win7 ����� ����� �����������
////extern HANDLE ghConIn;
//extern BOOL gbNeedPostTabSend;
//extern HANDLE ghServerTerminateEvent;
//extern CESERVER_REQ_CONINFO_HDR *gpConsoleInfo;
//extern DWORD gnSelfPID;
extern CeFullPanelInfo pviLeft, pviRight;
extern HANDLE ghDisplayThread; extern DWORD gnDisplayThreadId;
extern HWND ghLeftView, ghRightView;
extern bool gbWaitForKeySequenceEnd;
extern DWORD gnWaitForKeySeqTick;
//class CRgnDetect;
//extern CRgnDetect *gpRgnDetect;

// *** lng resources begin ***
extern wchar_t gsFolder[64], /*gsHardLink[64],*/ gsSymLink[64], gsJunction[64];
// *** lng resources end ***


//typedef struct tag_SynchroArg {
//	enum {
//		eCommand,
//		eInput
//	} SynchroType;
//	HANDLE hEvent;
//	LPARAM Result;
//	LPARAM Param1, Param2;
//	BOOL Obsolete;
//	//BOOL Processed;
//} SynchroArg;

BOOL LoadFarVersion();
void ExitPlugin(void);
void FUNC_X(ExitFARW)(void);
void FUNC_Y(ExitFARW)(void);
void FUNC_X(SetStartupInfoW)(void *aInfo);
void FUNC_Y(SetStartupInfoW)(void *aInfo);
int ShowMessage(int aiMsg, int aiButtons);
int ShowMessageA(int aiMsg, int aiButtons);
int FUNC_X(ShowMessage)(int aiMsg, int aiButtons);
int FUNC_Y(ShowMessage)(int aiMsg, int aiButtons);
int ShowMessageA(LPCSTR asMsg, int aiButtons);
int FUNC_X(ShowMessage)(LPCWSTR asMsg, int aiButtons);
int FUNC_Y(ShowMessage)(LPCWSTR asMsg, int aiButtons);
void PostMacro(wchar_t* asMacro);
void PostMacroA(char* asMacro);
void FUNC_X(PostMacro)(wchar_t* asMacro);
void FUNC_Y(PostMacro)(wchar_t* asMacro);
LPCWSTR GetMsgW(int aiMsg);
const wchar_t* GetMsgA(int aiMsg, wchar_t* rsMsg/*MAX_PATH*/);
LPCWSTR FUNC_Y(GetMsg)(int aiMsg);
LPCWSTR FUNC_X(GetMsg)(int aiMsg);
int ShowPluginMenu();
int ShowPluginMenuA();
int FUNC_Y(ShowPluginMenu)();
int FUNC_X(ShowPluginMenu)();
BOOL IsMacroActive();
BOOL IsMacroActiveA();
BOOL FUNC_X(IsMacroActive)();
BOOL FUNC_Y(IsMacroActive)();
//CeFullPanelInfo* LoadPanelInfo(BOOL abActive);
CeFullPanelInfo* GetActivePanel();
BOOL LoadPanelInfoA(BOOL abActive);
BOOL FUNC_X(LoadPanelInfo)(BOOL abActive);
BOOL FUNC_Y(LoadPanelInfo)(BOOL abActive);
void ReloadPanelsInfo();
void ReloadPanelsInfoA();
void FUNC_X(ReloadPanelsInfo)();
void FUNC_Y(ReloadPanelsInfo)();
BOOL IsLeftPanelActive();
BOOL IsLeftPanelActiveA();
BOOL FUNC_X(IsLeftPanelActive)();
BOOL FUNC_Y(IsLeftPanelActive)();
void LoadPanelItemInfo(CeFullPanelInfo* pi, int nItem);
void LoadPanelItemInfoA(CeFullPanelInfo* pi, int nItem);
void FUNC_X(LoadPanelItemInfo)(CeFullPanelInfo* pi, int nItem);
void FUNC_Y(LoadPanelItemInfo)(CeFullPanelInfo* pi, int nItem);
bool CheckWindows();
//bool CheckWindowsA();
//bool FUNC_X(CheckWindows)();
//bool FUNC_Y(CheckWindows)();

// ��� "����������" ������� ���������� �� �������� ����, ��� ����� ������� FAR Api
//void DisplayReloadPanel(CeFullPanelInfo* pi);

extern int gnCreateViewError;
extern DWORD gnWin32Error;
//HWND CreateView(CeFullPanelInfo* pi);
void UpdateEnvVar(BOOL abForceRedraw);
CeFullPanelInfo* IsThumbnailsActive(BOOL abFocusRequired);

// ConEmu.dll
typedef int (WINAPI *RegisterPanelView_t)(PanelViewInit *ppvi);
typedef HWND (WINAPI *GetFarHWND2_t)(BOOL abConEmuOnly);
extern RegisterPanelView_t gfRegisterPanelView;
extern GetFarHWND2_t gfGetFarHWND2;
BOOL CheckConEmu(/*BOOL abForceCheck=FALSE*/);
//HWND GetConEmuHWND();
//BOOL WINAPI OnReadConsole(PINPUT_RECORD lpBuffer, LPDWORD lpNumberOfEventsRead);
BOOL WINAPI OnPrePeekConsole(HANDLE hInput, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead, BOOL* pbResult);
BOOL WINAPI OnPostPeekConsole(HANDLE hInput, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead, BOOL* pbResult);
BOOL WINAPI OnPreReadConsole(HANDLE hInput, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead, BOOL* pbResult);
BOOL WINAPI OnPostReadConsole(HANDLE hInput, PINPUT_RECORD lpBuffer, DWORD nBufSize, LPDWORD lpNumberOfEventsRead, BOOL* pbResult);
BOOL WINAPI OnPreWriteConsoleOutput(HANDLE hOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);


/* Other plugin integrations */
#define IMPEX_MAGIC 0x78456D49 // 'ImEx'
struct ImpExPanelItem
{
	DWORD nMagic;			// IMPEX_MAGIC
	DWORD cbSizeOfStruct;	// 1196 � ���� ������
	DWORD nBinarySize;		// ������
	LPBYTE pBinaryData;		// � ���������� �������� ������
	// ��������� ���� �� ����������
};
