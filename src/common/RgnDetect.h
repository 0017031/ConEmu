#pragma once

#define MAX_DETECTED_DIALOGS 20

#include <pshpack1.h>
typedef struct tag_CharAttr
{
	TODO("OPTIMIZE: �������� �� ������� ���� �� ���� DWORD, � ������� ������� ����� ����� �� �����, ����������� ��� ������������ ������");
	union {
		// ���������� �����/������
		struct {
			unsigned int crForeColor : 24; // ����� � ui64 ���������� � nFontIndex
			unsigned int nFontIndex : 8; // 0 - normal, 1 - bold, 2 - italic
			unsigned int crBackColor : 32; // ������� ���� �������������, ����� ��� ������������ �����������
			unsigned int nForeIdx : 8;
			unsigned int nBackIdx : 8; // ����� ������������ ��� ExtendColors
			unsigned int crOrigForeColor : 32;
			unsigned int crOrigBackColor : 32; // �������� ����� � �������, crForeColor � crBackColor ����� ���� �������� ���������
			// ��������������� �����
			unsigned int bDialog : 1;
			unsigned int bDialogVBorder : 1;
			unsigned int bDialogCorner : 1;
			unsigned int bSomeFilled : 1;
			unsigned int bTransparent : 1; // UserScreen
		};
		// � ��� ��� ��������� (����� ���������)
		unsigned __int64 All;
		// ��� ���������, ����� ��� �� �����
		unsigned int ForeFont;
	};
	//
	//DWORD dwAttrubutes; // ����� ����� ����������� �������������� �����...
	//
    ///**
    // * Used exclusively by ConsoleView to append annotations to each character
    // */
    //AnnotationInfo annotationInfo;
} CharAttr;
#include <poppack.h>

inline bool operator==(const CharAttr& s1, const CharAttr& s2)
{
    return s1.All == s2.All;
}


#define FR_FLAGS_MASK     0xFF0000
#define FR_COMMONDLG_MASK 0x0000FF
#define FR_FREEDLG_MASK   0x00FF00
// ���������������� �� "��������"
#define FR_LEFTPANEL      0x000001 // ����� ������
#define FR_RIGHTPANEL     0x000002 // ������ ������
#define FR_FULLPANEL      0x000004 // ���� �� ������� ���������� �� ���� �����
#define FR_MENUBAR        0x000008 // ������ ���� (�������)
#define FR_ACTIVEMENUBAR  0x000018 // ���� MenuBar ����� �� ������, ��� �� ����������� (�.�. ������ ����������)
#define FR_PANELTABS      0x000020 // ������ ��� �������� (������ PanelTabs)
// �� ��� ��������� ��������/����/� ��.
#define FR_FIRSTDLGID     0x000100
#define FR_LASTDLGID      0x00FF00
// �������������� �����
#define FR_ERRORCOLOR     0x010000 // "�����������" �������
#define FR_MACRORECORDING 0x020000 // ������� "R" ��� "MACRO" � ����� ������� ����
#define FR_HASBORDER      0x040000 // ���� ������������� (������) ����� �����
#define FR_HASEXTENSION   0x080000 // ������ �������������� ������� ���� ��� ���������


