#include "Header.h"
#include <Tlhelp32.h>

TODO("CES_CONMANACTIVE deprecated");

WARNING("����� ���� ������ ServerThread? � StartProcessThread ��������������� ������ ��� ������� ��������, � ����� �� ���� �����");

WARNING("� ������ VCon ������� ����� BYTE[256] ��� �������� ������������ ������ (Ctrl,...,Up,PgDn,Add,� ��.");
WARNING("�������������� ����� �������� � ����� {VKEY,wchar_t=0}, � ������� ��������� ��������� wchar_t �� WM_CHAR/WM_SYSCHAR");
WARNING("��� WM_(SYS)CHAR �������� wchar_t � ������, � ������ ��������� VKEY");
WARNING("��� ������������ WM_KEYUP - �����(� ������) wchar_t �� ����� ������, ������ � ������� UP");
TODO("� ������������ - ��������� �������� isKeyDown, � ������� �����");
WARNING("��� ������������ �� ������ ������� (�� �������� � � �������� ������ ������� - ����������� ����� ���� ������� � ����� ���������) ��������� �������� caps, scroll, num");
WARNING("� ����� ���������� �������/������� ��������� ����� �� �� ���������� Ctrl/Shift/Alt");



//������, ��� ���������, ������ �������, ���������� �����, � ��...

#define VCURSORWIDTH 2
#define HCURSORHEIGHT 2

#define Assert(V) if ((V)==FALSE) { TCHAR szAMsg[MAX_PATH*2]; wsprintf(szAMsg, _T("Assertion (%s) at\n%s:%i"), _T(#V), _T(__FILE__), __LINE__); Box(szAMsg); }

CVirtualConsole* CVirtualConsole::Create()
{
    CVirtualConsole* pCon = new CVirtualConsole();
    TODO("CVirtualConsole::Create - ������� �������")
    
    if (gSet.nAttachPID) {
        // Attach - only once
        DWORD dwPID = gSet.nAttachPID; gSet.nAttachPID = 0;
        if (!pCon->AttachPID(dwPID)) {
            delete pCon;
            return NULL;
        }
    } else {
        if (!pCon->StartProcess()) {
            delete pCon;
            return NULL;
        }
        // ����� �� � �������� ����� ���� ����� ������ � ������� �������� ������ �����, ��
        // ����� ���� ������ ������ �� ������������
    }

    pCon->mh_Thread = CreateThread(NULL, 0, StartProcessThread, (LPVOID)pCon, 0, &pCon->mn_ThreadID);
    if (pCon->mh_Thread == NULL) {
        DisplayLastError(_T("Can't create console thread!"));
        delete pCon;
        return NULL;
    }

    /*if (gSet.wndHeight && gSet.wndWidth)
    {
        COORD b = {gSet.wndWidth, gSet.wndHeight};
        pCon->SetConsoleSize(b);
    }*/

    return pCon;
}

CVirtualConsole::CVirtualConsole(/*HANDLE hConsoleOutput*/)
{
	memset(Title,0,sizeof(Title)); memset(TitleCmp,0,sizeof(TitleCmp));
    SIZE_T nMinHeapSize = (1000 + (200 * 90 * 2) * 6 + MAX_PATH*2)*2 + 210*sizeof(*TextParts);
    mh_Heap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, nMinHeapSize, 0);
    mh_Thread = NULL; mn_ThreadID = 0; mn_ConEmuC_PID = 0; mh_ConEmuC = NULL; mh_ConEmuCInput = NULL;
	//mh_VConServerThread = NULL;
    ms_ConEmuC_Pipe[0] = 0; ms_ConEmuCInput_Pipe[0] = 0; ms_VConServer_Pipe[0] = 0;
    mh_TermEvent = CreateEvent(NULL,TRUE/*MANUAL - ������������ � ���������� �����!*/,FALSE,NULL); ResetEvent(mh_TermEvent);
    mh_ForceReadEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    mh_EndUpdateEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	mh_Sync2WindowEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	mh_ConChanged = CreateEvent(NULL,FALSE,FALSE,NULL);
	mh_CursorChanged = NULL;
	mb_FullRetrieveNeeded = FALSE;
    //mh_ReqSetSize = CreateEvent(NULL,FALSE,FALSE,NULL);
	//mh_ReqSetSizeEnd = CreateEvent(NULL,FALSE,FALSE,NULL);
    //m_ReqSetSize = MakeCoord ( 0,0 );
    mn_ActiveStatus = 0;
	isShowConsole = false;
	mb_ConsoleSelectMode = false;
	BufferHeight = 0;
	mn_ProcessCount = 0; mn_FarPID = 0;
	mh_ServerSemaphore = NULL;
	memset(mh_ServerThreads, 0, sizeof(mh_ServerThreads));
	memset(mn_ServerThreadsId, 0, sizeof(mn_ServerThreadsId));

	memset(&con, 0, sizeof(con)); //WARNING! �������� CriticalSection, ������� ����� ��������� ����� InitializeCriticalSection(&con.cs);

    //InitializeCriticalSection(&csDC); ncsTDC = 0;
    InitializeCriticalSection(&csCON); ncsTCON = 0;
	InitializeCriticalSection(&csPRC); ncsTPRC = 0;
	InitializeCriticalSection(&con.cs); con.ncsT = 0;
        
    //pVCon = this;
    //mh_ConOut = NULL; mb_ConHandleCreated = FALSE;
	mh_StdIn = NULL; mh_StdOut = NULL;
    
    mr_LeftPanel = mr_RightPanel = MakeRect(-1,-1);

	//m_dwConsoleCP = 0; m_dwConsoleOutputCP = 0; m_dwConsoleMode = 0;
	//memset(&m_sel, 0, sizeof(m_sel));
	//memset(&m_ci, 0, sizeof(m_ci));
	//memset(&m_sbi, 0, sizeof(m_sbi));

#ifdef _DEBUG
    mn_BackColorIdx = 2;
#else
    mn_BackColorIdx = 0;
#endif
    memset(&Cursor, 0, sizeof(Cursor));
    Cursor.nBlinkTime = GetCaretBlinkTime();

    TextWidth = TextHeight = Width = Height = 0;
    hDC = NULL; hBitmap = NULL;
    //hBgDc = NULL; hBgBitmap = NULL; gSet.bgBmp.X = 0; gSet.bgBmp.Y = 0;
    //hFont = NULL; hFont2 = NULL; 
    hSelectedFont = NULL; hOldFont = NULL;
    ConChar = NULL; ConAttr = NULL; ConCharX = NULL; tmpOem = NULL; TextParts = NULL;
    memset(&SelectionInfo, 0, sizeof(SelectionInfo));
    IsForceUpdate = false;
    hBrush0 = NULL; hSelectedBrush = NULL; hOldBrush = NULL;
    isEditor = false;
    memset(&csbi, 0, sizeof(csbi)); mdw_LastError = 0;
    m_LastMaxReadCnt = 0; m_LastMaxReadTick = 0;
    hConWnd = NULL; mh_GuiAttached = NULL;
    mn_LastVKeyPressed = 0;
	mn_LastConReadTick = 0;

    nSpaceCount = 1000;
    Spaces = (TCHAR*)Alloc(nSpaceCount,sizeof(TCHAR));
    for (UINT i=0; i<nSpaceCount; i++) Spaces[i]=L' ';

    hOldBrush = NULL;
    hOldFont = NULL;
    
    if (gSet.wndWidth)
        TextWidth = gSet.wndWidth;
    if (gSet.wndHeight)
        TextHeight = gSet.wndHeight;
    

    if (gSet.isShowBgImage)
        gSet.LoadImageFrom(gSet.sBgImage);


    // InitDC ����� ������������ - ������� ��� �� �������
    hDC = NULL;
    hBitmap = NULL;
    ConChar = NULL;
    ConAttr = NULL;
    ConCharX = NULL;
    tmpOem = NULL;
    TextParts = NULL;
}

CVirtualConsole::~CVirtualConsole()
{
    StopThread();
    
    MCHKHEAP
    if (hDC)
        { DeleteDC(hDC); hDC = NULL; }
    if (hBitmap)
        { DeleteObject(hBitmap); hBitmap = NULL; }
    if (ConChar)
        { Free(ConChar); ConChar = NULL; }
    if (ConAttr)
        { Free(ConAttr); ConAttr = NULL; }
    if (con.pConChar)
        { Free(con.pConChar); con.pConChar = NULL; }
    if (con.pConAttr)
        { Free(con.pConAttr); con.pConAttr = NULL; }
    if (ConCharX)
        { Free(ConCharX); ConCharX = NULL; }
    if (tmpOem)
        { Free(tmpOem); tmpOem = NULL; }
    if (TextParts)
        { Free(TextParts); TextParts = NULL; }
    if (Spaces) 
        { Free(Spaces); Spaces = NULL; nSpaceCount = 0; }

    // ���� ������ �� �����
    if (mh_Heap) {
        HeapDestroy(mh_Heap);
        mh_Heap = NULL;
    }

    //if (mb_ConHandleCreated && mh_ConOut && mh_ConOut!=INVALID_HANDLE_VALUE)
    //    CloseHandle(mh_ConOut);
    //mh_ConOut = NULL; mb_ConHandleCreated = FALSE;
	SafeCloseHandle(mh_StdIn); 
	SafeCloseHandle(mh_StdOut);

	SafeCloseHandle(mh_ConEmuC); mn_ConEmuC_PID = 0;
	SafeCloseHandle(mh_ConEmuCInput);
	
	SafeCloseHandle(mh_ServerSemaphore);
	SafeCloseHandle(mh_GuiAttached);

	//SafeCloseHandle(mh_ReqSetSize);
	//SafeCloseHandle(mh_ReqSetSizeEnd);
    
    //DeleteCriticalSection(&csDC);
    DeleteCriticalSection(&csCON);
	DeleteCriticalSection(&csPRC);
	DeleteCriticalSection(&con.cs);
}

/*HANDLE CVirtualConsole::hConIn()
{
	if (!this)
		return GetStdHandle(STD_INPUT_HANDLE);
	return mh_StdIn;
}*/

// ���� �������� ����������� Recreate �� ����� ���������� - ����������� ����� ������ � ������ Update!
/*HANDLE CVirtualConsole::hConOut(BOOL abAllowRecreate/ *=FALSE* /)
{
	// TODO: ��������� telnet'a (������������� �����?)
	if (!this)
		return NULL; //GetStdHandle(STD_OUTPUT_HANDLE);
	if (ghConWnd != hConWnd)
		return NULL;
	return mh_StdOut;
	/*
    if(mh_ConOut && mh_ConOut!=INVALID_HANDLE_VALUE && !abAllowRecreate) {
        return mh_ConOut;
    }
    
    if (gSet.isUpdConHandle)
    {
        if(mh_ConOut && mh_ConOut!=INVALID_HANDLE_VALUE && mb_ConHandleCreated)
            CloseHandle(mh_ConOut);
        mh_ConOut = NULL; mb_ConHandleCreated = FALSE;

        mh_ConOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
            0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (mh_ConOut == INVALID_HANDLE_VALUE) {
            // �������� ����� ������� ������� �����, ������ ������ ����������...
            mh_ConOut = hConOut();
            #ifdef _DEBUG
            _ASSERT(FALSE);
            #endif
        } else {
            mb_ConHandleCreated = TRUE; // OK
        }
    } else if (mh_ConOut==NULL) {
        mh_ConOut = hConOut();
        mb_ConHandleCreated = FALSE;
    }
    
    if (mh_ConOut == INVALID_HANDLE_VALUE) {
        mh_ConOut = NULL;
        DisplayLastError(_T("CVirtualConsole::hConOut fails"));
    }
    
    return mh_ConOut;
	* /
}*/

// InitDC ���������� ������ ��� ����������� ���������� (�������, �����, � �.�.) ����� ����� ����������� DC � Bitmap
bool CVirtualConsole::InitDC(bool abNoDc, bool abNoWndResize)
{
    CSection SCON(&csCON, &ncsTCON);
    //CSection SDC(&csDC, &ncsTDC);
    
    if (hDC)
        { DeleteDC(hDC); hDC = NULL; }
    if (hBitmap)
        { DeleteObject(hBitmap); hBitmap = NULL; }
    if (ConChar)
        { Free(ConChar); ConChar = NULL; }
    if (ConAttr)
        { Free(ConAttr); ConAttr = NULL; }
    if (ConCharX)
        { Free(ConCharX); ConCharX = NULL; }
    if (tmpOem)
        { Free(tmpOem); tmpOem = NULL; }
    if (TextParts)
        { Free(TextParts); TextParts = NULL; }

    //CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo())
        return false;

    IsForceUpdate = true;
    //TextWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    //TextHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	TextWidth = con.nTextWidth;
	TextHeight = con.nTextHeight;

#ifdef _DEBUG
	_ASSERT(TextHeight >= 5);
#endif

    if (!TextWidth || !TextHeight) {
        Assert(TextWidth && TextHeight);
        return false;
    }

    //if ((int)TextWidth < csbi.dwSize.X)
    //    TextWidth = csbi.dwSize.X;

    MCHKHEAP
    ConChar = (TCHAR*)Alloc((TextWidth * TextHeight * 2), sizeof(*ConChar));
    ConAttr = (WORD*)Alloc((TextWidth * TextHeight * 2), sizeof(*ConAttr));
    ConCharX = (DWORD*)Alloc((TextWidth * TextHeight), sizeof(*ConCharX));
    tmpOem = (char*)Alloc((TextWidth + 5), sizeof(*tmpOem));
    TextParts = (struct _TextParts*)Alloc((TextWidth + 2), sizeof(*TextParts));
    MCHKHEAP
    if (!ConChar || !ConAttr || !ConCharX || !tmpOem || !TextParts)
        return false;

    SelectionInfo.dwFlags = 0;

    hSelectedFont = NULL;

    // ��� ����� ����, ���� ��������� ����������� (debug) � ����� ���� ����� �� �����
    if (!abNoDc)
    {
        const HDC hScreenDC = GetDC(0);
        if (hDC = CreateCompatibleDC(hScreenDC))
        {
            /*SelectFont(gSet.mh_Font);
            TEXTMETRIC tm;
            GetTextMetrics(hDC, &tm);
            if (gSet.isForceMonospace)
                //Maximus - � Arial'� �������� MaxWidth ������� �������
                gSet.LogFont.lfWidth = gSet.FontSizeX3 ? gSet.FontSizeX3 : tm.tmMaxCharWidth;
            else
                gSet.LogFont.lfWidth = tm.tmAveCharWidth;
            gSet.LogFont.lfHeight = tm.tmHeight;

            if (ghOpWnd) // ������������� ������ ��� �������� ������ � ���������
                gSet.UpdateTTF ( (tm.tmMaxCharWidth - tm.tmAveCharWidth)>2 );*/

            Assert ( gSet.LogFont.lfWidth && gSet.LogFont.lfHeight );

            BOOL lbWasInitialized = TextWidth && TextHeight;
            // ��������� ����� ������ � ��������
            Width = TextWidth * gSet.LogFont.lfWidth;
            Height = TextHeight * gSet.LogFont.lfHeight;

            if (ghOpWnd)
                gConEmu.UpdateSizes();

            //if (!lbWasInitialized) // ���� ������� InitDC ������ ������ ������� ���������
			if (!abNoWndResize) {
				if (gConEmu.isActive(this))
					gConEmu.OnSize(-1);
			}

            hBitmap = CreateCompatibleBitmap(hScreenDC, Width, Height);
            SelectObject(hDC, hBitmap);
        }
        ReleaseDC(0, hScreenDC);

        return hBitmap!=NULL;
    }

    return true;
}

