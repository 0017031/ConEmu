#pragma once
#include "kl_parts.h"
#include "../Common/common.hpp"

#include "RealConsole.h"


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
		TCHAR ch[2];
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
public:
	bool isEditor, isFilePanel;
	BYTE attrBackLast;

	wchar_t *mpsz_ConChar, *mpsz_ConCharSave;
	WORD  *mpn_ConAttr, *mpn_ConAttrSave;
	DWORD *ConCharX;
	TCHAR *Spaces; WORD nSpaceCount;
	
	// ���������� ������� � ��������
	RECT mr_LeftPanel, mr_RightPanel;

	CONSOLE_SELECTION_INFO SelectionInfo;

	CVirtualConsole(/*HANDLE hConsoleOutput = NULL*/);
	~CVirtualConsole();
	static CVirtualConsole* Create(bool abDetached);

	bool InitDC(bool abNoDc, bool abNoWndResize);
	void DumpConsole();
	bool Update(bool isForce = false, HDC *ahDc=NULL);
	void SelectFont(HFONT hNew);
	void SelectBrush(HBRUSH hNew);
	bool isCharBorder(WCHAR inChar);
	bool isCharBorderVertical(WCHAR inChar);
	void BlitPictureTo(int inX, int inY, int inWidth, int inHeight);
	bool CheckSelection(const CONSOLE_SELECTION_INFO& select, SHORT row, SHORT col);
	bool GetCharAttr(TCHAR ch, WORD atr, TCHAR& rch, BYTE& foreColorNum, BYTE& backColorNum);
	void Paint();
	void UpdateInfo();
	void GetConsoleSelectionInfo(CONSOLE_SELECTION_INFO *sel) {	mp_RCon->GetConsoleSelectionInfo(sel); };
	void GetConsoleCursorInfo(CONSOLE_CURSOR_INFO *ci) { mp_RCon->GetConsoleCursorInfo(ci); };
	DWORD GetConsoleCP() { return mp_RCon->GetConsoleCP(); };
	DWORD GetConsoleOutputCP() { return mp_RCon->GetConsoleOutputCP(); };
	DWORD GetConsoleMode() { return mp_RCon->GetConsoleMode(); };
	void GetConsoleScreenBufferInfo(CONSOLE_SCREEN_BUFFER_INFO *sbi) { mp_RCon->GetConsoleScreenBufferInfo(sbi); };
	RECT GetRect();
	void OnFontChanged();

protected:
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
	void UpdateCursorDraw(HDC hPaintDC, COORD pos, UINT dwSize);
	bool UpdatePrepare(bool isForce, HDC *ahDc);
	void UpdateText(bool isForce, bool updateText, bool updateCursor);
	WORD CharWidth(TCHAR ch);
	bool CheckChangedTextAttr();
	void ParseLine(int row, TCHAR *ConCharLine, WORD *ConAttrLine);
	HANDLE mh_Heap;
	LPVOID Alloc(size_t nCount, size_t nSize);
	void Free(LPVOID ptr);
	CRITICAL_SECTION csDC;  DWORD ncsTDC;
	CRITICAL_SECTION csCON; DWORD ncsTCON;
	int mn_BackColorIdx; //==0
	void Box(LPCTSTR szText);
	BOOL RetrieveConsoleInfo(BOOL bShortOnly);
};
