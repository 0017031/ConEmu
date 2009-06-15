#include <windows.h>
#include <Commctrl.h>
#include <uxtheme.h>
#include "header.h"


CProgressBars::CProgressBars(HWND hWnd, HINSTANCE hInstance)
{
	this->hWnd=hWnd;
	Progressbar1 = CreateWindow(PROGRESS_CLASS, NULL, PBS_SMOOTH | WS_CHILD | TCS_FOCUSNEVER, 0, 0, 100, 10, hWnd, NULL, hInstance, NULL);
	Progressbar2 = CreateWindow(PROGRESS_CLASS, NULL, PBS_SMOOTH | WS_CHILD | TCS_FOCUSNEVER, 0, 0, 100, 10, hWnd, NULL, hInstance, NULL);

	SendMessage(Progressbar1, PBM_SETRANGE, 0, (LPARAM) MAKELPARAM (0, 100));
	SendMessage(Progressbar2, PBM_SETRANGE, 0, (LPARAM) MAKELPARAM (0, 100));		
	
	MultiByteToWideChar(CP_ACP,0,"�����������",-1,ms_CopyingLocal,countof(ms_CopyingLocal));
	MultiByteToWideChar(CP_ACP,0,"�������",-1,ms_MovingLocal,countof(ms_MovingLocal));

	mh_Uxtheme = LoadLibrary(_T("UxTheme.dll"));
	if (mh_Uxtheme) {
		SetWindowThemeF = (SetWindowThemeT)GetProcAddress(mh_Uxtheme, "SetWindowTheme");
		if (SetWindowThemeF) {
			SetWindowThemeF(Progressbar1, _T(" "), _T(" "));
			SetWindowThemeF(Progressbar2, _T(" "), _T(" "));
		}
	}

//	COL_DIALOGTEXT
}

CProgressBars::~CProgressBars()
{
	if (mh_Uxtheme) {
		FreeLibrary(mh_Uxtheme); mh_Uxtheme = NULL;
	}
}

void CProgressBars::OnTimer()
{
	if (gSet.isGUIpb && isCoping())
	{
		CVirtualConsole* pCon = gConEmu.ActiveCon();
		uint TextWidth  = pCon->TextWidth;
		uint TextHeight = pCon->TextHeight;
		LPCTSTR ConChar = pCon->mpsz_ConChar;

		int delta=0;
		if (ConChar[((TextHeight-2)/2)*TextWidth + (TextWidth-45)/2 + 45] == _T('%'))
			delta=1; //��� ������ ����������

		if (ConChar[((TextHeight-4)/2+delta)*TextWidth + (TextWidth-45)/2 + 45] == _T('%'))
		{
			RECT rcShift; memset(&rcShift, 0, sizeof(RECT));
			MapWindowPoints(ghWndDC, ghWnd, (LPPOINT)&rcShift, 2);

			WCHAR tmp[4];
			tmp[0]=ConChar[((TextHeight-4)/2+delta)*TextWidth + (TextWidth-45)/2 + 42];
			tmp[1]=ConChar[((TextHeight-4)/2+delta)*TextWidth + (TextWidth-45)/2 + 43];
			tmp[2]=ConChar[((TextHeight-4)/2+delta)*TextWidth + (TextWidth-45)/2 + 44];
			tmp[3]=0;
			int perc;
			#if !defined(__GNUC__)
			swscanf_s(tmp, _T("%i"), &perc);
			#else
			swscanf(tmp, _T("%i"), &perc);
			#endif

			SendMessage(Progressbar1, PBM_SETPOS, perc, 0);
			SetWindowPos(Progressbar1, 0, 
				((TextWidth-45)/2)*gSet.FontWidth()+rcShift.left, 
				(TextHeight-4+delta*2)/2*gSet.FontHeight()+rcShift.top/*TabBar.Height()*/, 
				gSet.FontWidth()*41, gSet.FontHeight(), 0);
			ShowWindow(Progressbar1, SW_SHOW);

			if (ConChar[((TextHeight)/2)*TextWidth + (TextWidth-45)/2 + 45] == _T('%'))
			{
				WCHAR tmp[4];
				tmp[0]=ConChar[((TextHeight)/2)*TextWidth + (TextWidth-45)/2 + 42];
				tmp[1]=ConChar[((TextHeight)/2)*TextWidth + (TextWidth-45)/2 + 43];
				tmp[2]=ConChar[((TextHeight)/2)*TextWidth + (TextWidth-45)/2 + 44];
				tmp[3]=0;
				int perc;
				#if !defined(__GNUC__)
				swscanf_s(tmp, _T("%i"), &perc);
				#else
				swscanf(tmp, _T("%i"), &perc);
				#endif

				SendMessage(Progressbar2, PBM_SETPOS, perc, 0);
				SetWindowPos(Progressbar2, 0, 
					((TextWidth-45)/2)*gSet.FontWidth()+rcShift.left, 
					(TextHeight)/2*gSet.FontHeight()+rcShift.top/*TabBar.Height()*/, 
					gSet.FontWidth()*41, gSet.FontHeight(), 0);
				ShowWindow(Progressbar2, SW_SHOW);
			}
			else
				ShowWindow(Progressbar2, SW_HIDE);
		}
	//	else
	//		ShowWindow(Progressbar1, SW_HIDE);
	}
	else
	{
		ShowWindow(Progressbar1, SW_HIDE);
		ShowWindow(Progressbar2, SW_HIDE);
	}
}

bool CProgressBars::isCoping()
{
	//TODO: �����������
	LPCTSTR pszTitle = gConEmu.GetTitle();
	if (wcsstr(pszTitle, ms_CopyingLocal)
		|| wcsstr(pszTitle, L"Copying")
		|| wcsstr(pszTitle, ms_MovingLocal)
		|| wcsstr(pszTitle, L"Moving")
		)
		return true;
	return false;
}