void CVirtualConsole::DumpConsole()
{
    OPENFILENAME ofn; memset(&ofn,0,sizeof(ofn));
    ofn.lStructSize=sizeof(ofn);
    ofn.hwndOwner = ghWnd;
    ofn.lpstrFilter = _T("ConEmu dumps (*.con)\0*.con\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = temp;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = _T("Dump console...");
    ofn.Flags = OFN_ENABLESIZING|OFN_NOCHANGEDIR
            | OFN_PATHMUSTEXIST|OFN_EXPLORER|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
    if (!GetSaveFileName(&ofn))
        return;
        
    HANDLE hFile = CreateFile(temp, GENERIC_WRITE, FILE_SHARE_READ,
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DisplayLastError(_T("Can't create file!"));
        return;
    }
    DWORD dw;
    LPCTSTR pszTitle = gConEmu.GetTitle();
    WriteFile(hFile, pszTitle, _tcslen(pszTitle)*sizeof(*pszTitle), &dw, NULL);
    swprintf(temp, _T("\r\nSize: %ix%i\r\n"), TextWidth, TextHeight);
    WriteFile(hFile, temp, _tcslen(temp)*sizeof(TCHAR), &dw, NULL);
    WriteFile(hFile, ConChar, TextWidth * TextHeight * 2, &dw, NULL);
    WriteFile(hFile, ConAttr, TextWidth * TextHeight * 2, &dw, NULL);
    CloseHandle(hFile);
}

/*HFONT CVirtualConsole::CreateFontIndirectMy(LOGFONT *inFont)
{
    memset(FontWidth, 0, sizeof(*FontWidth)*0x10000);
    //memset(Font2Width, 0, sizeof(*Font2Width)*0x10000);

    DeleteObject(hFont2); hFont2 = NULL;

    int width = gSet.FontSizeX2 ? gSet.FontSizeX2 : inFont->lfWidth;
    hFont2 = CreateFont(abs(inFont->lfHeight), abs(width), 0, 0, FW_NORMAL,
        0, 0, 0, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, 0, gSet.LogFont2.lfFaceName);

    return CreateFontIndirect(inFont);
}*/

// ��� ������� ����� � ��. ����. �������
//#define isCharBorder(inChar) (inChar>=0x2013 && inChar<=0x266B)
bool CVirtualConsole::isCharBorder(WCHAR inChar)
{
    if (inChar>=0x2013 && inChar<=0x266B)
        return true;
    else
        return false;
    //if (gSet.isFixFarBorders)
    //{
    //  //if (! (inChar > 0x2500 && inChar < 0x251F))
    //  if ( !(inChar > 0x2013/*En dash*/ && inChar < 0x266B/*Beamed Eighth Notes*/) /*&& inChar!=L'}'*/ )
    //      /*if (inChar != 0x2550 && inChar != 0x2502 && inChar != 0x2551 && inChar != 0x007D &&
    //      inChar != 0x25BC && inChar != 0x2593 && inChar != 0x2591 && inChar != 0x25B2 &&
    //      inChar != 0x2562 && inChar != 0x255F && inChar != 0x255A && inChar != 0x255D &&
    //      inChar != 0x2554 && inChar != 0x2557 && inChar != 0x2500 && inChar != 0x2534 && inChar != 0x2564) // 0x2520*/
    //      return false;
    //  else
    //      return true;
    //}
    //else
    //{
    //  if (inChar < 0x01F1 || inChar > 0x0400 && inChar < 0x045F || inChar > 0x2012 && inChar < 0x203D || /*? - not sure that optimal*/ inChar > 0x2019 && inChar < 0x2303 || inChar > 0x24FF && inChar < 0x266C)
    //      return false;
    //  else
    //      return true;
    //}
}

// � ��� ������ "��������" �������, � ������� ���� ����� (���� �� ���������) ������������ ����� + �������/���������
bool CVirtualConsole::isCharBorderVertical(WCHAR inChar)
{
    //if (inChar>=0x2502 && inChar<=0x25C4 && inChar!=0x2550)
    if (inChar==0x2502 || inChar==0x2503 || inChar==0x2506 || inChar==0x2507
        || (inChar>=0x250A && inChar<=0x254B) || inChar==0x254E || inChar==0x254F
        || (inChar>=0x2551 && inChar<=0x25C5)) // �� ������ �������� Arial Unicode MS
        return true;
    else
        return false;
}

void CVirtualConsole::BlitPictureTo(int inX, int inY, int inWidth, int inHeight)
{
    // � ������, ����� ����� ������������?
    if (gSet.bgBmp.X>inX && gSet.bgBmp.Y>inY)
        BitBlt(hDC, inX, inY, inWidth, inHeight, gSet.hBgDc, inX, inY, SRCCOPY);
    if (gSet.bgBmp.X<(inX+inWidth) || gSet.bgBmp.Y<(inY+inHeight))
    {
        if (hBrush0==NULL) {
            hBrush0 = CreateSolidBrush(gSet.Colors[0]);
            SelectBrush(hBrush0);
        }

        RECT rect = {max(inX,gSet.bgBmp.X), inY, inX+inWidth, inY+inHeight};
        if (!IsRectEmpty(&rect))
            FillRect(hDC, &rect, hBrush0);

        if (gSet.bgBmp.X>inX) {
            rect.left = inX; rect.top = max(inY,gSet.bgBmp.Y); rect.right = gSet.bgBmp.X;
            if (!IsRectEmpty(&rect))
                FillRect(hDC, &rect, hBrush0);
        }

        //DeleteObject(hBrush);
    }
}

void CVirtualConsole::SelectFont(HFONT hNew)
{
    if (!hNew) {
        if (hOldFont)
            SelectObject(hDC, hOldFont);
        hOldFont = NULL;
        hSelectedFont = NULL;
    } else
    if (hSelectedFont != hNew)
    {
        hSelectedFont = (HFONT)SelectObject(hDC, hNew);
        if (!hOldFont)
            hOldFont = hSelectedFont;
        hSelectedFont = hNew;
    }
}

void CVirtualConsole::SelectBrush(HBRUSH hNew)
{
    if (!hNew) {
        if (hOldBrush)
            SelectObject(hDC, hOldBrush);
        hOldBrush = NULL;
    } else
    if (hSelectedBrush != hNew)
    {
        hSelectedBrush = (HBRUSH)SelectObject(hDC, hNew);
        if (!hOldBrush)
            hOldBrush = hSelectedBrush;
        hSelectedBrush = hNew;
    }
}

bool CVirtualConsole::CheckSelection(const CONSOLE_SELECTION_INFO& select, SHORT row, SHORT col)
{
    if ((select.dwFlags & CONSOLE_SELECTION_NOT_EMPTY) == 0)
        return false;
    if (row < select.srSelection.Top || row > select.srSelection.Bottom)
        return false;
    if (col < select.srSelection.Left || col > select.srSelection.Right)
        return false;
    return true;
}

#ifdef MSGLOGGER
class DcDebug {
public:
    DcDebug(HDC* ahDcVar, HDC* ahPaintDC) {
        mb_Attached=FALSE; mh_OrigDc=NULL; mh_DcVar=NULL; mh_Dc=NULL;
        if (!ahDcVar || !ahPaintDC) return;
        mh_DcVar = ahDcVar;
        mh_OrigDc = *ahDcVar;
        mh_Dc = *ahPaintDC;
        *mh_DcVar = mh_Dc;
    };
    ~DcDebug() {
        if (mb_Attached && mh_DcVar) {
            mb_Attached = FALSE;
            *mh_DcVar = mh_OrigDc;
        }
    };
protected:
    BOOL mb_Attached;
    HDC mh_OrigDc, mh_Dc;
    HDC* mh_DcVar;
};
#endif

// ���������� true, ���� ��������� �������� ������������ ������
bool CVirtualConsole::GetCharAttr(TCHAR ch, WORD atr, TCHAR& rch, BYTE& foreColorNum, BYTE& backColorNum)
{
    bool bChanged = false;
    foreColorNum = atr & 0x0F;
    backColorNum = atr >> 4 & 0x0F;
    rch = ch; // �� ���������!
    if (isEditor && gSet.isVisualizer && ch==L' ' &&
        (backColorNum==gSet.nVizTab || backColorNum==gSet.nVizEOL || backColorNum==gSet.nVizEOF))
    {
        if (backColorNum==gSet.nVizTab)
            rch = gSet.cVizTab; else
        if (backColorNum==gSet.nVizEOL)
            rch = gSet.cVizEOL; else
        if (backColorNum==gSet.nVizEOF)
            rch = gSet.cVizEOF;
        backColorNum = gSet.nVizNormal;
        foreColorNum = gSet.nVizFore;
        bChanged = true;
    } else
    if (gSet.isExtendColors) {
        if (backColorNum==gSet.nExtendColor) {
            backColorNum = attrBackLast;
            foreColorNum += 0x10;
        } else {
            attrBackLast = backColorNum;
        }
    }
    return bChanged;
}

// ���������� ������ �������, ��������� FixBorders
WORD CVirtualConsole::CharWidth(TCHAR ch)
{
    if (gSet.isForceMonospace)
        return gSet.LogFont.lfWidth;

    WORD nWidth = gSet.LogFont.lfWidth;
    bool isBorder = false; //, isVBorder = false;

    if (gSet.isFixFarBorders) {
        isBorder = isCharBorder(ch);
        //if (isBorder) {
        //  isVBorder = ch == 0x2551 /*Light Vertical*/ || ch == 0x2502 /*Double Vertical*/;
        //}
    }

    //if (isBorder) {
        //if (!Font2Width[ch]) {
        //  if (!isVBorder) {
        //      Font2Width[ch] = gSet.LogFont.lfWidth;
        //  } else {
        //      SelectFont(hFont2);
        //      SIZE sz;
        //      if (GetTextExtentPoint32(hDC, &ch, 1, &sz) && sz.cx)
        //          Font2Width[ch] = sz.cx;
        //      else
        //          Font2Width[ch] = gSet.LogFont2.lfWidth;
        //  }
        //}
        //nWidth = Font2Width[ch];
    //} else {
    if (!isBorder) {
        if (!gSet.FontWidth[ch]) {
            SelectFont(gSet.mh_Font);
            SIZE sz;
            if (GetTextExtentPoint32(hDC, &ch, 1, &sz) && sz.cx)
                gSet.FontWidth[ch] = sz.cx;
            else
                gSet.FontWidth[ch] = nWidth;
        }
        nWidth = gSet.FontWidth[ch];
    }
    if (!nWidth) nWidth = 1; // �� ������ ������, ����� ������� �� 0 �� ��������
    return nWidth;
}

bool CVirtualConsole::CheckChangedTextAttr()
{
#ifdef MSGLOGGER
    COORD ch;
    if (textChanged) {
        for (UINT i=0,j=TextLen; i<TextLen; i++,j++) {
            if (ConChar[i] != ConChar[j]) {
                ch.Y = i % TextWidth;
                ch.X = i - TextWidth * ch.Y;
                break;
            }
        }
    }
    if (attrChanged) {
        for (UINT i=0,j=TextLen; i<TextLen; i++,j++) {
            if (ConAttr[i] != ConAttr[j]) {
                ch.Y = i % TextWidth;
                ch.X = i - TextWidth * ch.Y;
                break;
            }
        }
    }
#endif

    textChanged = 0!=memcmp(ConChar + TextLen, ConChar, TextLen * sizeof(*ConChar));
    attrChanged = 0!=memcmp(ConAttr + TextLen, ConAttr, TextLen * sizeof(*ConAttr));

    return textChanged || attrChanged;
}

bool CVirtualConsole::Update(bool isForce, HDC *ahDc)
{
    #pragma message (__FILE__ "(" STRING(__LINE__) "): TODO: CVirtualConsole::Update - ����� ������ ���� hConWnd==ghConWnd?")
    if (!hConWnd)
        return false;

	if (!ahDc && GetCurrentThreadId() != mn_ThreadID) {
		SetEvent(mh_ForceReadEvent);
		WaitForSingleObject(mh_EndUpdateEvent, 1000);
		//gConEmu.InvalidateAll(); -- ����� �� All??? Update � ��� Invalidate �����
		return false;
	}

	//RetrieveConsoleInfo();

    #ifdef MSGLOGGER
    DcDebug dbg(&hDC, ahDc); // ��� ������� - ������ ����� �� ������� ����
    #endif

    // ����������� ����� Output'� ��� �������������
    /*if (!hConOut(TRUE)) {
        return false;
    }*/

    MCHKHEAP
    bool lRes = false;
    
    CSection SCON(&csCON, &ncsTCON);
    //CSection SDC(&csDC, &ncsTDC);

    if (!GetConsoleScreenBufferInfo()) {
        DisplayLastError(_T("Update:GetConsoleScreenBufferInfo"));
        return lRes;
    }

    // start timer before ReadConsoleOutput* calls, they do take time
    //gSet.Performance(tPerfRead, FALSE);

    //if (gbNoDblBuffer) isForce = TRUE; // Debug, dblbuffer

    //------------------------------------------------------------------------
    ///| Read console output and cursor info... |/////////////////////////////
    //------------------------------------------------------------------------
    if (!UpdatePrepare(isForce, ahDc)) {
        gConEmu.DnDstep(_T("DC initialization failed!"));
        return false;
    }
    
    //gSet.Performance(tPerfRead, TRUE);

    //gSet.Performance(tPerfRender, FALSE);

    //------------------------------------------------------------------------
    ///| Drawing text (if there were changes in console) |////////////////////
    //------------------------------------------------------------------------
    bool updateText, updateCursor;
    if (isForce)
    {
        updateText = updateCursor = attrChanged = textChanged = true;
    }
    else
    {

        // Do we have to update changed text?
        updateText = doSelect || CheckChangedTextAttr();
            //(textChanged = 0!=memcmp(ConChar + TextLen, ConChar, TextLen * sizeof(TCHAR))) || 
            //(attrChanged = 0!=memcmp(ConAttr + TextLen, ConAttr, TextLen * 2));
        
        // Do we have to update text under changed cursor?
        // Important: check both 'cinf.bVisible' and 'Cursor.isVisible',
        // because console may have cursor hidden and its position changed -
        // in this case last visible cursor remains shown at its old place.
        // Also, don't check foreground window here for the same reasons.
        // If position is the same then check the cursor becomes hidden.
        if (Cursor.x != csbi.dwCursorPosition.X || Cursor.y != csbi.dwCursorPosition.Y)
            // ��������� �������. ��������� ���� ������ �����
            updateCursor = cinf.bVisible || Cursor.isVisible || Cursor.isVisiblePrevFromInfo;
        else
            updateCursor = Cursor.isVisiblePrevFromInfo && !cinf.bVisible;
    }
    
    gSet.Performance(tPerfRender, FALSE);

    if (updateText /*|| updateCursor*/)
    {
        lRes = true;

        //------------------------------------------------------------------------
        ///| Drawing modified text |//////////////////////////////////////////////
        //------------------------------------------------------------------------
        UpdateText(isForce, updateText, updateCursor);


        //MCHKHEAP
        //------------------------------------------------------------------------
        ///| Now, store data for further comparison |/////////////////////////////
        //------------------------------------------------------------------------
        //if (updateText)
        {
            memcpy(ConChar + TextLen, ConChar, TextLen * sizeof(TCHAR));
            memcpy(ConAttr + TextLen, ConAttr, TextLen * 2);
        }
    }

    //MCHKHEAP
    //------------------------------------------------------------------------
    ///| Drawing cursor |/////////////////////////////////////////////////////
    //------------------------------------------------------------------------
    UpdateCursor(lRes);
    
    MCHKHEAP
    
    //SDC.Leave();
    SCON.Leave();

    gSet.Performance(tPerfRender, TRUE);

    /* ***************************************** */
    /*       Finalization, release objects       */
    /* ***************************************** */
    SelectBrush(NULL);
    if (hBrush0) {
        DeleteObject(hBrush0); hBrush0=NULL;
    }
    SelectFont(NULL);
    MCHKHEAP
    return lRes;
}

bool CVirtualConsole::UpdatePrepare(bool isForce, HDC *ahDc)
{
    CSection S(&csCON, &ncsTCON);
    
    attrBackLast = 0;
    isEditor = gConEmu.isEditor();
    isFilePanel = gConEmu.isFilePanel(true);

    //winSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1; winSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    //if (winSize.X < csbi.dwSize.X)
    //    winSize.X = csbi.dwSize.X;
	winSize = MakeCoord(con.nTextWidth,con.nTextHeight);

    //csbi.dwCursorPosition.X -= csbi.srWindow.Left; -- �������������� ��������� ������������!
    csbi.dwCursorPosition.Y -= csbi.srWindow.Top;
    isCursorValid =
        csbi.dwCursorPosition.X >= 0 && csbi.dwCursorPosition.Y >= 0 &&
        csbi.dwCursorPosition.X < winSize.X && csbi.dwCursorPosition.Y < winSize.Y;

    // ������ �������������, ��� ����� �������
    if (isForce || !ConChar || TextWidth != winSize.X || TextHeight != winSize.Y) {
        if (!InitDC(ahDc!=NULL, false))
            return false;
    }

    // use and reset additional force flag
    if (IsForceUpdate)
    {
        isForce = IsForceUpdate;
        IsForceUpdate = false;
    }
    if (ahDc)
        isForce = true;

    drawImage = gSet.isShowBgImage && gSet.isBackgroundImageValid;
    TextLen = TextWidth * TextHeight;
    coord.X = csbi.srWindow.Left; coord.Y = csbi.srWindow.Top;

	// ����������� ������ �� ��������� ������� � ConAttr/ConChar
	CSection csData(&con.cs, &con.ncsT);
	memmove(ConChar, con.pConChar, TextLen*2);
	memmove(ConAttr, con.pConAttr, TextLen*2);
	csData.Leave();

    MCHKHEAP
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
    /*DWORD nbActuallyRead;
    if (!ReadConsoleOutputAttribute(hConOut(), ConAttr, TextLen, coord, &nbActuallyRead) ||
        !ReadConsoleOutputCharacter(hConOut(), ConChar, TextLen, coord, &nbActuallyRead))
    {
        WORD* ConAttrNow = ConAttr;
        TCHAR* ConCharNow = ConChar;
        for(int y = 0; y < (int)TextHeight; ++y)
        {
            ReadConsoleOutputAttribute(hConOut(), ConAttrNow, TextWidth, coord, &nbActuallyRead);
            ReadConsoleOutputCharacter(hConOut(), ConCharNow, TextWidth, coord, &nbActuallyRead);
            ConAttrNow += TextWidth;
            ConCharNow += TextWidth;
            ++coord.Y;
        }
    }*/
    
    MCHKHEAP
    
    // ���������� ���������� �������
    mr_LeftPanel = mr_RightPanel = MakeRect(-1,-1);
    if (gConEmu.isFar() && TextHeight >= 7 && TextWidth >= 28)
    {
		uint nY = 0;
		if (ConChar[0] == L' ')
			nY ++; // ������ �����, ������ ������ - ����
		else if (ConChar[0] == L'R' && ConChar[1] == L' ')
			nY ++; // ������ �����, ������ ������ - ����, ��� ���������� ������ �������
			
		uint nIdx = nY*TextWidth;
		
		if (( (ConChar[nIdx] == L'[' && (ConChar[nIdx+1]>=L'0' && ConChar[nIdx+1]<=L'9')) // ������� ��������� ����������/��������
		      || (ConChar[nIdx] == 0x2554 && ConChar[nIdx+1] == 0x2550) // ���.���� ���, ������ �����
		    ) && ConChar[nIdx+TextWidth] == 0x2551)
		{
			LPCWSTR pszCenter = ConChar + nIdx;
			LPCWSTR pszLine = ConChar + nIdx;
			uint nCenter = 0;
			while ( (pszCenter = wcsstr(pszCenter+1, L"\x2557\x2554")) != NULL ) {
				nCenter = pszCenter - pszLine;
				if (ConChar[nIdx+TextWidth+nCenter] == 0x2551 && ConChar[nIdx+TextWidth+nCenter+1] == 0x2551) {
					break; // �����
				}
			}
			
			uint nBottom = TextHeight - 1;
			while (nBottom > 4) {
				if (ConChar[TextWidth*nBottom] == 0x255A && ConChar[TextWidth*(nBottom-1)] == 0x2551)
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
				mr_RightPanel.right = TextWidth - 2;
				mr_RightPanel.bottom = mr_LeftPanel.bottom;
			}
		}
	}

    // get cursor info
    GetConsoleCursorInfo(/*hConOut(),*/ &cinf);

    // get selection info in buffer mode
    
    doSelect = BufferHeight > 0;
    if (doSelect)
    {
        select1 = SelectionInfo;
        GetConsoleSelectionInfo(&select2);
        select2.srSelection.Top -= csbi.srWindow.Top;
        select2.srSelection.Bottom -= csbi.srWindow.Top;
        SelectionInfo = select2;
        doSelect = (select1.dwFlags & CONSOLE_SELECTION_NOT_EMPTY) || (select2.dwFlags & CONSOLE_SELECTION_NOT_EMPTY);
        if (doSelect)
        {
            if (select1.dwFlags == select2.dwFlags &&
                select1.srSelection.Top == select2.srSelection.Top &&
                select1.srSelection.Left == select2.srSelection.Left &&
                select1.srSelection.Right == select2.srSelection.Right &&
                select1.srSelection.Bottom == select2.srSelection.Bottom)
            {
                doSelect = false;
            }
        }
    }

    MCHKHEAP
    return true;
}

enum CVirtualConsole::_PartType CVirtualConsole::GetCharType(TCHAR ch)
{
    enum _PartType cType = pNull;

    if (ch == L' ')
        cType = pSpace;
    //else if (ch == L'_')
    //  cType = pUnderscore;
    else if (isCharBorder(ch)) {
        if (isCharBorderVertical(ch))
            cType = pVBorder;
        else
            cType = pBorder;
    }
    else if (isFilePanel && ch == L'}')
        cType = pRBracket;
    else
        cType = pText;

    return cType;
}

// row - 0-based
void CVirtualConsole::ParseLine(int row, TCHAR *ConCharLine, WORD *ConAttrLine)
{
    UINT idx = 0;
    struct _TextParts *pStart=TextParts, *pEnd=TextParts;
    enum _PartType cType1, cType2;
    UINT i1=0, i2=0;
    
    pEnd->partType = pNull; // ����� ��������� ������
    
    TCHAR ch1, ch2;
    BYTE af1, ab1, af2, ab2;
    DWORD pixels;
    while (i1<TextWidth)
    {
        GetCharAttr(ConCharLine[i1], ConAttrLine[i1], ch1, af1, ab1);
        cType1 = GetCharType(ch1);
        if (cType1 == pRBracket) {
            if (!(row>=2 && isCharBorderVertical(ConChar[TextWidth+i1]))
                && (((UINT)row)<=(TextHeight-4)))
                cType1 = pText;
        }
        pixels = CharWidth(ch1);

        i2 = i1+1;
        // � ������ Force Monospace ��������� ���� �� ������ �������
        if (!gSet.isForceMonospace && i2 < TextWidth && 
            (cType1 != pVBorder && cType1 != pRBracket))
        {
            GetCharAttr(ConCharLine[i2], ConAttrLine[i2], ch2, af2, ab2);
            // �������� ���� �������� � ������������ �������
            while (i2 < TextWidth && af2 == af1 && ab2 == ab1) {
                // ���� ������ ���������� �� �������

                cType2 = GetCharType(ch2);
                if ((ch2 = ConCharLine[i2]) != ch1) {
                    if (cType2 == pRBracket) {
                        if (!(row>=2 && isCharBorderVertical(ConChar[TextWidth+i2]))
                            && (((UINT)row)<=(TextHeight-4)))
                            cType2 = pText;
                    }

                    // � �� ������ �� ������ ������
                    if (cType2 != cType1)
                        break; // �� ��������� �����
                }
                pixels += CharWidth(ch2); // �������� ������ ������� � ��������
                i2++; // ��������� ������
                GetCharAttr(ConCharLine[i2], ConAttrLine[i2], ch2, af2, ab2);
                if (cType2 == pRBracket) {
                    if (!(row>=2 && isCharBorderVertical(ConChar[TextWidth+i2]))
                        && (((UINT)row)<=(TextHeight-4)))
                        cType2 = pText;
                }
            }
        }

        // ��� ������� ������ ����� ��������, ���� ����� pText,pSpace,pText �� pSpace,pText �������� � ������ pText
        if (cType1 == pText && (pEnd - pStart) >= 2) {
            if (pEnd[-1].partType == pSpace && pEnd[-2].partType == pText &&
                pEnd[-1].attrBack == ab1 && pEnd[-1].attrFore == af1 &&
                pEnd[-2].attrBack == ab1 && pEnd[-2].attrFore == af1
                )
            {   
                pEnd -= 2;
                pEnd->i2 = i2 - 1;
                pEnd->iwidth = i2 - pEnd->i1;
                pEnd->width += pEnd[1].width + pixels;
                pEnd ++;
                i1 = i2;
                continue;
            }
        }
        pEnd->i1 = i1; pEnd->i2 = i2 - 1; // ����� "�������"
        pEnd->partType = cType1;
        pEnd->attrBack = ab1; pEnd->attrFore = af1;
        pEnd->iwidth = i2 - i1;
        pEnd->width = pixels;
        if (gSet.isForceMonospace ||
            (gSet.isTTF && (cType1 == pVBorder || cType1 == pRBracket)))
        {
            pEnd->x1 = i1 * gSet.LogFont.lfWidth;
        } else {
            pEnd->x1 = -1;
        }

        pEnd ++; // ������ �� ����� ���� ������ ���������� �������� � ������, ��� ��� � ������������ ��� ��
        i1 = i2;
    }
    // ���� �������� ����� ������, �����, ���� ������ �� ������ - ������� pDummy
    pEnd->partType = pNull;
}

void CVirtualConsole::UpdateText(bool isForce, bool updateText, bool updateCursor)
{
    if (!updateText)
        return;

    SelectFont(gSet.mh_Font);

    // pointers
    TCHAR* ConCharLine;
    WORD* ConAttrLine;
    DWORD* ConCharXLine;
    // counters
    int pos, row;
    {
        int i;
        if (updateText)
        {
            i = 0; //TextLen - TextWidth; // TextLen = TextWidth/*80*/ * TextHeight/*25*/;
            pos = 0; //Height - gSet.LogFont.lfHeight; // Height = TextHeight * gSet.LogFont.lfHeight;
            row = 0; //TextHeight - 1;
        }
        else
        { // �� ����, ���� ������ �� �������
            i = TextWidth * Cursor.y;
            pos = gSet.LogFont.lfHeight * Cursor.y;
            row = Cursor.y;
        }
        ConCharLine = ConChar + i;
        ConAttrLine = ConAttr + i;
        ConCharXLine = ConCharX + i;
    }
    int nMaxPos = Height - gSet.LogFont.lfHeight;

    if (/*gSet.isForceMonospace ||*/ !drawImage)
        SetBkMode(hDC, OPAQUE);
    else
        SetBkMode(hDC, TRANSPARENT);

    // rows
    //TODO: � ����� � isForceMonospace ������������� �������������� ���?
    const bool skipNotChanged = !isForce /*&& !gSet.isForceMonospace*/;
    for (; pos <= nMaxPos; 
        ConCharLine += TextWidth, ConAttrLine += TextWidth, ConCharXLine += TextWidth,
        pos += gSet.LogFont.lfHeight, row++)
    {
        // the line
        const WORD* const ConAttrLine2 = ConAttrLine + TextLen;
        const TCHAR* const ConCharLine2 = ConCharLine + TextLen;

        // skip not changed symbols except the old cursor or selection
        int j = 0, end = TextWidth;
        if (skipNotChanged)
        {
            // *) Skip not changed tail symbols.
            while(--end >= 0 && ConCharLine[end] == ConCharLine2[end] && ConAttrLine[end] == ConAttrLine2[end])
            {
                if (updateCursor && row == Cursor.y && end == Cursor.x)
                    break;
                if (doSelect)
                {
                    if (CheckSelection(select1, row, end))
                        break;
                    if (CheckSelection(select2, row, end))
                        break;
                }
            }
            if (end < j)
                continue;
            ++end;

            // *) Skip not changed head symbols.
            while(j < end && ConCharLine[j] == ConCharLine2[j] && ConAttrLine[j] == ConAttrLine2[j])
            {
                if (updateCursor && row == Cursor.y && j == Cursor.x)
                    break;
                if (doSelect)
                {
                    if (CheckSelection(select1, row, j))
                        break;
                    if (CheckSelection(select2, row, j))
                        break;
                }
                ++j;
            }
            if (j >= end)
                continue;
        }
        
        if (Cursor.isVisiblePrev && row == Cursor.y
            && (j <= Cursor.x && Cursor.x <= end))
            Cursor.isVisiblePrev = false;

        // *) Now draw as much as possible in a row even if some symbols are not changed.
        // More calls for the sake of fewer symbols is slower, e.g. in panel status lines.
        int j2=j+1;
        for (; j < end; j = j2)
        {
            const WORD attr = ConAttrLine[j];
            WCHAR c = ConCharLine[j];
            BYTE attrFore, attrBack;
            bool isUnicode = isCharBorder(c/*ConCharLine[j]*/);
            bool lbS1 = false, lbS2 = false;
            int nS11 = 0, nS12 = 0, nS21 = 0, nS22 = 0;

            if (GetCharAttr(c, attr, c, attrFore, attrBack))
                isUnicode = true;

            MCHKHEAP
            // ������������� ���������� �������� � �����
            if (gSet.isTTF && (c==0x2550 || c==0x2500)) // 'Box light horizontal' & 'Box double horizontal' - ��� ������
            {
                lbS1 = true; nS11 = nS12 = j;
                while ((nS12 < end) && (ConCharLine[nS12+1] == c))
                    nS12 ++;
                // ��������� ������� ���� �� �������� ����� ������
                if (nS12<end) {
                    nS21 = nS12+1; // ��� ������ ���� �� c 
                    // ���� ������ "��������" ������
                    while ((nS21<end) && (ConCharLine[nS21] != c) && !isCharBorder(ConCharLine[nS21]))
                        nS21 ++;
                    if (nS21<end && ConCharLine[nS21]==c) {
                        lbS2 = true; nS22 = nS21;
                        while ((nS22 < end) && (ConCharLine[nS22+1] == c))
                            nS22 ++;
                    }
                }
            } MCHKHEAP
            // � ��� ��� �������� - ����� �� ����� ������
            /*else if (c==L' ' && j<end && ConCharLine[j+1]==L' ')
            {
                lbS1 = true; nS11 = nS12 = j;
                while ((nS12 < end) && (ConCharLine[nS12+1] == c))
                    nS12 ++;
            }*/

            // �������� ��� ���� ����� ������� � GetCharAttr?
            if (doSelect && CheckSelection(select2, row, j))
            {
                WORD tmp = attrFore;
                attrFore = attrBack;
                attrBack = tmp;
            }

            SetTextColor(hDC, gSet.Colors[attrFore]);

            // ������������� ��������� ������������ �����
            if (gSet.isTTF && j)
            {
                MCHKHEAP
                DWORD nPrevX = ConCharXLine[j-1];
                if (isCharBorderVertical(c)) {
					//2009-04-21 ���� (j-1) * gSet.LogFont.lfWidth;
                    ConCharXLine[j-1] = j * gSet.LogFont.lfWidth;
                } else if (isFilePanel && c==L'}') {
                    // �������� } ��� �������� ����� ������ �������� �� ������� �������...
                    // ���� ������ ��� ����� �� ��� �� ������� ����� (��� ��� �� '}')
                    // ������������ �������
                    if ((row>=2 && isCharBorderVertical(ConChar[TextWidth+j]))
                        && (((UINT)row)<=(TextHeight-4)))
						//2009-04-21 ���� (j-1) * gSet.LogFont.lfWidth;
						ConCharXLine[j-1] = j * gSet.LogFont.lfWidth;
                    //row = TextHeight - 1;
                } MCHKHEAP
                if (nPrevX < ConCharXLine[j-1]) {
                    // ��������� ���������� ����������� �������. ��������� ���-��?

                    RECT rect;
                    rect.left = nPrevX;
                    rect.top = pos;
                    rect.right = ConCharXLine[j-1];
                    rect.bottom = rect.top + gSet.LogFont.lfHeight;

                    if (gbNoDblBuffer) GdiFlush();
                    if (! (drawImage && (attrBack) < 2)) {
                        SetBkColor(hDC, gSet.Colors[attrBack]);
                        int nCnt = ((rect.right - rect.left) / CharWidth(L' '))+1;

                        UINT nFlags = ETO_CLIPPED; // || ETO_OPAQUE;
                        ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect,
                            Spaces, min(nSpaceCount, nCnt), 0);

                    } else if (drawImage) {
                        BlitPictureTo(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
                    } MCHKHEAP
                    if (gbNoDblBuffer) GdiFlush();
                }
            }

            ConCharXLine[j] = (j ? ConCharXLine[j-1] : 0)+CharWidth(c);
            MCHKHEAP


            if (gSet.isForceMonospace)
            {
                MCHKHEAP
                SetBkColor(hDC, gSet.Colors[attrBack]);

                j2 = j + 1;

                if (gSet.isFixFarBorders) {
                    if (!isUnicode)
                        SelectFont(gSet.mh_Font);
                    else if (isUnicode)
                        SelectFont(gSet.mh_Font2);
                }


                RECT rect;
                if (!gSet.isTTF)
                    rect = MakeRect(j * gSet.LogFont.lfWidth, pos, j2 * gSet.LogFont.lfWidth, pos + gSet.LogFont.lfHeight);
                else {
                    rect.left = j ? ConCharXLine[j-1] : 0;
                    rect.top = pos;
                    rect.right = (TextWidth>(UINT)j2) ? ConCharXLine[j2-1] : Width;
                    rect.bottom = rect.top + gSet.LogFont.lfHeight;
                }
                UINT nFlags = ETO_CLIPPED | ((drawImage && (attrBack) < 2) ? 0 : ETO_OPAQUE);
                int nShift = 0;

                MCHKHEAP
                if (c != 0x20 && !isUnicode) {
                    ABC abc;
                    //This function succeeds only with TrueType fonts
                    if (GetCharABCWidths(hDC, c, c, &abc))
                    {
                        
                        if (abc.abcA<0) {
                            // ����� ������ �������� ������� �� ����������?
                            nShift = -abc.abcA;
                        } else if (abc.abcA<(((int)gSet.LogFont.lfWidth-(int)abc.abcB-1)/2)) {
                            // ������ I, i, � ��. ����� ������ - ������ ����������
                            nShift = ((gSet.LogFont.lfWidth-abc.abcB)/2)-abc.abcA;
                        }
                    }
                }

                MCHKHEAP
                if (! (drawImage && (attrBack) < 2)) {
                    SetBkColor(hDC, gSet.Colors[attrBack]);
                    //TODO: ���� ��������������� � ����-�� ���������...
                    if (nShift>0) ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect, L" ", 1, 0);
                } else if (drawImage) {
                    BlitPictureTo(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
                }

                if (nShift>0) {
                    rect.left += nShift; rect.right += nShift;
                }

                if (gbNoDblBuffer) GdiFlush();
                ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect, &c, 1, 0);
                if (gbNoDblBuffer) GdiFlush();
                MCHKHEAP

            }
            else if (gSet.isTTF && c==L' ')
            {
                j2 = j + 1; MCHKHEAP
                if (!doSelect) // doSelect ���������������� ������ ��� BufferHeight>0
                {
                    TCHAR ch;
                    while(j2 < end && ConAttrLine[j2] == attr && (ch=ConCharLine[j2]) == L' ')
                    {
                        ConCharXLine[j2] = (j2 ? ConCharXLine[j2-1] : 0)+CharWidth(ch);
                        j2++;
                    } MCHKHEAP
                    if (j2>=(int)TextWidth || isCharBorderVertical(ConCharLine[j2])) //ch ����� ���� �� ���������������
                    {
                        ConCharXLine[j2-1] = (j2>=(int)TextWidth) ? Width : (j2) * gSet.LogFont.lfWidth; // ��� ��� [j] ������ ����?
                        MCHKHEAP
                        DWORD n1 = ConCharXLine[j];
                        DWORD n3 = j2-j; // ���-�� ��������
                        DWORD n2 = ConCharXLine[j2-1] - n1; // ���������� �� ������� ������
                        MCHKHEAP

                        for (int k=j+1; k<(j2-1); k++) {
                            ConCharXLine[k] = n1 + (n3 ? klMulDivU32(k-j, n2, n3) : 0);
                            MCHKHEAP
                        }
                    } MCHKHEAP
                }
                if (gSet.isFixFarBorders)
                    SelectFont(gSet.mh_Font);
            }
            else if (!isUnicode)
            {
                j2 = j + 1; MCHKHEAP
                if (!doSelect) // doSelect ���������������� ������ ��� BufferHeight>0
                {
                    #ifndef DRAWEACHCHAR
                    // ���� ����� �� ������ - � ���������������� ������� ����� ����� �������� ���� �� ������
                    TCHAR ch;
                    while(j2 < end && ConAttrLine[j2] == attr && 
                        !isCharBorder(ch = ConCharLine[j2]) 
                        && (!gSet.isTTF || !isFilePanel || (ch != L'}' && ch!=L' '))) // ������������� ���� � ��������
                    {
                        ConCharXLine[j2] = (j2 ? ConCharXLine[j2-1] : 0)+CharWidth(ch);
                        j2++;
                    }
                    #endif
                }
                if (gSet.isFixFarBorders)
                    SelectFont(gSet.mh_Font);
                MCHKHEAP
            }
            else //Border and specials
            {
                j2 = j + 1; MCHKHEAP
                if (!doSelect) // doSelect ���������������� ������ ��� BufferHeight>0
                {
                    if (!gSet.isFixFarBorders)
                    {
                        TCHAR ch;
                        while(j2 < end && ConAttrLine[j2] == attr && isCharBorder(ch = ConCharLine[j2]))
                        {
                            ConCharXLine[j2] = (j2 ? ConCharXLine[j2-1] : 0)+CharWidth(ch);
                            j2++;
                        }
                    }
                    else
                    {
                        TCHAR ch;
                        while(j2 < end && ConAttrLine[j2] == attr && 
                            isCharBorder(ch = ConCharLine[j2]) && ch == ConCharLine[j2+1])
                        {
                            ConCharXLine[j2] = (j2 ? ConCharXLine[j2-1] : 0)+CharWidth(ch);
                            j2++;
                        }
                    }
                }
                if (gSet.isFixFarBorders)
                    SelectFont(gSet.mh_Font2);
                MCHKHEAP
            }

            if (!gSet.isForceMonospace)
            {
                RECT rect;
                if (!gSet.isTTF)
                    rect = MakeRect(j * gSet.LogFont.lfWidth, pos, j2 * gSet.LogFont.lfWidth, pos + gSet.LogFont.lfHeight);
                else {
                    rect.left = j ? ConCharXLine[j-1] : 0;
                    rect.top = pos;
                    rect.right = (TextWidth>(UINT)j2) ? ConCharXLine[j2-1] : Width;
                    rect.bottom = rect.top + gSet.LogFont.lfHeight;
                }

                MCHKHEAP
                if (! (drawImage && (attrBack) < 2))
                    SetBkColor(hDC, gSet.Colors[attrBack]);
                else if (drawImage)
                    BlitPictureTo(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);

                UINT nFlags = ETO_CLIPPED | ((drawImage && (attrBack) < 2) ? 0 : ETO_OPAQUE);

                MCHKHEAP
                if (gbNoDblBuffer) GdiFlush();
                if (gSet.isTTF && c == ' ') {
                    int nCnt = ((rect.right - rect.left) / CharWidth(L' '))+1;
                    ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect,
                        Spaces, nCnt, 0);
                } else
                if (gSet.LogFont.lfCharSet == OEM_CHARSET && !isUnicode)
                {
                    WideCharToMultiByte(CP_OEMCP, 0, ConCharLine + j, j2 - j, tmpOem, TextWidth+4, 0, 0);
                    ExtTextOutA(hDC, rect.left, rect.top, nFlags,
                        &rect, tmpOem, j2 - j, 0);
                }
                else
                {
                    if ((j2-j)==1) // support visualizer change
                    ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect,
                        &c/*ConCharLine + j*/, 1, 0);
                    else
                    ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect,
                        ConCharLine + j, j2 - j, 0);
                }
                if (gbNoDblBuffer) GdiFlush();
                MCHKHEAP
            }
        
            // stop if all is done
            if (!updateText)
                goto done;

            // skip next not changed symbols again
            if (skipNotChanged)
            {
                MCHKHEAP
                // skip the same except the old cursor
                while(j2 < end && ConCharLine[j2] == ConCharLine2[j2] && ConAttrLine[j2] == ConAttrLine2[j2])
                {
                    if (updateCursor && row == Cursor.y && j2 == Cursor.x)
                        break;
                    if (doSelect)
                    {
                        if (CheckSelection(select1, row, j2))
                            break;
                        if (CheckSelection(select2, row, j2))
                            break;
                    }
                    ++j2;
                }
            }
        }
        if (!updateText)
            break; // ������ ���������� �������? � ������� ����� UpdateText ��������...
    }
