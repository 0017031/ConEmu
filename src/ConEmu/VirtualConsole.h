#pragma once
#include "kl_parts.h"
#include "../Common/common.hpp"

#include "RealConsole.h"


class CVirtualConsole
{
public:
    WARNING("������� protected!");
	bool IsForceUpdate;
	uint TextWidth, TextHeight; // ������ � ��������
	uint Width, Height; // ������ � ��������
private:
	uint nMaxTextWidth, nMaxTextHeight; // ������ � ��������
private:
	struct
	{
		bool isVisible;
		bool isVisiblePrev;
		bool isVisiblePrevFromInfo;
		short x;
		short y;
		COLORREF foreColor;
		COLORREF bgColor;
		BYTE foreColorNum, bgColorNum;
		TCHAR ch[2];
		DWORD nBlinkTime, nLastBlink;
		RECT lastRect;
		UINT lastSize; // ���������� ������ ������� (� ���������)
	} Cursor;
public:
    //HANDLE  hConOut(BOOL abAllowRecreate=FALSE);
	//HANDLE  hConIn();
	HWND    hConWnd;
	HDC     hDC; //, hBgDc;
	HBITMAP hBitmap; //, hBgBitmap;
	HBRUSH  hBrush0, hOldBrush, hSelectedBrush;
	//SIZE	bgBmp;
	HFONT   /*hFont, hFont2,*/ hSelectedFont, hOldFont;

	bool isEditor, isFilePanel;
	BYTE attrBackLast;

	wchar_t *ConChar;
	WORD  *ConAttr;
	//WORD  FontWidth[0x10000]; //, Font2Width[0x10000];
	DWORD *ConCharX;
	TCHAR *Spaces; WORD nSpaceCount;
	
	// ���������� ������� � ��������
	RECT mr_LeftPanel, mr_RightPanel;

	CONSOLE_SELECTION_INFO SelectionInfo;

	CVirtualConsole(/*HANDLE hConsoleOutput = NULL*/);
	~CVirtualConsole();
	static CVirtualConsole* Create(bool abDetached);

