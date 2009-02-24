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
#include "..\common\pluginW789.hpp"
#include "PluginHeader.h"

extern "C" {
#include "../common/ConEmuCheck.h"
}

#define MAKEFARVERSION(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))

// minimal(?) FAR version 2.0 alpha build 757
int WINAPI _export GetMinFarVersionW(void)
{
	return MAKEFARVERSION(2,0,757);
}

/* COMMON - ���� ��������� �� ����������� */
void WINAPI _export GetPluginInfoW(struct PluginInfo *pi)
{
	pi->StructSize = sizeof(struct PluginInfo);
	pi->Flags = PF_EDITOR | PF_VIEWER | PF_PRELOAD;
	pi->DiskMenuStrings = NULL;
	pi->DiskMenuNumbers = 0;
	pi->PluginMenuStrings = NULL;
	pi->PluginMenuStringsNumber =0;
	pi->PluginConfigStrings = NULL;
	pi->PluginConfigStringsNumber = 0;
	pi->CommandPrefix = NULL;
	pi->Reserved = 0;	
}
/* COMMON - end */


HWND ConEmuHwnd=NULL;
BOOL TerminalMode = FALSE;
HWND FarHwnd=NULL;
HANDLE hEventCmd[MAXCMDCOUNT], hEventAlive=NULL, hEventReady=NULL;
HANDLE hThread=NULL;
FarVersion gFarVersion;
WCHAR gszDir1[CONEMUTABMAX], gszDir2[CONEMUTABMAX];
int maxTabCount = 0, lastWindowCount = 0, gnCurTabCount = 0;
ConEmuTab* tabs = NULL; //(ConEmuTab*) calloc(maxTabCount, sizeof(ConEmuTab));
LPBYTE gpData=NULL, gpCursor=NULL;
DWORD  gnDataSize=0;
HANDLE ghMapping = NULL;

#if defined(__GNUC__)
typedef HWND (APIENTRY *FGetConsoleWindow)();
FGetConsoleWindow GetConsoleWindow = NULL;
#endif
extern void SetConsoleFontSizeTo(HWND inConWnd, int inSizeX, int inSizeY);

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			{
				#ifdef _DEBUG
				//if (!IsDebuggerPresent())
				//	MessageBoxA(GetForegroundWindow(), "ConEmu.dll loaded", "ConEmu", 0);
				#endif
				#if defined(__GNUC__)
				GetConsoleWindow = (FGetConsoleWindow)GetProcAddress(GetModuleHandle(L"kernel32.dll"),"GetConsoleWindow");
				#endif
				HWND hConWnd = GetConsoleWindow();
				InitHWND(hConWnd);
				
			    // Check Terminal mode
			    TCHAR szVarValue[MAX_PATH];
			    szVarValue[0] = 0;
			    if (GetEnvironmentVariable(L"TERM", szVarValue, 63)) {
				    TerminalMode = TRUE;
			    }
			}
			break;
	}
	return TRUE;
}

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
  HWND WINAPI GetFarHWND();
  HWND WINAPI GetFarHWND2(BOOL abConEmuOnly);
  void WINAPI GetFarVersion ( FarVersion* pfv );
  int  WINAPI ProcessEditorInputW(void* Rec);
  void WINAPI SetStartupInfoW(void *aInfo);
  BOOL WINAPI IsTerminalMode();
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  /*(void) lpReserved;
  (void) dwReason;
  (void) hDll;*/
  return DllMain(hDll, dwReason,lpReserved);
}
#endif

HWND WINAPI GetFarHWND2(BOOL abConEmuOnly)
{
	if (ConEmuHwnd) {
		if (IsWindow(ConEmuHwnd))
			return ConEmuHwnd;
		ConEmuHwnd = NULL;
	}
	if (abConEmuOnly)
		return NULL;
	return FarHwnd;
}

HWND WINAPI _export GetFarHWND()
{
    return GetFarHWND2(FALSE);
}