done:
    return;
}

void CVirtualConsole::UpdateCursorDraw(COORD pos, BOOL vis, UINT dwSize, LPRECT prcLast/*=NULL*/)
{
    int CurChar = pos.Y * TextWidth + pos.X;
    if (CurChar < 0 || CurChar>=(int)(TextWidth * TextHeight))
        return; // ����� ���� ��� ���� - ��� ������ ������� ��� ����� �������� � ���������� ������� ������� ������� � ������
    if (!ConCharX)
        return; // ������
    COORD pix;
    /*if (prcLast) {
        //pix = MakeCoord(prcLast->left, prcLast->top);
        blitSize = MakeCoord(prcLast->right-prcLast->left, prcLast->bottom-prcLast->top);
    } else */
    {
        pix.X = pos.X * gSet.LogFont.lfWidth;
        pix.Y = pos.Y * gSet.LogFont.lfHeight;
        if (pos.X && ConCharX[CurChar-1])
            pix.X = ConCharX[CurChar-1];
    }

    if (vis)
    {
        if (gSet.isCursorColor)
        {
            SetTextColor(hDC, Cursor.foreColor);
            SetBkColor(hDC, Cursor.bgColor);
        }
        else
        {
            SetTextColor(hDC, Cursor.foreColor);
            SetBkColor(hDC, Cursor.foreColorNum < 5 ? gSet.Colors[15] : gSet.Colors[0]);
        }
    }
    else
    {
        if (drawImage && (Cursor.foreColorNum < 2) && prcLast)
            BlitPictureTo(prcLast->left, prcLast->top, prcLast->right-prcLast->left, prcLast->bottom-prcLast->top);

        SetTextColor(hDC, Cursor.bgColor);
        SetBkColor(hDC, Cursor.foreColor);
        dwSize = 99;
    }

    RECT rect;
    
    if (prcLast) {
        rect = *prcLast;
    } else
    if (!gSet.isCursorV)
    {
        if (gSet.isTTF) {
            rect.left = pix.X; /*Cursor.x * gSet.LogFont.lfWidth;*/
            rect.right = pix.X + gSet.LogFont.lfWidth; /*(Cursor.x+1) * gSet.LogFont.lfWidth;*/ //TODO: � ���� ������� ���������� ������� ��������!
        } else {
            rect.left = pos.X * gSet.LogFont.lfWidth;
            rect.right = (pos.X+1) * gSet.LogFont.lfWidth;
        }
        //rect.top = (Cursor.y+1) * gSet.LogFont.lfHeight - MulDiv(gSet.LogFont.lfHeight, cinf.dwSize, 100);
        rect.bottom = (pos.Y+1) * gSet.LogFont.lfHeight;
        rect.top = (pos.Y * gSet.LogFont.lfHeight) /*+ 1*/;
        //if (cinf.dwSize<50)
        int nHeight = 0;
        if (dwSize) {
            nHeight = MulDiv(gSet.LogFont.lfHeight, dwSize, 100);
            if (nHeight < HCURSORHEIGHT) nHeight = HCURSORHEIGHT;
        }
        //if (nHeight < HCURSORHEIGHT) nHeight = HCURSORHEIGHT;
        rect.top = max(rect.top, (rect.bottom-nHeight));
    }
    else
    {
        if (gSet.isTTF) {
            rect.left = pix.X; /*Cursor.x * gSet.LogFont.lfWidth;*/
            //rect.right = rect.left/*Cursor.x * gSet.LogFont.lfWidth*/ //TODO: � ���� ������� ���������� ������� ��������!
            //  + klMax(1, MulDiv(gSet.LogFont.lfWidth, cinf.dwSize, 100) 
            //  + (cinf.dwSize > 10 ? 1 : 0));
        } else {
            rect.left = pos.X * gSet.LogFont.lfWidth;
            //rect.right = Cursor.x * gSet.LogFont.lfWidth
            //  + klMax(1, MulDiv(gSet.LogFont.lfWidth, cinf.dwSize, 100) 
            //  + (cinf.dwSize > 10 ? 1 : 0));
        }
        rect.top = pos.Y * gSet.LogFont.lfHeight;
        int nR = (gSet.isTTF && ConCharX[CurChar]) // ������ �������
            ? ConCharX[CurChar] : ((pos.X+1) * gSet.LogFont.lfWidth);
        //if (cinf.dwSize>=50)
        //  rect.right = nR;
        //else
        //  rect.right = min(nR, (rect.left+VCURSORWIDTH));
        int nWidth = 0;
        if (dwSize) {
            nWidth = MulDiv((nR - rect.left), dwSize, 100);
            if (nWidth < VCURSORWIDTH) nWidth = VCURSORWIDTH;
        }
        rect.right = min(nR, (rect.left+nWidth));
        //rect.right = rect.left/*Cursor.x * gSet.LogFont.lfWidth*/ //TODO: � ���� ������� ���������� ������� ��������!
        //      + klMax(1, MulDiv(gSet.LogFont.lfWidth, cinf.dwSize, 100) 
        //      + (cinf.dwSize > 10 ? 1 : 0));
        rect.bottom = (pos.Y+1) * gSet.LogFont.lfHeight;
    }
    
    if (!prcLast) Cursor.lastRect = rect;

    if (gSet.LogFont.lfCharSet == OEM_CHARSET && !isCharBorder(Cursor.ch[0]))
    {
        if (gSet.isFixFarBorders)
            SelectFont(gSet.mh_Font);

        char tmp[2];
        WideCharToMultiByte(CP_OEMCP, 0, Cursor.ch, 1, tmp, 1, 0, 0);
        ExtTextOutA(hDC, pix.X, pix.Y,
            ETO_CLIPPED | ((drawImage && (Cursor.foreColorNum < 2) &&
            !vis) ? 0 : ETO_OPAQUE),&rect, tmp, 1, 0);
    }
    else
    {
        if (gSet.isFixFarBorders && isCharBorder(Cursor.ch[0]))
            SelectFont(gSet.mh_Font2);
        else
            SelectFont(gSet.mh_Font);

        ExtTextOut(hDC, pix.X, pix.Y,
            ETO_CLIPPED | ((drawImage && (Cursor.foreColorNum < 2) &&
            !vis) ? 0 : ETO_OPAQUE),&rect, Cursor.ch, 1, 0);
    }
}

