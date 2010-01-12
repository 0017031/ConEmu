#pragma once
#include "kl_parts.h"
#include "../Common/common.hpp"

#include "RealConsole.h"

#define MAX_COUNT_PART_BRUSHES 16*16*4
#define MAX_SPACES 0x400

class CVirtualConsole
{
private:
	// RealConsole
	CRealConsole *mp_RCon;
public:
	CRealConsole *RCon() { if (this) return mp_RCon; return NULL; };
public:
    WARNING("������� protected!");
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
		wchar_t ch;
		DWORD nBlinkTime, nLastBlink;
		RECT lastRect;
		UINT lastSize; // ���������� ������ ������� (� ���������)
	} Cursor;
	//
	bool    mb_IsForceUpdate; // ��� ��������������� � InitDC, ����� �������� isForce �� ���������
	bool    mb_RequiredForceUpdate; // �������� �����, ��������...
private:
	HDC     hDC;
	HBITMAP hBitmap;
	HBRUSH  hBrush0, hOldBrush, hSelectedBrush;
	HFONT   hSelectedFont, hOldFont;
	#ifdef _DEBUG
	BOOL    mb_DebugDumpDC;
	#endif
public:
	bool isEditor, isViewer, isFilePanel;
	BYTE attrBackLast;

	wchar_t *mpsz_ConChar, *mpsz_ConCharSave;
	WORD  *mpn_ConAttr, *mpn_ConAttrSave;
	DWORD *ConCharX;
	//TCHAR *Spaces; WORD nSpaceCount;
	static wchar_t ms_Spaces[MAX_SPACES], ms_HorzDbl[MAX_SPACES], ms_HorzSingl[MAX_SPACES];
	// ��� ��������� ��������� �������� �����
	BYTE  m_ForegroundColors[0x100], m_BackgroundColors[0x100];
	HFONT mh_FontByIndex[0x100]; // �������� ������ (�� �����) �� ������ normal/bold/italic

	//CONSOLE_SELECTION_INFO SelectionInfo;

	CVirtualConsole(/*HANDLE hConsoleOutput = NULL*/);
	~CVirtualConsole();
	static CVirtualConsole* CreateVCon(RConStartArgs *args);

	bool InitDC(bool abNoDc, bool abNoWndResize);
	void DumpConsole();
	BOOL Dump(LPCWSTR asFile);
	bool Update(bool isForce = false, HDC *ahDc=NULL);
	void UpdateCursor(bool& lRes);
	void SelectFont(HFONT hNew);
	void SelectBrush(HBRUSH hNew);
	inline bool isCharBorder(WCHAR inChar);
	bool isCharBorderVertical(WCHAR inChar);
	bool isCharProgress(WCHAR inChar);
	bool isCharScroll(WCHAR inChar);
	void BlitPictureTo(int inX, int inY, int inWidth, int inHeight);
	bool CheckSelection(const CONSOLE_SELECTION_INFO& select, SHORT row, SHORT col);
	//bool GetCharAttr(TCHAR ch, WORD atr, TCHAR& rch, BYTE& foreColorNum, BYTE& backColorNum, FONT* pFont);
	void Paint(HDC hDc, RECT rcClient);
	void UpdateInfo();
	//void GetConsoleCursorInfo(CONSOLE_CURSOR_INFO *ci) { mp_RCon->GetConsoleCursorInfo(ci); };
	//DWORD GetConsoleCP() { return mp_RCon->GetConsoleCP(); };
	//DWORD GetConsoleOutputCP() { return mp_RCon->GetConsoleOutputCP(); };
	//DWORD GetConsoleMode() { return mp_RCon->GetConsoleMode(); };
	//void GetConsoleScreenBufferInfo(CONSOLE_SCREEN_BUFFER_INFO *sbi) { mp_RCon->GetConsoleScreenBufferInfo(sbi); };
	RECT GetRect();
	void OnFontChanged();
	COORD ClientToConsole(LONG x, LONG y);
	void OnConsoleSizeChanged();
	static void ClearPartBrushes();

protected:
	inline void GetCharAttr(WORD atr, BYTE& foreColorNum, BYTE& backColorNum, HFONT* pFont);
	wchar_t* mpsz_LogScreen; DWORD mn_LogScreenIdx;
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
	//CONSOLE_SELECTION_INFO select1, select2;
	uint TextLen;
	bool isCursorValid, drawImage, textChanged, attrChanged;
	char *tmpOem;
	void UpdateCursorDraw(HDC hPaintDC, RECT rcClient, COORD pos, UINT dwSize);
	bool UpdatePrepare(bool isForce, HDC *ahDc, MSectionLock *pSDC);
	void UpdateText(bool isForce); //, bool updateText, bool updateCursor);
	WORD CharWidth(TCHAR ch);
	void CharABC(TCHAR ch, ABC *abc);
	bool CheckChangedTextAttr();
	void ParseLine(int row, TCHAR *ConCharLine, WORD *ConAttrLine);
	HANDLE mh_Heap;
	LPVOID Alloc(size_t nCount, size_t nSize);
	void Free(LPVOID ptr);
	MSection csDC;  /*DWORD ncsTDC;*/ BOOL mb_PaintRequested; BOOL mb_PaintLocked;
	MSection csCON; /*DWORD ncsTCON;*/
	int mn_BackColorIdx; //==0
	void Box(LPCTSTR szText);
	static wchar_t mc_Uni2Oem[0x10000];
	wchar_t Uni2Oem(wchar_t ch);
	//BOOL RetrieveConsoleInfo(BOOL bShortOnly);
	typedef struct tag_PARTBRUSHES {
		wchar_t ch; // 0x2591 0x2592 0x2593 0x2588 - �� ���������� ���������
		SHORT   nBackIdx;
		SHORT   nForeIdx;
		HBRUSH  hBrush;
	} PARTBRUSHES;
	//std::vector<PARTBRUSHES> m_PartBrushes;
	static PARTBRUSHES m_PartBrushes[MAX_COUNT_PART_BRUSHES];
	static HBRUSH PartBrush(wchar_t ch, SHORT nBackIdx, SHORT nForeIdx);
	BOOL mb_InPaintCall;
	//
	void DistributeSpaces(wchar_t* ConCharLine, WORD* ConAttrLine, DWORD* ConCharXLine, const int j, const int j2, const int end);
	LONG nFontHeight, nFontWidth;
	BYTE nFontCharSet;
	BYTE nLastNormalBack;
    bool bExtendFonts, bExtendColors;
    BYTE nFontNormalColor, nFontBoldColor, nFontItalicColor, nExtendColor;
};

#include <pshpack1.h>
typedef struct tagMYRGB {
	union {
		COLORREF color;
		struct {
			BYTE    rgbBlue;
			BYTE    rgbGreen;
			BYTE    rgbRed;
			BYTE    rgbReserved;
		};
	};
} MYRGB;
#include <poppack.h>