BOOL WINAPI IsTerminalMode()
{
    return TerminalMode;
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
				if (VerQueryValue ((void*)pVerData, 
						L"\\", (void**)&lvs, &nLen)) {
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
		gFarVersion.dwBuild = 789;
	}

	return lbRc;
}


DWORD WINAPI ThreadProcW(LPVOID lpParameter)
{
	DWORD dwProcId = GetCurrentProcessId();

	while (true)
	{
		DWORD dwWait = 0;
		DWORD dwTimeout = 1000;
		/*#ifdef _DEBUG
		dwTimeout = INFINITE;
		#endif*/

		dwWait = WaitForMultipleObjects(MAXCMDCOUNT, hEventCmd, FALSE, dwTimeout);

		// ������������, ���� ��������� ����� ����������� � ��� ConEmuHwnd (��� ��������)
	    if (ConEmuHwnd && FarHwnd && (dwWait>=(WAIT_OBJECT_0+MAXCMDCOUNT))) {
			// ��� ���� ������ Detach! (CtrlAltTab)
			HWND hConWnd = GetConsoleWindow();
			
		    if (!IsWindow(ConEmuHwnd) || hConWnd!=FarHwnd) {
			    ConEmuHwnd = NULL;

				if (hConWnd!=FarHwnd || !IsWindow(FarHwnd))
				{
					if (hConWnd != FarHwnd && IsWindow(hConWnd)) {
						FarHwnd = hConWnd;
						int nBtn = ShowMessage(1, 2);
						if (nBtn == 0) {
							// Create process, with flag /Attach GetCurrentProcessId()
							// Sleep for sometimes, try InitHWND(hConWnd); several times
							WCHAR  szExe[0x200];
							WCHAR  szCurArgs[0x600];
							
							PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
							STARTUPINFO si; memset(&si, 0, sizeof(si));
							si.cb = sizeof(si);
							
							//TODO: ConEmu.exe
							int nLen = 0;
							if ((nLen=GetModuleFileName(0, szExe, 0x190))==0) {
								goto closethread;
							}
							WCHAR* pszSlash = szExe+nLen-1;
			                while (pszSlash>szExe && *(pszSlash-1)!=L'\\') pszSlash--;
			                lstrcpyW(pszSlash, L"ConEmu.exe");
			                
							DWORD dwPID = GetCurrentProcessId();
							wsprintf(szCurArgs, L"\"%s\" /Attach %i ", szExe, dwPID);
							
							GetEnvironmentVariableW(L"ConEmuArgs", szCurArgs+lstrlenW(szCurArgs), 0x380);
			                
							
							if (!CreateProcess(NULL, szCurArgs, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL,
									NULL, &si, &pi))
							{
								// ������ �� ������ ��������?
								goto closethread;
							}
							
							// ����
							while (TRUE)
							{
								dwWait = WaitForSingleObject(hEventCmd[CMD_EXIT], 200);
								// ��� �����������
								if (dwWait == WAIT_OBJECT_0) {
									CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
									goto closethread;
								}
								if (!GetExitCodeProcess(pi.hProcess, &dwPID) || dwPID!=STILL_ACTIVE) {
									CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
									goto closethread;
								}
								InitHWND(hConWnd);
								if (ConEmuHwnd) {
									// ����������, �� ����� ConEmu ������ ����� �������� �� ����...
									SetConsoleFontSizeTo(FarHwnd, 4, 6);
									break;
								}
							}
							
							// ������� ������
							CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
							// ������ �� ����� ���� ��������...
							SendTabs(gnCurTabCount, FALSE/*abForce*/);
							continue;
						} else {
							// ������������ ���������, ������� �� ���� ���������
							goto closethread;
						}
					} else {
						MessageBox(0, L"ConEmu was abnormally termintated!\r\nExiting from FAR", L"ConEmu plugin", MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
						TerminateProcess(GetCurrentProcess(), 100);
					}
				}
				else 
				{
					//if (bWasSetParent) { -- ��� ����� ��� �� �������, ���� ��� ���� �������� - ������
					//	SetParent(FarHwnd, NULL);
					//}
				    
					ShowWindowAsync(FarHwnd, SW_SHOWNORMAL);
					EnableWindow(FarHwnd, true);
				}
			    
				goto closethread;
		    }
	    }

		if (dwWait == (WAIT_OBJECT_0+CMD_EXIT))
		{
			goto closethread;
		}

		if (dwWait>=(WAIT_OBJECT_0+MAXCMDCOUNT))
		{
			continue;
		}

		SafeCloseHandle(ghMapping);
		// �������� ������, ��� �� ���������� � ���������
		SetEvent(hEventAlive);


		switch (dwWait)
		{
			case (WAIT_OBJECT_0+CMD_DRAGFROM):
			{
				if (gFarVersion.dwVerMajor==1)
					ProcessDragFromA();
				else if (gFarVersion.dwBuild>=789)
					ProcessDragFrom789();
				else
					ProcessDragFrom757();
				break;
			}
			case (WAIT_OBJECT_0+CMD_DRAGTO):
			{
				if (gFarVersion.dwVerMajor==1)
					ProcessDragToA();
				if (gFarVersion.dwBuild>=789)
					ProcessDragTo789();
				else
					ProcessDragTo757();
				break;
			}
		}

		// ������������� � �������� ������
		wsprintf(gszDir1, CONEMUMAPPING, dwProcId);
		gnDataSize = (gpCursor - gpData);
		ghMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, gnDataSize+4, gszDir1);
		if (ghMapping) {
			LPBYTE lpMap = (LPBYTE)MapViewOfFile(ghMapping, FILE_MAP_ALL_ACCESS, 0,0,0);
			if (!lpMap) {
				// ������
				SafeCloseHandle(ghMapping);
			} else {
				// copy data
				if (gpData && gnDataSize) {
					*((DWORD*)lpMap) = gnDataSize+4;
					memcpy(lpMap+4, gpData, gnDataSize);
				} else {
					*((DWORD*)lpMap) = 0;
				}

				//unmaps a mapped view of a file from the calling process's address space
				UnmapViewOfFile(lpMap);
			}
		}

		// �������� ������, ��� ������ ��� � ��������
		SetEvent(hEventReady);

		//Sleep(1);
	}


closethread:
	// ��������� ��� ������ � �������
	for (int i=0; i<MAXCMDCOUNT; i++)
		SafeCloseHandle(hEventCmd[i]);
	SafeCloseHandle(hEventAlive);
	SafeCloseHandle(hEventReady);
	SafeCloseHandle(ghMapping);

	return 0;
}

