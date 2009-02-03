/* ****************************************** 
   Changes history 
   Maximus5: ����� ��� static
   2009-01-31 Maximus5
	*) ���������� ��� ������ InfoW.Control() � ��� ���������� ���������� � ��������� ��������� �������.
	*) ACTL_GETWINDOWINFO �������� �� ����� ����������� ACTL_GETSHORTWINDOWINFO, ����� � �� ���� FREE ��������. ��� ���, ��� ��������� ������ �� ���� �� ������������. � ��� ������������, ����� ACTL_GETWINDOWINFO ���� �����������.
	*) ����� ECTL_GETINFO ���� �������� ECTL_FREEINFO.
****************************************** */

//#include <stdio.h>
#include <windows.h>
//#include <windowsx.h>
//#include <string.h>
//#include <tchar.h>
#include "..\common\common.hpp"
#include "..\common\pluginW757.hpp"
#include "PluginHeader.h"

#ifndef FORWARD_WM_COPYDATA
#define FORWARD_WM_COPYDATA(hwnd, hwndFrom, pcds, fn) \
    (BOOL)(UINT)(DWORD)(fn)((hwnd), WM_COPYDATA, (WPARAM)(hwndFrom), (LPARAM)(pcds))
#endif
#define MAKEFARVERSION(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))

// minimal(?) FAR version 2.0 alpha build 748
int WINAPI _export GetMinFarVersionW(void)
{
	return MAKEFARVERSION(2,0,748);
}


HWND ConEmuHwnd=NULL;
BOOL bWasSetParent=FALSE;
HWND FarHwnd=NULL;
HANDLE hPipe=NULL, hPipeEvent=NULL;
HANDLE hThread=NULL;
FarVersion gFarVersion;
WCHAR gszDir1[0x400], gszDir2[0x400];
//UINT_PTR uTimerID=0;

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	return TRUE;
}

HWND WINAPI _export GetFarHWND()
{
	if (ConEmuHwnd) {
		if (IsWindow(ConEmuHwnd))
			return ConEmuHwnd;
		ConEmuHwnd = NULL;
	}
	return FarHwnd;
}

void WINAPI _export GetFarVersion ( FarVersion* pfv )
{
	if (!pfv)
		return;

	*pfv = gFarVersion;
}

BOOL LoadFarVersion()
{
    BOOL lbRc=FALSE;
    WCHAR FarPath[MAX_PATH+1];
    if (GetModuleFileName(0,FarPath,MAX_PATH)) {
		DWORD dwRsrvd = 0;
		DWORD dwSize = GetFileVersionInfoSize(FarPath, &dwRsrvd);
		if (dwSize>0) {
			void *pVerData = calloc(dwSize, 1);
			VS_FIXEDFILEINFO *lvs = NULL;
			UINT nLen = sizeof(lvs);
			if (GetFileVersionInfo(FarPath, 0, dwSize, pVerData)) {
				if (VerQueryValue ((void*)pVerData, L"\\", (void**)&lvs, &nLen)) {
					gFarVersion.dwVer = lvs->dwFileVersionMS;
					gFarVersion.dwBuild = lvs->dwFileVersionLS;
					lbRc = TRUE;
				}
			}
			free(pVerData);
		}
	}

	if (!lbRc) {
		gFarVersion.dwVerMajor = 2;
		gFarVersion.dwVerMinor = 0;
		gFarVersion.dwBuild = 757;
	}

	return lbRc;
}

VOID CALLBACK ConEmuCheckTimerProc(
  HWND hwnd,         // handle to window
  UINT uMsg,         // WM_TIMER message
  UINT_PTR idEvent,  // timer identifier
  DWORD dwTime       // current system time
);

void UpdateConEmuTabsW(int event, bool losingFocus, bool editorSave);

/*extern const WCHAR *GetMsgW684(int CompareLng);
extern const WCHAR *GetMsgW757(int CompareLng);
const WCHAR *GetMsgW(int CompareLng)
{
	if (gFarVersion.dwBuild>=757)
		return GetMsgW757(CompareLng);
	else
		return GetMsgW684(CompareLng);
}*/


