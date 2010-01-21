
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


#define SHOWDEBUGSTR

#include "Header.h"
#include <Tlhelp32.h>
#include "ScreenDump.h"

#define DEBUGSTRDRAW(s) //DEBUGSTR(s)
#define DEBUGSTRCOORD(s) //DEBUGSTR(s)

// WARNING("�� ���������� ���� �� ������ �������");
WARNING("�� ���������� ������ ������� ��� �������� ������� �����");
WARNING("��� ������� ������ ������ ������ ����� '���������' �� ����� �� ����, �� ���������� ��������� ������");

WARNING("���� � ��������� ���������� ������ ����� ���������� ��� Alt-F7, Enter. ��������, ���� ����� ���");
// ������ �� �������������� ������ ������ ��������� - ������ ����� ������� ����������� ����������.
// ������ ������ �����������, �� ����� �� ������ "..." �����������

//PRAGMA_ERROR("��� ������� ���������� F:\\VCProject\\FarPlugin\\PPCReg\\compile.cmd - Enter � ������� �� ������");

WARNING("����� ���� ������ ServerThread? � StartProcessThread ��������������� ������ ��� ������� ��������, � ����� �� ���� �����");

WARNING("� ������ VCon ������� ����� BYTE[256] ��� �������� ������������ ������ (Ctrl,...,Up,PgDn,Add,� ��.");
WARNING("�������������� ����� �������� � ����� {VKEY,wchar_t=0}, � ������� ��������� ��������� wchar_t �� WM_CHAR/WM_SYSCHAR");
WARNING("��� WM_(SYS)CHAR �������� wchar_t � ������, � ������ ��������� VKEY");
WARNING("��� ������������ WM_KEYUP - �����(� ������) wchar_t �� ����� ������, ������ � ������� UP");
TODO("� ������������ - ��������� �������� isKeyDown, � ������� �����");
WARNING("��� ������������ �� ������ ������� (�� �������� � � �������� ������ ������� - ����������� ����� ���� ������� � ����� ���������) ��������� �������� caps, scroll, num");
WARNING("� ����� ���������� �������/������� ��������� ����� �� �� ���������� Ctrl/Shift/Alt");

WARNING("����� ����� ��������������� ���������� ������ ������� ���������� (OK), �� ����������� ���������� ������� �� �������� � GUI - ��� ������� ���������� ������� � ������ ������");

WARNING("! � ������ ������ (��������) ������������ � VirtualConsole? �� ����� �� ����������� ������ ������� ������, � ������ �������� ��� � ����, ��� �������������");


//������, ��� ���������, ������ �������, ���������� �����, � ��...

#define VCURSORWIDTH 2
#define HCURSORHEIGHT 2

#define Assert(V) if ((V)==FALSE) { TCHAR szAMsg[MAX_PATH*2]; wsprintf(szAMsg, _T("Assertion (%s) at\n%s:%i"), _T(#V), _T(__FILE__), __LINE__); Box(szAMsg); }

#ifdef _DEBUG
#undef MCHKHEAP
#define MCHKHEAP //HeapValidate(mh_Heap, 0, NULL);
#define CURSOR_ALWAYS_VISIBLE
#endif

#define ISBGIMGCOLOR(a) (gSet.nBgImageColors & (1 << a))

#ifndef CONSOLE_SELECTION_NOT_EMPTY
#define CONSOLE_SELECTION_NOT_EMPTY     0x0002   // non-null select rectangle
#endif

#ifdef _DEBUG
#define DUMPDC(f) if (mb_DebugDumpDC) DumpImage(hDC, Width, Height, f);
#else
#define DUMPDC(f)
#endif

#define isCharSpace(c) (c == ucSpace || c == ucNoBreakSpace || c == 0)



CVirtualConsole::PARTBRUSHES CVirtualConsole::m_PartBrushes[MAX_COUNT_PART_BRUSHES] = {{0}};
char CVirtualConsole::mc_Uni2Oem[0x10000];
// MAX_SPACES == 0x400
wchar_t CVirtualConsole::ms_Spaces[MAX_SPACES];
wchar_t CVirtualConsole::ms_HorzDbl[MAX_SPACES];
wchar_t CVirtualConsole::ms_HorzSingl[MAX_SPACES];

CVirtualConsole* CVirtualConsole::CreateVCon(RConStartArgs *args)
{
    CVirtualConsole* pCon = new CVirtualConsole();
    
    if (!pCon->mp_RCon->PreCreate(args)) {
        delete pCon;
        return NULL;
    }

    return pCon;
}

CVirtualConsole::CVirtualConsole(/*HANDLE hConsoleOutput*/)
{
    mp_RCon = new CRealConsole(this);

	gConEmu.OnVConCreated(this);

    SIZE_T nMinHeapSize = (1000 + (200 * 90 * 2) * 6 + MAX_PATH*2)*2 + 210*sizeof(*TextParts);
    mh_Heap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, nMinHeapSize, 0);
   
    cinf.dwSize = 15; cinf.bVisible = TRUE;
    ZeroStruct(winSize); ZeroStruct(coord);
    TextLen = 0;
	mb_RequiredForceUpdate = true;
  
    //InitializeCriticalSection(&csDC); ncsTDC = 0; 
	mb_PaintRequested = FALSE; mb_PaintLocked = FALSE;
    //InitializeCriticalSection(&csCON); ncsTCON = 0;

	mb_InPaintCall = FALSE;

	nFontHeight = gSet.FontHeight();
	nFontWidth = gSet.FontWidth();
	nFontCharSet = gSet.FontCharSet();
	nLastNormalBack = 255;

#ifdef _DEBUG
    mn_BackColorIdx = 2;
#else
    mn_BackColorIdx = 0;
#endif
    memset(&Cursor, 0, sizeof(Cursor));
    Cursor.nBlinkTime = GetCaretBlinkTime();

    TextWidth = TextHeight = Width = Height = nMaxTextWidth = nMaxTextHeight = 0;
    hDC = NULL; hBitmap = NULL;
    hSelectedFont = NULL; hOldFont = NULL;
    mpsz_ConChar = NULL; mpsz_ConCharSave = NULL;
    mpn_ConAttr = NULL; mpn_ConAttrSave = NULL;
    ConCharX = NULL; 
    tmpOem = NULL; 
    TextParts = NULL;
    mb_IsForceUpdate = false;
    hBrush0 = NULL; hSelectedBrush = NULL; hOldBrush = NULL;
    isEditor = false;
    memset(&csbi, 0, sizeof(csbi)); mdw_LastError = 0;

    //nSpaceCount = 1000;
    //Spaces = (TCHAR*)Alloc(nSpaceCount,sizeof(TCHAR));
    //for (UINT i=0; i<nSpaceCount; i++) Spaces[i]=L' ';
    if (!ms_Spaces[0]) {
    	wmemset(ms_Spaces, L' ', MAX_SPACES);
    	wmemset(ms_HorzDbl, ucBoxDblHorz, MAX_SPACES);
    	wmemset(ms_HorzSingl, ucBoxSinglHorz, MAX_SPACES);
	}


    hOldBrush = NULL;
    hOldFont = NULL;
    
    if (gSet.wndWidth)
        TextWidth = gSet.wndWidth;
    if (gSet.wndHeight)
        TextHeight = gSet.wndHeight;
    

	#ifdef _DEBUG
	mb_DebugDumpDC = FALSE;
	#endif

    if (gSet.isShowBgImage)
        gSet.LoadImageFrom(gSet.sBgImage);

    if (gSet.isAdvLogging != 3) {
        mpsz_LogScreen = NULL;
    } else {
        mn_LogScreenIdx = 0;
        //DWORD dwErr = 0;
        wchar_t szFile[MAX_PATH+64], *pszDot;
		lstrcpyW(szFile, gConEmu.ms_ConEmuExe);
        if ((pszDot = wcsrchr(szFile, L'\\')) == NULL) {
            DisplayLastError(L"wcsrchr failed!");
            return; // ������
        }
        *pszDot = 0;

        mpsz_LogScreen = (wchar_t*)calloc(pszDot - szFile + 64, 2);
        wcscpy(mpsz_LogScreen, szFile);
        wcscpy(mpsz_LogScreen+wcslen(mpsz_LogScreen), L"\\ConEmu-VCon-%i-%i.con"/*, RCon()->GetServerPID()*/);
    }

    // InitDC ����� ������������ - ������� ��� �� �������
}

CVirtualConsole::~CVirtualConsole()
{
    if (mp_RCon)
        mp_RCon->StopThread();

    MCHKHEAP;
    if (hDC)
        { DeleteDC(hDC); hDC = NULL; }
    if (hBitmap)
        { DeleteObject(hBitmap); hBitmap = NULL; }
    if (mpsz_ConChar)
        { Free(mpsz_ConChar); mpsz_ConChar = NULL; }
    if (mpsz_ConCharSave)
        { Free(mpsz_ConCharSave); mpsz_ConCharSave = NULL; }
    if (mpn_ConAttr)
        { Free(mpn_ConAttr); mpn_ConAttr = NULL; }
    if (mpn_ConAttrSave)
        { Free(mpn_ConAttrSave); mpn_ConAttrSave = NULL; }
    if (ConCharX)
        { Free(ConCharX); ConCharX = NULL; }
    if (tmpOem)
        { Free(tmpOem); tmpOem = NULL; }
    if (TextParts)
        { Free(TextParts); TextParts = NULL; }
    //if (Spaces) 
    //    { Free(Spaces); Spaces = NULL; nSpaceCount = 0; }

    // ���� ������ �� �����
    if (mh_Heap) {
        HeapDestroy(mh_Heap);
        mh_Heap = NULL;
    }

    if (mpsz_LogScreen) {
        //wchar_t szMask[MAX_PATH*2]; wcscpy(szMask, mpsz_LogScreen);
        //wchar_t *psz = wcsrchr(szMask, L'%');
        //if (psz) {
        //    wcscpy(psz, L"*.*");
        //    psz = wcsrchr(szMask, L'\\'); 
        //    if (psz) {
        //        psz++;
        //        WIN32_FIND_DATA fnd;
        //        HANDLE hFind = FindFirstFile(szMask, &fnd);
        //        if (hFind != INVALID_HANDLE_VALUE) {
        //            do {
        //                if ((fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0) {
        //                    wcscpy(psz, fnd.cFileName);
        //                    DeleteFile(szMask);
        //                }
        //            } while (FindNextFile(hFind, &fnd));
        //            FindClose(hFind);
        //        }
        //    }
        //}
        free(mpsz_LogScreen); mpsz_LogScreen = NULL;
    }
    
    //DeleteCriticalSection(&csDC);
    //DeleteCriticalSection(&csCON);

    if (mp_RCon) {
        delete mp_RCon;
        mp_RCon = NULL;
    }
}

#ifdef _DEBUG
#define HEAPVAL HeapValidate(mh_Heap, 0, NULL);
#else
#define HEAPVAL 
#endif