void WINAPI _export SetStartupInfoW(void *aInfo)
{
	if (!gFarVersion.dwVerMajor) LoadFarVersion();

	if (gFarVersion.dwBuild>=789)
		SetStartupInfoW789(aInfo);
	else
		SetStartupInfoW757(aInfo);

	CheckMacro();
}

#define CREATEEVENT(fmt,h) \
		wsprintf(szEventName, fmt, dwCurProcId ); \
		h = CreateEvent(NULL, FALSE, FALSE, szEventName); \
		if (h==INVALID_HANDLE_VALUE) h=NULL;
	
void InitHWND(HWND ahFarHwnd)
{
	LoadFarVersion(); // ���������� ��� �����!

	ConEmuHwnd = NULL;
	FarHwnd = ahFarHwnd;

	memset(hEventCmd, 0, sizeof(HANDLE)*MAXCMDCOUNT);
	
	int nChk = 0;
	ConEmuHwnd = GetConEmuHWND ( FALSE, &nChk );

	
	// ���� �� �� ��� ���������� - ������ ������ ������ �� �����
	if (ConEmuHwnd) {
		WCHAR szEventName[128];
		DWORD dwCurProcId = GetCurrentProcessId();

		
		CREATEEVENT(CONEMUDRAGFROM, hEventCmd[CMD_DRAGFROM]);
		CREATEEVENT(CONEMUDRAGTO, hEventCmd[CMD_DRAGTO]);
		CREATEEVENT(CONEMUEXIT, hEventCmd[CMD_EXIT]);
		CREATEEVENT(CONEMUALIVE, hEventAlive);
		CREATEEVENT(CONEMUREADY, hEventReady);

		hThread=CreateThread(NULL, 0, &ThreadProcW, 0, 0, 0);

		// ������� ����, ���� ��� �����
		int tabCount = 0;
		CreateTabs(1);
		AddTab(tabCount, true, false, WTYPE_PANELS, NULL, NULL, 0, 0);
		SendTabs(tabCount=1, TRUE);
	}
}