extern void ProcessDragFrom684();
extern void ProcessDragFrom757();
extern void ProcessDragTo684();
extern void ProcessDragTo757();
DWORD WINAPI ThreadProcW(LPVOID lpParameter)
{
	while (true)
	{
		if (hPipeEvent && ConEmuHwnd) {
			DWORD dwWait = WaitForSingleObject(hPipeEvent, 1000);

		    if (ConEmuHwnd && FarHwnd) {
			    if (!IsWindow(ConEmuHwnd)) {
				    ConEmuHwnd = NULL;

					if (!IsWindow(FarHwnd))
					{
						MessageBox(0, L"ConEmu was abnormally termintated!\r\nExiting from FAR", L"ConEmu plugin", MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
						TerminateProcess(GetCurrentProcess(), 100);
					}
					else 
					{
						if (bWasSetParent) {
							SetParent(FarHwnd, NULL);
						}
					    
						ShowWindowAsync(FarHwnd, SW_SHOWNORMAL);
						EnableWindow(FarHwnd, true);
					}
				    
				    CloseHandle(hPipe); hPipe=NULL;
				    CloseHandle(hPipeEvent); hPipeEvent=NULL;
				    
				    return 0; // ��� ���� ����� �� ���������
			    }
		    }

			if (dwWait!=WAIT_OBJECT_0) {
				continue;
			}
		}

		if (hPipeEvent) ResetEvent(hPipeEvent);
	
		PipeCmd cmd;
		DWORD cout;
		ReadFile(hPipe, &cmd, sizeof(PipeCmd), &cout, NULL);
		switch (cmd)
		{
			case DragFrom:
			{
				if (gFarVersion.dwBuild>=757)
					ProcessDragFrom757();
				else
					ProcessDragFrom684();
				break;
			}
			case DragTo:
			{
				if (gFarVersion.dwBuild>=757)
					ProcessDragTo757();
				else
					ProcessDragTo684();
				break;
			}
		}
		Sleep(1);
	}
}

HWND AtoH(WCHAR *Str, int Len)
{
  DWORD Ret=0;
  for (; Len && *Str; Len--, Str++)
  {
    if (*Str>=L'0' && *Str<=L'9')
      (Ret*=16)+=*Str-L'0';
    else if (*Str>=L'a' && *Str<=L'f')
      (Ret*=16)+=(*Str-L'a'+10);
    else if (*Str>=L'A' && *Str<=L'F')
      (Ret*=16)+=(*Str-L'A'+10);
  }
  return (HWND)Ret;
}

void SetStartupInfoW684(void *aInfo);
void SetStartupInfoW757(void *aInfo);
void WINAPI _export SetStartupInfoW(void *aInfo)
{
	LoadFarVersion();

	if (gFarVersion.dwBuild>=757)
		SetStartupInfoW757(aInfo);
	else
		SetStartupInfoW684(aInfo);

	WCHAR *pipename = (WCHAR*)calloc(MAX_PATH,sizeof(WCHAR));

	if (!ConEmuHwnd) {
		if (GetEnvironmentVariable(L"ConEmuHWND", pipename, MAX_PATH)) {
			if (pipename[0]==L'0' && pipename[1]==L'x') {
				ConEmuHwnd = AtoH(pipename+2, 8);
				if (ConEmuHwnd) {
					WCHAR className[100];
					GetClassName(ConEmuHwnd, (LPWSTR)className, 100);
					if (lstrcmpiW(className, L"VirtualConsoleClass") != 0 &&
						lstrcmpiW(className, L"VirtualConsoleClassMain") != 0)
					{
						ConEmuHwnd = NULL;
					} else {
						// ����� ���������, ��� ������� ����������� �������� ConEmu
						DWORD dwEmuPID = 0;
						GetWindowThreadProcessId(ConEmuHwnd, &dwEmuPID);
						if (dwEmuPID = 0) {
							// ������, �� ������� �������� ��� �������� ConEmu
						} else {
							DWORD *ProcList = (DWORD*)calloc(1000,sizeof(DWORD));
							DWORD dwCount = GetConsoleProcessList(ProcList,1000);
							if(dwCount>1000) {
								// ������, � ������� ������� ����� ���������
							} else {
								for (DWORD dw=0; dw<dwCount; dw++) {
									if (dwEmuPID == ProcList[dw]) {
										dwEmuPID = 0; break;
									}
								}
								if (dwEmuPID)
									ConEmuHwnd = NULL; // ������� �� ����������� ConEmu
							}
							free(ProcList);
						}
					}
				}
			}
		}
	}
	
	// ���� �� �� ��� ���������� - ������ ������ ������ �� �����
	if (ConEmuHwnd) {
		//Maximus5 - ������ ��� ����� - �� ������ ���� FAR'�
		//wsprintf(pipename, L"\\\\.\\pipe\\ConEmu%h", ConEmuHwnd);
		//DWORD dwFarProcID = GetCurrentProcessId();
		wsprintf(pipename, L"\\\\.\\pipe\\ConEmuP%u", FarHwnd);
		hPipe = CreateFile( 
			 pipename,   // pipe name 
			 GENERIC_READ |  // read and write access 
			 GENERIC_WRITE, 
			 0,              // no sharing 
			 NULL,           // default security attributes
			 OPEN_EXISTING,  // opens existing pipe 
			 0,              // default attributes 
			 NULL);          // no template file 
		if (hPipe && hPipe!=INVALID_HANDLE_VALUE)
		{
			wsprintf(pipename, L"ConEmuPEvent%u", /*hWnd*/ FarHwnd );
			hPipeEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, pipename);
			if (hPipeEvent==INVALID_HANDLE_VALUE) hPipeEvent=NULL;
		
			hThread=CreateThread(NULL, 0, &ThreadProcW, 0, 0, 0);
		}
	}

	free(pipename);

#ifdef _DEBUG
	//MessageBox(ConEmuHwnd,L"Debug",L"ConEmu plugin",MB_SETFOREGROUND);
#endif

}