void CVirtualConsole::UpdateCursor(bool& lRes)
{
    //------------------------------------------------------------------------
    ///| Drawing cursor |/////////////////////////////////////////////////////
    //------------------------------------------------------------------------
    Cursor.isVisiblePrevFromInfo = cinf.bVisible;

    BOOL lbUpdateTick = FALSE;
    if ((Cursor.x != csbi.dwCursorPosition.X) || (Cursor.y != csbi.dwCursorPosition.Y)) 
    {
        Cursor.isVisible = gConEmu.isMeForeground();
        if (Cursor.isVisible) lRes = true; //force, pos changed
        lbUpdateTick = TRUE;
    } else
    if ((GetTickCount() - Cursor.nLastBlink) > Cursor.nBlinkTime)
    {
        Cursor.isVisible = gConEmu.isMeForeground() && !Cursor.isVisible;
        lbUpdateTick = TRUE;
    }

    if ((lRes || Cursor.isVisible != Cursor.isVisiblePrev) && cinf.bVisible && isCursorValid)
    {
        lRes = true;

        SelectFont(gSet.mh_Font);

        if ((Cursor.x != csbi.dwCursorPosition.X || Cursor.y != csbi.dwCursorPosition.Y))
        {
            Cursor.isVisible = gConEmu.isMeForeground();
            //gConEmu.cBlinkNext = 0;
        }

        ///++++
        if (Cursor.isVisiblePrev) {
            UpdateCursorDraw(MakeCoord(Cursor.x, Cursor.y), false, Cursor.lastSize, &Cursor.lastRect);
        }

        int CurChar = csbi.dwCursorPosition.Y * TextWidth + csbi.dwCursorPosition.X;
        Cursor.ch[1] = 0;
        GetCharAttr(ConChar[CurChar], ConAttr[CurChar], Cursor.ch[0], Cursor.bgColorNum, Cursor.foreColorNum);
        Cursor.foreColor = gSet.Colors[Cursor.foreColorNum];
        Cursor.bgColor = gSet.Colors[Cursor.bgColorNum];

        UpdateCursorDraw(csbi.dwCursorPosition, Cursor.isVisible, cinf.dwSize);

        Cursor.isVisiblePrev = Cursor.isVisible;
    }

    // update cursor anyway to avoid redundant updates
    Cursor.x = csbi.dwCursorPosition.X;
    Cursor.y = csbi.dwCursorPosition.Y;

    if (lbUpdateTick)
        Cursor.nLastBlink = GetTickCount();
}

void CVirtualConsole::SetConsoleSizeInt(COORD size)
{
    const COLORREF DefaultColors[16] = 
    {
        0x00000000, 0x00800000, 0x00008000, 0x00808000,
        0x00000080, 0x00800080, 0x00008080, 0x00c0c0c0, 
        0x00808080, 0x00ff0000, 0x0000ff00, 0x00ffff00,
        0x000000ff, 0x00ff00ff, 0x0000ffff, 0x00ffffff
    };

    CONSOLE_INFO ci = { sizeof(ci) };
    int i;

    // get current size/position settings rather than using defaults..
    GetConsoleSizeInfo(&ci);
    ci.ScreenBufferSize = size;
    ci.WindowSize = size;

    // set these to zero to keep current settings
    ci.FontSize.X               = 4;
    ci.FontSize.Y               = 6;
    ci.FontFamily               = 0;//0x30;//FF_MODERN|FIXED_PITCH;//0x30;
    ci.FontWeight               = 0;//0x400;
    _tcscpy(ci.FaceName, _T("Lucida Console"));

    ci.CursorSize               = 25;
    ci.FullScreen               = FALSE;
    ci.QuickEdit                = FALSE;
    ci.AutoPosition             = 0x10000;
    ci.InsertMode               = TRUE;
    ci.ScreenColors             = MAKEWORD(0x7, 0x0);
    ci.PopupColors              = MAKEWORD(0x5, 0xf);

    ci.HistoryNoDup             = FALSE;
    ci.HistoryBufferSize        = 50;
    ci.NumberOfHistoryBuffers   = 4;

    // color table
    for(i = 0; i < 16; i++)
        ci.ColorTable[i] = DefaultColors[i];

    ci.CodePage                 = GetConsoleOutputCP();//0;//0x352;
    ci.Hwnd                     = hConWnd;

    *ci.ConsoleTitle = NULL;

    SetConsoleInfo(&ci);
}

bool CVirtualConsole::SetConsoleSize(COORD size)
{
	WARNING("������� ��� � ConEmuC � �������� ����� ����");

    //CSection SCON(&csCON, &ncsTCON);
	CSection SCON(NULL,NULL);

    if (!hConWnd) {
        Box(_T("Console was not created (CVirtualConsole::SetConsoleSize)"));
        return false; // ������� ���� �� �������?
    }

    if (size.X<4) size.X = 4;
    if (size.Y<3) size.Y = 3;
    
	//!!! ������. ������ ����� ���� ����������, ���� ������ ������ ����������...
    // ��� ��������� ��������� - ���������� � ������� ��������� ��������
    //if (TextWidth == size.X && TextHeight == size.Y)
	//    return TRUE;
    
    /*if (GetCurrentThreadId() != mn_ThreadID) {
	    m_ReqSetSize = size;
	    SetEvent ( mh_ReqSetSize );
		#ifdef _DEBUG
		if (WaitForSingleObject(mh_ReqSetSizeEnd, 2000)!=WAIT_OBJECT_0) {
			_ASSERT(FALSE);
		}
		#endif
	    if (WaitForSingleObject(mh_ReqSetSizeEnd, 10000)!=WAIT_OBJECT_0) {
		    // ���� �� ��������� ��������� ������� �������� - ������� ��� �� ����������
		    ResetEvent ( mh_ReqSetSize );
		    m_ReqSetSize = MakeCoord(TextWidth,TextHeight);
		    return FALSE;
		}
		// ��������� ���������, �� ���������?
		return (TextWidth == size.X && TextHeight == size.Y);
	}*/

    RECT rcConPos; GetWindowRect(hConWnd, &rcConPos);

    // case: simple mode
    if (BufferHeight == 0)
    {
        //HANDLE hConsoleOut = hConOut();
        bool lbRc = true;
        //HANDLE h = hConOut();
		//#ifdef _DEBUG
		//_ASSERT(h!=NULL);
		//#endif
        BOOL lbNeedChange = FALSE;
        //CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo()) {
            lbNeedChange = (csbi.dwSize.X != size.X) || (csbi.dwSize.Y != size.Y);
        }
        if (lbNeedChange) {
			SCON.Enter(&csCON, &ncsTCON);

            BOOL lbWS = FALSE; DWORD dwErr = 0;
			// ���� ����� �� ������� - ������ ������� ������ ���������
            MOVEWINDOW(hConWnd, rcConPos.left, rcConPos.top, 1, 1, 1); // ��������� �� ���������?
			//specified width and height cannot be less than the width and height of the console screen buffer's window
			TODO("SETCONSOLESCREENBUFFERSIZERET");
            //SETCONSOLESCREENBUFFERSIZERET(h, size, lbWS);
			dwErr = GetLastError();
            GetConsoleScreenBufferInfo();
            if (csbi.dwSize.X != size.X || csbi.dwSize.Y != size.Y) {
                dwErr = GetLastError();
                
                lbRc = false;

                //SETCONSOLESCREENBUFFERSIZERET(hConsoleOut, size, lbWS);
                //GetConsoleScreenBufferInfo(h, &csbi);

                //SetConsoleSizeInt(size); -- ����� �� ���� �� ������� �������� - �� ��������
                //GetConsoleScreenBufferInfo(h, &csbi);

                //size.X--; size.Y--;
                //SETCONSOLESCREENBUFFERSIZERET(h, csbi.dwSize, lbWS);
                //size.X++; size.Y++;
                //SETCONSOLESCREENBUFFERSIZERET(h, size, lbWS);
            }
            #ifdef _DEBUG
            /*if (!GetConsoleScreenBufferInfo(h, &csbi))
                _ASSERT(FALSE);
            else if (csbi.dwSize.X != size.X || csbi.dwSize.Y != size.Y)
                _ASSERT(FALSE);*/
            #endif
            // ������, �� �� ����� �������, �� ����������� ���������� �� ������� ���������� ������. ������?
            /*INPUT_RECORD r = {WINDOW_BUFFER_SIZE_EVENT};
            r.Event.WindowBufferSizeEvent.dwSize = size;
            DWORD dwWritten = 0;
            if (!WriteConsoleInput(hConIn(), &r, 1, &dwWritten)) {
                #ifdef _DEBUG
                DisplayLastError(L"WindowBufferSizeEvent failed!");
                #endif
            }*/
        } else {
            #ifdef _DEBUG
            lbNeedChange = lbNeedChange;
            #endif
        }
        //TODO: ���� ������ ������ ���� ������� �� ������� ������?
        MOVEWINDOW(hConWnd, rcConPos.left, rcConPos.top, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1);
		SCON.Leave();
        if (lbRc && lbNeedChange) { //�� ���� lbNeedChange
	        // ���� ������ �������� ����� ������� ����������
	        InitDC(false, true);
			// � ���������� ������ (����������� � �������������� ����)
			Update(true);
        }
        return lbRc;
    }


	// ������� ������ ��� BufferHeight
	SCON.Enter(&csCON, &ncsTCON);

    // global flag of the first call which is:
    // *) after getting all the settings
    // *) before running the command
    static bool s_isFirstCall = true;

    // case: buffer mode: change buffer
    //CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo())
        return false;
    csbi.dwSize.X = size.X;
    if (s_isFirstCall)
    {
        // first call: buffer height = from settings
        s_isFirstCall = false;
        csbi.dwSize.Y = max(BufferHeight, size.Y);
    }
    else
    {
        if (csbi.dwSize.Y == csbi.srWindow.Bottom - csbi.srWindow.Top + 1)
            // no y-scroll: buffer height = new window height
            csbi.dwSize.Y = size.Y;
        else
            // y-scroll: buffer height = old buffer height
            csbi.dwSize.Y = max(csbi.dwSize.Y, size.Y);
    }
    MOVEWINDOW(hConWnd, rcConPos.left, rcConPos.top, 1, 1, 1);
	TODO("SETCONSOLESCREENBUFFERSIZE");
    //SETCONSOLESCREENBUFFERSIZE(hConOut(), csbi.dwSize);
    //������ ���������� ������ �� ������!
    GetWindowRect(hConWnd, &rcConPos);
    MOVEWINDOW(hConWnd, rcConPos.left, rcConPos.top, GetSystemMetrics(SM_CXSCREEN), rcConPos.bottom-rcConPos.top, 1);

    
    // set console window
    if (!GetConsoleScreenBufferInfo())
        return false;
    SMALL_RECT rect;
    rect.Top = csbi.srWindow.Top;
    rect.Left = csbi.srWindow.Left;
    rect.Right = rect.Left + size.X - 1;
    rect.Bottom = rect.Top + size.Y - 1;
    if (rect.Right >= csbi.dwSize.X)
    {
        int shift = csbi.dwSize.X - 1 - rect.Right;
        rect.Left += shift;
        rect.Right += shift;
    }
    if (rect.Bottom >= csbi.dwSize.Y)
    {
        int shift = csbi.dwSize.Y - 1 - rect.Bottom;
        rect.Top += shift;
        rect.Bottom += shift;
    }
	TODO("SetConsoleWindowInfo");
    //SetConsoleWindowInfo(hConOut(), TRUE, &rect);
    return true;
}

// �������� ������ ������� �� ������� ���� (��������)
void CVirtualConsole::SyncConsole2Window()
{
    if (!this)
        return;

	if (GetCurrentThreadId() != mn_ThreadID) {
		RECT rcClient; GetClientRect(ghWnd, &rcClient);
		// ��������� ������ ������ �������
		RECT newCon = gConEmu.CalcRect(CER_CONSOLE, rcClient, CER_MAINCLIENT);

		if (newCon.right==TextWidth && newCon.bottom==TextHeight)
			return; // ������� �� ��������

		SetEvent(mh_Sync2WindowEvent);
		return;
	}

    DEBUGLOGFILE("SyncConsoleToWindow\n");

    RECT rcClient; GetClientRect(ghWnd, &rcClient);

    // ��������� ������ ������ �������
    RECT newCon = gConEmu.CalcRect(CER_CONSOLE, rcClient, CER_MAINCLIENT);

    SetConsoleSize(MakeCoord(newCon.right, newCon.bottom));
}