void CheckMacro()
{
	// ���� �� �� ��� ���������� - ������ ������ ������ �� �����
	if (ConEmuHwnd)
	{
		// �������� ������� �������
		BOOL lbMacroAdded = FALSE, lbNeedMacro = FALSE;
		HKEY hkey=NULL;
		char szValue[1024], szMacroKey[128], szCheckKey[16];
		DWORD dwSize = 0;
		bool lbMacroDontCheck = false;

		if (gFarVersion.dwVerMajor==2) {
			lstrcpyA(szCheckKey, "F14DontCheck2");
			lstrcpyA(szMacroKey, "Software\\Far2\\KeyMacros\\Common\\F14");
		} else {
			lstrcpyA(szCheckKey, "F14DontCheck2");
			lstrcpyA(szMacroKey, "Software\\Far\\KeyMacros\\Common\\F14");
		}

		if (0==RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\ConEmu", 0, KEY_ALL_ACCESS, &hkey))
		{

			if (RegQueryValueExA(hkey, szCheckKey, 0, 0, (LPBYTE)&lbMacroDontCheck, &(dwSize=sizeof(lbMacroDontCheck))))
				lbMacroDontCheck = false;
			RegCloseKey(hkey); hkey=NULL;
		}

		if (!lbMacroDontCheck)
		{
			if (0==RegOpenKeyExA(HKEY_CURRENT_USER, szMacroKey, 0, KEY_ALL_ACCESS, &hkey))
			{
				if (0!=RegQueryValueExA(hkey, "Sequence", 0, 0, (LPBYTE)szValue, &(dwSize=1022))) {
					lbNeedMacro = TRUE; // �������� ����������
				} else {
					szValue[dwSize]=0;
					if (strstr(szValue, " F12 ")==0)
						lbNeedMacro = TRUE; // �������� ������������
				}
				RegCloseKey(hkey); hkey=NULL;
			} else {
				lbNeedMacro = TRUE;
			}

			if (lbNeedMacro) {
				int nBtn = ShowMessage(0, 3);
				if (nBtn == 1) { // Don't disturb
					DWORD disp=0;
					lbMacroDontCheck = true;
					if (0==RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\ConEmu", 0, 0, 
						0, KEY_ALL_ACCESS, 0, &hkey, &disp))
					{
						RegSetValueExA(hkey, szCheckKey, 0, REG_BINARY, (LPBYTE)&lbMacroDontCheck, (dwSize=sizeof(lbMacroDontCheck)));
						RegCloseKey(hkey); hkey=NULL;
					}
				} else if (nBtn == 0) {
					DWORD disp=0;
					lbMacroAdded = TRUE;
					if (0==RegCreateKeyExA(HKEY_CURRENT_USER, szMacroKey, 0, 0, 
						0, KEY_ALL_ACCESS, 0, &hkey, &disp))
					{
						lstrcpyA(szValue, 
							"ConEmu tab switching support");
						RegSetValueExA(hkey, "", 0, REG_SZ, (LPBYTE)szValue, (dwSize=strlen(szValue)+1));

						lstrcpyA(szValue, 
							"$If (Shell || Info || QView || Tree || Viewer || Editor) F12 $Else waitkey(100) $End");
						RegSetValueExA(hkey, "Sequence", 0, REG_SZ, (LPBYTE)szValue, (dwSize=strlen(szValue)+1));

						lstrcpyA(szValue, 
							"For ConEmu - eats next key if changing screen is impossible");
						RegSetValueExA(hkey, "Description", 0, REG_SZ, (LPBYTE)szValue, (dwSize=strlen(szValue)+1));

						RegSetValueExA(hkey, "DisableOutput", 0, REG_DWORD, (LPBYTE)&(disp=1), (dwSize=4));

						RegCloseKey(hkey); hkey=NULL;
					}
				}
			}

			// ���������� ������� � FAR?
			if (lbMacroAdded) {
				if (gFarVersion.dwVerMajor==1)
					ReloadMacroA();
				else if (gFarVersion.dwBuild>=789)
					ReloadMacro789();
				else
					ReloadMacro757();
			}
		}
	}

}