class CRgnDetect
{
public:
	// Initializers
	CRgnDetect();
	~CRgnDetect();
	
public:
	// Public methods
	int GetDetectedDialogs(int anMaxCount, SMALL_RECT* rc, DWORD* rf) const;
	DWORD GetDialog(DWORD nDlgID, SMALL_RECT* rc) const;
	void PrepareTransparent(const CEFAR_INFO *apFarInfo, const COLORREF *apColors, const CONSOLE_SCREEN_BUFFER_INFO *apSbi, wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight);
	DWORD GetFlags() const;
	// Methods for plugins
	void PrepareTransparent(const CEFAR_INFO *apFarInfo, const COLORREF *apColors);
	void OnWindowSizeChanged();
	void OnWriteConsoleOutput(const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion, const COLORREF *apColors);
	BOOL InitializeSBI(const COLORREF *apColors);
	
	
protected:
	// Private methods
	void DetectDialog(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int nFromX, int nFromY, int *pnMostRight=NULL, int *pnMostBottom=NULL);
	bool FindDialog_TopLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder);
	bool FindDialog_TopRight(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder);
	bool FindDialog_Left(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder);
	bool FindDialog_Right(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder);
	bool FindDialog_Any(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder);
	bool FindDialog_Inner(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY);
	bool FindFrame_TopLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nFrameX, int &nFrameY);
	bool FindFrameTop_ByRight(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostTop);
	bool FindFrameTop_ByLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostTop);
	bool FindFrameBottom_ByRight(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostBottom);
	bool FindFrameBottom_ByLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostBottom);
	bool FindFrameRight_ByTop(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight);
	bool FindFrameRight_ByBottom(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight);
	bool FindFrameLeft_ByTop(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostLeft);
	bool FindFrameLeft_ByBottom(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostLeft);
	// ��������� ����
	bool FindByBackground(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder);
	// ���������
	bool ExpandDialogFrame(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int nFrameX, int nFrameY, int &nMostRight, int &nMostBottom);
	void MarkDialog(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int nX1, int nY1, int nX2, int nY2, bool bMarkBorder = false, bool bFindExterior = true);
	bool ConsoleRect2ScreenRect(const RECT &rcCon, RECT *prcScr);
	

protected:
	// Members
	bool    mb_SelfBuffers;
	const CEFAR_INFO *mp_FarInfo;
	const COLORREF *mp_Colors;
	CONSOLE_SCREEN_BUFFER_INFO m_sbi;
	bool   mb_BufferHeight;

	DWORD   mn_AllFlags, mn_NextDlgId;
	BOOL    mb_NeedPanelDetect;
	SMALL_RECT mrc_LeftPanel, mrc_RightPanel;

	int     mn_DetectCallCount;
	struct {
		int Count;
		SMALL_RECT Rects[MAX_DETECTED_DIALOGS];
		//bool bWasFrame[MAX_DETECTED_DIALOGS];
		DWORD DlgFlags[MAX_DETECTED_DIALOGS];
	} m_DetectedDialogs;
	
protected:
	// ������������ ��� ����������������� ������������ �������
	wchar_t   *mpsz_Chars;
	CharAttr  *mp_Attrs;
	int mn_CurWidth, mn_CurHeight, mn_MaxCells;
	bool mb_SBI_Loaded;
	CharAttr mca_Table[0x100];
	bool mb_TableCreated;
	//void GetConsoleData(const CHAR_INFO *pCharInfo, const COLORREF *apColors, wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight);
};

//#include <pshpack1.h>
class CRgnRects
{
public:
	int nRectCount;
	#define MAX_RGN_RECTS MAX_DETECTED_DIALOGS // 20.
	RECT rcRect[MAX_RGN_RECTS]; // rcRect[0] - ��������, rcRect[1...] - �� ��� ���������� �� rcRect[0]
/*	Current region state:
	#define ERROR               0
	#define NULLREGION          1
	#define SIMPLEREGION        2
	#define COMPLEXREGION       3
	#define RGN_ERROR ERROR
*/	int nRgnState;
	
	CRgnRects();
	~CRgnRects();

	// ����� ����� � NULLREGION
	void Reset();
	// �������� ��� �������������� � ���������� rcRect[0]
	void Init(LPRECT prcInit);
	// Combines the parts of rcRect[..] that are not part of prcAddDiff.
	int Diff(LPRECT prcAddDiff);
	int DiffSmall(SMALL_RECT *prcAddDiff);
	// ����������� �� pRgn, ������� true - ���� ���� �������
	bool LoadFrom(CRgnRects* pRgn);
	
/*	Service variables for nRgnState tesgins */
	int nFieldMaxCells, nFieldWidth, nFieldHeight;
	bool* pFieldCells;
};
//#include <poppack.h>