BOOL CVirtualConsole::AttachPID(DWORD dwPID)
{
	TODO("AttachPID ���� �������� �� �����");
	return FALSE;

#ifdef ALLOW_ATTACHPID
    #ifdef MSGLOGGER
        TCHAR szMsg[100]; wsprintf(szMsg, _T("Attach to process %i"), (int)dwPID);
        DEBUGSTR(szMsg);
    #endif
    BOOL lbRc = AttachConsole(dwPID);
    if (!lbRc) {
        DEBUGSTR(_T(" - failed\n"));
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
    DEBUGSTR(_T(" - OK"));

	TODO("InitHandler � GUI �������� ��� � �� �����...");
    //InitHandlers(FALSE);

    // ���������� ������� ������ ��� ��������� ������.
    CConEmuPipe pipe;
    
    //DEBUGSTR(_T("CheckProcesses\n"));
    //gConEmu.CheckProcesses(0,TRUE);
    
    if (pipe.Init(_T("DefFont.in.attach"), TRUE))
        pipe.Execute(CMD_DEFFONT);

    return TRUE;
#endif
}

//void CVirtualConsole::InitHandlers(BOOL abCreated)
//{
//    //hConWnd = GetConsoleWindow();
//	//2009-05-13 ������ ������ �� �����
//	//ghConWnd = hConWnd; // ������ �����, ����� ���� ��������, � ����� ������� �� ������ ���������� (����� hConOut() ������ NULL)
//
//    //SetConsoleCtrlHandler((PHANDLER_ROUTINE)CConEmuMain::HandlerRoutine, true);
//    
//    // �������� ��� ����� ����� ������ ��� �������� �������, � �� ��� ������
//    SetHandleInformation(GetStdHandle(STD_INPUT_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
//	// DuplicateHandle ������ �� �����, ��� ������ �� FreeConsole - ��� ������ ���������
//	//if (!DuplicateHandle(GetCurrentProcess(), h, GetCurrentProcess(), &mh_StdIn, 0, TRUE, DUPLICATE_SAME_ACCESS)) {
//	//	dwErr = GetLastError();
//	mh_StdIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
//            0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
//	//}
//    SetHandleInformation(GetStdHandle(STD_OUTPUT_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
//	//if (!DuplicateHandle(GetCurrentProcess(), h, GetCurrentProcess(), &mh_StdOut, 0, TRUE, DUPLICATE_SAME_ACCESS)) {
//	//	dwErr = GetLastError();
//	mh_StdOut = CreateFile(_T("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ,
//            0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
//	//}
//    SetHandleInformation(GetStdHandle(STD_ERROR_HANDLE), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
//
//    TODO("��������� � ConEmuC");
//	/*
//    DWORD mode = 0;
//    BOOL lb = GetConsoleMode(hConIn(), &mode);
//    if (!(mode & ENABLE_MOUSE_INPUT)) {
//        mode |= ENABLE_MOUSE_INPUT;
//        lb = SetConsoleMode(hConIn(), mode);
//    }
//	*/
//
//    //hConOut();
//
//    if (abCreated) {
//        SetConsoleFontSizeTo(4, 6);
//
//        if (IsIconic(hConWnd)) {
//            // ������ ����� ����������!
//            WINDOWPLACEMENT wplMain, wplCon;
//            memset(&wplMain, 0, sizeof(wplMain)); wplMain.length = sizeof(wplMain);
//            memset(&wplCon, 0, sizeof(wplCon)); wplCon.length = sizeof(wplCon);
//            GetWindowPlacement(ghWnd, &wplMain);
//            GetWindowPlacement(hConWnd, &wplCon);
//
//            int n = wplMain.rcNormalPosition.left - wplCon.rcNormalPosition.left;
//            wplCon.rcNormalPosition.left += n; wplCon.rcNormalPosition.right += n;
//            n = wplMain.rcNormalPosition.top - wplCon.rcNormalPosition.top;
//            wplCon.rcNormalPosition.top += n; wplCon.rcNormalPosition.bottom += n;
//            wplCon.showCmd = SW_RESTORE;
//            SetWindowPlacement(hConWnd, &wplCon);
//        }
//
//        //if (gSet.wndHeight && gSet.wndWidth)
//        {
//			//// ��������������� ��� ������ ������!
//			//RECT rcCon = MakeRect(gSet.wndWidth, gSet.wndHeight);
//			//RECT rcWnd = gConEmu.CalcRect(CER_MAIN, rcCon, CER_CONSOLE);
//			//rcWnd = gConEmu.CalcRect(CER_CORRECTED, rcWnd, gSet.isFullScreen ? CER_FULLSCREEN : CER_MAXIMIZED);
//			//rcCon = gConEmu.CalcRect(CER_CONSOLE, rcWnd, CER_MAIN);
//			//
//			//COORD b = {min(((int)gSet.wndWidth),rcCon.right), min(((int)gSet.wndHeight),rcCon.bottom)};
//			//SetConsoleSize(b);
//
//			// ������������ �� ������� ����
//			RECT rcWnd; GetClientRect(ghWnd, &rcWnd);
//			RECT rcCon = gConEmu.CalcRect(CER_CONSOLE, rcWnd, CER_MAINCLIENT);
//			SetConsoleSize(MakeCoord(rcCon.right,rcCon.bottom));
//        }
//
//        SetConsoleTitle(gSet.GetCmd());
//    } else {
//        SetConsoleFontSizeTo(4, 6);
//    }
//
//	if (gConEmu.isActive(this))
//		gConEmu.ConsoleCreated(hConWnd);
//}

// asExeName ����� ���� NULL, ����� ������ ������ ���� � ConEmu (F:_VCProject_FarPlugin_#FAR180_ConEmu.exe)
// � ����� ���� ��� ������ ������ "FAR", ��� � � ����������� "FAR.EXE" (������� �� ��������� ������)
void CVirtualConsole::RegistryProps(BOOL abRollback, ConExeProps& props, LPCTSTR asExeName/*=NULL*/)
{
    HKEY hkey = NULL;
    DWORD dwDisp = 0;
    TCHAR *pszExeName = NULL;
    
    if (!abRollback) {
        memset(&props, 0, sizeof(props));

        /*if (gSet.ourSI.lpTitle && *gSet.ourSI.lpTitle) {
            int nLen = _tcslen(gSet.ourSI.lpTitle);
            if (nLen>4 && _tcsicmp(gSet.ourSI.lpTitle+nLen-4, _T(".lnk"))==0) {
                props.FullKeyName = (TCHAR*)Alloc(10, sizeof(TCHAR));
                _tcscpy(props.FullKeyName, _T("Console"));
                pszExeName = props.FullKeyName+_tcslen(props.FullKeyName);
            } else {
                props.FullKeyName = (TCHAR*)Alloc(nLen+10, sizeof(TCHAR));
                _tcscpy(props.FullKeyName, _T("Console\\"));
                pszExeName = props.FullKeyName+_tcslen(props.FullKeyName);
                _tcscpy(pszExeName, gSet.ourSI.lpTitle);
            }
        } else*/
        if (asExeName && *asExeName) {
            props.FullKeyName = (TCHAR*)Alloc(_tcslen(asExeName)+10, sizeof(TCHAR));
            _tcscpy(props.FullKeyName, _T("Console\\"));
            pszExeName = props.FullKeyName+_tcslen(props.FullKeyName);
            _tcscpy(pszExeName, asExeName);
        } else {
            props.FullKeyName = (TCHAR*)Alloc(MAX_PATH+10, sizeof(TCHAR));
            _tcscpy(props.FullKeyName, _T("Console\\"));
            pszExeName = props.FullKeyName+_tcslen(props.FullKeyName);
            if (!GetModuleFileName(NULL, pszExeName, MAX_PATH+1)) {
                DisplayLastError(_T("Can't get module file name"));
                if (props.FullKeyName) { Free(props.FullKeyName); props.FullKeyName = NULL; }
                return;
            }
        }
        
        for (TCHAR* psz=pszExeName; pszExeName && *psz; psz++) {
            if (*psz == _T('\\')) *psz = _T('_');
        }
    } else if (!props.FullKeyName) {
        return;
    }
    
    
    if (abRollback && !props.bKeyExists) {
        // ������ ������� ������� pszExeName
        if (0 == RegOpenKeyEx ( HKEY_CURRENT_USER, _T("Console"), NULL, DELETE , &hkey)) {
            RegDeleteKey(hkey, pszExeName);
            RegCloseKey(hkey);
        }
        if (props.FullKeyName) { Free(props.FullKeyName); props.FullKeyName = NULL;}
        if (props.FaceName) { Free(props.FaceName); props.FaceName = NULL;}
        return;
    }
    
    if (0 == RegCreateKeyEx ( HKEY_CURRENT_USER, props.FullKeyName, NULL, NULL, NULL, KEY_ALL_ACCESS, NULL, &hkey, &dwDisp)) {
        if (!abRollback) {
			GetConsoleScreenBufferInfo();

            props.bKeyExists = (dwDisp == REG_OPENED_EXISTING_KEY);
            // ������� ��������
            DWORD dwSize, dwVal;
            if (0!=RegQueryValueEx(hkey, _T("ScreenBufferSize"), 0, NULL, (LPBYTE)&props.ScreenBufferSize, &(dwSize=sizeof(DWORD))))
                props.ScreenBufferSize = -1;
            if (0!=RegQueryValueEx(hkey, _T("WindowSize"), 0, NULL, (LPBYTE)&props.WindowSize, &(dwSize=sizeof(DWORD))))
                props.WindowSize = -1;
            if (0!=RegQueryValueEx(hkey, _T("WindowPosition"), 0, NULL, (LPBYTE)&props.WindowPosition, &(dwSize=sizeof(DWORD))))
                props.WindowPosition = -1;
            if (0!=RegQueryValueEx(hkey, _T("FontSize"), 0, NULL, (LPBYTE)&props.FontSize, &(dwSize=sizeof(DWORD))))
                props.FontSize = -1;
            if (0!=RegQueryValueEx(hkey, _T("FontFamily"), 0, NULL, (LPBYTE)&props.FontFamily, &(dwSize=sizeof(DWORD))))
                props.FontFamily = -1;
            props.FaceName = (TCHAR*)Alloc(MAX_PATH+1,sizeof(TCHAR));
            if (0!=RegQueryValueEx(hkey, _T("FaceName"), 0, NULL, (LPBYTE)props.FaceName, &(dwSize=(sizeof(TCHAR)*(MAX_PATH+1)))))
                props.FaceName[0] = 0;
            
            // ���������� ��������� ���������
			/*RECT rcWnd; GetClientRect(ghWnd, &rcWnd);
			RECT rcCon = gConEmu.CalcRect(CER_CONSOLE, rcWnd, CER_MAINCLIENT);*/
			dwVal = (csbi.dwSize.X) | (csbi.dwSize.Y<<16);
            RegSetValueEx(hkey, _T("ScreenBufferSize"), 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(DWORD));
            RegSetValueEx(hkey, _T("WindowSize"), 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(DWORD));
            if (!ghWndDC) dwVal = 0; else {
                RECT rcWnd; GetWindowRect(ghWndDC, &rcWnd);
                rcWnd.left = max(0, (rcWnd.left & 0xFFFF));
                rcWnd.top = max(0, (rcWnd.top & 0xFFFF));
                dwVal = (rcWnd.left) | (rcWnd.top<<16);
            }
            RegSetValueEx(hkey, _T("WindowPosition"), 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(DWORD));
            dwVal = 0x00060000;
            RegSetValueEx(hkey, _T("FontSize"), 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(DWORD));
            dwVal = 0x00000036;
            RegSetValueEx(hkey, _T("FontFamily"), 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(DWORD));
            TCHAR szLucida[64]; _tcscpy(szLucida, _T("Lucida Console"));
            RegSetValueEx(hkey, _T("FaceName"), 0, REG_SZ, (LPBYTE)szLucida, sizeof(TCHAR)*(_tcslen(szLucida)+1));
        } else {
            // ������� ��������
            if (props.ScreenBufferSize == -1)
                RegDeleteValue(hkey, _T("ScreenBufferSize"));
            else
                RegSetValueEx(hkey, _T("ScreenBufferSize"), 0, REG_DWORD, (LPBYTE)&props.ScreenBufferSize, sizeof(DWORD));
            if (props.WindowSize == -1)
                RegDeleteValue(hkey, _T("WindowSize"));
            else
                RegSetValueEx(hkey, _T("WindowSize"), 0, REG_DWORD, (LPBYTE)&props.WindowSize, sizeof(DWORD));
            if (props.WindowPosition == -1)
                RegDeleteValue(hkey, _T("WindowPosition"));
            else
                RegSetValueEx(hkey, _T("WindowPosition"), 0, REG_DWORD, (LPBYTE)&props.WindowPosition, sizeof(DWORD));
            if (props.FontSize == -1)
                RegDeleteValue(hkey, _T("FontSize"));
            else
                RegSetValueEx(hkey, _T("FontSize"), 0, REG_DWORD, (LPBYTE)&props.FontSize, sizeof(DWORD));
            if (props.FontFamily == -1)
                RegDeleteValue(hkey, _T("FontFamily"));
            else
                RegSetValueEx(hkey, _T("FontFamily"), 0, REG_DWORD, (LPBYTE)&props.FontFamily, sizeof(DWORD));
            if (props.FaceName && *props.FaceName)
                RegSetValueEx(hkey, _T("FaceName"), 0, REG_SZ, (LPBYTE)props.FaceName, sizeof(TCHAR)*(_tcslen(props.FaceName)+1));
            else
                RegDeleteValue(hkey, _T("FaceName"));
            
            // � ���������� ������
            if (props.FullKeyName) { Free(props.FullKeyName); props.FullKeyName = NULL;}
            if (props.FaceName) { Free(props.FaceName); props.FaceName = NULL;}
        }
        RegCloseKey(hkey);
    }
}

DWORD CVirtualConsole::StartProcessThread(LPVOID lpParameter)
{
    CVirtualConsole* pCon = (CVirtualConsole*)lpParameter;
    //BOOL lbRc = pCon->StartProcess(); -- ����� ��� ���-���� �� ������. ������ ����������� ���������� ��� ��������� TOPMOST �������� ����
    
    //if (!pCon->RetrieveConsoleInfo()) {
	//    return 1;
    //}
    
    _ASSERTE(pCon->mh_ConChanged!=NULL);
	_ASSERTE(pCon->mh_CursorChanged!=NULL);

    //TODO: � ��� �� ����� ������ �������...
	#define IDEVENT_TERM  0
	#define IDEVENT_CONCLOSED 1
	#define IDEVENT_CONCHANGED 2
	#define IDEVENT_CURSORCHANGED 3
	#define IDEVENV_FORCEREADEVENT 4
	#define IDEVENT_SYNC2WINDOW 5
	#define EVENTS_COUNT (IDEVENT_SYNC2WINDOW+1)
    HANDLE hEvents[EVENTS_COUNT];
	hEvents[IDEVENT_TERM] = pCon->mh_TermEvent;
	hEvents[IDEVENT_CONCLOSED] = pCon->mh_ConEmuC;
	hEvents[IDEVENT_CONCHANGED] = pCon->mh_ConChanged;
	hEvents[IDEVENT_CURSORCHANGED] = pCon->mh_CursorChanged;
	hEvents[IDEVENV_FORCEREADEVENT] = pCon->mh_ForceReadEvent; // ������������, ����� ������� Update & Invalidate
	hEvents[IDEVENT_SYNC2WINDOW] = pCon->mh_Sync2WindowEvent;
	DWORD  nEvents = countof(hEvents);
	_ASSERT(EVENTS_COUNT==nEvents);

    DWORD  nWait = 0;
    BOOL   bLoop = TRUE, bIconic = FALSE, bFirst = TRUE;

	DWORD nElapse = max(10,gSet.nMainTimerElapse);
    
	TODO("���� �� ����������� ��� F10 � ���� - �������� ���� �� ����������������...")
    while (TRUE/*bLoop*/)
    {
	    gSet.Performance(tPerfInterval, TRUE); // ������ �������� ������. �� ������� �� ���������� ����� ���������

        bIconic = IsIconic(ghWnd);
        // � ���������������� ������ - ��������� �������
        nWait = WaitForMultipleObjects(nEvents, hEvents, FALSE, bIconic ? 1000 : nElapse);

		if (nWait == IDEVENT_TERM /*|| !bLoop*/ || nWait == IDEVENT_CONCLOSED)
            break; // ���������� ���������� ����

		// ��������, ��� ConEmuC ���
		{
			DWORD dwExitCode = 0;
			BOOL fSuccess = GetExitCodeProcess(pCon->mh_ConEmuC, &dwExitCode);
			if (dwExitCode!=STILL_ACTIVE) {
				pCon->StopSignal();
				return 0;
			}
		}

        // ���� ������� �� ������ ���� �������� - �� �� ���-�� ��������� 
        if (!pCon->isShowConsole && !gSet.isConVisible)
        {
            /*if (foreWnd == hConWnd)
                SetForegroundWindow(ghWnd);*/
            if (IsWindowVisible(pCon->hConWnd))
                ShowWindow(pCon->hConWnd, SW_HIDE);
        }

		// ������ ������� ������ � ��� �����, � ������� ��� ���������. ����� ����� ��������������� ��� Update (InitDC)
        // ��������� ��������� �������� �������
        /*if (nWait == (IDEVENT_SYNC2WINDOW)) {
	        pCon->SetConsoleSize(pCon->m_ReqSetSize);
	        //SetEvent(pCon->mh_ReqSetSizeEnd);
	        //continue; -- � ����� ������� ���������� � ���
        }*/
        
        bool bActive = gConEmu.isActive(pCon);

		DWORD dwT1 = GetTickCount();

        try {   
            ResetEvent(pCon->mh_EndUpdateEvent);
            
			if (bFirst || (nWait==IDEVENT_CONCHANGED) || (nWait==IDEVENT_CURSORCHANGED)) {
				bFirst = FALSE;
				if (!pCon->RetrieveConsoleInfo((nWait==IDEVENT_CURSORCHANGED))) {
					_ASSERT(FALSE);
				}
			}

			if (nWait == IDEVENT_SYNC2WINDOW) {
				pCon->SyncConsole2Window();
			}

			if (GetWindowText(pCon->hConWnd, pCon->TitleCmp, countof(pCon->TitleCmp)-2)
				&& wcscmp(pCon->Title, pCon->TitleCmp))
			{
				wcscpy(pCon->Title, pCon->TitleCmp);
				if (bActive)
					gConEmu.UpdateTitle(pCon->TitleCmp);
			}

			bool lbForceUpdate = pCon->CheckBufferSize();
			if (lbForceUpdate && gConEmu.isActive(pCon)) // ������ �������� ����������� ���� ��� �������
				PostMessage(ghWnd, WM_TIMER, 0, 0); // ������� � ������� ���� ������ �� ���������� �������
			if (!lbForceUpdate && (nWait == (WAIT_OBJECT_0+1)))
				lbForceUpdate = true;

            if (pCon->Update(lbForceUpdate) && bActive)
				gConEmu.m_Child.Invalidate();
                //InvalidateRect(ghWndDC, NULL, FALSE);
            
            SetEvent(pCon->mh_EndUpdateEvent);
        } catch(...) {
            bLoop = FALSE;
        }

		/*if (nWait == (WAIT_OBJECT_0+2)) {
	        SetEvent(pCon->mh_ReqSetSizeEnd);
		}*/

		DWORD dwT2 = GetTickCount();
		DWORD dwD = max(10,(dwT2 - dwT1));
		nElapse = nElapse*0.7 + dwD*0.3;

		if (!bLoop) {
			#ifdef _DEBUG
			_ASSERT(FALSE);
			#endif
			pCon->Box(_T("Exception triggered in CVirtualConsole::StartProcessThread"));
			bLoop = true;
		}

	    gSet.Performance(tPerfInterval, FALSE);
    }
   
	// ���������� ��������� ����� ���� �������
	if (pCon->ms_VConServer_Pipe[0]) // ������ ���� �� ���� ���� ���� ��������
	{	
		pCon->StopSignal(); // ��� ������ ���� ���������, �� �� ������ ������
		//
		HANDLE hPipe = INVALID_HANDLE_VALUE;
		// ����������� �����, ����� ���� ������� �����������
		for (int i=0; i<MAX_SERVER_THREADS; i++) {
			hPipe = CreateFile(pCon->ms_VConServer_Pipe,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
			if (hPipe == INVALID_HANDLE_VALUE)
				break;
			// ������ ������� ���� - ��� ����� ���� �����������
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;
		}
		// ������� ��������, ���� ��� ���� ����������
		WaitForMultipleObjects(MAX_SERVER_THREADS, pCon->mh_ServerThreads, TRUE, 500);
		for (int i=0; i<MAX_SERVER_THREADS; i++) {
			if (WaitForSingleObject(pCon->mh_ServerThreads[i],0) != WAIT_OBJECT_0)
				TerminateThread(pCon->mh_ServerThreads[i],0);
			CloseHandle(pCon->mh_ServerThreads[i]);
			pCon->mh_ServerThreads[i] = NULL;
		}
	}
    
    // Finalize
    //SafeCloseHandle(pCon->mh_Thread);
    
    return 0;
}

BOOL CVirtualConsole::StartProcess()
{
    BOOL lbRc = FALSE;

    //CSection SCON(&csCON, &ncsTCON);
    
    // ���������������� ���������� m_sbi, m_ci, m_sel
	RECT rcWnd; GetClientRect(ghWnd, &rcWnd);
	RECT rcCon = gConEmu.CalcRect(CER_CONSOLE, rcWnd, CER_MAINCLIENT);
	_ASSERTE(rcCon.right!=0 && rcCon.bottom!=0);
	con.m_sbi.dwSize = MakeCoord(rcCon.right,rcCon.bottom);
	con.m_sbi.wAttributes = 7;
	con.m_sbi.srWindow.Right = rcCon.right-1; con.m_sbi.srWindow.Bottom = rcCon.bottom-1;
	con.m_sbi.dwMaximumWindowSize = con.m_sbi.dwSize;
	con.m_ci.dwSize = 15; con.m_ci.bVisible = TRUE;
	if (!InitBuffers(0))
		return FALSE;

    //if (ghConWnd) {
    //    // ������� ����� ���������� �� ������� �������
    //    FreeConsole(); ghConWnd = NULL;
    //}
    
    
    ConExeProps props;
    
	// 2009-05-13 ������ ������ ������� ���� � ��������� ��������, ��� ��� ����� �� ��������
    // ���� ����������� � ������ - ��� ������� �� �������... ���������� ������ � .lnk �����������...
    RegistryProps(FALSE, props, CEC_INITTITLE);

    /*if (!isShowConsole && !gSet.isConVisible
        #ifdef MSGLOGGER
        && !IsDebuggerPresent()
        #endif
        ) SetWindowPos(ghWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
    AllocConsole();

    InitHandlers(TRUE);

    if (!isShowConsole && !gSet.isConVisible
        #ifdef MSGLOGGER
        && !IsDebuggerPresent()
        #endif
        ) SetWindowPos(ghWnd, HWND_NOTOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);

    SetForegroundWindow(ghWnd);*/
    
    //RegistryProps(TRUE, props);


	// ��� ������ ���� ������� � InitHandlers
	/*RECT rcWnd; GetClientRect(ghWnd, &rcWnd);
	RECT rcCon = gConEmu.CalcRect(CER_CONSOLE, rcWnd, CER_MAINCLIENT);
	SetConsoleSize(MakeCoord(rcCon.right,rcCon.bottom));*/

//#pragma message("error: ������-�� ������� ��������� ���������� �� ������/������, ���� �� ������� ���� ����������!")
	// TODO: ����� Update ����� ������� ��������� ������� �������!

	//Update(true);

	//SCON.Leave();

    /*if (gSet.isConMan) {
        if (!gConEmu.InitConMan(gSet.GetCmd())) {
            // ����� ������� ����������. ConEmu ������ ����� ������ ���������...
            gSet.isConMan = false;
        } else {
            //SetForegroundWindow(ghWnd);
            return TRUE;
        }
    }*/ 
    
    //if (!gSet.isConMan)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW|STARTF_USECOUNTCHARS|STARTF_USEPOSITION;
		si.lpTitle = CEC_INITTITLE;
		si.dwXCountChars = con.m_sbi.dwSize.X; si.dwYCountChars = con.m_sbi.dwSize.Y;
		si.wShowWindow = gSet.isConVisible ? SW_SHOWNORMAL : SW_HIDE;
		RECT rcDC; GetWindowRect(ghWndDC, &rcDC);
		si.dwX = rcDC.left; si.dwY = rcDC.top;
        ZeroMemory( &pi, sizeof(pi) );

        int nStep = 1;
		wchar_t* psCurCmd = NULL;
        while (nStep <= 2)
        {
			MCHKHEAP
            /*if (!*gSet.GetCmd()) {
                gSet.psCurCmd = _tcsdup(gSet.BufferHeight == 0 ? _T("far") : _T("cmd"));
                nStep ++;
            }*/

            LPTSTR lpszCmd = (LPTSTR)gSet.GetCmd();

			int nLen = _tcslen(lpszCmd);
			TCHAR *pszSlash=NULL;
			nLen += _tcslen(gConEmu.ms_ConEmuExe) + 20;
			psCurCmd = (wchar_t*)malloc(nLen*sizeof(wchar_t));
			_ASSERTE(psCurCmd);
			wcscpy(psCurCmd, L"\"");
			wcscat(psCurCmd, gConEmu.ms_ConEmuExe);
			pszSlash = wcsrchr(psCurCmd, _T('\\'));
			MCHKHEAP
			wcscpy(pszSlash+1, L"ConEmuC.exe\" /CMD ");
			wcscat(psCurCmd, lpszCmd);
			MCHKHEAP

            #ifdef MSGLOGGER
            DEBUGSTR(psCurCmd);DEBUGSTR(_T("\n"));
            #endif
            try {
                lbRc = CreateProcess(NULL, psCurCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
                DEBUGSTR(_T("CreateProcess finished\n"));
            } catch(...) {
                lbRc = FALSE;
            }
            if (lbRc)
            {
                DEBUGSTR(_T("CreateProcess OK\n"));
                lbRc = TRUE;

                /*if (!AttachPID(pi.dwProcessId)) {
                    DEBUGSTR(_T("AttachPID failed\n"));
                    return FALSE;
                }
                DEBUGSTR(_T("AttachPID OK\n"));*/

                break; // OK, ���������
            } else {
                //Box("Cannot execute the command.");
                DWORD dwLastError = GetLastError();
                DEBUGSTR(_T("CreateProcess failed\n"));
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
                if (psz[_tcslen(psz)-1]!=_T('\n')) _tcscat(psz, _T("\r\n"));
                if (!gSet.psCurCmd && StrStrI(gSet.GetCmd(), gSet.BufferHeight == 0 ? _T("far.exe") : _T("cmd.exe"))==NULL) {
                    _tcscat(psz, _T("\r\n\r\n"));
                    _tcscat(psz, gSet.BufferHeight == 0 ? _T("Do You want to simply start far?") : _T("Do You want to simply start cmd?"));
                    nButtons |= MB_YESNO;
                }
				MCHKHEAP
                //Box(psz);
                int nBrc = MessageBox(NULL, psz, _T("ConEmu"), nButtons);
                Free(psz); Free(pszErr);
                if (nBrc!=IDYES) {
                    gConEmu.Destroy();
                    return FALSE;
                }
                // ��������� ����������� �������...
                gSet.psCurCmd = _tcsdup(gSet.BufferHeight == 0 ? _T("far") : _T("cmd"));
                nStep ++;
				MCHKHEAP
				if (psCurCmd) free(psCurCmd); psCurCmd = NULL;
            }
        }

		MCHKHEAP
		if (psCurCmd) free(psCurCmd); psCurCmd = NULL;
		MCHKHEAP

        //TODO: � ������ �� ���?
        CloseHandle(pi.hThread); pi.hThread = NULL;
        //CloseHandle(pi.hProcess); pi.hProcess = NULL;
		mn_ConEmuC_PID = pi.dwProcessId;
		mh_ConEmuC = pi.hProcess; pi.hProcess = NULL;
		
		// ������� "���������" ������� //2009-05-14 ������ ������� �������������� � GUI, �� ������ �� ������� ����� ��������� ������� �������
		wsprintfW(ms_ConEmuC_Pipe, CE_CURSORUPDATE, mn_ConEmuC_PID);
		mh_CursorChanged = CreateEvent ( NULL, FALSE, FALSE, ms_ConEmuC_Pipe );
		
		// ��� ����� ��� ���������� ConEmuC
		wsprintfW(ms_ConEmuC_Pipe, CESERVERPIPENAME, L".", mn_ConEmuC_PID);
		wsprintfW(ms_ConEmuCInput_Pipe, CESERVERINPUTNAME, L".", mn_ConEmuC_PID);
		MCHKHEAP
		
    }

    return lbRc;
}

bool CVirtualConsole::CheckBufferSize()
{
    bool lbForceUpdate = false;

    if (!this)
        return false;

    CSection SCON(&csCON, &ncsTCON);
    
    //CONSOLE_SCREEN_BUFFER_INFO inf; memset(&inf, 0, sizeof(inf));
    GetConsoleScreenBufferInfo();
    if (csbi.dwSize.X>(csbi.srWindow.Right-csbi.srWindow.Left+1)) {
        DEBUGLOGFILE("Wrong screen buffer width\n");
        // ������ ������� ������-�� ����������� �� �����������
        MOVEWINDOW(hConWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1);
    } else {
        // ����� �������� � �� �������, �� ����� ������ ����...
        /*if (mn_ActiveStatus & CES_FARACTIVE) {
            if (BufferHeight) {
                BufferHeight = 0; // ����� �� ����� ���������� ����
                lbForceUpdate = true;
            }
        } else*/
        if ( (csbi.dwSize.Y<(csbi.srWindow.Bottom-csbi.srWindow.Top+10)) && BufferHeight &&
             !gSet.BufferHeight /*&& (BufferHeight != csbi.dwSize.Y)*/)
        {
            // ����� ���� ���������� ��������� ��������� ����� ��������������?
            // TODO: ��������� ���������!!!
            BufferHeight = 0;

            SCROLLINFO si;
            ZeroMemory(&si, sizeof(si));
            si.cbSize = sizeof(si);
            si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
            if (GetScrollInfo(hConWnd, SB_VERT, &si))
                SetScrollInfo(gConEmu.m_Back.mh_Wnd, SB_VERT, &si, true);

            lbForceUpdate = true;
        } else 
        if ( (csbi.dwSize.Y>(csbi.srWindow.Bottom-csbi.srWindow.Top+10)) ||
             (BufferHeight && (BufferHeight != csbi.dwSize.Y)) )
        {
            // ����� ���� ���������� ��������� ��������� ����� ��������������?
            if (BufferHeight != csbi.dwSize.Y) {
                // TODO: �������� ���������!!!
                BufferHeight = csbi.dwSize.Y;
                lbForceUpdate = true;
            }
        }
        
        if ((BufferHeight == 0) && (csbi.dwSize.Y>(csbi.srWindow.Bottom-csbi.srWindow.Top+1))) {
            #pragma message (__FILE__ "(" STRING(__LINE__) "): TODO: ��� ����� ���� ���������� ��������� ��������� ����� ��������������!")
            DEBUGLOGFILE("Wrong screen buffer height\n");
            // ������ ������� ������-�� ����������� �� ���������
            MOVEWINDOW(hConWnd, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1);
        }
    }

    // ��� ������ �� FAR -> CMD � BufferHeight - ����� QuickEdit ������
    DWORD mode = 0;
    BOOL lb = FALSE;
    TODO("��������� � ConEmuC");
    if (BufferHeight) {
        //TODO: ������, ��� ��� BufferHeight ��� ���������� ���������?
        //lb = GetConsoleMode(hConIn(), &mode);
        mode = GetConsoleMode();

        if (csbi.dwSize.Y>(csbi.srWindow.Bottom-csbi.srWindow.Top+1)) {
            // ����� ������ ������ ����
            mode |= ENABLE_QUICK_EDIT_MODE|ENABLE_INSERT_MODE|ENABLE_EXTENDED_FLAGS;
        } else {
            // ����� ����� ������ ���� (������ ��� ����������)
            mode &= ~(ENABLE_QUICK_EDIT_MODE|ENABLE_INSERT_MODE);
            mode |= ENABLE_EXTENDED_FLAGS;
        }

		TODO("SetConsoleMode");
        //lb = SetConsoleMode(hConIn(), mode);
    }
    
    return lbForceUpdate;
}

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL                  0x020E
#endif
void CVirtualConsole::SendMouseEvent(UINT messg, WPARAM wParam, int x, int y)
{
    if (!this || !hConWnd)
        return;

    BOOL lbStdMode = FALSE;
    if (BufferHeight == 0)
        lbStdMode = TRUE;

    if (lbStdMode)
    {
        Assert ( gSet.LogFont.lfWidth && gSet.LogFont.lfHeight );
        if (!gSet.LogFont.lfWidth | !gSet.LogFont.lfHeight)
            return;
        #pragma message (__FILE__ "(" STRING(__LINE__) "): TODO: X ���������� ��� ��������, ��� ��� ����� �� ����� ��������� ������� ����������...")
        
        INPUT_RECORD r; memset(&r, 0, sizeof(r));
        r.EventType = MOUSE_EVENT;
		#pragma message (__FILE__ "(" STRING(__LINE__) "): TODO: � ����� ������ �� �������� ��������� ���������� ��������, � �� ������� ��������")
        r.Event.MouseEvent.dwMousePosition = MakeCoord(x/gSet.LogFont.lfWidth, y/gSet.LogFont.lfHeight);
        
        // Mouse Buttons
        if (messg != WM_LBUTTONUP && (messg == WM_LBUTTONDOWN || messg == WM_LBUTTONDBLCLK || isPressed(VK_LBUTTON)))
            r.Event.MouseEvent.dwButtonState |= FROM_LEFT_1ST_BUTTON_PRESSED;
        if (messg != WM_RBUTTONUP && (messg == WM_RBUTTONDOWN || messg == WM_RBUTTONDBLCLK || isPressed(VK_RBUTTON)))
            r.Event.MouseEvent.dwButtonState |= RIGHTMOST_BUTTON_PRESSED;
        if (messg != WM_MBUTTONUP && (messg == WM_MBUTTONDOWN || messg == WM_MBUTTONDBLCLK || isPressed(VK_MBUTTON)))
            r.Event.MouseEvent.dwButtonState |= FROM_LEFT_2ND_BUTTON_PRESSED;

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

		// �������� ������� � ������� ����� ConEmuC
		SendConsoleEvent ( &r );

        /*DWORD dwWritten = 0;
        if (!WriteConsoleInput(hConIn(), &r, 1, &dwWritten)) {
            DisplayLastError(L"SendMouseEvent failed!");
        }*/
    } else {
        /*���� ���������� ��� - ����� �����:
        1) ����� ���� ������ �� ������� �� �������
        2) ������ ���� �������� "�����" ���������� �������
        3) �������� � ���������� ����(?)*/
        /* ��! ����� �� �������� ��������� ������ */
        static bool bInitialized = false;
        if (!bInitialized && (messg == WM_LBUTTONDOWN || messg == WM_RBUTTONDOWN))
        {
	        bInitialized = true; // ��������� ��� �������� ���� ���
	        if (gConEmu.isConmanAlternative()) {
		        POSTMESSAGE(hConWnd, WM_COMMAND, SC_PASTE_SECRET, 0, TRUE);
	        } else {
		        POSTMESSAGE(hConWnd, WM_COMMAND, (messg == WM_LBUTTONDOWN) ? SC_MARK_SECRET : SC_PASTE_SECRET, 0, TRUE);
		        if (messg == WM_RBUTTONDOWN)
			        return; // ���������� SC_PASTE_SECRET
			}
        }

        RECT conRect;
        GetClientRect(hConWnd, &conRect);
        short newX = MulDiv(x, conRect.right, klMax<uint>(1, Width));
        short newY = MulDiv(y, conRect.bottom, klMax<uint>(1, Height));

        POSTMESSAGE(hConWnd, messg, wParam, MAKELPARAM( newX, newY ), TRUE);
    }
}

void CVirtualConsole::SendConsoleEvent(INPUT_RECORD* piRec)
{
	DWORD dwErr = 0, dwMode = 0;
	BOOL fSuccess = FALSE;

	// ���� ����. ��������, ��� ConEmuC ���
	DWORD dwExitCode = 0;
	fSuccess = GetExitCodeProcess(mh_ConEmuC, &dwExitCode);
	if (dwExitCode!=STILL_ACTIVE) {
		//DisplayLastError(L"ConEmuC was terminated");
		return;
	}

	TODO("���� ���� � ����� ������ �� �������� � ������� 10 ������ (������?) - ������� VirtualConsole ������� ������");
	if (mh_ConEmuCInput==NULL || mh_ConEmuCInput==INVALID_HANDLE_VALUE) {
		// Try to open a named pipe; wait for it, if necessary. 
		while (1) 
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
			if (dwErr = WAIT_OBJECT_0)
				return;
			continue;
			//DisplayLastError(L"Could not open pipe", dwErr);
			//return 0;
		  }

		  // All pipe instances are busy, so wait for 1 second.
		  if (!WaitNamedPipe(ms_ConEmuC_Pipe, 1000) ) 
		  {
			dwErr = WaitForSingleObject(mh_ConEmuC, 100);
			if (dwErr = WAIT_OBJECT_0) {
				DEBUGSTR(L" - FAILED!\n");
				return;
			}
		    //DisplayLastError(L"WaitNamedPipe failed"); 
			//return 0;
		  }
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
		  DEBUGSTR(L" - FAILED!\n");
		  DisplayLastError(L"SetNamedPipeHandleState failed");
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

LPVOID CVirtualConsole::Alloc(size_t nCount, size_t nSize)
{
#ifdef _DEBUG
    HeapValidate(mh_Heap, 0, NULL);
#endif
    size_t nWhole = nCount * nSize;
    LPVOID ptr = HeapAlloc ( mh_Heap, HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY, nWhole );
#ifdef _DEBUG
        HeapValidate(mh_Heap, 0, NULL);
#endif
    return ptr;
}

void CVirtualConsole::Free(LPVOID ptr)
{
    if (ptr && mh_Heap) {
#ifdef _DEBUG
        HeapValidate(mh_Heap, 0, NULL);
#endif
        HeapFree ( mh_Heap, 0, ptr );
#ifdef _DEBUG
        HeapValidate(mh_Heap, 0, NULL);
#endif
    }
}

void CVirtualConsole::StopSignal()
{
	if (mn_ProcessCount) {
		CSection SPRC(&csPRC, &ncsTPRC);
		m_Processes.clear();
		SPRC.Leave();
		mn_ProcessCount = 0;
	}

    SetEvent(mh_TermEvent);
}

void CVirtualConsole::StopThread()
{
#ifdef _DEBUG
        HeapValidate(mh_Heap, 0, NULL);
#endif

    if (mh_Thread) {
        // ����������� ������ � ���������� ����
        StopSignal(); //SetEvent(mh_TermEvent);
        if (WaitForSingleObject(mh_Thread, 300) != WAIT_OBJECT_0)
            TerminateThread(mh_Thread, 1);
    }
    
    SafeCloseHandle(mh_TermEvent);
    SafeCloseHandle(mh_ForceReadEvent);
    SafeCloseHandle(mh_EndUpdateEvent);
	SafeCloseHandle(mh_Sync2WindowEvent);
	SafeCloseHandle(mh_ConChanged);
	SafeCloseHandle(mh_CursorChanged);
    
    SafeCloseHandle(mh_Thread);

#ifdef _DEBUG
        HeapValidate(mh_Heap, 0, NULL);
#endif
}

void CVirtualConsole::Paint()
{
    if (!ghWndDC)
        return;
    
    if (!this) {
        // ������ ������ 0
		#ifdef _DEBUG
			int nBackColorIdx = 2;
		#else
			int nBackColorIdx = 0;
		#endif
        HBRUSH hBr = CreateSolidBrush(gSet.Colors[nBackColorIdx]);
        RECT rcClient; GetClientRect(ghWndDC, &rcClient);
        PAINTSTRUCT ps;
        HDC hDc = BeginPaint(ghWndDC, &ps);
        FillRect(hDc, &rcClient, hBr);
        DeleteObject(hBr);
        EndPaint(ghWndDC, &ps);
        return;
    }

    BOOL lbExcept = FALSE;
    RECT client;
    PAINTSTRUCT ps;
    HDC hPaintDc = NULL;

    //CSection S(&csDC, &ncsTDC);
	CSection S(&csCON, &ncsTCON);
    if (!S.isLocked())
        return; // �� ������� �������� ������ � CS

    GetClientRect(ghWndDC, &client);

    if (!gConEmu.isNtvdm()) {
        // ����� ������ � �������� ����� ���� �������� � �������� ���� DC
        if (client.right < (LONG)Width || client.bottom < (LONG)Height)
            gConEmu.OnSize();
    }
    
    try {
        hPaintDc = BeginPaint(ghWndDC, &ps);

        HBRUSH hBr = NULL;
        if (((ULONG)client.right) > Width) {
            if (!hBr) hBr = CreateSolidBrush(gSet.Colors[mn_BackColorIdx]);
            RECT rcFill = MakeRect(Width, 0, client.right, client.bottom);
            FillRect(hPaintDc, &rcFill, hBr);
            client.right = Width;
        }
        if (((ULONG)client.bottom) > Height) {
            if (!hBr) hBr = CreateSolidBrush(gSet.Colors[mn_BackColorIdx]);
            RECT rcFill = MakeRect(0, Height, client.right, client.bottom);
            FillRect(hPaintDc, &rcFill, hBr);
            client.bottom = Height;
        }
        if (hBr) { DeleteObject(hBr); hBr = NULL; }


        if (!gbNoDblBuffer) {
            // ������� �����
            BitBlt(hPaintDc, 0, 0, client.right, client.bottom, hDC, 0, 0, SRCCOPY);
        } else {
            GdiSetBatchLimit(1); // ��������� ����������� ������ ��� ������� ����

            GdiFlush();
            // ������ ����� �� �������, ��� �����������
            Update(true, &hPaintDc);
        }

        if (gbNoDblBuffer) GdiSetBatchLimit(0); // ������� ����������� �����
    } catch(...) {
        lbExcept = TRUE;
    }
    
    S.Leave();
    
    if (lbExcept)
        Box(_T("Exception triggered in CVirtualConsole::Paint"));

    if (hPaintDc && ghWndDC) {
        EndPaint(ghWndDC, &ps);
    }
}

void CVirtualConsole::UpdateInfo()
{
    if (!ghOpWnd || !this)
        return;

    if (!gConEmu.isMainThread()) {
        return;
    }

    TCHAR szSize[128];
    wsprintf(szSize, _T("%ix%i"), TextWidth, TextHeight);
    SetDlgItemText(gSet.hInfo, tConSizeChr, szSize);
    wsprintf(szSize, _T("%ix%i"), Width, Height);
    SetDlgItemText(gSet.hInfo, tConSizePix, szSize);

    wsprintf(szSize, _T("(%i, %i)-(%i, %i), %ix%i"), mr_LeftPanel.left+1, mr_LeftPanel.top+1, mr_LeftPanel.right+1, mr_LeftPanel.bottom+1, mr_LeftPanel.right-mr_LeftPanel.left+1, mr_LeftPanel.bottom-mr_LeftPanel.top+1);
    SetDlgItemText(gSet.hInfo, tPanelLeft, szSize);
    wsprintf(szSize, _T("(%i, %i)-(%i, %i), %ix%i"), mr_RightPanel.left+1, mr_RightPanel.top+1, mr_RightPanel.right+1, mr_RightPanel.bottom+1, mr_RightPanel.right-mr_RightPanel.left+1, mr_RightPanel.bottom-mr_RightPanel.top+1);
    SetDlgItemText(gSet.hInfo, tPanelRight, szSize);
}

void CVirtualConsole::Box(LPCTSTR szText)
{
#ifdef _DEBUG
    _ASSERT(FALSE);
#endif
    MessageBox(NULL, szText, _T("ConEmu"), MB_ICONSTOP);
}

//
//  Fill the CONSOLE_INFO structure with information
//  about the current console window
//
void CVirtualConsole::GetConsoleSizeInfo(CONSOLE_INFO *pci)
{
    CSection SCON(&csCON, &ncsTCON);
    //CONSOLE_SCREEN_BUFFER_INFO csbi;

    //HANDLE hConsoleOut = hConOut(); //hConOut();

    GetConsoleScreenBufferInfo();

    pci->ScreenBufferSize = csbi.dwSize;
    pci->WindowSize.X     = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    pci->WindowSize.Y     = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    pci->WindowPosX       = csbi.srWindow.Left;
    pci->WindowPosY       = csbi.srWindow.Top;
}

//
//  Wrapper around WM_SETCONSOLEINFO. We need to create the
//  necessary section (file-mapping) object in the context of the
//  process which owns the console, before posting the message
//
BOOL CVirtualConsole::SetConsoleInfo(CONSOLE_INFO *pci)
{
    DWORD   dwConsoleOwnerPid, dwCurProcId;
    HANDLE  hProcess=NULL;
    HANDLE  hSection=NULL, hDupSection=NULL;
    PVOID   ptrView = 0;
    DWORD   dwLastError=0;
    WCHAR   ErrText[255];
    
    //
    //  Open the process which "owns" the console
    //  
    dwCurProcId = GetCurrentProcessId();
    dwConsoleOwnerPid = dwCurProcId;
    hProcess = GetCurrentProcess();
    
    // ���� �� �� � ������� ����������� - ������� �� �����!
    /*GetWindowThreadProcessId(hwndConsole, &dwConsoleOwnerPid);
    
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwConsoleOwnerPid);
    dwLastError = GetLastError();
    //������ ����� OpenProcess �� ���� ���� �������? GetLastError ���������� 5
    if (hProcess==NULL) {
        if (dwConsoleOwnerPid == dwCurProcId) {
            hProcess = GetCurrentProcess();
        } else {
            wsprintf(ErrText, L"Can't open console process. ErrCode=%i", dwLastError);
            MessageBox(NULL, ErrText, L"ConEmu", MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
            return FALSE;
        }
    }*/

    //
    // Create a SECTION object backed by page-file, then map a view of
    // this section into the owner process so we can write the contents 
    // of the CONSOLE_INFO buffer into it
    //
    hSection = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, pci->Length, 0);
    if (!hSection) {
        dwLastError = GetLastError();
        wsprintf(ErrText, L"Can't CreateFileMapping. ErrCode=%i", dwLastError);
        MessageBox(NULL, ErrText, L"ConEmu", MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

    } else {
        //
        //  Copy our console structure into the section-object
        //
        ptrView = MapViewOfFile(hSection, FILE_MAP_WRITE|FILE_MAP_READ, 0, 0, pci->Length);
        if (!ptrView) {
            dwLastError = GetLastError();
            wsprintf(ErrText, L"Can't MapViewOfFile. ErrCode=%i", dwLastError);
            MessageBox(NULL, ErrText, L"ConEmu", MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

        } else {
            memcpy(ptrView, pci, pci->Length);

            UnmapViewOfFile(ptrView);

            //
            //  Map the memory into owner process
            //
            if (!DuplicateHandle(GetCurrentProcess(), hSection, hProcess, &hDupSection, 0, FALSE, DUPLICATE_SAME_ACCESS)
                || !hDupSection)
            {
                dwLastError = GetLastError();
                wsprintf(ErrText, L"Can't DuplicateHandle. ErrCode=%i", dwLastError);
                MessageBox(NULL, ErrText, L"ConEmu", MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

            } else {
                //  Send console window the "update" message
                DWORD dwConInfoRc = 0;
                DWORD dwConInfoErr = 0;
                
                dwConInfoRc = SendMessage(hConWnd, WM_SETCONSOLEINFO, (WPARAM)hDupSection, 0);
                dwConInfoErr = GetLastError();
            }
        }
    }



    //
    // clean up
    //

    if (hDupSection) {
        if (dwConsoleOwnerPid == dwCurProcId) {
            // ���� ��� ���� ������� - ����� � ������ �����������
            CloseHandle(hDupSection);
        } else {
            HANDLE  hThread=NULL;
            hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)CloseHandle, hDupSection, 0, 0);
            if (hThread) CloseHandle(hThread);
        }
    }

    if (hSection)
        CloseHandle(hSection);
    if ((dwConsoleOwnerPid!=dwCurProcId) && hProcess)
        CloseHandle(hProcess);

    return TRUE;
}

//VISTA support:
#ifndef ENABLE_AUTO_POSITION
typedef struct _CONSOLE_FONT_INFOEX {
    ULONG cbSize;
    DWORD nFont;
    COORD dwFontSize;
    UINT FontFamily;
    UINT FontWeight;
    WCHAR FaceName[LF_FACESIZE];
} CONSOLE_FONT_INFOEX, *PCONSOLE_FONT_INFOEX;
#endif


typedef BOOL (WINAPI *PGetCurrentConsoleFontEx)(__in HANDLE hConsoleOutput,__in BOOL bMaximumWindow,__out PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx);
typedef BOOL (WINAPI *PSetCurrentConsoleFontEx)(__in HANDLE hConsoleOutput,__in BOOL bMaximumWindow,__out PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx);


void CVirtualConsole::SetConsoleFontSizeTo(int inSizeX, int inSizeY)
{
    PGetCurrentConsoleFontEx GetCurrentConsoleFontEx = (PGetCurrentConsoleFontEx)
        GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetCurrentConsoleFontEx");
    PSetCurrentConsoleFontEx SetCurrentConsoleFontEx = (PSetCurrentConsoleFontEx)
        GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "SetCurrentConsoleFontEx");

    if (GetCurrentConsoleFontEx && SetCurrentConsoleFontEx) // We have Vista
    {
        CONSOLE_FONT_INFOEX cfi = {sizeof(CONSOLE_FONT_INFOEX)};
        //GetCurrentConsoleFontEx(hConOut(), FALSE, &cfi);
        cfi.dwFontSize.X = inSizeX;
        cfi.dwFontSize.Y = inSizeY;
        //TODO: � ������ ��� ������� �����???
        _tcscpy(cfi.FaceName, _T("Lucida Console"));
		TODO("SetCurrentConsoleFontEx");
        //SetCurrentConsoleFontEx(hConOut()/*hConOut()*/, FALSE, &cfi);
    }
    else // We have other NT
    {
        const COLORREF DefaultColors[16] = 
        {
            0x00000000, 0x00800000, 0x00008000, 0x00808000,
            0x00000080, 0x00800080, 0x00008080, 0x00c0c0c0, 
            0x00808080, 0x00ff0000, 0x0000ff00, 0x00ffff00,
            0x000000ff, 0x00ff00ff, 0x0000ffff, 0x00ffffff
        };

        CONSOLE_INFO ci = { sizeof(ci) };
        int i;

        // get current size/position settings rather than using defaults..
        GetConsoleSizeInfo(&ci);

        // set these to zero to keep current settings
        ci.FontSize.X               = inSizeX;
        ci.FontSize.Y               = inSizeY;
        ci.FontFamily               = 0;//0x30;//FF_MODERN|FIXED_PITCH;//0x30;
        ci.FontWeight               = 0;//0x400;
        _tcscpy(ci.FaceName, _T("Lucida Console"));

        ci.CursorSize               = 25;
        ci.FullScreen               = FALSE;
        ci.QuickEdit                = FALSE;
        ci.AutoPosition             = 0x10000;
        ci.InsertMode               = TRUE;
        ci.ScreenColors             = MAKEWORD(0x7, 0x0);
        ci.PopupColors              = MAKEWORD(0x5, 0xf);

        ci.HistoryNoDup             = FALSE;
        ci.HistoryBufferSize        = 50;
        ci.NumberOfHistoryBuffers   = 4;

        // color table
        for(i = 0; i < 16; i++)
            ci.ColorTable[i] = DefaultColors[i];

        ci.CodePage                 = GetConsoleOutputCP();//0;//0x352;
        ci.Hwnd                     = hConWnd;

        *ci.ConsoleTitle = NULL;

        //!!! ����-�� �� ��������... � �
        // F:\VCProject\FarPlugin\ConEmu\080703\ConEmu\setconsoleinfo.cpp
        //����� ��
        SetConsoleInfo(&ci);
    }
}

BOOL CVirtualConsole::isBufferHeight()
{
    if (!this)
        return FALSE;

	//TODO: ����� �� �����
    //CSection SCON(&csCON, &ncsTCON);

    BOOL lbScrollMode = FALSE;
    //CONSOLE_SCREEN_BUFFER_INFO inf; memset(&inf, 0, sizeof(inf));
	if (GetConsoleScreenBufferInfo()) {
		if (csbi.dwSize.Y>(csbi.srWindow.Bottom-csbi.srWindow.Top+1))
			lbScrollMode = TRUE;
	}

    return lbScrollMode;
}

bool CVirtualConsole::isConSelectMode()
{
	if (!this) return false;
	return mb_ConsoleSelectMode;
}

bool CVirtualConsole::isFar()
{
	if (!this) return false;
	return GetFarPID()!=0;
}

BOOL CVirtualConsole::GetConsoleScreenBufferInfo()
{
	csbi = con.m_sbi;
	return TRUE;
	//mdw_LastError = 0;
	//BOOL lbRc = FALSE;
	//HANDLE h = hConOut();
	//if (!h) {
	//	mdw_LastError = ERROR_INVALID_HANDLE;
	//} else {
	//	memset(&csbi, 0, sizeof(csbi));
	//	lbRc = ::GetConsoleScreenBufferInfo(h, &csbi);
	//	if (!lbRc)
	//		mdw_LastError = GetLastError();
	//}
	//return lbRc;
}

LPCTSTR CVirtualConsole::GetTitle()
{
	return Title;
}

LRESULT CVirtualConsole::OnScroll(int nDirection)
{
	// SB_LINEDOWN / SB_LINEUP / SB_PAGEDOWN / SB_PAGEUP
	TODO("���������� � ������� �����");
	POSTMESSAGE(ghConWnd, WM_VSCROLL, SB_LINEDOWN, NULL, FALSE);
	return 0;
}

LRESULT CVirtualConsole::OnKeyboard(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;


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
                POSTMESSAGE(hConWnd, WM_KEYDOWN, VK_MENU, 0, TRUE);
                POSTMESSAGE(hConWnd, WM_KEYDOWN, VK_RETURN, 0, TRUE);
                POSTMESSAGE(hConWnd, WM_KEYUP, VK_RETURN, 0, TRUE);
                POSTMESSAGE(hConWnd, WM_KEYUP, VK_MENU, 0, TRUE);
            }
            else
            {
                if (isPressed(VK_SHIFT))
                    return 0;

                if (!gSet.isFullScreen)
                    gConEmu.SetWindowMode(rFullScreen);
                else
                    gConEmu.SetWindowMode(gConEmu.isWndNotFSMaximized ? rMaximized : rNormal);

                isSkipNextAltUp = true;
            }
        }
        else if (messg == WM_SYSKEYDOWN && wParam == VK_SPACE && lParam & (1<<29)/*����. ��� 29-� ���, � �� ����� 29*/ && !isPressed(VK_SHIFT))
        {
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

			WORD nCaps = GetKeyState(VK_CAPITAL);
			WORD nNum = GetKeyState(VK_NUMLOCK);
			WORD nScroll = GetKeyState(VK_SCROLL);
			WORD nLAlt = GetKeyState(VK_LMENU);
			WORD nRAlt = GetKeyState(VK_RMENU);
			WORD nLCtrl = GetKeyState(VK_LCONTROL);
			WORD nRCtrl = GetKeyState(VK_RCONTROL);
			WORD nShift = GetKeyState(VK_SHIFT);

			if (messg == WM_CHAR || messg == WM_SYSCHAR) {
				if (((WCHAR)wParam) <= 32 || mn_LastVKeyPressed == 0)
					return 0; // ��� ��� ����������
				r.Event.KeyEvent.bKeyDown = TRUE;
				r.Event.KeyEvent.uChar.UnicodeChar = (WCHAR)wParam;
				r.Event.KeyEvent.wRepeatCount = 1; TODO("0-15 ? Specifies the repeat count for the current message. The value is the number of times the keystroke is autorepeated as a result of the user holding down the key. If the keystroke is held long enough, multiple messages are sent. However, the repeat count is not cumulative.");
				r.Event.KeyEvent.wVirtualKeyCode = mn_LastVKeyPressed;
			} else {
				mn_LastVKeyPressed = wParam & 0xFFFF;
				//POSTMESSAGE(hConWnd, messg, wParam, lParam, FALSE);
				if ((wParam >= VK_F1 && wParam <= /*VK_F24*/ VK_SCROLL) || wParam <= 32 ||
					(wParam >= VK_LSHIFT && wParam <= /*VK_RMENU*/ 0xB7 /*=VK_LAUNCH_APP2*/) ||
					(wParam >= VK_LWIN && wParam <= VK_APPS) ||
					/*(wParam >= VK_NUMPAD0 && wParam <= VK_DIVIDE) ||*/ //TODO:
					(wParam >= VK_PRIOR && wParam <= VK_HELP) ||
					FALSE)
				{
					r.Event.KeyEvent.wRepeatCount = 1; TODO("0-15 ? Specifies the repeat count for the current message. The value is the number of times the keystroke is autorepeated as a result of the user holding down the key. If the keystroke is held long enough, multiple messages are sent. However, the repeat count is not cumulative.");
					r.Event.KeyEvent.wVirtualKeyCode = mn_LastVKeyPressed;
					mn_LastVKeyPressed = 0; // ����� �� ������������ WM_(SYS)CHAR
				} else {
					return 0;
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
			SendConsoleEvent(&r);

			if (messg == WM_CHAR || messg == WM_SYSCHAR) {
				// � ����� �������� ����������
				r.Event.KeyEvent.bKeyDown = FALSE;
				SendConsoleEvent(&r);
			}
		}
    }

    /*if (IsDebuggerPresent()) {
        if (hWnd ==ghWnd)
            DEBUGSTR(L"   focused ghWnd\n"); else
        if (hWnd ==hConWnd)
            DEBUGSTR(L"   focused hConWnd\n"); else
        if (hWnd ==ghWndDC)
            DEBUGSTR(L"   focused ghWndDC\n"); 
        else
            DEBUGSTR(L"   focused UNKNOWN\n"); 
    }*/

    return 0;
}

void CVirtualConsole::OnWinEvent(DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	_ASSERTE(hwnd!=NULL);
	
	if (hConWnd == NULL && event == EVENT_CONSOLE_START_APPLICATION && idObject == mn_ConEmuC_PID)
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
			ProcessAdd(idObject);
			// ���� �������� 16������ ���������� - ���������� �������� ��������� ������ ��������, ����� ����� �������
			if (idChild == CONSOLE_APPLICATION_16BIT) {
				mn_ActiveStatus |= CES_NTVDM;
				SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
			}
		} break;
    case EVENT_CONSOLE_END_APPLICATION:
		//A console process has exited. 
		//The idObject parameter contains the process identifier of the terminated process.
		{
			ProcessDelete(idObject);
			//
			if (idChild == CONSOLE_APPLICATION_16BIT) {
                gConEmu.gbPostUpdateWindowSize = true;
                gConEmu.mn_ActiveStatus &= ~CES_NTVDM;
				TODO("������-�� ������ �� ���������, ��� 16��� �� �������� � ������ ��������");
                SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
			}
		} break;
	case EVENT_CONSOLE_UPDATE_REGION: // 0x4002 
		{
		//More than one character has changed. 
		//The idObject parameter is a COORD structure that specifies the start of the changed region. 
		//The idChild parameter is a COORD structure that specifies the end of the changed region.
			SetEvent(mh_ConChanged);
		} break;
	case EVENT_CONSOLE_UPDATE_SCROLL: //0x4004
		{
		//The console has scrolled.
		//The idObject parameter is the horizontal distance the console has scrolled. 
		//The idChild parameter is the vertical distance the console has scrolled.
			SetEvent(mh_ConChanged);
		} break;
	case EVENT_CONSOLE_UPDATE_SIMPLE: //0x4003
		{
		//A single character has changed.
		//The idObject parameter is a COORD structure that specifies the character that has changed.
		//Warning! � ������� ��  ���������� ��� ������!
		//The idChild parameter specifies the character in the low word and the character attributes in the high word.
			COORD crWhere; memmove(&crWhere, &idObject, sizeof(idObject));
			WCHAR ch = (WCHAR)LOWORD(idChild); WORD wA = HIWORD(idChild);
			CSection cs(&con.cs, &con.ncsT);
			if (con.pConChar && con.pConAttr && !isBufferHeight()) {
				int nIdx = crWhere.X+crWhere.Y*con.m_sbi.dwSize.X;
				con.pConChar[nIdx] = ch;
				con.pConAttr[nIdx] = wA;
				cs.Leave();
				SetEvent(mh_ForceReadEvent);
			} else {
				cs.Leave();
				SetEvent(mh_ConChanged); TODO("������-�� ����� � ��� ������� � ������� - ����� ����� ������, �� ����� ������, ��� ����� ���� ���������");
			}
		} break;
	case EVENT_CONSOLE_CARET: //0x4001
		{
		//Warning! WinXPSP3. ��� ������� �������� ������ ���� ������� � ������. 
		//         � � ConEmu ��� ������� �� � ������, ��� ��� ������ �� �����������.
		//The console caret has moved.
		//The idObject parameter is one or more of the following values:
		//		CONSOLE_CARET_SELECTION or CONSOLE_CARET_VISIBLE.
		//The idChild parameter is a COORD structure that specifies the cursor's current position.
			COORD crWhere; memmove(&crWhere, &idChild, sizeof(idChild));
			TODO("� ��� ���� ����� ����� ������ ���������������, �� ������ ��� ���������");
			if (isBufferHeight()) {
				SetEvent(mh_CursorChanged); 
			} else {
				con.m_sbi.dwCursorPosition = crWhere;
				SetEvent(mh_ForceReadEvent);
			}
		} break;
	case EVENT_CONSOLE_LAYOUT: //0x4005
		{
		//The console layout has changed.
			SetEvent(mh_ConChanged);
		} break;
    }
}