void UpdateConEmuTabsW(int event, bool losingFocus, bool editorSave)
{
	if (gFarVersion.dwBuild>=789)
		UpdateConEmuTabsW789(event, losingFocus, editorSave);
	else
		UpdateConEmuTabsW757(event, losingFocus, editorSave);
}

BOOL CreateTabs(int windowCount)
{
	if ((tabs==NULL) || (maxTabCount <= (windowCount + 1)))
	{
		maxTabCount = windowCount + 10; // � �������
		if (tabs) {
			free(tabs); tabs = NULL;
		}
		
		tabs = (ConEmuTab*) calloc(maxTabCount, sizeof(ConEmuTab));
	}
	
	lastWindowCount = windowCount;
	
	return tabs!=NULL;
}

BOOL AddTab(int &tabCount, bool losingFocus, bool editorSave, 
			int Type, LPCWSTR Name, LPCWSTR FileName, int Current, int Modified)
{
    BOOL lbCh = FALSE;
    
	if (Type == WTYPE_PANELS) {
	    lbCh = (tabs[0].Current != (losingFocus ? 1 : 0)) ||
	           (tabs[0].Type != WTYPE_PANELS);
		tabs[0].Current = losingFocus ? 1 : 0;
		//lstrcpyn(tabs[0].Name, GetMsgW789(0), CONEMUTABMAX-1);
		tabs[0].Name[0] = 0;
		tabs[0].Pos = 0;
		tabs[0].Type = WTYPE_PANELS;
	} else
	if (Type == WTYPE_EDITOR || Type == WTYPE_VIEWER)
	{
		// when receiving saving event receiver is still reported as modified
		if (editorSave && lstrcmpi(FileName, Name) == 0)
			Modified = 0;
	
	    lbCh = (tabs[tabCount].Current != (losingFocus ? 0 : Current)) ||
	           (tabs[tabCount].Type != Type) ||
	           (tabs[tabCount].Modified != Modified) ||
	           (lstrcmp(tabs[tabCount].Name, Name) != 0);
	
		// when receiving losing focus event receiver is still reported as current
		tabs[tabCount].Type = Type;
		tabs[tabCount].Current = losingFocus ? 0 : Current;
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
	
	return lbCh;
}

void SendTabs(int tabCount, BOOL abForce/*=FALSE*/)
{
    gnCurTabCount = tabCount;
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
		if (tabCount || abForce) {
			cds.cbData = tabCount * sizeof(ConEmuTab);
			SendMessage(ConEmuHwnd, WM_COPYDATA, (WPARAM)FarHwnd, (LPARAM)&cds);
		}
	}
	//free(tabs); - ������������� � ExitFARW
}

// watch non-modified -> modified editor status change

int lastModifiedStateW = -1;

int WINAPI _export ProcessEditorInputW(void* Rec)
{
	if (!ConEmuHwnd)
		return 0; // ���� �� �� ��� ���������� - ������
	if (gFarVersion.dwBuild>=789)
		return ProcessEditorInputW789((LPCVOID)Rec);
	else
		return ProcessEditorInputW757((LPCVOID)Rec);
}