	void OnActivate(int nNewNum, int nOldNum);
	void OnFocus(BOOL abFocused);
	BOOL PreInit(BOOL abCreateBuffers=TRUE);
	bool InitDC(bool abNoDc, bool abNoWndResize);
	//void InitHandlers(BOOL abCreated);
	void DumpConsole();
	bool Update(bool isForce = false, HDC *ahDc=NULL);
	void SelectFont(HFONT hNew);
	void SelectBrush(HBRUSH hNew);
	//HFONT CreateFontIndirectMy(LOGFONT *inFont);
	bool isCharBorder(WCHAR inChar);
	bool isCharBorderVertical(WCHAR inChar);
	void BlitPictureTo(int inX, int inY, int inWidth, int inHeight);
	bool CheckSelection(const CONSOLE_SELECTION_INFO& select, SHORT row, SHORT col);
	bool GetCharAttr(TCHAR ch, WORD atr, TCHAR& rch, BYTE& foreColorNum, BYTE& backColorNum);
	bool SetConsoleSize(COORD size);
	bool CheckBufferSize();
	void SendMouseEvent(UINT messg, WPARAM wParam, int x, int y);
	void SendConsoleEvent(INPUT_RECORD* piRec);
	void StopSignal();
	void StopThread(BOOL abRecreating=FALSE);
	void Paint();
	void UpdateInfo();
	BOOL isBufferHeight();
	LPCTSTR GetTitle();
	BOOL GetConsoleScreenBufferInfo();
	void GetConsoleSelectionInfo(CONSOLE_SELECTION_INFO *sel) {	*sel = con.m_sel; };
	void GetConsoleCursorInfo(CONSOLE_CURSOR_INFO *ci) { *ci = con.m_ci; };
	DWORD GetConsoleCP() { return con.m_dwConsoleCP; };
	DWORD GetConsoleOutputCP() { return con.m_dwConsoleOutputCP; };
	DWORD GetConsoleMode() { return con.m_dwConsoleMode; };
	DWORD GetConsoleScreenBufferInfo(CONSOLE_SCREEN_BUFFER_INFO *sbi) { *sbi = con.m_sbi; };
	void SyncConsole2Window();
	void OnWinEvent(DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
	int  GetProcesses(ConProcess** ppPrc);
	DWORD GetFarPID();
	DWORD GetActiveStatus();
	DWORD GetServerPID();
	LRESULT OnKeyboard(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
	LRESULT OnScroll(int nDirection);
	bool isConSelectMode();
	bool isFar();
	void ShowConsole(int nMode); // -1 Toggle, 0 - Hide, 1 - Show
	BOOL isDetached();
	BOOL AttachConemuC(HWND ahConWnd, DWORD anConemuC_PID);
	RECT GetRect();
	bool RecreateProcess();

protected:
	DWORD mn_ConEmuC_PID; HANDLE mh_ConEmuC, mh_ConEmuCInput;
	TCHAR ms_ConEmuC_Pipe[MAX_PATH], ms_ConEmuCInput_Pipe[MAX_PATH], ms_VConServer_Pipe[MAX_PATH];
	TCHAR Title[MAX_TITLE_SIZE+1], TitleCmp[MAX_TITLE_SIZE+1];
	//HANDLE  mh_ConOut; BOOL mb_ConHandleCreated;
	HANDLE mh_StdIn, mh_StdOut;
	i64 m_LastMaxReadCnt; DWORD m_LastMaxReadTick;
	enum _PartType{
		pNull,  // ����� ������/���������, �������������� �������
		pSpace, // ��� ������� ������ ����� ��������, ���� ����� pText,pSpace,pText �� pSpace,pText �������� � ������ pText
		pUnderscore, // '_' �������. �� ���� ����� ������ � ����� ������
		pBorder,
		pVBorder, // ������� ������������ �����, ������� ������ ��������
		pRBracket, // �������� '}' ��� �������� �����, �������� �� �������
		pText,
		pDummy  // �������������� "�������", ������� ����� ���������� ����� ����� ������
	};
	enum _PartType GetCharType(TCHAR ch);
	struct _TextParts {
		enum _PartType partType;
		BYTE attrFore, attrBack; // ����������� ������ ���� �� ������ �������, �� � ��������� ��������!
		WORD i1,i2;  // ������� � ������� ������, 0-based
		WORD iwidth; // ���������� �������� � �����
		DWORD width; // ������ ������ � ��������. ��� pSpace & pBorder ����� ���������� � ������ pText/pVBorder

		int x1; // ���������� � �������� (�����������������)
	} *TextParts;
	CONSOLE_SCREEN_BUFFER_INFO csbi; DWORD mdw_LastError;
	CONSOLE_CURSOR_INFO	cinf;
	COORD winSize, coord;
	CONSOLE_SELECTION_INFO select1, select2;
	uint TextLen;
	bool isCursorValid, drawImage, doSelect, textChanged, attrChanged;
	char *tmpOem;
	void UpdateCursor(bool& lRes);
	void UpdateCursorDraw(COORD pos, BOOL vis, UINT dwSize,  LPRECT prcLast=NULL);
	bool UpdatePrepare(bool isForce, HDC *ahDc);
	void UpdateText(bool isForce, bool updateText, bool updateCursor);
	WORD CharWidth(TCHAR ch);
	bool CheckChangedTextAttr();
	void ParseLine(int row, TCHAR *ConCharLine, WORD *ConAttrLine);
	BOOL AttachPID(DWORD dwPID);
	BOOL StartProcess();
	//typedef struct _ConExeProps {
	//	BOOL  bKeyExists;
	//	DWORD ScreenBufferSize; //Loword-Width, Hiword-Height
	//	DWORD WindowSize;
	//	DWORD WindowPosition;
	//	DWORD FontSize;
	//	DWORD FontFamily;
	//	TCHAR *FaceName;
	//	TCHAR *FullKeyName;
	//} ConExeProps;
	//void RegistryProps(BOOL abRollback, ConExeProps& props, LPCTSTR asExeName=NULL);
	static DWORD WINAPI StartProcessThread(LPVOID lpParameter);
	HANDLE mh_Heap, mh_Thread, mh_VConServerThread;
	HANDLE mh_TermEvent, mh_ForceReadEvent, mh_EndUpdateEvent, mh_Sync2WindowEvent, mh_ConChanged, mh_CursorChanged;
	BOOL mb_FullRetrieveNeeded, mb_Detached;
	//HANDLE mh_ReqSetSize, mh_ReqSetSizeEnd; COORD m_ReqSetSize;
	DWORD mn_ThreadID;
	LPVOID Alloc(size_t nCount, size_t nSize);
	void Free(LPVOID ptr);
	CRITICAL_SECTION csDC;  DWORD ncsTDC;
	CRITICAL_SECTION csCON; DWORD ncsTCON;
	int mn_BackColorIdx; //==0
	void Box(LPCTSTR szText);
	void SetConsoleSizeInt(COORD size);
	void GetConsoleSizeInfo(CONSOLE_INFO *pci);
	BOOL SetConsoleInfo(CONSOLE_INFO *pci);
	void SetConsoleFontSizeTo(int inSizeX, int inSizeY);
	BOOL RetrieveConsoleInfo(BOOL bShortOnly);
	BOOL InitBuffers(DWORD OneBufferSize);
private:
	// ��� ���������� ���������������� � RetrieveConsoleInfo()
	struct {
		CRITICAL_SECTION cs; DWORD ncsT;
		CONSOLE_SELECTION_INFO m_sel;
		CONSOLE_CURSOR_INFO m_ci;
		DWORD m_dwConsoleCP, m_dwConsoleOutputCP, m_dwConsoleMode;
		CONSOLE_SCREEN_BUFFER_INFO m_sbi;
		wchar_t *pConChar;
		WORD  *pConAttr;
		int nTextWidth, nTextHeight;
		size_t nPacketIdx;
	} con;
	// 
	CRITICAL_SECTION csPRC; DWORD ncsTPRC;
	std::vector<ConProcess> m_Processes;
	int mn_ProcessCount;
	//
	DWORD mn_FarPID;
	//
	void ProcessAdd(DWORD addPID);
	void ProcessDelete(DWORD addPID);
	void ProcessUpdateFlags(BOOL abProcessChanged);
	void ProcessCheckName(struct ConProcess &ConPrc, LPWSTR asFullFileName);
	DWORD mn_ActiveStatus;
	bool isShowConsole;
	bool mb_ConsoleSelectMode;
	int BufferHeight;
	static DWORD WINAPI ServerThread(LPVOID lpvParam);
	HANDLE mh_ServerThreads[MAX_SERVER_THREADS], mh_ActiveServerThread;
	DWORD  mn_ServerThreadsId[MAX_SERVER_THREADS];
	HANDLE mh_ServerSemaphore, mh_GuiAttached;
	//typedef struct tag_ServerThreadCommandArg {
	//	CVirtualConsole *pCon;
	//	HANDLE hPipe;
	//} ServerThreadCommandArg;
	void ServerThreadCommand(HANDLE hPipe);
	void ApplyConsoleInfo(CESERVER_REQ* pInfo);
	void SetHwnd(HWND ahConWnd);
	WORD mn_LastVKeyPressed;
	DWORD mn_LastConReadTick;
	BOOL GetConWindowSize(const CONSOLE_SCREEN_BUFFER_INFO& sbi, int& nNewWidth, int& nNewHeight);
	int mn_Focused; //-1 ����� �������, 1 - � ������, 0 - �� � ������
	DWORD mn_InRecreate; // Tick, ����� ������ ������������
	BOOL mb_ProcessRestarted;
	// ����
	BOOL mb_UseLogs;
	HANDLE mh_LogInput; wchar_t *mpsz_LogInputFile, *mpsz_LogPackets; UINT mn_LogPackets;
	void CreateLogFiles();
	void CloseLogFiles();
	void LogInput(INPUT_RECORD* pRec);
	void LogPacket(CESERVER_REQ* pInfo);
	bool RecreateProcessStart();
};