WARNING("����������� ������� ��� ��������� ����� � �� ��������� ����, � ������� � ��� �� ����� instance �����");
WARNING("��������� ���������� ����� ����� ����������� - ����� ������� ������ ���� �����, ��� �������� �� ����������");
DWORD CVirtualConsole::ServerThread(LPVOID lpvParam) 
{ 
	CVirtualConsole *pCon = (CVirtualConsole*)lpvParam;
	BOOL fConnected = FALSE;
	DWORD dwErr = 0;
	HANDLE hPipe = NULL; 
	HANDLE hWait[2] = {NULL,NULL};

	_ASSERTE(pCon->hConWnd!=NULL);
	_ASSERTE(pCon->ms_VConServer_Pipe[0]!=0);
	_ASSERTE(pCon->mh_ServerSemaphore!=NULL);
	//wsprintf(pCon->ms_VConServer_Pipe, CEGUIPIPENAME, L".", (DWORD)pCon->hConWnd); //��� mn_ConEmuC_PID

	// The main loop creates an instance of the named pipe and 
	// then waits for a client to connect to it. When the client 
	// connects, a thread is created to handle communications 
	// with that client, and the loop is repeated. 
	
	hWait[0] = pCon->mh_TermEvent;
	hWait[1] = pCon->mh_ServerSemaphore;

	// ���� �� ����������� ���������� �������
	{
		while (!fConnected)
		{ 
			_ASSERTE(hPipe == NULL);

			// ��������� ���������� ��������, ��� �������� �������
			dwErr = WaitForMultipleObjects ( 2, hWait, FALSE, INFINITE );
			if (dwErr == WAIT_OBJECT_0) {
				return 0; // ������� �����������
			}

			hPipe = CreateNamedPipe( 
				pCon->ms_VConServer_Pipe, // pipe name 
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
			if (WaitForSingleObject ( pCon->mh_TermEvent, 0 ) == WAIT_OBJECT_0) {
				//FlushFileBuffers(hPipe); -- ��� �� �����, �� ������ �� ����������
				//DisconnectNamedPipe(hPipe); 
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
			ReleaseSemaphore(pCon->mh_ServerSemaphore, 1, NULL);
			
			/*{	// ��������� ����� ��������� ����. ���� instance ����� ������ ����� ��������� �������.
				DWORD dwServerTID = 0;
				HANDLE hThread = CreateThread(NULL, 0, ServerThread, (LPVOID)pCon, 0, &dwServerTID);
				_ASSERTE(hThread!=NULL);
				SafeCloseHandle(hThread);
			}*/

			//ServerThreadCommandArg* pArg = (ServerThreadCommandArg*)calloc(sizeof(ServerThreadCommandArg),1);
			//pArg->pCon = pCon;
			//pArg->hPipe = hPipe;
			pCon->ServerThreadCommand ( hPipe ); // ��� ������������� - ���������� � ���� ��������� ����
			//DWORD dwCommandTID = 0;
			//HANDLE hCommandThread = CreateThread(NULL, 0, ServerThreadCommand, (LPVOID)pArg, 0, &dwCommandTID);
			//_ASSERTE(hCommandThread!=NULL);
			//SafeCloseHandle(hCommandThread);
		}

		FlushFileBuffers(hPipe); 
		//DisconnectNamedPipe(hPipe); 
		SafeCloseHandle(hPipe);
	} // ������� � �������� ������ instance �����
	while (WaitForSingleObject ( pCon->mh_TermEvent, 0 ) != WAIT_OBJECT_0);

	return 0; 
}

void CVirtualConsole::ServerThreadCommand(HANDLE hPipe)
{
	//ServerThreadCommandArg* pArg = (ServerThreadCommandArg*)lpvParam;
	//CVirtualConsole* pCon = pArg->pCon;
	//HANDLE hPipe = pArg->hPipe;
	//free(pArg); pArg = NULL;

	CESERVER_REQ in={0}, *pOut=NULL;
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
	_ASSERTE(in.nSize>=12 && cbRead>=12);
	_ASSERTE(in.nVersion == CESERVER_REQ_VER);
	if (cbRead < 12 || /*in.nSize < cbRead ||*/ in.nVersion != CESERVER_REQ_VER) {
		//CloseHandle(hPipe);
		return;
	}

	int nAllSize = in.nSize;
	pOut = (CESERVER_REQ*)calloc(nAllSize,1);
	_ASSERTE(pOut!=NULL);
	memmove(pOut, &in, cbRead);
	_ASSERTE(pOut->nVersion==CESERVER_REQ_VER);

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

	// ��� ������ �� ����� ��������, ������������ ������� � ���������� (���� �����) ���������
	if (pOut->nCmd == CECMD_GETFULLINFO || pOut->nCmd == CECMD_GETSHORTINFO) {
		ApplyConsoleInfo(pOut);
		
	} else if (pOut->nCmd == CECMD_GETGUIHWND) {
		CESERVER_REQ *pRet = NULL;
		int nSize = sizeof(CESERVER_REQ) - sizeof(pRet->Data/*BYTE*/) + 2*sizeof(DWORD);
		pRet = (CESERVER_REQ*)calloc(nSize, 1);
		pRet->nSize = nSize;
		pRet->nCmd = pOut->nCmd;
		pRet->nVersion = CESERVER_REQ_VER;
		((DWORD*)pRet->Data)[0] = (DWORD)ghWnd;
		((DWORD*)pRet->Data)[1] = (DWORD)ghWndDC;
		// ����������
		fSuccess = WriteFile( 
			hPipe,        // handle to pipe 
			pRet,         // buffer to write from 
			pRet->nSize,  // number of bytes to write 
			&cbWritten,   // number of bytes written 
			NULL);        // not overlapped I/O 
	}

	// ���������� ������
	free(pOut);

	//CloseHandle(hPipe);
	return;
}

#define COPYBUFFERS(v,s) { \
	nVarSize = s; \
	if ((lpCur+nVarSize)>lpEnd) { \
		_ASSERT(FALSE); \
		return; \
	} \
	memmove(&(v), lpCur, nVarSize); \
	lpCur += nVarSize; \
}
#define COPYBUFFER(v) { \
	COPYBUFFERS(v, sizeof(v)); \
}