int WINAPI _export ProcessEditorEventW(int Event, void *Param)
{
	if (!ConEmuHwnd)
		return 0; // ���� �� �� ��� ���������� - ������
	if (gFarVersion.dwBuild>=789)
		return ProcessEditorEventW789(Event,Param);
	else
		return ProcessEditorEventW757(Event,Param);
}

int WINAPI _export ProcessViewerEventW(int Event, void *Param)
{
	if (!ConEmuHwnd)
		return 0; // ���� �� �� ��� ���������� - ������
	if (gFarVersion.dwBuild>=789)
		return ProcessViewerEventW789(Event,Param);
	else
		return ProcessViewerEventW757(Event,Param);
}

void StopThread(void)
{
	if (hEventCmd[CMD_EXIT])
		SetEvent(hEventCmd[CMD_EXIT]); // ��������� ����
	//CloseTabs(); -- ConEmu ���� ����������
	if (hThread) { // �������� ����-����, ��� ������������� ������� ���� ��������
		if (WaitForSingleObject(hThread,1000))
			TerminateThread(hThread, 100);
		CloseHandle(hThread); hThread = NULL;
	}
	
    if (tabs) {
	    free(tabs);
	    tabs = NULL;
    }
}

void   WINAPI _export ExitFARW(void)
{
	StopThread();

	if (gFarVersion.dwBuild>=789)
		ExitFARW789();
	else
		ExitFARW757();
}

void CloseTabs()
{
	if (ConEmuHwnd && IsWindow(ConEmuHwnd)) {
		COPYDATASTRUCT cds;
		cds.dwData = 0;
		cds.lpData = &cds.dwData;
		cds.cbData = sizeof(cds.dwData);
		SendMessage(ConEmuHwnd, WM_COPYDATA, (WPARAM)FarHwnd, (LPARAM)&cds);
	}
}

// ���� �� �������� - ����� ������������� �������������. ������ � ������
// ���������� FALSE ��� ������� ��������� ������
BOOL OutDataAlloc(DWORD anSize)
{
	gpData = (LPBYTE)calloc(anSize,1);
	if (!gpData)
		return FALSE;

	gnDataSize = anSize;
	gpCursor = gpData;

	return TRUE;
}

// ������ � ������. ���������� ������������� �� OutDataWrite
// ���������� FALSE ��� ������� ��������� ������
BOOL OutDataRealloc(DWORD anNewSize)
{
	if (!gpData)
		return OutDataAlloc(anNewSize);

	if (anNewSize < gnDataSize)
		return FALSE; // ������ �������� ������ ������, ��� ��� ����

	// realloc ������ �� ��������, ��� ��� ���� � �� ��������
	LPBYTE lpNewData = (LPBYTE)calloc(anNewSize,1);
	if (!lpNewData)
		return FALSE;

	// ����������� ������������ ������
	memcpy(lpNewData, gpData, gnDataSize);
	// ��������� ����� ������� �������
	gpCursor = lpNewData + (gpCursor - gpData);
	// � ����� ����� � ��������
	free(gpData);
	gpData = lpNewData;
	gnDataSize = anNewSize;

	return TRUE;
}

// ������ � ������
// ���������� FALSE ��� ������� ��������� ������
BOOL OutDataWrite(LPVOID apData, DWORD anSize)
{
	if (!gpData) {
		if (!OutDataAlloc(max(1024, (anSize+128))))
			return FALSE;
	} else if (((gpCursor-gpData)+anSize)>gnDataSize) {
		if (!OutDataRealloc(gnDataSize+max(1024, (anSize+128))))
			return FALSE;
	}

	// ����������� ������
	memcpy(gpCursor, apData, anSize);
	gpCursor += anSize;

	return TRUE;
}

int ShowMessage(int aiMsg, int aiButtons)
{
	if (gFarVersion.dwVerMajor==1)
		return ShowMessageA(aiMsg, aiButtons);
	else if (gFarVersion.dwBuild>=789)
		return ShowMessage789(aiMsg, aiButtons);
	else
		return ShowMessage757(aiMsg, aiButtons);
}