void UpdateConEmuTabsW684(int event, bool losingFocus, bool editorSave);
void UpdateConEmuTabsW757(int event, bool losingFocus, bool editorSave);
void UpdateConEmuTabsW(int event, bool losingFocus, bool editorSave)
{
	if (gFarVersion.dwBuild>=757)
		UpdateConEmuTabsW757(event, losingFocus, editorSave);
	else
		UpdateConEmuTabsW684(event, losingFocus, editorSave);
}

void AddTab(ConEmuTab* tabs, int &tabCount, bool losingFocus, bool editorSave, 
			int Type, LPCWSTR Name, LPCWSTR FileName, int Current, int Modified)
{
	if (Type == WTYPE_PANELS) {
		tabs[0].Current = losingFocus ? 1 : 0;
		//lstrcpyn(tabs[0].Name, GetMsgW757(0), CONEMUTABMAX-1);
		tabs[0].Name[0] = 0;
		tabs[0].Pos = 0;
		tabs[0].Type = WTYPE_PANELS;
	} else
	if (Type == WTYPE_EDITOR || Type == WTYPE_VIEWER)
	{
		// when receiving losing focus event receiver is still reported as current
		tabs[tabCount].Type = Type;
		tabs[tabCount].Current = losingFocus ? 0 : Current;
		// when receiving saving event receiver is still reported as modified
		if (editorSave && lstrcmpi(FileName, Name) == 0)
			Modified = 0;
		tabs[tabCount].Modified = Modified;

		if (tabs[tabCount].Current != 0)
		{
			lastModifiedStateW = Modified != 0 ? 1 : 0;
		}
		else
		{
			lastModifiedStateW = -1;
		}

		int nLen=min(lstrlen(Name),(CONEMUTABMAX-1));
		lstrcpyn(tabs[tabCount].Name, Name, nLen+1);
		tabs[tabCount].Name[nLen]=0;

		tabs[tabCount].Pos = tabCount;
		tabCount++;
	}
}

void SendTabs(ConEmuTab* tabs, int &tabCount)
{
	if (ConEmuHwnd && IsWindow(ConEmuHwnd)) {
		COPYDATASTRUCT cds;
		if (tabs[0].Type == WTYPE_PANELS) {
			cds.dwData = tabCount;
			cds.lpData = tabs;
		} else {
			// ������� ��� - ��� ��� ������ � ������ ���������!
			cds.dwData = --tabCount;
			cds.lpData = tabs+1;
		}
		if (tabCount) {
			cds.cbData = tabCount * sizeof(ConEmuTab);
			FORWARD_WM_COPYDATA(ConEmuHwnd, FarHwnd, &cds, SendMessage);
		}
	}
	free(tabs);
}

// watch non-modified -> modified editor status change
extern int ProcessEditorInputW684(LPCVOID Rec);
extern int ProcessEditorInputW757(LPCVOID Rec);
int lastModifiedStateW = -1;
int WINAPI _export ProcessEditorInputW(LPCVOID Rec)
{
	if (!ConEmuHwnd)
		return 0; // ���� �� �� ��� ���������� - ������
	if (gFarVersion.dwBuild>=757)
		return ProcessEditorInputW757(Rec);
	else
		return ProcessEditorInputW684(Rec);
}

extern int ProcessEditorEventW684(int Event, void *Param);
extern int ProcessEditorEventW757(int Event, void *Param);
int WINAPI _export ProcessEditorEventW(int Event, void *Param)
{
	if (!ConEmuHwnd)
		return 0; // ���� �� �� ��� ���������� - ������
	if (gFarVersion.dwBuild>=757)
		return ProcessEditorEventW757(Event,Param);
	else
		return ProcessEditorEventW684(Event,Param);
}

extern int ProcessViewerEventW684(int Event, void *Param);
extern int ProcessViewerEventW757(int Event, void *Param);
int WINAPI _export ProcessViewerEventW(int Event, void *Param)
{
	if (!ConEmuHwnd)
		return 0; // ���� �� �� ��� ���������� - ������
	if (gFarVersion.dwBuild>=757)
		return ProcessViewerEventW757(Event,Param);
	else
		return ProcessViewerEventW684(Event,Param);
}

extern void ExitFARW684(void);
extern void ExitFARW757(void);
void   WINAPI _export ExitFARW(void)
{
	if (gFarVersion.dwBuild>=757)
		return ExitFARW757();
	else
		return ExitFARW684();
}