WARNING("��� ����������� ������� � ����������� �������� ����� ������������ Semaphore nor CriticalSection");
void CVirtualConsole::ApplyConsoleInfo(CESERVER_REQ* pInfo)
{
	_ASSERTE(this!=NULL);
	if (this==NULL) return;

	BOOL bBufRecreated = FALSE;
	// ������ ����� ��������� ������ ��� ����������
	LPBYTE lpCur = (LPBYTE)pInfo->Data;
	LPBYTE lpEnd = ((LPBYTE)pInfo)+pInfo->nSize;
	DWORD  nVarSize;
	// 1
	HWND hWnd = NULL;
	COPYBUFFER(hWnd);
	_ASSERTE(hWnd!=NULL);
	if (hConWnd != hWnd) {
		SetHwnd ( hWnd );
	}
	// 2 - GetTickCount ���������� ������
	DWORD nLastConReadTick = 0;
	COPYBUFFER(nLastConReadTick);
	// 3
	// �� ����� ������ ����������� ������� ����� ����������� ���������� ���������
	// ������� �������� ���������� - �������� ����� ������ � CriticalSection(csProc);
	//CSection SPRC(&csPRC, &ncsTPRC);
	//TODO("������� ���������� ���������! �� ������� m_Processes �� �����, � ������� ������ �������������! ����� �������� ���������� � ��������");
	DWORD dwProcCount = 0;
	COPYBUFFER(dwProcCount);
	_ASSERTE(dwProcCount==0); // ������ ��������� ����������� � GUI. Reserved...
	if (dwProcCount) lpCur += sizeof(DWORD)*dwProcCount;
	//m_Processes.clear();
	//ConProcess prc = {0};
	//while (dwProcCount>0) {
	//	prc.ProcessID = *((DWORD*)lpCur); lpCur += sizeof(DWORD);
	//	m_Processes.push_back(prc);
	//	dwProcCount--;
	//}
	//SPRC.Leave();

	// ������ ����� ������� ������ - �������� ��������� ���������� ������
	CSection sc(&con.cs, &con.ncsT);

	// 4
	DWORD dwSelRc = 0; //CONSOLE_SELECTION_INFO sel = {0}; // GetConsoleSelectionInfo
	COPYBUFFER(dwSelRc);
	if (dwSelRc != 0) {
		_ASSERTE(dwSelRc == sizeof(con.m_sel));
		COPYBUFFER(con.m_sel);
	}
	// 5
	DWORD dwCiRc = 0; //CONSOLE_CURSOR_INFO ci = {0}; // GetConsoleCursorInfo
	COPYBUFFER(dwCiRc);
	if (dwCiRc != 0) {
		_ASSERTE(dwCiRc == sizeof(con.m_ci));
		COPYBUFFER(con.m_ci);
	}
	// 6, 7, 8
	COPYBUFFER(con.m_dwConsoleCP);       // GetConsoleCP()
	COPYBUFFER(con.m_dwConsoleOutputCP); // GetConsoleOutputCP()
	COPYBUFFER(con.m_dwConsoleMode);     // GetConsoleMode(hConIn, &dwConsoleMode);
	// 9
	DWORD dwSbiRc = 0; //CONSOLE_SCREEN_BUFFER_INFO sbi = {{0,0}}; // GetConsoleScreenBufferInfo
	COPYBUFFER(dwSbiRc);
	int nNewWidth = 0, nNewHeight = 0;
	if (dwSbiRc != 0) {
		_ASSERTE(dwSbiRc == sizeof(con.m_sbi));
		COPYBUFFER(con.m_sbi);
		if (GetConWindowSize(con.m_sbi, nNewWidth, nNewHeight)) {
			if (nNewWidth != con.nTextWidth || nNewHeight != con.nTextHeight) {
				bBufRecreated = TRUE;
				InitBuffers(nNewWidth*nNewHeight*2);
			}
		}
	}
	// 10
	DWORD dwCharChanged = 0;
	COPYBUFFER(dwCharChanged);
	if (dwCharChanged != 0) {
		_ASSERTE(dwCharChanged >= sizeof(CESERVER_CHAR));
		_ASSERTE(!bBufRecreated && con.pConChar && con.pConAttr);

		// ���� ����� ��� ���������� (bBufRecreated) ��������� ������ ����� �������� ������������ - ����� ������ ������!
		if (!bBufRecreated && con.pConChar && con.pConAttr) {
			CESERVER_CHAR* pch = (CESERVER_CHAR*)calloc(dwCharChanged,1);
			COPYBUFFERS(*pch,2*sizeof(COORD));
			int nLineLen = pch->crEnd.X - pch->crStart.X + 1;
			int nLineCount = pch->crEnd.Y - pch->crStart.Y + 1;
			_ASSERTE((nLineLen*nLineCount*4+2*sizeof(COORD))==dwCharChanged);
			COPYBUFFERS(pch->data,dwCharChanged-2*sizeof(COORD));
			_ASSERTE(!isBufferHeight());
			wchar_t* pszLine = (wchar_t*)(pch->data);
			WORD*    pnLine  = ((WORD*)pch->data)+(nLineLen*nLineCount);
			// ���������� ������ ���������� � ����� ���������� - ������� ����� �������
			for (int y = pch->crStart.Y; y <= pch->crEnd.Y; y++) {
				int nIdx = pch->crStart.X + y * con.m_sbi.dwSize.X;
				memmove(con.pConChar+nIdx, pszLine, nLineLen*2);
					pszLine += nLineLen;
				memmove(con.pConAttr+nIdx, pnLine, nLineLen*2);
					pnLine += nLineLen;
			}
		} else {
			lpCur += dwCharChanged;
		}
	}
	// 11
	DWORD OneBufferSize = 0;
	COPYBUFFER(OneBufferSize);
	if (OneBufferSize != 0) {
		if (InitBuffers(OneBufferSize)) {
			memmove(con.pConChar, lpCur, OneBufferSize); lpCur += OneBufferSize;
			memmove(con.pConAttr, lpCur, OneBufferSize); lpCur += OneBufferSize;
		}
	}

	sc.Leave();

	if (mn_LastConReadTick) {
		DWORD dwDelta = nLastConReadTick - mn_LastConReadTick;
		// ����� ���������� ����� 0
		_ASSERTE(mn_LastConReadTick <= nLastConReadTick || dwDelta > 0x1000000);
	}
	mn_LastConReadTick = nLastConReadTick;

	
	// ����������� GUI
	SetEvent(mh_ForceReadEvent);
}