// InitDC ���������� ������ ��� ����������� ���������� (�������, �����, � �.�.) ����� ����� ����������� DC � Bitmap
bool CVirtualConsole::InitDC(bool abNoDc, bool abNoWndResize)
{
    MSectionLock SCON; SCON.Lock(&csCON);
    BOOL lbNeedCreateBuffers = FALSE;

#ifdef _DEBUG
    if (mp_RCon->IsConsoleThread()) {
        //_ASSERTE(!mp_RCon->IsConsoleThread());
    }
#endif

    // ����� ����������� ������ ���� ��������� ��� ����������
    if (!mpsz_ConChar || !mpsz_ConCharSave || !mpn_ConAttr || !mpn_ConAttrSave || !ConCharX || !tmpOem || !TextParts ||
        (nMaxTextWidth * nMaxTextHeight) < (mp_RCon->TextWidth() * mp_RCon->TextHeight()) ||
        (nMaxTextWidth < mp_RCon->TextWidth()) // � ��� ����� ��� TextParts & tmpOem
        )
        lbNeedCreateBuffers = TRUE;

    if (!mp_RCon->TextWidth() || !mp_RCon->TextHeight()) {
        WARNING("���� ��� ������ - ����� ������ DC Initialization failed, ��� �� �������...");
        Assert(mp_RCon->TextWidth() && mp_RCon->TextHeight());
        return false;
    }


    if (lbNeedCreateBuffers) {
		DEBUGSTRDRAW(L"Relocking SCON exclusively\n");
        SCON.RelockExclusive();
		DEBUGSTRDRAW(L"Relocking SCON exclusively (done)\n");

        HEAPVAL
        if (mpsz_ConChar)
            { Free(mpsz_ConChar); mpsz_ConChar = NULL; }
        if (mpsz_ConCharSave)
            { Free(mpsz_ConCharSave); mpsz_ConCharSave = NULL; }
        if (mpn_ConAttr)
            { Free(mpn_ConAttr); mpn_ConAttr = NULL; }
        if (mpn_ConAttrSave)
            { Free(mpn_ConAttrSave); mpn_ConAttrSave = NULL; }
        if (ConCharX)
            { Free(ConCharX); ConCharX = NULL; }
        if (tmpOem)
            { Free(tmpOem); tmpOem = NULL; }
        if (TextParts)
            { Free(TextParts); TextParts = NULL; }
    }

    #ifdef _DEBUG
    HeapValidate(mh_Heap, 0, NULL);
    #endif

    //CONSOLE_SCREEN_BUFFER_INFO csbi;
    //if (!GetConsoleScreenBufferInfo())         return false;

    mb_IsForceUpdate = true;
    //TextWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    //TextHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    TextWidth = mp_RCon->TextWidth();
    TextHeight = mp_RCon->TextHeight();

#ifdef _DEBUG
    if (TextHeight == 24)
        TextHeight = 24;
    _ASSERT(TextHeight >= 5);
#endif


    //if ((int)TextWidth < csbi.dwSize.X)
    //    TextWidth = csbi.dwSize.X;

    //MCHKHEAP
    if (lbNeedCreateBuffers) {
        if (nMaxTextWidth < TextWidth)
            nMaxTextWidth = TextWidth;
        if (nMaxTextHeight < TextHeight)
            nMaxTextHeight = TextHeight;

        HEAPVAL;
        mpsz_ConChar = (TCHAR*)Alloc((nMaxTextWidth * nMaxTextHeight), sizeof(*mpsz_ConChar));
        mpsz_ConCharSave = (TCHAR*)Alloc((nMaxTextWidth * nMaxTextHeight), sizeof(*mpsz_ConCharSave));
        mpn_ConAttr = (WORD*)Alloc((nMaxTextWidth * nMaxTextHeight), sizeof(*mpn_ConAttr));
        mpn_ConAttrSave = (WORD*)Alloc((nMaxTextWidth * nMaxTextHeight), sizeof(*mpn_ConAttrSave));
        ConCharX = (DWORD*)Alloc((nMaxTextWidth * nMaxTextHeight), sizeof(*ConCharX));
        tmpOem = (char*)Alloc((nMaxTextWidth + 5), sizeof(*tmpOem));
        TextParts = (struct _TextParts*)Alloc((nMaxTextWidth + 2), sizeof(*TextParts));
        HEAPVAL;
    }
    //MCHKHEAP
    if (!mpsz_ConChar || !mpsz_ConCharSave || !mpn_ConAttr || !mpn_ConAttrSave || !ConCharX || !tmpOem || !TextParts) {
        WARNING("���� ��� ������ - ����� ������ DC Initialization failed, ��� �� �������...");
        return false;
    }

    if (!lbNeedCreateBuffers) {
        HEAPVAL
        ZeroMemory(mpsz_ConChar, (TextWidth * TextHeight)*sizeof(*mpsz_ConChar));
        ZeroMemory(mpsz_ConCharSave, (TextWidth * TextHeight)*sizeof(*mpsz_ConChar));
        HEAPVAL
        ZeroMemory(mpn_ConAttr, (TextWidth * TextHeight)*sizeof(*mpn_ConAttr));
        ZeroMemory(mpn_ConAttrSave, (TextWidth * TextHeight)*sizeof(*mpn_ConAttr));
        HEAPVAL
        ZeroMemory(ConCharX, (TextWidth * TextHeight)*sizeof(*ConCharX));
        HEAPVAL
        ZeroMemory(tmpOem, (TextWidth + 5)*sizeof(*tmpOem));
        HEAPVAL
        ZeroMemory(TextParts, (TextWidth + 2)*sizeof(*TextParts));
        HEAPVAL
    }

    SCON.Unlock();

    HEAPVAL

    hSelectedFont = NULL;

    // ��� ����� ����, ���� ��������� ����������� (debug) � ����� ���� ����� �� �����
    if (!abNoDc)
    {
		DEBUGSTRDRAW(L"*** Recreate DC\n");
        MSectionLock SDC;
        // ���� � ���� ���� ��� ������������ - ������ �� ���������
        SDC.Lock(&csDC, TRUE, 200); // �� �� ��������, ����� �� ������� ���������

        if (hDC)
        { DeleteDC(hDC); hDC = NULL; }
        if (hBitmap)
        { DeleteObject(hBitmap); hBitmap = NULL; }

        const HDC hScreenDC = GetDC(0);
        if ((hDC = CreateCompatibleDC(hScreenDC)) != NULL)
        {
            Assert ( gSet.FontWidth() && gSet.FontHeight() );
			nFontHeight = gSet.FontHeight();
			nFontWidth = gSet.FontWidth();
			nFontCharSet = gSet.FontCharSet();

            #ifdef _DEBUG
            BOOL lbWasInitialized = TextWidth && TextHeight;
            #endif
            // ��������� ����� ������ � ��������
            Width = TextWidth * nFontWidth;
            Height = TextHeight * nFontHeight;
            _ASSERTE(Height <= 1200);

            if (ghOpWnd)
                gConEmu.UpdateSizes();

            //if (!lbWasInitialized) // ���� ������� InitDC ������ ������ ������� ���������
            if (!abNoWndResize) {
                if (gConEmu.isVisible(this))
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
    if (!this) return;
    
    OPENFILENAME ofn; memset(&ofn,0,sizeof(ofn));
    WCHAR temp[MAX_PATH+5];
    ofn.lStructSize=sizeof(ofn);
    ofn.hwndOwner = ghWnd;
    ofn.lpstrFilter = _T("ConEmu dumps (*.con)\0*.con\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = temp; temp[0] = 0;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = L"Dump console...";
    ofn.lpstrDefExt = L"con";
    ofn.Flags = OFN_ENABLESIZING|OFN_NOCHANGEDIR
            | OFN_PATHMUSTEXIST|OFN_EXPLORER|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
    if (!GetSaveFileName(&ofn))
        return;

    Dump(temp);
}

BOOL CVirtualConsole::Dump(LPCWSTR asFile)
{
    // ��� ������� ������ ������ ������ (hDC) � png ����
    DumpImage(hDC, Width, Height, asFile);
        
    HANDLE hFile = CreateFile(asFile, GENERIC_WRITE, FILE_SHARE_READ,
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DisplayLastError(_T("Can't create file!"));
        return FALSE;
    }
    DWORD dw;
    LPCTSTR pszTitle = gConEmu.GetTitle();
    WriteFile(hFile, pszTitle, _tcslen(pszTitle)*sizeof(*pszTitle), &dw, NULL);
    wchar_t temp[100];
	swprintf(temp, _T("\r\nSize: %ix%i   Cursor: %ix%i\r\n"), TextWidth, TextHeight, Cursor.x, Cursor.y);
    WriteFile(hFile, temp, _tcslen(temp)*sizeof(TCHAR), &dw, NULL);
    WriteFile(hFile, mpsz_ConChar, TextWidth * TextHeight * 2, &dw, NULL);
    WriteFile(hFile, mpn_ConAttr, TextWidth * TextHeight * 2, &dw, NULL);
    WriteFile(hFile, mpsz_ConCharSave, TextWidth * TextHeight * 2, &dw, NULL);
    WriteFile(hFile, mpn_ConAttrSave, TextWidth * TextHeight * 2, &dw, NULL);
    if (mp_RCon) {
        mp_RCon->DumpConsole(hFile);
    }
    CloseHandle(hFile);
	return TRUE;
}

// ��� ������� ����� � ��. ����. �������
//#define isCharBorder(inChar) (inChar>=0x2013 && inChar<=0x266B)
bool CVirtualConsole::isCharBorder(WCHAR inChar)
{
	if (!gSet.isFixFarBorders)
		return false;
	return gSet.isCharBorder(inChar);

	//CSettings::CharRanges *pcr = gSet.icFixFarBorderRanges;
	//for (int i = 10; --i && pcr->bUsed; pcr++) {
	//	CSettings::CharRanges cr = *pcr;
	//	if (inChar>=cr.cBegin && inChar<=cr.cEnd)
	//		return true;
	//}

	//return false;

    //if (inChar>=0x2013 && inChar<=0x266B)
    //    return true;
    //else
    //    return false;

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
	//2009-07-12 ��� ��������� - ������� �������� ����� �� ����������� �������������� �����
    //if (inChar==ucBoxSinglVert || inChar==0x2503 || inChar==0x2506 || inChar==0x2507
    //    || (inChar>=0x250A && inChar<=0x254B) || inChar==0x254E || inChar==0x254F
    //    || (inChar>=0x2551 && inChar<=0x25C5)) // �� ������ �������� Arial Unicode MS
	if (inChar != ucBoxDblHorz && (inChar >= ucBoxSinglVert && inChar <= ucBoxDblVertHorz))
        return true;
    else
        return false;
}

bool CVirtualConsole::isCharProgress(WCHAR inChar)
{
    bool isProgress = (inChar == ucBox25 || inChar == ucBox50 || inChar == ucBox75 || inChar == ucBox100);
    return isProgress;
}

bool CVirtualConsole::isCharScroll(WCHAR inChar)
{
	bool isScrollbar = (inChar == ucBox25 || inChar == ucBox50 || inChar == ucBox75 || inChar == ucBox100
		|| inChar == ucUpScroll || inChar == ucDnScroll);
	return isScrollbar;
}

void CVirtualConsole::BlitPictureTo(int inX, int inY, int inWidth, int inHeight)
{
	#ifdef _DEBUG
	BOOL lbDump = FALSE;
	if (lbDump) DumpImage(gSet.hBgDc, gSet.bgBmp.X, gSet.bgBmp.Y, L"F:\\bgtemp.png");
	#endif
    if (gSet.bgBmp.X>inX && gSet.bgBmp.Y>inY)
        BitBlt(hDC, inX, inY, inWidth, inHeight, gSet.hBgDc, inX, inY, SRCCOPY);
    if (gSet.bgBmp.X<(inX+inWidth) || gSet.bgBmp.Y<(inY+inHeight))
    {
        if (hBrush0 == NULL) {
            hBrush0 = CreateSolidBrush(gSet.Colors[0]);
            SelectBrush(hBrush0);
        }

        RECT rect = {max(inX,gSet.bgBmp.X), inY, inX+inWidth, inY+inHeight};
        #ifndef SKIP_ALL_FILLRECT
        if (!IsRectEmpty(&rect))
            FillRect(hDC, &rect, hBrush0);
        #endif

        if (gSet.bgBmp.X>inX) {
            rect.left = inX; rect.top = max(inY,gSet.bgBmp.Y); rect.right = gSet.bgBmp.X;
            #ifndef SKIP_ALL_FILLRECT
            if (!IsRectEmpty(&rect))
                FillRect(hDC, &rect, hBrush0);
            #endif
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
	TODO("��������� ���� �� �����");
	return false;
    //if ((select.dwFlags & CONSOLE_SELECTION_NOT_EMPTY) == 0)
    //    return false;
    //if (row < select.srSelection.Top || row > select.srSelection.Bottom)
    //    return false;
    //if (col < select.srSelection.Left || col > select.srSelection.Right)
    //    return false;
    //return true;
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

// ������������� ���������� �������� ������� (���+�����) � ����� ���� � ������
// (!) ���������� ������ ������ ����� ���� ��������� �� ���� ������ ����� ����
// atr ������������� � BYTE, �.�. ������� ����� ��� �� ����������
void CVirtualConsole::GetCharAttr(WORD atr, BYTE& foreColorNum, BYTE& backColorNum, HFONT* pFont)
{
	// ��� ���������� ������ ������ ����
    atr &= 0xFF;
    
    // ������� ����� ������� ������� ������ � �������, � �������� ������ �� �������
    foreColorNum = m_ForegroundColors[atr];
    backColorNum = m_BackgroundColors[atr];
    if (pFont) *pFont = mh_FontByIndex[atr];

    if (bExtendColors) {
        if (backColorNum == nExtendColor) {
            backColorNum = attrBackLast; // ��� ����� �������� �� ������� ���� �� �������� ������
            foreColorNum += 0x10;
        } else {
            attrBackLast = backColorNum; // �������� ������� ���� �������� ������
        }
    }
}

void CVirtualConsole::CharABC(TCHAR ch, ABC *abc)
{
	BOOL lbCharABCOk;

	if (!gSet.CharABC[ch].abcB) {
		if (gSet.mh_Font2 && isCharBorder(ch))
			SelectFont(gSet.mh_Font2);
		else
			SelectFont(gSet.mh_Font);

		//This function succeeds only with TrueType fonts
		lbCharABCOk = GetCharABCWidths(hDC, ch, ch, &gSet.CharABC[ch]);
		if (!lbCharABCOk) {
			// ������ ����� �� TTF/OTF
			gSet.CharABC[ch].abcB = CharWidth(ch);
			_ASSERTE(gSet.CharABC[ch].abcB);
			if (!gSet.CharABC[ch].abcB) gSet.CharABC[ch].abcB = 1;
			gSet.CharABC[ch].abcA = gSet.CharABC[ch].abcC = 0;
		}
	}

	*abc = gSet.CharABC[ch];
}

// ���������� ������ �������, ��������� FixBorders
WORD CVirtualConsole::CharWidth(TCHAR ch)
{
	// ��������� �����, ����� �� �������� �� ������
	WORD nWidth = gSet.CharWidth[ch];
	if (nWidth)
		return nWidth;

	if (!gSet.isProportional
		|| gSet.isForceMonospace 
		|| (gSet.isFixFarBorders && isCharBorder(ch))
		|| (gSet.isEnhanceGraphics && isCharProgress(ch))
		)
	{
		//2009-09-09 ��� �����������. ������ ������ ����� ����� ���� ������ ����������
		//return gSet.BorderFontWidth();
		gSet.CharWidth[ch] = nFontWidth;
		return nFontWidth;
	}

    //nWidth = nFontWidth;
    //bool isBorder = false; //, isVBorder = false;


	// �������� ��� �� ����� ������� ������ � ��� ������, ������� ����� ���� �����������
	if (gSet.mh_Font2 && isCharBorder(ch))
		SelectFont(gSet.mh_Font2);
	else
		SelectFont(gSet.mh_Font);
	//SelectFont(gSet.mh_Font);
    SIZE sz;
    //This function succeeds only with TrueType fonts
	//#ifdef _DEBUG
	//ABC abc;
	//BOOL lb1 = GetCharABCWidths(hDC, ch, ch, &abc);
	//#endif

    if (GetTextExtentPoint32(hDC, &ch, 1, &sz) && sz.cx)
        nWidth = sz.cx;
    else
        nWidth = nFontWidth;

	if (!nWidth)
		nWidth = 1; // �� ������ ������, ����� ������� �� 0 �� ��������

    gSet.CharWidth[ch] = nWidth;

    return nWidth;
}

char CVirtualConsole::Uni2Oem(wchar_t ch)
{
	if (mc_Uni2Oem[ch])
		return mc_Uni2Oem[ch];

	char c = '?';
	WideCharToMultiByte(CP_OEMCP, 0, &ch, 1, &c, 1, 0, 0);
	mc_Uni2Oem[ch] = c;
	return c;
}

bool CVirtualConsole::CheckChangedTextAttr()
{
    textChanged = 0!=memcmp(mpsz_ConChar, mpsz_ConCharSave, TextLen * sizeof(*mpsz_ConChar));
    attrChanged = 0!=memcmp(mpn_ConAttr, mpn_ConAttrSave, TextLen * sizeof(*mpn_ConAttr));

#ifdef MSGLOGGER
    COORD ch;
    if (textChanged) {
        for (UINT i=0; i<TextLen; i++) {
            if (mpsz_ConChar[i] != mpsz_ConCharSave[i]) {
                ch.Y = i % TextWidth;
                ch.X = i - TextWidth * ch.Y;
                break;
            }
        }
    }
    if (attrChanged) {
        for (UINT i=0; i<TextLen; i++) {
            if (mpn_ConAttr[i] != mpn_ConAttrSave[i]) {
                ch.Y = i % TextWidth;
                ch.X = i - TextWidth * ch.Y;
                break;
            }
        }
    }
#endif

    return textChanged || attrChanged;
}

bool CVirtualConsole::Update(bool isForce, HDC *ahDc)
{
    if (!this || !mp_RCon || !mp_RCon->ConWnd())
        return false;

	if (!gConEmu.isMainThread()) {
		if (isForce)
			mb_RequiredForceUpdate = true;

		//if (mb_RequiredForceUpdate || updateText || updateCursor)
		{
			if (gConEmu.isVisible(this)) {
				if (gSet.isAdvLogging>=3) mp_RCon->LogString("Invalidating from CVirtualConsole::Update.1");
				gConEmu.Invalidate(this);
			}

			return true;
		}

		return false;
	}


	/* -- 2009-06-20 ���������. ������� ������ �������� ������ � MainThread
    if (!mp_RCon->IsConsoleThread()) {
        if (!ahDc) {
            mp_RCon->SetForceRead();
            mp_RCon->WaitEndUpdate(1000);
            //gConEmu.InvalidateAll(); -- ����� �� All??? Update � ��� Invalidate �����
            return false;
        }
    }
	*/

    // ������ ���������� ���������� �� CRealConsole. ����� ������� � ������� �� ��������.
    //RetrieveConsoleInfo();

    #ifdef MSGLOGGER
    DcDebug dbg(&hDC, ahDc); // ��� ������� - ������ ����� �� ������� ����
    #endif


    MCHKHEAP
    bool lRes = false;
    
    MSectionLock SCON; SCON.Lock(&csCON);
    MSectionLock SDC; //&csDC, &ncsTDC);
    //if (mb_PaintRequested) -- �� ������ ����. ��� ������� �������� ������ � ���������� ����
    if (mb_PaintLocked) // ������ ���� ����������� Paint (BitBlt) - ��� ����� ���� �� ����� �������, ��� ��� ������� ���-�� ���������
        SDC.Lock(&csDC, TRUE, 200); // �� �� ��������, ����� �� ������� ���������


    mp_RCon->GetConsoleScreenBufferInfo(&csbi);

    // start timer before "Read Console Output*" calls, they do take time
    //gSet.Performance(tPerfRead, FALSE);

    //if (gbNoDblBuffer) isForce = TRUE; // Debug, dblbuffer

    //------------------------------------------------------------------------
    ///| Read console output and cursor info... |/////////////////////////////
    //------------------------------------------------------------------------
    if (!UpdatePrepare(isForce, ahDc, &SDC)) {
        gConEmu.DebugStep(_T("DC initialization failed!"));
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
        updateText = CheckChangedTextAttr();

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

        DEBUGSTRDRAW(L" +++ updateText detected in VCon\n");

        //------------------------------------------------------------------------
        ///| Drawing modified text |//////////////////////////////////////////////
        //------------------------------------------------------------------------
        UpdateText(isForce);


        //MCHKHEAP
        //------------------------------------------------------------------------
        ///| Now, store data for further comparison |/////////////////////////////
        //------------------------------------------------------------------------
        memcpy(mpsz_ConCharSave, mpsz_ConChar, TextLen * sizeof(*mpsz_ConChar));
        memcpy(mpn_ConAttrSave, mpn_ConAttr, TextLen * sizeof(*mpn_ConAttr));
    }

    //MCHKHEAP
    //------------------------------------------------------------------------
    ///| Checking cursor |////////////////////////////////////////////////////
    //------------------------------------------------------------------------
    UpdateCursor(lRes);
    
    MCHKHEAP
    
    //SDC.Leave();
    SCON.Unlock();

    // ����� ��������� ���������� ���������� ������� (�������/��������)
    // 
    if (lRes && gConEmu.isVisible(this)) {
        if (mpsz_LogScreen && mp_RCon && mp_RCon->GetServerPID()) {
        	// ������� ������ � ���
            mn_LogScreenIdx++;
            wchar_t szLogPath[MAX_PATH]; wsprintfW(szLogPath, mpsz_LogScreen, mp_RCon->GetServerPID(), mn_LogScreenIdx);
            Dump(szLogPath);
        }

        // ���� ��������� - ����������� ���������� viewport'�
		if (!mb_InPaintCall) {
			// ������ ���������� � �������� ����
			_ASSERTE(gConEmu.isMainThread());
			mb_PaintRequested = TRUE;
			gConEmu.Invalidate(this);
			if (gSet.isAdvLogging>=3) mp_RCon->LogString("Invalidating from CVirtualConsole::Update.2");
			//09.06.13 � ���� ���? ������� ��������� �� ������ �� ��������?
			//UpdateWindow(ghWndDC); // ��� �������� ��������� � ����, � ���� ��������� ���������
			#ifdef _DEBUG
			//_ASSERTE(!gConEmu.m_Child.mb_Invalidated);
			#endif
			mb_PaintRequested = FALSE;
		}
    }

    gSet.Performance(tPerfRender, TRUE);

    /* ***************************************** */
    /*       Finalization, release objects       */
    /* ***************************************** */
    SelectBrush(NULL);
    if (hBrush0) { // ��������� � BlitPictureTo
        DeleteObject(hBrush0); hBrush0 = NULL;
    }
	/*
    for (UINT br=0; br<m_PartBrushes.size(); br++) {
        DeleteObject(m_PartBrushes[br].hBrush);
    }
    m_PartBrushes.clear();
	*/
    SelectFont(NULL);
    MCHKHEAP
    return lRes;
}

bool CVirtualConsole::UpdatePrepare(bool isForce, HDC *ahDc, MSectionLock *pSDC)
{
    MSectionLock SCON; SCON.Lock(&csCON);
    
    attrBackLast = 0;
    isEditor = gConEmu.isEditor();
	isViewer = gConEmu.isViewer();
    isFilePanel = gConEmu.isFilePanel(true);


	nFontHeight = gSet.FontHeight();
	nFontWidth = gSet.FontWidth();
	nFontCharSet = gSet.FontCharSet();
	
	bExtendColors = gSet.isExtendColors;
	nExtendColor = gSet.nExtendColor;
	
    bExtendFonts = gSet.isExtendFonts;
    nFontNormalColor = gSet.nFontNormalColor;
    nFontBoldColor = gSet.nFontBoldColor;
    nFontItalicColor = gSet.nFontItalicColor;
    
    //m_ForegroundColors[0x100], m_BackgroundColors[0x100];
    int nColorIndex = 0;
    for (int nBack = 0; nBack <= 0xF; nBack++) {
    	for (int nFore = 0; nFore <= 0xF; nFore++, nColorIndex++) {
    		m_ForegroundColors[nColorIndex] = nFore;
    		m_BackgroundColors[nColorIndex] = nBack;
    		mh_FontByIndex[nColorIndex] = gSet.mh_Font;
			if (bExtendFonts) {
				if (nBack == nFontBoldColor) { // nFontBoldColor may be -1, ����� �� ���� �� ��������
					if (nFontNormalColor != 0xFF)
						m_BackgroundColors[nColorIndex] = nFontNormalColor;
					mh_FontByIndex[nColorIndex] = gSet.mh_FontB;
				} else if (nBack == nFontItalicColor) { // nFontItalicColor may be -1, ����� �� ���� �� ��������
					if (nFontNormalColor != 0xFF)
						m_BackgroundColors[nColorIndex] = nFontNormalColor;
					mh_FontByIndex[nColorIndex] = gSet.mh_FontI;
				}
			}
		}
    }


    //winSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1; winSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    //if (winSize.X < csbi.dwSize.X)
    //    winSize.X = csbi.dwSize.X;
    winSize = MakeCoord(mp_RCon->TextWidth(),mp_RCon->TextHeight());

    //csbi.dwCursorPosition.X -= csbi.srWindow.Left; -- �������������� ��������� ������������!
    csbi.dwCursorPosition.Y -= csbi.srWindow.Top;
    isCursorValid =
        csbi.dwCursorPosition.X >= 0 && csbi.dwCursorPosition.Y >= 0 &&
        csbi.dwCursorPosition.X < winSize.X && csbi.dwCursorPosition.Y < winSize.Y;

    if (mb_RequiredForceUpdate || mb_IsForceUpdate) {
        isForce = true;
        mb_RequiredForceUpdate = false;
    }

    // ������ �������������, ��� ����� �������
    if (isForce || !mpsz_ConChar || TextWidth != (uint)winSize.X || TextHeight != (uint)winSize.Y) {
        if (pSDC && !pSDC->isLocked()) // ���� ������ ��� �� ������������� (��������� - ���������� �������)
            pSDC->Lock(&csDC, TRUE, 200); // �� �� ��������, ����� �� ������� ���������
        if (!InitDC(ahDc!=NULL && !isForce/*abNoDc*/, false/*abNoWndResize*/))
            return false;
		#ifdef _DEBUG
		if (TextWidth == 80 && !mp_RCon->isNtvdm()) {
			TextWidth = TextWidth;
		}
		#endif
        gConEmu.OnConsoleResize();
    }

    // ��������� ������ �����������!
    if (mb_IsForceUpdate)
    {
        isForce = true;
        mb_IsForceUpdate = false;
    }
    // ���������� �����. ������� ����������� ���������, ��������� ���� ����� �� ahDc
    if (ahDc)
        isForce = true;


    drawImage = (gSet.isShowBgImage == 1 || (gSet.isShowBgImage == 2 && !(isEditor || isViewer)) )
		&& gSet.isBackgroundImageValid;
    TextLen = TextWidth * TextHeight;
    coord.X = csbi.srWindow.Left; coord.Y = csbi.srWindow.Top;

    // ����������� ������ �� ��������� ������� � mpn_ConAttr/mpsz_ConChar
    mp_RCon->GetData(mpsz_ConChar, mpn_ConAttr, TextWidth, TextHeight); //TextLen*2);

    HEAPVAL
 
    
    // ���������� ���������� ������� (��������� � CRealConsole)

    // get cursor info
    mp_RCon->GetConsoleCursorInfo(/*hConOut(),*/ &cinf);

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
    //UINT idx = 0;
    struct _TextParts *pStart=TextParts, *pEnd=TextParts;
    enum _PartType cType1, cType2;
    UINT i1=0, i2=0;
    
    pEnd->partType = pNull; // ����� ��������� ������
    
    TCHAR ch1, ch2;
    BYTE af1, ab1, af2, ab2;
    DWORD pixels;
    while (i1<TextWidth)
    {
        //GetCharAttr(ConCharLine[i1], ConAttrLine[i1], ch1, af1, ab1);
		ch1 = ConCharLine[i1];
		GetCharAttr(ConAttrLine[i1], af1, ab1, NULL);
        cType1 = GetCharType(ch1);
        if (cType1 == pRBracket) {
            if (!(row>=2 && isCharBorderVertical(mpsz_ConChar[TextWidth+i1]))
                && (((UINT)row)<=(TextHeight-4)))
                cType1 = pText;
        }
        pixels = CharWidth(ch1);

        i2 = i1+1;
        // � ������ Force Monospace ��������� ���� �� ������ �������
        if (!gSet.isForceMonospace && i2 < TextWidth && 
            (cType1 != pVBorder && cType1 != pRBracket))
        {
            //GetCharAttr(ConCharLine[i2], ConAttrLine[i2], ch2, af2, ab2);
			ch2 = ConCharLine[i2];
			GetCharAttr(ConAttrLine[i2], af2, ab2, NULL);
            // �������� ���� �������� � ������������ �������
            while (i2 < TextWidth && af2 == af1 && ab2 == ab1) {
                // ���� ������ ���������� �� �������

                cType2 = GetCharType(ch2);
                if ((ch2 = ConCharLine[i2]) != ch1) {
                    if (cType2 == pRBracket) {
                        if (!(row>=2 && isCharBorderVertical(mpsz_ConChar[TextWidth+i2]))
                            && (((UINT)row)<=(TextHeight-4)))
                            cType2 = pText;
                    }

                    // � �� ������ �� ������ ������
                    if (cType2 != cType1)
                        break; // �� ��������� �����
                }
                pixels += CharWidth(ch2); // �������� ������ ������� � ��������
                i2++; // ��������� ������
                //GetCharAttr(ConCharLine[i2], ConAttrLine[i2], ch2, af2, ab2);
				ch2 = ConCharLine[i2];
				GetCharAttr(ConAttrLine[i2], af2, ab2, NULL);
                if (cType2 == pRBracket) {
                    if (!(row>=2 && isCharBorderVertical(mpsz_ConChar[TextWidth+i2]))
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
            (gSet.isProportional && (cType1 == pVBorder || cType1 == pRBracket)))
        {
            pEnd->x1 = i1 * gSet.FontWidth();
        } else {
            pEnd->x1 = -1;
        }

        pEnd ++; // ������ �� ����� ���� ������ ���������� �������� � ������, ��� ��� � ������������ ��� ��
        i1 = i2;
    }
    // ���� �������� ����� ������, �����, ���� ������ �� ������ - ������� pDummy
    pEnd->partType = pNull;
}

WARNING("!!! �����������. ��������� GetCharAttr � ������");
WARNING("!!! �����������. ��������� ������ � ABC ������� �������, ����� ����� ���������� ������ � ��������");
void CVirtualConsole::UpdateText(bool isForce)
{
    //if (!updateText) {
    //    _ASSERTE(updateText);
    //    return;
    //}

#ifdef _DEBUG
    if (mp_RCon->IsConsoleThread()) {
        //_ASSERTE(!mp_RCon->IsConsoleThread());
    }
    // ������ ��� ������ ���� ���������, � ��� �� ������ ���� ����
    BOOL lbDataValid = TRUE; uint n = 0;
    while (n<TextLen) {
        if (mpsz_ConChar[n] == 0) {
            lbDataValid = FALSE; //break;
            mpsz_ConChar[n] = L'�';
            mpn_ConAttr[n] = 12;
        } else if (mpsz_ConChar[n] != L' ') {
            // 0 - ����� ���� ������ ��� �������. ����� ������ ����� �������, ���� �� ����, ���� �� ������
            if (mpn_ConAttr[n] == 0) {
                lbDataValid = FALSE; //break;
                mpn_ConAttr[n] = 12;
            }
        }
        n++;
    }
    //_ASSERTE(lbDataValid);
#endif

	

    SelectFont(gSet.mh_Font);

    // pointers
    wchar_t* ConCharLine;
    WORD* ConAttrLine;
    DWORD* ConCharXLine;
    // counters
    int pos, row;
    {
        int i;

        i = 0; //TextLen - TextWidth; // TextLen = TextWidth/*80*/ * TextHeight/*25*/;
        pos = 0; //Height - nFontHeight; // Height = TextHeight * nFontHeight;
        row = 0; //TextHeight - 1;

		ConCharLine = mpsz_ConChar + i;
        ConAttrLine = mpn_ConAttr + i;
        ConCharXLine = ConCharX + i;
    }
    int nMaxPos = Height - nFontHeight;

    if (/*gSet.isForceMonospace ||*/ !drawImage)
        SetBkMode(hDC, OPAQUE);
    else
        SetBkMode(hDC, TRANSPARENT);

	int nDX[500];

    // rows
    // ����� � isForceMonospace ������������� �������������� ���?
    // const bool skipNotChanged = !isForce /*&& !gSet.isForceMonospace*/;
    const bool skipNotChanged = !isForce;
	bool bEnhanceGraphics = gSet.isEnhanceGraphics;
	bool bProportional = gSet.isProportional;
	bool bForceMonospace = gSet.isForceMonospace;
	bool bFixFarBorders = gSet.isFixFarBorders;
	HFONT hFont = gSet.mh_Font;
	HFONT hFont2 = gSet.mh_Font2;
    for (; pos <= nMaxPos; 
        ConCharLine += TextWidth, ConAttrLine += TextWidth, ConCharXLine += TextWidth,
        pos += nFontHeight, row++)
    {
		//2009-09-25. ��������� (������?) ��������� ���������� �������� � ������� ������� (ASC<32)
		//            �� ����� �������� �� ��������� �������
		//{
		//	wchar_t* pszEnd = ConCharLine + TextWidth;
		//	for (wchar_t* pch = ConCharLine; pch < pszEnd; pch++) {
		//		if (((WORD)*pch) < 32) *pch = gszAnalogues[(WORD)*pch];
		//	}
		//}

        // the line
        const WORD* const ConAttrLine2 = mpn_ConAttrSave + (ConAttrLine - mpn_ConAttr);
        const TCHAR* const ConCharLine2 = mpsz_ConCharSave + (ConCharLine - mpsz_ConChar);

        // skip not changed symbols except the old cursor or selection
        int j = 0, end = TextWidth;
		// � ������ ���������������� ������� ��� isForce==true - ������������ ����� �������
		// ��� ���������������� ��� ����������� ���, ��� ��� ����������� ���������� �����
		// ����� ��������� ������ ����������� ��������, � ���������� �������� ���������
        if (skipNotChanged)
        {
            // *) Skip not changed tail symbols. �� ������ ���� ����� ����������
			if (!bProportional) {
				while(--end >= 0 && ConCharLine[end] == ConCharLine2[end] && ConAttrLine[end] == ConAttrLine2[end])
					;
				// [%] ClearType, TTF fonts (isProportional ����� ���� � ��������)
				if (end >= 0  // ���� ���� ���� �����-�� ���������
					&& end < (int)(TextWidth - 1) // �� ����� ������
					&& (ConCharLine[end+1] == ucSpace || ConCharLine[end+1] == ucNoBreakSpace || isCharProgress(ConCharLine[end+1])) // ����� ������������ ��� �������
					)
				{
					int n = TextWidth - 1;
					while ((end < n) 
					&& (ConCharLine[end+1] == ucSpace || ConCharLine[end+1] == ucNoBreakSpace || isCharProgress(ConCharLine[end+1])))
						end++;
				}
				if (end < j)
					continue;
				++end;
			}

            // *) Skip not changed head symbols.
            while(j < end && ConCharLine[j] == ConCharLine2[j] && ConAttrLine[j] == ConAttrLine2[j])
                ++j;
			// [%] ClearType, proportional fonts
			if (j > 0 // ���� � ������ ������
				&& (ConCharLine[j-1] == ucSpace || ConCharLine[j-1] == ucNoBreakSpace || isCharProgress(ConCharLine[j-1])) // ���� �������
				)
			{
				while ((j > 0)
			    && (ConCharLine[j-1] == ucSpace || ConCharLine[j-1] == ucNoBreakSpace || isCharProgress(ConCharLine[j-1])))
					j--;
			}
			if (j >= end) {
				// ��� ���������������� ������� ��������� ����������� ������ � ������ ������,
				// ������� ���� �� ������ ����� ������� - ������ ������ �� ��������
                continue;
			}

        } // if (skipNotChanged)
        
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
            bool isProgress = false, isSpace = false, isUnicodeOrProgress = false;
            bool lbS1 = false, lbS2 = false;
            int nS11 = 0, nS12 = 0, nS21 = 0, nS22 = 0;

			//GetCharAttr(c, attr, c, attrFore, attrBack);
			GetCharAttr(attr, attrFore, attrBack, &hFont);
            //if (GetCharAttr(c, attr, c, attrFore, attrBack))
            //    isUnicode = true;

            if (isUnicode || bEnhanceGraphics)
                isProgress = isCharProgress(c); // ucBox25 / ucBox50 / ucBox75 / ucBox100
			isUnicodeOrProgress = isUnicode || isProgress;
            if (!isUnicodeOrProgress)
                isSpace = isCharSpace(c);
                

			TODO("��� ���������������� (�������� � DistrubuteSpaces) �������� ��������������� �� ����������� ���������� ������");
			// ����� ��������� �������� �������� �� ���������������� �������

            MCHKHEAP
            // ������������� ���������� �������� � �����
            if (bProportional && (c==ucBoxDblHorz || c==ucBoxSinglHorz))
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


            SetTextColor(hDC, gSet.Colors[attrFore]);

            // ������������� ��������� ������������ ����� (Coord.X>0)
            if (bProportional && j)
            {
                MCHKHEAP;
                DWORD nPrevX = ConCharXLine[j-1];
				// ����� (�� ������������ �����) � ������ ���������;
				// �������� } ��� �������� ����� ������ �������� �� ������� �������...
				// ���� ������ ��� ����� �� ��� �� ������� ����� (��� ��� �� '}')
				// ������������ �������
				bool bBord = isCharBorderVertical(c);
                if (bBord || isCharScroll(c) || (isFilePanel && c==L'}')) {
                    //2009-04-21 ���� (j-1) * nFontWidth;
                    //ConCharXLine[j-1] = j * nFontWidth;
					bool bMayBeFrame = true;
					if (!bBord)
					{
						int R;
						wchar_t prevC;
						for (R = (row-1); bMayBeFrame && !bBord && R>=0; R--) {
							prevC = mpsz_ConChar[R*TextWidth+j];
							bBord = isCharBorderVertical(prevC);
							bMayBeFrame = (bBord || isCharScroll(prevC) || (isFilePanel && prevC==L'}'));
						}
						for (R = (row+1); bMayBeFrame && !bBord && R < (int)TextHeight; R++) {
							prevC = mpsz_ConChar[R*TextWidth+j];
							bBord = isCharBorderVertical(prevC);
							bMayBeFrame = (bBord || isCharScroll(prevC) || (isFilePanel && prevC==L'}'));
						}
					}
					if (bMayBeFrame)
						ConCharXLine[j-1] = j * nFontWidth;
                }
				//else if (isFilePanel && c==L'}') {
				//    if ((row>=2 && isCharBorderVertical(mpsz_ConChar[TextWidth+j]))
				//        && (((UINT)row)<=(TextHeight-4)))
				//        //2009-04-21 ���� (j-1) * nFontWidth;
				//        ConCharXLine[j-1] = j * nFontWidth;
				//    //row = TextHeight - 1;
				//}
				MCHKHEAP;

                if (nPrevX < ConCharXLine[j-1]) {
                    // ��������� ���������� ����������� �������. ��������� ���-��?

                    RECT rect;
                    rect.left = nPrevX;
                    rect.top = pos;
                    rect.right = ConCharXLine[j-1];
                    rect.bottom = rect.top + nFontHeight;

                    if (gbNoDblBuffer) GdiFlush();
                    if (! (drawImage && ISBGIMGCOLOR(attrBack))) {

						BYTE PrevAttrFore = attrFore, PrevAttrBack = attrBack;
						const WORD PrevAttr = ConAttrLine[j-1];
						WCHAR PrevC = ConCharLine[j-1];

						//GetCharAttr(PrevC, PrevAttr, PrevC, PrevAttrFore, PrevAttrBack);
						GetCharAttr(PrevAttr, PrevAttrFore, PrevAttrBack, NULL);
						//if (GetCharAttr(PrevC, PrevAttr, PrevC, PrevAttrFore, PrevAttrBack))
						//	isUnicode = true;

						// ���� ������� ������ - ������������ �����, � ���������� ������ - �����
						// ����� �������� ����� �� �������� �������
						if (isCharBorderVertical(c) && isCharBorder(PrevC)) {
							SetBkColor(hDC, gSet.Colors[attrBack]);
							wchar_t *pchBorder = (c == ucBoxDblDownLeft || c == ucBoxDblUpLeft 
								|| c == ucBoxSinglDownDblHorz || c == ucBoxSinglUpDblHorz || c == ucBoxDblVertLeft
								|| c == ucBoxDblVertHorz) ? ms_HorzDbl : ms_HorzSingl;
							int nCnt = ((rect.right - rect.left) / CharWidth(pchBorder[0]))+1;
							if (nCnt > MAX_SPACES) {
								_ASSERTE(nCnt<=MAX_SPACES);
								nCnt = MAX_SPACES;
							}
							//UINT nFlags = ETO_CLIPPED; // || ETO_OPAQUE;
							//ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect, Spaces, min(nSpaceCount, nCnt), 0);
							if (! (drawImage && ISBGIMGCOLOR(attrBack)))
								SetBkColor(hDC, gSet.Colors[attrBack]);
							else if (drawImage)
								BlitPictureTo(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
							UINT nFlags = ETO_CLIPPED | ((drawImage && ISBGIMGCOLOR(attrBack)) ? 0 : ETO_OPAQUE);
							//wmemset(Spaces, chBorder, nCnt);
							if (bFixFarBorders)
								SelectFont(hFont2);
							ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect, pchBorder, nCnt, 0);

						} else {
							HBRUSH hbr = PartBrush(L' ', PrevAttrBack, PrevAttrFore);
							FillRect(hDC, &rect, hbr);

						}

                    } else if (drawImage) {
                        BlitPictureTo(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
                    } MCHKHEAP
                    if (gbNoDblBuffer) GdiFlush();
                }
            }

            ConCharXLine[j] = (j ? ConCharXLine[j-1] : 0)+CharWidth(c);
            MCHKHEAP


            if (bForceMonospace)
            {
                MCHKHEAP
                SetBkColor(hDC, gSet.Colors[attrBack]);

                j2 = j + 1;

                if (bFixFarBorders) {
                    if (!isUnicode)
                        SelectFont(hFont);
                    else //if (isUnicode)
                        SelectFont(hFont2);
                }


                RECT rect;
                if (!bProportional)
                    rect = MakeRect(j * nFontWidth, pos, j2 * nFontWidth, pos + nFontHeight);
                else {
                    rect.left = j ? ConCharXLine[j-1] : 0;
                    rect.top = pos;
                    rect.right = (TextWidth>(UINT)j2) ? ConCharXLine[j2-1] : Width;
                    rect.bottom = rect.top + nFontHeight;
                }
                UINT nFlags = ETO_CLIPPED | ((drawImage && ISBGIMGCOLOR(attrBack)) ? 0 : ETO_OPAQUE);
                int nShift = 0;

                MCHKHEAP
                if (!isSpace && !isUnicodeOrProgress) {
                    ABC abc;
                    //��� �� TrueType ���������� wrapper (CharWidth)
                    CharABC(c, &abc);

					if (abc.abcA<0) {
                        // ����� ������ �������� ������� �� ����������?
                        nShift = -abc.abcA;

					} else if (abc.abcB < (UINT)nFontWidth)  {
						int nEdge = (nFontWidth - abc.abcB - 1) >> 1;
						if (abc.abcA < nEdge) {
                            // ������ I, i, � ��. ����� ������ - ������ ����������
	                        nShift = nEdge - abc.abcA;
						}
                    }
                }

                MCHKHEAP
                if (! (drawImage && ISBGIMGCOLOR(attrBack))) {
                    SetBkColor(hDC, gSet.Colors[attrBack]);
					// � ������ ForceMonospace ������� �������� �������� �� ������ (���� ��� ��� ����������)
					// ����� �� ���������� ������ �� ���������� ��������� - ����� ������ ���������� �����
					if (nShift>0 && !isSpace && !isProgress) {
						HBRUSH hbr = PartBrush(L' ', attrBack, attrFore);
						FillRect(hDC, &rect, hbr);
					}
                } else if (drawImage) {
                    BlitPictureTo(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
                }

                if (nShift>0) {
                    rect.left += nShift;

					// ���� ������� ��� - ��� �������� �� ������ ������� (nShift > 0)
					//rect.right += nShift;
                }

                if (gbNoDblBuffer) GdiFlush();
				if (isSpace || (isProgress && bEnhanceGraphics)) {
					HBRUSH hbr = PartBrush(c, attrBack, attrFore);
					FillRect(hDC, &rect, hbr);
				} else {
					// ��� ����� Force monospace
					if (nFontCharSet == OEM_CHARSET && !isUnicode) {
						char cOem = Uni2Oem(c);
						ExtTextOutA(hDC, rect.left, rect.top, nFlags, &rect, &cOem, 1, 0);
					} else {
						ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect, &c, 1, 0);
					}
				}
                if (gbNoDblBuffer) GdiFlush();
                MCHKHEAP

            } // end  - if (gSet.isForceMonospace)
			else
			{
				wchar_t* pszDraw = NULL;
				int      nDrawLen = -1;
				bool     bDrawReplaced = false;
				RECT rect;

				//else if (/*gSet.isProportional &&*/ (c==ucSpace || c==ucNoBreakSpace || c==0))
				if (isSpace)
				{
					j2 = j + 1; MCHKHEAP;
					while(j2 < end && ConAttrLine[j2] == attr && isCharSpace(ConCharLine[j2]))
						j2++;
					DistributeSpaces(ConCharLine, ConAttrLine, ConCharXLine, j, j2, end);

				}
				else if (!isUnicodeOrProgress)
				{
					j2 = j + 1; MCHKHEAP

					#ifndef DRAWEACHCHAR
					// ���� ����� �� ������ - � ���������������� ������� ����� ����� �������� ���� �� ������
					TCHAR ch;
					WARNING("*** �������� � ��������� �������: (!gSet.isProportional || !isFilePanel || (ch != L'}' && ch!=L' '))");
					while(j2 < end && ConAttrLine[j2] == attr && 
						!isCharBorder(ch = ConCharLine[j2]) 
						&& (!bProportional || !isFilePanel || (ch != L'}' && ch!=L' '))) // ������������� ���� � ��������
					{
						ConCharXLine[j2] = (j2 ? ConCharXLine[j2-1] : 0)+CharWidth(ch);
						j2++;
					}
					#endif

					if (bFixFarBorders)
						SelectFont(hFont);
					MCHKHEAP
				}
				else //Border and specials
				{
					j2 = j + 1; MCHKHEAP

					if (bEnhanceGraphics && isProgress) {
						TCHAR ch;
						ch = c; // ����������� ��������� ��������� � ���������
						while(j2 < end && ConAttrLine[j2] == attr && ch == ConCharLine[j2+1])
						{
							ConCharXLine[j2] = (j2 ? ConCharXLine[j2-1] : 0)+CharWidth(ch);
							j2++;
						}
					} else
					if (!bFixFarBorders)
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
						//TCHAR ch;
						//WARNING("��� ������������ �� ucBoxDblVert � ������ �������. �� ������ ������ � ...");

						// ��� ��������� ����� (ucBoxDblHorz) �� ���� ������
						if (c == ucBoxDblHorz || c == ucBoxSinglHorz)
						{
							while(j2 < end && ConAttrLine[j2] == attr && c == ConCharLine[j2])
								j2++;
						}
						DistributeSpaces(ConCharLine, ConAttrLine, ConCharXLine, j, j2, end);
						int nBorderWidth = CharWidth(c);
						rect.left = j ? ConCharXLine[j-1] : 0;
						rect.right = (TextWidth>(UINT)j2) ? ConCharXLine[j2-1] : Width;
						int nCnt = (rect.right - rect.left + (nBorderWidth>>1)) / nBorderWidth;
						if (nCnt > (j2 - j)) {
							if (c==ucBoxDblHorz || c==ucBoxSinglHorz) {
								_ASSERTE(nCnt <= MAX_SPACES);
								if (nCnt > MAX_SPACES) nCnt = MAX_SPACES;
								bDrawReplaced = true;
								nDrawLen = nCnt;
								pszDraw = (c==ucBoxDblHorz) ? ms_HorzDbl : ms_HorzSingl;
							} else {
								_ASSERTE(c==ucBoxDblHorz || c==ucBoxSinglHorz);
							}
						}
						//while(j2 < end && ConAttrLine[j2] == attr && 
						//    isCharBorder(ch = ConCharLine[j2]) && ch == ConCharLine[j2+1])
						//{
						//    ConCharXLine[j2] = (j2 ? ConCharXLine[j2-1] : 0)+CharWidth(ch);
						//    j2++;
						//}
					}

					if (bFixFarBorders)
						SelectFont(hFont2);
					MCHKHEAP
				}


				if (!pszDraw) {
					pszDraw = ConCharLine + j;
					nDrawLen = j2 - j;
				}


				//if (!gSet.isProportional) {
				//	TODO("���-�� ���-��... ���� ��������� ��� ����� ���������?");
				//  rect = MakeRect(j * nFontWidth, pos, j2 * nFontWidth, pos + nFontHeight);
				//} else {
                rect.left = j ? ConCharXLine[j-1] : 0;
                rect.top = pos;
                rect.right = (TextWidth>(UINT)j2) ? ConCharXLine[j2-1] : Width;
                rect.bottom = rect.top + nFontHeight;
                //}

                MCHKHEAP
				BOOL lbImgDrawn = FALSE;
                if (! (drawImage && ISBGIMGCOLOR(attrBack)))
                    SetBkColor(hDC, gSet.Colors[attrBack]);
				else if (drawImage) {
                    BlitPictureTo(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
					lbImgDrawn = TRUE;
				}

				WARNING("ETO_CLIPPED ����� ������ �� �������� ������, ���� ��� ����� �� �������?");
                UINT nFlags = ETO_CLIPPED | ((drawImage && ISBGIMGCOLOR(attrBack)) ? 0 : ETO_OPAQUE);

                MCHKHEAP
                if (gbNoDblBuffer) GdiFlush();
                if (isProgress && bEnhanceGraphics) {
					HBRUSH hbr = PartBrush(c, attrBack, attrFore);
					FillRect(hDC, &rect, hbr);
                } else
                if (/*gSet.isProportional &&*/ isSpace/*c == ' '*/) {
                    //int nCnt = ((rect.right - rect.left) / CharWidth(L' '))+1;
                    //Ext Text Out(hDC, rect.left, rect.top, nFlags, &rect, Spaces, nCnt, 0);
					TODO("���������, ��� ����� ���� �������� ������ �� ������ ��� ������� ���������");
					if (!lbImgDrawn) {
						HBRUSH hbr = PartBrush(L' ', attrBack, attrFore);
						FillRect(hDC, &rect, hbr);
					}
				} else {
					if (nFontCharSet == OEM_CHARSET && !isUnicode)
					{
						WideCharToMultiByte(CP_OEMCP, 0, pszDraw, nDrawLen, tmpOem, TextWidth+4, 0, 0);
						if (!bProportional)
							for (int idx = 0, n = (j2-j); n; idx++, n--)
								nDX[idx] = CharWidth(tmpOem[idx]);
						ExtTextOutA(hDC, rect.left, rect.top, nFlags,
							&rect, tmpOem, j2 - j, bProportional ? 0 : nDX);
					}
					else
					{
						if (nDrawLen == 1) // support visualizer change
						ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect,
							&c/*ConCharLine + j*/, 1, 0);
						else {
							if (!bProportional)
								for (int idx = 0, n = nDrawLen; n; idx++, n--)
									nDX[idx] = CharWidth(pszDraw[idx]);
							ExtTextOut(hDC, rect.left, rect.top, nFlags, &rect,
								pszDraw, nDrawLen, bProportional ? 0 : nDX);
						}
					}
				}
                if (gbNoDblBuffer) GdiFlush();
                MCHKHEAP
            }

			DUMPDC(L"F:\\vcon.png");
        

			//-- �� ��������� ������� � ����������������� �������� � ClearType
			//-- ������ ���������� ������ �������
			//// skip next not changed symbols again
			//if (skipNotChanged)
			//{
			//    MCHKHEAP
			//	wchar_t ch;
			//    // skip the same except spaces
			//    while(j2 < end && (ch = ConCharLine[j2]) == ConCharLine2[j2] && ConAttrLine[j2] == ConAttrLine2[j2]
			//		&& (ch != ucSpace && ch != ucNoBreakSpace && ch != 0))
			//    {
			//        ++j2;
			//    }
			//}
        }
    }

	return;
}

void CVirtualConsole::ClearPartBrushes()
{
	_ASSERTE(gConEmu.isMainThread());
	for (UINT br=0; br<MAX_COUNT_PART_BRUSHES; br++) {
		DeleteObject(m_PartBrushes[br].hBrush);
	}
	memset(m_PartBrushes, 0, sizeof(m_PartBrushes));
}

HBRUSH CVirtualConsole::PartBrush(wchar_t ch, SHORT nBackIdx, SHORT nForeIdx)
{
	_ASSERTE(gConEmu.isMainThread());
    //std::vector<PARTBRUSHES>::iterator iter = m_PartBrushes.begin();
    //while (iter != m_PartBrushes.end()) {
	PARTBRUSHES *pbr = m_PartBrushes;
	for (UINT br=0; pbr->ch && br<MAX_COUNT_PART_BRUSHES; br++, pbr++) {
		if (pbr->ch == ch && pbr->nBackIdx == nBackIdx && pbr->nForeIdx == nForeIdx) {
			_ASSERTE(pbr->hBrush);
            return pbr->hBrush;
		}
    }

    MYRGB clrBack, clrFore, clrMy;
    clrBack.color = gSet.Colors[nBackIdx];
    clrFore.color = gSet.Colors[nForeIdx];

    clrMy.color = clrFore.color; // 100 %

    //#define   PART_75(f,b) ((((int)f) + ((int)b)*3) / 4)
    //#define   PART_50(f,b) ((((int)f) + ((int)b)) / 2)
    //#define   PART_25(f,b) (((3*(int)f) + ((int)b)) / 4)
    //#define   PART_75(f,b) (b + 0.80*(f-b))
    //#define   PART_50(f,b) (b + 0.75*(f-b))
    //#define   PART_25(f,b) (b + 0.50*(f-b))
    #define PART_75(f,b) (b + ((gSet.isPartBrush75*(f-b))>>8))
    #define PART_50(f,b) (b + ((gSet.isPartBrush50*(f-b))>>8))
    #define PART_25(f,b) (b + ((gSet.isPartBrush25*(f-b))>>8))

    if (ch == ucBox75 /* 75% */) {
        clrMy.rgbRed = PART_75(clrFore.rgbRed,clrBack.rgbRed);
        clrMy.rgbGreen = PART_75(clrFore.rgbGreen,clrBack.rgbGreen);
        clrMy.rgbBlue = PART_75(clrFore.rgbBlue,clrBack.rgbBlue);
        clrMy.rgbReserved = 0;
    } else if (ch == ucBox50 /* 50% */) {
        clrMy.rgbRed = PART_50(clrFore.rgbRed,clrBack.rgbRed);
        clrMy.rgbGreen = PART_50(clrFore.rgbGreen,clrBack.rgbGreen);
        clrMy.rgbBlue = PART_50(clrFore.rgbBlue,clrBack.rgbBlue);
        clrMy.rgbReserved = 0;
    } else if (ch == ucBox25 /* 25% */) {
        clrMy.rgbRed = PART_25(clrFore.rgbRed,clrBack.rgbRed);
        clrMy.rgbGreen = PART_25(clrFore.rgbGreen,clrBack.rgbGreen);
        clrMy.rgbBlue = PART_25(clrFore.rgbBlue,clrBack.rgbBlue);
        clrMy.rgbReserved = 0;
    } else if (ch == ucSpace || ch == ucNoBreakSpace /* Non breaking space */ || !ch) {
        clrMy.color = clrBack.color;
    }

    PARTBRUSHES pb;
    pb.ch = ch; pb.nBackIdx = nBackIdx; pb.nForeIdx = nForeIdx;
    pb.hBrush = CreateSolidBrush(clrMy.color);

    //m_PartBrushes.push_back(pb);
	*pbr = pb;
    return pb.hBrush;
}

void CVirtualConsole::UpdateCursorDraw(HDC hPaintDC, RECT rcClient, COORD pos, UINT dwSize)
{
    Cursor.x = csbi.dwCursorPosition.X;
    Cursor.y = csbi.dwCursorPosition.Y;

    int CurChar = pos.Y * TextWidth + pos.X;
    if (CurChar < 0 || CurChar>=(int)(TextWidth * TextHeight)) {
        return; // ����� ���� ��� ���� - ��� ������ ������� ��� ����� �������� � ���������� ������� ������� ������� � ������
    }
    if (!ConCharX) {
        return; // ������
    }
    COORD pix;
    
    pix.X = pos.X * nFontWidth;
    pix.Y = pos.Y * nFontHeight;
    if (pos.X && ConCharX[CurChar-1])
        pix.X = ConCharX[CurChar-1];

        
    RECT rect;
    
    if (!gSet.isCursorV)
    {
        if (gSet.isProportional) {
            rect.left = pix.X; /*Cursor.x * nFontWidth;*/
            rect.right = pix.X + nFontWidth; /*(Cursor.x+1) * nFontWidth;*/ //TODO: � ���� ������� ���������� ������� ��������!
        } else {
            rect.left = pos.X * nFontWidth;
            rect.right = (pos.X+1) * nFontWidth;
        }
        //rect.top = (Cursor.y+1) * nFontHeight - MulDiv(nFontHeight, cinf.dwSize, 100);
        rect.bottom = (pos.Y+1) * nFontHeight;
        rect.top = (pos.Y * nFontHeight) /*+ 1*/;
        //if (cinf.dwSize<50)
        int nHeight = 0;
        if (dwSize) {
            nHeight = MulDiv(nFontHeight, dwSize, 100);
            if (nHeight < HCURSORHEIGHT) nHeight = HCURSORHEIGHT;
        }
        //if (nHeight < HCURSORHEIGHT) nHeight = HCURSORHEIGHT;
        rect.top = max(rect.top, (rect.bottom-nHeight));
    }
    else
    {
        if (gSet.isProportional) {
            rect.left = pix.X; /*Cursor.x * nFontWidth;*/
            //rect.right = rect.left/*Cursor.x * nFontWidth*/ //TODO: � ���� ������� ���������� ������� ��������!
            //  + klMax(1, MulDiv(nFontWidth, cinf.dwSize, 100) 
            //  + (cinf.dwSize > 10 ? 1 : 0));
        } else {
            rect.left = pos.X * nFontWidth;
            //rect.right = Cursor.x * nFontWidth
            //  + klMax(1, MulDiv(nFontWidth, cinf.dwSize, 100) 
            //  + (cinf.dwSize > 10 ? 1 : 0));
        }
        rect.top = pos.Y * nFontHeight;
        int nR = (gSet.isProportional && ConCharX[CurChar]) // ������ �������
            ? ConCharX[CurChar] : ((pos.X+1) * nFontWidth);
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
        //rect.right = rect.left/*Cursor.x * nFontWidth*/ //TODO: � ���� ������� ���������� ������� ��������!
        //      + klMax(1, MulDiv(nFontWidth, cinf.dwSize, 100) 
        //      + (cinf.dwSize > 10 ? 1 : 0));
        rect.bottom = (pos.Y+1) * nFontHeight;
    }
    
    
	//if (!gSet.isCursorBlink) {
	//	gConEmu.m_Child.SetCaret ( 1, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top );
	//	return;
	//} else {
	//	gConEmu.m_Child.SetCaret ( -1 ); // ���� ��� ������ ��������� ������ - �� ����������
	//}

	rect.left += rcClient.left;
	rect.right += rcClient.left;
	rect.top += rcClient.top;
	rect.bottom += rcClient.top;

	// ���� ������ �������� ����� 40% ������� - ������������� ������� 
	// XOR �����, ����� (��� ����� ��� ����������) ������ ��������� 
	// ���� ������ � ��� �� �����
	bool bCursorColor = gSet.isCursorColor || (dwSize >= 40);

	// ������ � rect ����� ���������� ������ (XOR'�� ���������?)
	if (bCursorColor)
	{
		HBRUSH hBr = CreateSolidBrush(0xC0C0C0);
		HBRUSH hOld = (HBRUSH)SelectObject ( hPaintDC, hBr );

		BitBlt(hPaintDC, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, hDC, 0,0,
			PATINVERT);

		SelectObject ( hPaintDC, hOld );
		DeleteObject ( hBr );

		return;
    }


	//lbDark = Cursor.foreColorNum < 5; // ���� ������
	//BOOL lbIsProgr = isCharProgress(Cursor.ch[0]);
    
	BOOL lbDark = FALSE;
	DWORD clr = (Cursor.ch != ucBox100 && Cursor.ch != ucBox75) ? Cursor.bgColor : Cursor.foreColor;
	BYTE R = (clr & 0xFF);
	BYTE G = (clr & 0xFF00) >> 8;
	BYTE B = (clr & 0xFF0000) >> 16;
	lbDark = (R <= 0xC0) && (G <= 0xC0) && (B <= 0xC0);
	clr = lbDark ? gSet.Colors[15] : gSet.Colors[0];

	HBRUSH hBr = CreateSolidBrush(clr);
	FillRect(hPaintDC, &rect, hBr);
	DeleteObject ( hBr );
}

void CVirtualConsole::UpdateCursor(bool& lRes)
{
    //------------------------------------------------------------------------
    ///| Drawing cursor |/////////////////////////////////////////////////////
    //------------------------------------------------------------------------
    Cursor.isVisiblePrevFromInfo = cinf.bVisible;

    BOOL lbUpdateTick = FALSE;
    bool bConActive = gConEmu.isActive(this) // � ��� ������ Active, �.�. ������ ������ ������ � �������� �������
        #ifndef _DEBUG
        && gConEmu.isMeForeground()
        #endif
        ;

    // ���� ������ (� �������) �����, � ��������� � ������� ������� (��� ���������)
    if (cinf.bVisible && isCursorValid)
    {
		if (!gSet.isCursorBlink) {
            Cursor.isVisible = true; // ����� ������ (���� � ���������� �������), �� ������
		} else {
            // ����� ������� ������� - ��� ����� ��������, ���� ���� �������
            if ((Cursor.x != csbi.dwCursorPosition.X) || (Cursor.y != csbi.dwCursorPosition.Y)) 
            {
                Cursor.isVisible = bConActive;
                if (Cursor.isVisible) lRes = true; //force, pos changed
                lbUpdateTick = TRUE;
            } else
            // ������� ����� �������
            if ((GetTickCount() - Cursor.nLastBlink) > Cursor.nBlinkTime)
            {
                Cursor.isVisible = bConActive && !Cursor.isVisible;
                lbUpdateTick = TRUE;
            }
		}
    } else {
        // ����� - ��� ����� �������� (������ ����� � �������, ��� ���� �� ������� ������)
        Cursor.isVisible = false;
    }

    // ����� ��������� ����� � ConEmu
    if (Cursor.isVisible != Cursor.isVisiblePrev) {
        lRes = true;
        lbUpdateTick = TRUE;
    }

    // �������� ����� ���������
    //Cursor.x = csbi.dwCursorPosition.X;
    //Cursor.y = csbi.dwCursorPosition.Y;
    Cursor.isVisiblePrev = Cursor.isVisible;

    if (lbUpdateTick)
        Cursor.nLastBlink = GetTickCount();
}


LPVOID CVirtualConsole::Alloc(size_t nCount, size_t nSize)
{
    #ifdef _DEBUG
    //HeapValidate(mh_Heap, 0, NULL);
    #endif
    size_t nWhole = nCount * nSize;
    LPVOID ptr = HeapAlloc ( mh_Heap, HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY, nWhole );
    #ifdef _DEBUG
    //HeapValidate(mh_Heap, 0, NULL);
    #endif
    return ptr;
}

void CVirtualConsole::Free(LPVOID ptr)
{
    if (ptr && mh_Heap) {
        #ifdef _DEBUG
        //HeapValidate(mh_Heap, 0, NULL);
        #endif
        HeapFree ( mh_Heap, 0, ptr );
        #ifdef _DEBUG
        //HeapValidate(mh_Heap, 0, NULL);
        #endif
    }
}


// hdc - ��� DC ������������� ���� (ghWnd)
// rcClient - ��� �����, ���� ����� "��������" ������. ����� ���� ������������!
void CVirtualConsole::Paint(HDC hDc, RECT rcClient)
{
    if (!ghWndDC)
        return;
	_ASSERTE(hDc);
	_ASSERTE(rcClient.left!=rcClient.right && rcClient.top!=rcClient.bottom);

//#ifdef _DEBUG
//    if (this) {
//        if (!mp_RCon || !mp_RCon->isPackets()) {
//        	DEBUGSTRDRAW(L"*** Painting ***\n");
//        } else {
//            DEBUGSTRDRAW(L"*** Painting (!!! Non processed packets are queued !!!) ***\n");
//        }
//    }
//#endif
    
	BOOL lbSimpleBlack = FALSE;
	if (!this) 
		lbSimpleBlack = TRUE;
	else if (!mp_RCon)
		lbSimpleBlack = TRUE;
	else if (!mp_RCon->ConWnd())
		lbSimpleBlack = TRUE;

	//else if (!mpsz_ConChar || !mpn_ConAttr)
	//	lbSimpleBlack = TRUE;
    if (lbSimpleBlack) {
        // ������ ������ 0
        #ifdef _DEBUG
            int nBackColorIdx = 2;
        #else
            int nBackColorIdx = 0;
        #endif
        HBRUSH hBr = CreateSolidBrush(gSet.Colors[nBackColorIdx]);
        //RECT rcClient; GetClientRect(ghWndDC, &rcClient);
        //PAINTSTRUCT ps;
        //HDC hDc = BeginPaint(ghWndDC, &ps);
        #ifndef SKIP_ALL_FILLRECT
        FillRect(hDc, &rcClient, hBr);
        #endif
		HFONT hOldF = (HFONT)SelectObject(hDc, gSet.mh_Font);
		LPCWSTR pszStarting = L"Initializing ConEmu.";
		if (this) {
			if (mp_RCon)
				pszStarting = mp_RCon->GetConStatus();
		}
		UINT nFlags = ETO_CLIPPED;
		SetTextColor(hDc, gSet.Colors[7]);
		SetBkColor(hDc, gSet.Colors[0]);
		ExtTextOut(hDc, rcClient.left, rcClient.top, nFlags, &rcClient, pszStarting, wcslen(pszStarting), 0);
		SelectObject(hDc, hOldF);
        DeleteObject(hBr);
        //EndPaint(ghWndDC, &ps);
        return;
    }
    
    gSet.Performance(tPerfFPS, TRUE); // ��������� �� ������

	mb_InPaintCall = TRUE;
	Update(mb_RequiredForceUpdate);
	mb_InPaintCall = FALSE;

    BOOL lbExcept = FALSE;
    RECT client = rcClient;
    //PAINTSTRUCT ps;
    //HDC hPaintDc = NULL;


    //GetClientRect(ghWndDC, &client);

    if (!gConEmu.isNtvdm()) {
        // ����� ������ � �������� ����� ���� �������� � �������� ���� DC
        if ((client.right-client.left) < (LONG)Width || (client.bottom-client.top) < (LONG)Height)
		{
			WARNING("�������������. �������� Paint, ������� �������� OnSize. � ����� - 100% �������� ����.");
            gConEmu.OnSize(-1); // ������ ������ �������� ����
		}
    }
    
    MSectionLock S; //&csDC, &ncsTDC);

	//RECT rcUpdateRect = {0};
	//BOOL lbInval = GetUpdateRect(ghWndDC, &rcUpdateRect, FALSE);

	//if (lbInval)
	//  hPaintDc = BeginPaint(ghWndDC, &ps);
	//else
	//	hPaintDc = GetDC(ghWndDC);

    // ���� ���� ������ �������� DC - ������ ���� (������/�����) ������� ������
    HBRUSH hBr = NULL;
    if (((ULONG)(client.right-client.left)) > Width) {
        if (!hBr) hBr = CreateSolidBrush(gSet.Colors[mn_BackColorIdx]);
        RECT rcFill = MakeRect(client.left+Width, client.top, client.right, client.bottom);
        #ifndef SKIP_ALL_FILLRECT
        FillRect(hDc, &rcFill, hBr);
        #endif
        client.right = client.left+Width;
    }
    if (((ULONG)(client.bottom-client.top)) > Height) {
        if (!hBr) hBr = CreateSolidBrush(gSet.Colors[mn_BackColorIdx]);
        RECT rcFill = MakeRect(client.left, client.top+Height, client.right, client.bottom);
        #ifndef SKIP_ALL_FILLRECT
        FillRect(hDc, &rcFill, hBr);
        #endif
        client.bottom = client.top+Height;
    }
    if (hBr) { DeleteObject(hBr); hBr = NULL; }


    BOOL lbPaintLocked = FALSE;
    if (!mb_PaintRequested) { // ���� Paint ������� �� �� Update (� �������� ������, ��� ��� ����� ������� ���-�� ���������).
        if (S.Lock(&csDC, 200)) // �� �� ��������, ����� �� ������� ���������
            mb_PaintLocked = lbPaintLocked = TRUE;
    }

    // ����������, ����������� �������� bitmap
    if (!gbNoDblBuffer) {
        // ������� �����
		if (gSet.isAdvLogging>=3) mp_RCon->LogString("Blitting to Display");

        BitBlt(hDc, client.left, client.top, client.right-client.left, client.bottom-client.top, hDC, 0, 0, SRCCOPY);
    } else {
        GdiSetBatchLimit(1); // ��������� ����������� ������ ��� ������� ����

        GdiFlush();
        // ������ ����� �� �������, ��� �����������
		// ��� ��� ������� ���������, ������� ����������� ���� � ������
		_ASSERTE(gSet.isTabs == 0);
        Update(true, &hDc);
    }

    S.Unlock();
    
    if (lbPaintLocked)
        mb_PaintLocked = FALSE;

        // ����� ������� ������ ������ ������ � ����
        //UpdateCursor(hPaintDc);

        if (gbNoDblBuffer) GdiSetBatchLimit(0); // ������� ����������� �����

        

        if (Cursor.isVisible && cinf.bVisible && isCursorValid)
        {
			if (mpsz_ConChar && mpsz_ConChar)
			{
				HFONT hOldFont = (HFONT)SelectObject(hDc, gSet.mh_Font);

				MSectionLock SCON; SCON.Lock(&csCON);

				if (mpsz_ConChar && mpn_ConAttr)
				{
					int CurChar = csbi.dwCursorPosition.Y * TextWidth + csbi.dwCursorPosition.X;
					//Cursor.ch[1] = 0;
					//CVirtualConsole* p = this;
					//GetCharAttr(mpsz_ConChar[CurChar], mpn_ConAttr[CurChar], Cursor.ch[0], Cursor.foreColorNum, Cursor.bgColorNum);
					Cursor.ch = mpsz_ConChar[CurChar];
					GetCharAttr(mpn_ConAttr[CurChar], Cursor.foreColorNum, Cursor.bgColorNum, NULL);
					Cursor.foreColor = gSet.Colors[Cursor.foreColorNum];
					Cursor.bgColor = gSet.Colors[Cursor.bgColorNum];
				}

				UpdateCursorDraw(hDc, rcClient, csbi.dwCursorPosition, cinf.dwSize);

				Cursor.isVisiblePrev = Cursor.isVisible;

				SelectObject(hDc, hOldFont);

				SCON.Unlock();
			}
        }

#ifdef _DEBUG
	if (mp_RCon && (GetKeyState(VK_SCROLL) & 1)) {
		HWND hConWnd = mp_RCon->hConWnd;
		RECT rcCon; GetWindowRect(hConWnd, &rcCon);
		MapWindowPoints(NULL, ghWndDC, (LPPOINT)&rcCon, 2);
		SelectObject(hDc, GetStockObject(WHITE_PEN));
		SelectObject(hDc, GetStockObject(HOLLOW_BRUSH));
		Rectangle(hDc, rcCon.left, rcCon.top, rcCon.right, rcCon.bottom);
	}
#endif

    
    if (lbExcept)
        Box(_T("Exception triggered in CVirtualConsole::Paint"));

  //  if (hPaintDc && ghWndDC) {
		//if (lbInval)
		//	EndPaint(ghWndDC, &ps);
		//else
		//	ReleaseDC(ghWndDC, hPaintDc);
  //  }
    
    //gSet.Performance(tPerfFPS, FALSE); // ��������
}

void CVirtualConsole::UpdateInfo()
{
    if (!ghOpWnd || !this)
        return;

    if (!gConEmu.isMainThread()) {
        return;
    }

    TCHAR szSize[128];
	if (!mp_RCon) {
		SetDlgItemText(gSet.hInfo, tConSizeChr, L"(None)");
		SetDlgItemText(gSet.hInfo, tConSizePix, L"(None)");
		
		SetDlgItemText(gSet.hInfo, tPanelLeft, L"(None)");
		SetDlgItemText(gSet.hInfo, tPanelRight, L"(None)");
	} else {
		wsprintf(szSize, _T("%ix%i"), mp_RCon->TextWidth(), mp_RCon->TextHeight());
		SetDlgItemText(gSet.hInfo, tConSizeChr, szSize);
		wsprintf(szSize, _T("%ix%i"), Width, Height);
		SetDlgItemText(gSet.hInfo, tConSizePix, szSize);
		
		RECT rcPanel;
		RCon()->GetPanelRect(FALSE, &rcPanel);
		if (rcPanel.right>rcPanel.left)
			wsprintf(szSize, L"(%i, %i)-(%i, %i), %ix%i", rcPanel.left+1, rcPanel.top+1, rcPanel.right+1, rcPanel.bottom+1, rcPanel.right-rcPanel.left+1, rcPanel.bottom-rcPanel.top+1);
		else
			lstrcpy(szSize, L"<Absent>");
		SetDlgItemText(gSet.hInfo, tPanelLeft, szSize);
		RCon()->GetPanelRect(TRUE, &rcPanel);
		if (rcPanel.right>rcPanel.left)
			wsprintf(szSize, L"(%i, %i)-(%i, %i), %ix%i", rcPanel.left+1, rcPanel.top+1, rcPanel.right+1, rcPanel.bottom+1, rcPanel.right-rcPanel.left+1, rcPanel.bottom-rcPanel.top+1);
		else
			lstrcpy(szSize, L"<Absent>");
		SetDlgItemText(gSet.hInfo, tPanelRight, szSize);
	}
}

void CVirtualConsole::Box(LPCTSTR szText)
{
#ifdef _DEBUG
    _ASSERT(FALSE);
#endif
    MessageBox(NULL, szText, _T("ConEmu"), MB_ICONSTOP);
}

RECT CVirtualConsole::GetRect()
{
    RECT rc;
    if (Width == 0 || Height == 0) { // ���� ������� ��� �� ����������� - ����������� �� ������� GUI ����
        //rc = MakeRect(winSize.X, winSize.Y);
        RECT rcWnd; GetClientRect(ghWnd, &rcWnd);
        RECT rcCon = gConEmu.CalcRect(CER_CONSOLE, rcWnd, CER_MAINCLIENT);
        RECT rcDC = gConEmu.CalcRect(CER_DC, rcCon, CER_CONSOLE);
        rc = MakeRect(rcDC.right, rcDC.bottom);
    } else {
        rc = MakeRect(Width, Height);
    }
    return rc;
}

void CVirtualConsole::OnFontChanged()
{
    if (!this) return;
    mb_RequiredForceUpdate = true;
	//ClearPartBrushes();
}

void CVirtualConsole::OnConsoleSizeChanged()
{
	// �� ���� ��� ������� ���������� ��� ������� ���� GUI ConEmu
	BOOL lbLast = mb_InPaintCall;
	mb_InPaintCall = TRUE; // ����� Invalidate ������ ��� �� ��������
	Update(mb_RequiredForceUpdate);
	mb_InPaintCall = lbLast;
}

// ������� ����� �����, � �� � gSet, �.�. ����� �� ����� ����� ����� ����������� ����������
COORD CVirtualConsole::ClientToConsole(LONG x, LONG y)
{
    TODO("X ���������� ��� ��������, ��� ��� ����� �� ����� ��������� ������� ����������...");
    _ASSERTE(nFontWidth!=0 && nFontHeight!=0);
    COORD cr = {0,0};
    // ������� ��������������� �������� �� �������� ������
    if (nFontHeight)
        cr.Y = y/nFontHeight;
    if (nFontWidth)
        cr.X = x/nFontWidth;
    // � ������, ���� ��������, ������� X ����������
    if (this) {
    	if (ConCharX && cr.Y >= 0 && cr.Y < (int)TextHeight) {
    		DWORD* ConCharXLine = ConCharX + cr.Y * TextWidth;
			for (uint i = 0; i < TextWidth; i++, ConCharXLine++) {
				if (((int)*ConCharXLine) >= x) {
					if (cr.X != (int)i) {
						#ifdef _DEBUG
						wchar_t szDbg[120]; wsprintf(szDbg, L"Coord corrected from {%i-%i} to {%i-%i}",
							cr.X, cr.Y, i, cr.Y);
						DEBUGSTRCOORD(szDbg);
						#endif

						cr.X = i;
					}
					break;
				}
			}
    	}
    }
    return cr;
}

// �� ����� ���� �� ������ �������, �� � ucBoxSinglHorz/ucBoxDblVertHorz
void CVirtualConsole::DistributeSpaces(wchar_t* ConCharLine, WORD* ConAttrLine, DWORD* ConCharXLine, const int j, const int j2, const int end)
{
	//WORD attr = ConAttrLine[j];
	//wchar_t ch, c;
	//
	//if ((c=ConCharLine[j]) == ucSpace || c == ucNoBreakSpace || c == 0)
	//{
	//	while(j2 < end && ConAttrLine[j2] == attr
	//		// ����� � ��� &nbsp; � 0x00
	//		&& ((ch=ConCharLine[j2]) == ucSpace || ch == ucNoBreakSpace || ch == 0))
	//		j2++;
	//} else
	//if ((c=ConCharLine[j]) == ucBoxSinglHorz || c == ucBoxDblVertHorz)
	//{
	//	while(j2 < end && ConAttrLine[j2] == attr
	//		&& ((ch=ConCharLine[j2]) == ucBoxSinglHorz || ch == ucBoxDblVertHorz))
	//		j2++;
	//} else
	//if (isCharProgress(c=ConCharLine[j]))
	//{
	//	while(j2 < end && ConAttrLine[j2] == attr && (ch=ConCharLine[j2]) == c)
	//		j2++;
	//}

	_ASSERTE(j2 > j && j >= 0);

	// ������ ������� "�������" (��� ������� �����) ����� ������� ��������������� ����������� ��� �����

	if (j2>=(int)TextWidth) { // ����� ������
		ConCharXLine[j2-1] = Width;
	} else {
		//2009-09-09 ��� �����������. ������ ������ ����� ����� ���� ������ ����������
		//int nBordWidth = gSet.BorderFontWidth(); if (!nBordWidth) nBordWidth = nFontWidth;
		int nBordWidth = nFontWidth;

		// ���������� ���������� ����� ������������������
		if (isCharBorderVertical(ConCharLine[j2])) {
			//2009-09-09 � ��� �������������� �� ����� (���� nFontWidth == nBordWidth)
			//ConCharXLine[j2-1] = (j2-1) * nFontWidth + nBordWidth; // ��� ��� [j] ������ ����?
			ConCharXLine[j2-1] = j2 * nBordWidth;
		} else {
			TODO("��� ���������������� ������� ���� ������ ���-�� �� �������!");
			//2009-09-09 � ��� �������������� �� ����� (���� nFontWidth == nBordWidth)
			//ConCharXLine[j2-1] = (j2-1) * nFontWidth + nBordWidth; // ��� ��� [j] ������ ����?
			if (gSet.isProportional && j > 1) {
				//2009-12-31 ����� ������� �� ����������� �������!
				ConCharXLine[j2-1] = ConCharXLine[j-1] + (j2 - j) * nBordWidth;
			} else {
				ConCharXLine[j2-1] = j2 * nBordWidth;
			}
		}
	}

	if (j2 > (j + 1))
	{
		MCHKHEAP
		DWORD n1 = (j ? ConCharXLine[j-1] : 0); // j? ���� j==0 �� ��� � ��� 10 (������ ������� 0�� ������� � ������)
		DWORD n3 = j2-j; // ���-�� ��������
		_ASSERTE(n3>0);
		DWORD n2 = ConCharXLine[j2-1] - n1; // ���������� �� ������� ������
		MCHKHEAP

		for (int k=j, l=1; k<(j2-1); k++, l++) {
			#ifdef _DEBUG
			DWORD nn = n1 + (n3 ? klMulDivU32(l, n2, n3) : 0);
			if (nn != ConCharXLine[k])
				ConCharXLine[k] = nn;
			#endif
			ConCharXLine[k] = n1 + (n3 ? klMulDivU32(l, n2, n3) : 0);
			//n1 + (n3 ? klMulDivU32(k-j, n2, n3) : 0);
			MCHKHEAP
		}
	}
}