int CVirtualConsole::GetProcesses(ConProcess** ppPrc)
{
	// ���� ����� ������ ������ ���������� ���������
	if (ppPrc == NULL || mn_ProcessCount == 0) {
		if (ppPrc) *ppPrc = NULL;
		return mn_ProcessCount;
	}

	CSection SPRC(&csPRC, &ncsTPRC);
	
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
	
	SPRC.Leave();
	return dwProcCount;
}

DWORD CVirtualConsole::GetActiveStatus()
{
	return mn_ActiveStatus;
}

DWORD CVirtualConsole::GetServerPID()
{
	return mn_ConEmuC_PID;
}

DWORD CVirtualConsole::GetFarPID()
{
	if (!this)
		return 0;

	if ((mn_ActiveStatus & CES_FARACTIVE) == 0)
		return 0;

	return mn_FarPID;
}

// �������� ������ ���������� ��������
void CVirtualConsole::ProcessUpdateFlags(BOOL abProcessChanged)
{
	//Warning: ������ ���������� ������ �� ProcessAdd/ProcessDelete, �.�. ��� ������ �� ���������

    bool bIsFar=false, bIsTelnet=false, bIsNtvdm=false, bIsCmd=false;
	DWORD dwPID = 0;

	std::vector<ConProcess>::reverse_iterator iter = m_Processes.rbegin();
	while (iter!=m_Processes.rend()) {
		// �������� ������� ConEmuC �� ���������!
		if (iter->ProcessID != mn_ConEmuC_PID) {
			if (!bIsFar && iter->IsFar) bIsFar = true;
			if (!bIsTelnet && iter->IsTelnet) bIsTelnet = true;
			if (!bIsNtvdm && iter->IsNtvdm) bIsNtvdm = true;
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
	    
    mn_ActiveStatus &= ~CES_PROGRAMS2;
    if (bIsFar) mn_ActiveStatus |= CES_FARACTIVE;
    if (bIsTelnet) mn_ActiveStatus |= CES_TELNETACTIVE;
    if (bIsNtvdm) mn_ActiveStatus |= CES_NTVDM;

	mn_ProcessCount = m_Processes.size();
	mn_FarPID = dwPID;

	if (mn_ProcessCount == 0)
		StopSignal();

    // �������� ������ ��������� � ���� �������� � ��������
    if (abProcessChanged) {
        gConEmu.UpdateProcessDisplay(abProcessChanged);
	    TabBar.Refresh(mn_ActiveStatus & CES_FARACTIVE);
    }
}

void CVirtualConsole::ProcessAdd(DWORD addPID)
{
	CSection SPRC(&csPRC, &ncsTPRC);
	
	std::vector<ConProcess>::iterator iter;
	BOOL bAlive = FALSE;
	BOOL bProcessChanged = FALSE;
	
	// ����� �� ��� ���� � ������?
	iter=m_Processes.begin();
	while (iter!=m_Processes.end()) {
		if (addPID == iter->ProcessID) {
			addPID = 0;
			break;
		}
		iter++;
	}

	

	// ������ ����� �������� ����� �������
	ConProcess cp;
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    _ASSERTE(h!=INVALID_HANDLE_VALUE);
    if (h==INVALID_HANDLE_VALUE) {
        DWORD dwErr = GetLastError();
        // ������ ��������, ��� �� ������� ����� Snapshoot
		memset(&cp, 0, sizeof(cp));
		cp.ProcessID = addPID; cp.RetryName = true;
		wsprintf(cp.Name, L"Can't create snaphoot. ErrCode=0x%08X", dwErr);
		m_Processes.push_back(cp);
		
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
	            if (addPID && addPID == p.th32ProcessID) {
		            if (!bProcessChanged) bProcessChanged = TRUE;
					memset(&cp, 0, sizeof(cp));
					cp.ProcessID = addPID; cp.ParentPID = p.th32ParentProcessID;
                    ProcessCheckName(cp, p.szExeFile); //far, telnet, cmd, conemuc, � ��.
                    cp.Alive = true;
					m_Processes.push_back(cp);
		            break;
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
        
        // ������ ��������, ������� ��� ���
        iter = m_Processes.begin();
        while (iter != m_Processes.end())
        {
            if (!iter->Alive) {
	            if (!bProcessChanged) bProcessChanged = TRUE;
	            iter = m_Processes.erase(iter);
	        } else {
                iter ++;
            }
        }
        
        // ������� shapshoot
        SafeCloseHandle(h);
    }
    
    // �������� ������ ���������� ��������, �������� PID FAR'�, ��������� ���������� ��������� � �������
	ProcessUpdateFlags(bProcessChanged);
}

void CVirtualConsole::ProcessDelete(DWORD addPID)
{
	CSection SPRC(&csPRC, &ncsTPRC);

	BOOL bProcessChanged = FALSE;
	std::vector<ConProcess>::iterator iter=m_Processes.begin();
	while (iter!=m_Processes.end()) {
		if (addPID == iter->ProcessID) {
			m_Processes.erase(iter);
			bProcessChanged = TRUE;
			break;
		}
		iter++;
	}

    // �������� ������ ���������� ��������, �������� PID FAR'�, ��������� ���������� ��������� � �������
	ProcessUpdateFlags(bProcessChanged);
}

void CVirtualConsole::ProcessCheckName(struct ConProcess &ConPrc, LPWSTR asFullFileName)
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


#define BUFSIZE 512
 
BOOL CVirtualConsole::RetrieveConsoleInfo(BOOL bShortOnly)
{
	TODO("!!! WinEvent ����� ��������� � ConEmu. �������� ������ �������� �� ��������� ������� ����� ������������ ��� ������� ConEmuC");

	TODO("!!! ���������� ������� ������, ��� ������ ����� CECMD_GETFULLINFO/CECMD_GETSHORTINFO");

	BOOL lbRc = FALSE;
	HANDLE hPipe = NULL; 
	CESERVER_REQ in={0}, *pOut=NULL;
	BYTE cbReadBuf[BUFSIZE];
	BOOL fSuccess;
	DWORD cbRead, dwMode, dwErr;

	gSet.Performance(tPerfRead, FALSE);
	//#ifdef _DEBUG
	//DWORD dwStartTick = GetTickCount(), dwEndTick, dwDelta;
	//DEBUGSTR(L"CVirtualConsole::RetrieveConsoleInfo");
	//#endif

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
		if (dwErr = WAIT_OBJECT_0)
			return FALSE;
		continue;
		//DisplayLastError(L"Could not open pipe", dwErr);
		//return 0;
	  }

	  // All pipe instances are busy, so wait for 1 second.
	  if (!WaitNamedPipe(ms_ConEmuC_Pipe, 1000) ) 
	  {
		dwErr = WaitForSingleObject(mh_ConEmuC, 100);
		if (dwErr = WAIT_OBJECT_0) {
			DEBUGSTR(L" - FAILED!\n");
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
	  DEBUGSTR(L" - FAILED!\n");
	  DisplayLastError(L"SetNamedPipeHandleState failed");
	  CloseHandle(hPipe);
	  return 0;
	}

	in.nSize = 12;
	in.nVersion = CESERVER_REQ_VER;
	in.nCmd  = bShortOnly ? CECMD_GETSHORTINFO : CECMD_GETFULLINFO;

	// Send a message to the pipe server and read the response. 
	fSuccess = TransactNamedPipe( 
	  hPipe,                  // pipe handle 
	  &in,                    // message to server
	  in.nSize,               // message length 
	  cbReadBuf,              // buffer to receive reply
	  BUFSIZE*sizeof(BYTE),   // size of read buffer
	  &cbRead,                // bytes read
	  NULL);                  // not overlapped 

	if (!fSuccess && (GetLastError() != ERROR_MORE_DATA)) 
	{
	  DEBUGSTR(L" - FAILED!\n");
	  DisplayLastError(L"TransactNamedPipe failed"); 
	  CloseHandle(hPipe);
	  return 0;
	}

	int nAllSize = *((DWORD*)cbReadBuf);
	if (nAllSize==0) {
	   DEBUGSTR(L" - FAILED!\n");
	   DisplayLastError(L"Empty data recieved from server", 0);
	   CloseHandle(hPipe);
	   return 0;
	}
	pOut = (CESERVER_REQ*)calloc(nAllSize,1);
	_ASSERTE(pOut!=NULL);
	memmove(pOut, cbReadBuf, cbRead);
	_ASSERTE(pOut->nVersion==CESERVER_REQ_VER);

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

	CloseHandle(hPipe);

	ApplyConsoleInfo ( pOut );

	//// ������ ����� ������� ������ - �������� ��������� ���������� ������
	//CSection SCON(&csCON, &ncsTCON);
	//// ������ ����� ��������� ������ ��� ����������
	//LPBYTE lpCur = (LPBYTE)pOut->Data;
	//// 1
	//HWND hWnd = *((HWND*)lpCur);
	//_ASSERTE(hWnd!=NULL);
	//if (hConWnd != hWnd) {
	//	hConWnd = hWnd;
	//	InitHandlers(TRUE);
	//}
	//lpCur += sizeof(hWnd);
	//// 2
	//// �� ����� ������ ����������� ������� ����� ����������� ���������� ���������
	//// ������� �������� ���������� - �������� ����� ������ � CriticalSection(csProc);
	////CSection SPRC(&csPRC, &ncsTPRC);
	////TODO("������� ���������� ���������! �� ������� m_Processes �� �����, � ������� ������ �������������! ����� �������� ���������� � ��������");
	//int dwProcCount = (int)*((DWORD*)lpCur); lpCur += sizeof(DWORD);
	//if (dwProcCount) lpCur += sizeof(DWORD)*dwProcCount;
	////m_Processes.clear();
	////ConProcess prc = {0};
	////while (dwProcCount>0) {
	////	prc.ProcessID = *((DWORD*)lpCur); lpCur += sizeof(DWORD);
	////	m_Processes.push_back(prc);
	////	dwProcCount--;
	////}
	////SPRC.Leave();
	//// 3
	//DWORD dwSelRc = *((DWORD*)lpCur); lpCur += sizeof(DWORD); //CONSOLE_SELECTION_INFO sel = {0}; // GetConsoleSelectionInfo
	//memmove(&m_sel, lpCur, sizeof(m_sel));
	//lpCur += sizeof(m_sel);
	//// 4
	//DWORD dwCiRc = *((DWORD*)lpCur); lpCur += sizeof(DWORD); //CONSOLE_CURSOR_INFO ci = {0}; // GetConsoleCursorInfo
	//memmove(&m_ci, lpCur, sizeof(m_ci));
	//lpCur += sizeof(cinf);
	//// 5, 6, 7
	//m_dwConsoleCP = *((DWORD*)lpCur); lpCur += sizeof(DWORD);       // GetConsoleCP()
	//m_dwConsoleOutputCP = *((DWORD*)lpCur); lpCur += sizeof(DWORD); // GetConsoleOutputCP()
	//m_dwConsoleMode = *((DWORD*)lpCur); lpCur += sizeof(DWORD);     // GetConsoleMode(hConIn, &dwConsoleMode);
	//// 8
	//DWORD dwSbiRc = *((DWORD*)lpCur); lpCur += sizeof(DWORD); //CONSOLE_SCREEN_BUFFER_INFO sbi = {{0,0}}; // GetConsoleScreenBufferInfo
	//memmove(&m_sbi, lpCur, sizeof(csbi));
	//lpCur += sizeof(csbi);
	//// 9
	//if (!bShortOnly) {
	//	DWORD OneBufferSize = *((DWORD*)lpCur); lpCur += sizeof(DWORD);
	//	TODO("����������� ���������� �������, ���� OneBufferSize>0");
	//	if (InitBuffers()) {
	//		memmove(ConChar, lpCur, OneBufferSize); lpCur += OneBufferSize;
	//		memmove(ConAttr, lpCur, OneBufferSize); lpCur += OneBufferSize;
	//	}
	//}

	// ���������� ������
	free(pOut);

	//#ifdef _DEBUG
	//dwEndTick = GetTickCount();
	//dwDelta = dwEndTick - dwStartTick;
	//WCHAR szText[64]; wsprintfW(szText, L" - SUCCEEDED (%i ms)!\n", dwDelta);
	//DEBUGSTR(szText);
	//#endif
	gSet.Performance(tPerfRead, TRUE);
    return TRUE;
}

BOOL CVirtualConsole::GetConWindowSize(const CONSOLE_SCREEN_BUFFER_INFO& sbi, int& nNewWidth, int& nNewHeight)
{
	nNewWidth  = con.m_sbi.srWindow.Right - con.m_sbi.srWindow.Left + 1;
	nNewHeight = con.m_sbi.srWindow.Bottom - con.m_sbi.srWindow.Top + 1;

#ifdef _DEBUG
	_ASSERTE(nNewHeight >= 5);
#endif
	if (!nNewWidth || !nNewHeight) {
		Assert(nNewWidth && nNewHeight);
		return FALSE;
	}

	if (nNewWidth < con.m_sbi.dwSize.X)
		nNewWidth = con.m_sbi.dwSize.X;

	return TRUE;
}

BOOL CVirtualConsole::InitBuffers(DWORD OneBufferSize)
{
	BOOL lbRc = FALSE;
	int nNewWidth = 0, nNewHeight = 0;

	if (!GetConWindowSize(con.m_sbi, nNewWidth, nNewHeight))
		return FALSE;

	if (OneBufferSize) {
		_ASSERTE((nNewWidth * nNewHeight * sizeof(*con.pConChar)) == OneBufferSize);
		if ((nNewWidth * nNewHeight * sizeof(*con.pConChar)) != OneBufferSize)
			return FALSE;
	}

	// ���� ��������� ��������� ��� ������� (��������) ������
	if (!con.pConChar || (con.nTextWidth*con.nTextHeight) < (nNewWidth*nNewHeight))
	{
		CSection sc(&con.cs, &con.ncsT);

		if (con.pConChar)
			{ Free(con.pConChar); con.pConChar = NULL; }
		if (con.pConAttr)
			{ Free(con.pConAttr); con.pConAttr = NULL; }

		MCHKHEAP
		con.pConChar = (TCHAR*)Alloc((nNewWidth * nNewHeight * 2), sizeof(*con.pConChar));
			_ASSERTE(con.pConChar!=NULL);
		con.pConAttr = (WORD*)Alloc((nNewWidth * nNewHeight * 2), sizeof(*con.pConAttr));
			_ASSERTE(con.pConAttr!=NULL);

		sc.Leave();

		lbRc = con.pConChar!=NULL && con.pConAttr!=NULL;
	} else if (TextWidth!=nNewWidth || TextHeight!=nNewHeight) {
		memset(con.pConChar, 0, (nNewWidth * nNewHeight * 2) * sizeof(*con.pConChar));
		memset(con.pConAttr, 0, (nNewWidth * nNewHeight * 2) * sizeof(*con.pConAttr));

		lbRc = TRUE;
	} else {
		lbRc = TRUE;
	}
	MCHKHEAP

	con.nTextWidth = nNewWidth;
	con.nTextHeight = nNewHeight;

	InitDC(false,true);

	return lbRc;
}

void CVirtualConsole::ShowConsole(int nMode) // -1 Toggle, 0 - Hide, 1 - Show
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
        SetWindowPos(hConWnd, HWND_TOPMOST, 
            rcWnd.right-rcCon.right+rcCon.left,rcWnd.bottom-rcCon.bottom+rcCon.top,0,0, SWP_NOSIZE|SWP_SHOWWINDOW);
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

void CVirtualConsole::SetHwnd(HWND ahConWnd)
{
	hConWnd = ahConWnd;

#ifdef _DEBUG
	//MessageBox(0,L"Attached",L"ComEmu",0);
#endif
	
	if (gSet.isConVisible)
		ShowConsole(1); // ���������� ����������� ���� ���� AlwaysOnTop
	
	TODO("InitHandler � GUI �������� ��� � �� �����...");
	//InitHandlers(TRUE);

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
}

