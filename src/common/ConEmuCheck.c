
#include <windows.h>
#define _T(s) s
#include "ConEmuCheck.h"

typedef HWND (APIENTRY *FGetConsoleWindow)();
typedef DWORD (APIENTRY *FGetConsoleProcessList)(LPDWORD,DWORD);


//#if defined(__GNUC__)
//extern "C" {
//#endif
//	WINBASEAPI DWORD GetEnvironmentVariableW(LPCWSTR lpName,LPWSTR lpBuffer,DWORD nSize);
//#if defined(__GNUC__)
//};
//#endif


// 0 -- All OK (ConEmu found, Version OK)
// 1 -- NO ConEmu (simple console mode)
// 2 -- ConEmu found, but old version
int ConEmuCheck(HWND* ahConEmuWnd)
{
    int nChk = -1;
    HWND ConEmuWnd = NULL;
    
    ConEmuWnd = GetConEmuHWND(FALSE, &nChk);

    // ���� ������ ������ ����� - ���������� ���
    if (ahConEmuWnd) *ahConEmuWnd = ConEmuWnd;
    
    if (ConEmuWnd == NULL) {
	    return 1; // NO ConEmu (simple console mode)
    } else {
	    if (nChk>=3)
		    return 2; // ConEmu found, but old version
		return 0;
    }
}


// Returns HWND of Gui console DC window
//     pnConsoleIsChild [out]
//        -1 -- Non ConEmu mode
//         0 -- console is standalone window
//         1 -- console is child of ConEmu DC window
//         2 -- console is child of Main ConEmu window (why?)
//         3 -- same as 2, but ConEmu DC window - absent (old conemu version?)
//         4 -- same as 0, but ConEmu DC window - absent (old conemu version?)
HWND CheckConEmuChild(HWND ConEmuHwnd, int* pnConsoleIsChild/*=NULL*/)
{
	char className[100];
	HWND hParent=NULL;
	
	// ��������� �����
	if (pnConsoleIsChild) *pnConsoleIsChild = -1;
	if (!ConEmuHwnd) return NULL;

	
	GetClassNameA(ConEmuHwnd, className, 100);
	if (lstrcmpiA(className, VirtualConsoleClass) != 0 &&
		lstrcmpiA(className, VirtualConsoleClassMain) != 0)
	{   // �������� ������� - �� ������
		ConEmuHwnd = NULL; // ����� ���������� ��� ������
	} else {
		//bWasSetParent = TRUE;
		if (pnConsoleIsChild) *pnConsoleIsChild = 2; // init
		// ��������� �������� �������� �������
		hParent = GetAncestor(ConEmuHwnd, GA_PARENT);
		if (hParent) {
		    // ������ ���� �����
			GetClassNameA(hParent, className, 100);
			if (lstrcmpiA(className, VirtualConsoleClassMain) == 0) {
				if (pnConsoleIsChild) *pnConsoleIsChild = 1; // console is child of ConEmu DC window
			} else {
				hParent = NULL; // ���������, ��� ������ ������������, ������ ������� � �������� ���� ConEmu
			}
		}
		if (!hParent) {
			// 2 -- console is child of Main ConEmu window (why?)
			if (pnConsoleIsChild) *pnConsoleIsChild = 2;
			hParent = ConEmuHwnd;
			ConEmuHwnd = FindWindowExA(hParent, NULL, VirtualConsoleClass, NULL);
			if (ConEmuHwnd==NULL) {
			    // same as 2, but ConEmu DC windows - absent (old conemu version?)
				if (pnConsoleIsChild) *pnConsoleIsChild = 3;
				ConEmuHwnd = hParent;
			}
		}
	}
	
	return ConEmuHwnd;
}

// Returns HWND of Gui console DC window
//     pnConsoleIsChild [out]
//        -1 -- Non ConEmu mode
//         0 -- console is standalone window
//         1 -- console is child of ConEmu DC window
//         2 -- console is child of Main ConEmu window (why?)
//         3 -- same as 2, but ConEmu DC window - absent (old conemu version?)
//         4 -- same as 0, but ConEmu DC window - absent (old conemu version?)
HWND GetConEmuHWND(BOOL abRoot, int* pnConsoleIsChild/*=NULL*/)
{
	HWND FarHwnd=NULL, ConEmuHwnd=NULL, ConEmuRoot=NULL;
	FGetConsoleWindow fGetConsoleWindow = NULL;
	int nChk = -1;
	
	fGetConsoleWindow = (FGetConsoleWindow)GetProcAddress( GetModuleHandleA("kernel32.dll"), "GetConsoleWindow" );

	if (fGetConsoleWindow)
		FarHwnd = fGetConsoleWindow();
	// ��������� �����
	if (pnConsoleIsChild) *pnConsoleIsChild = nChk;

	// ���� ������� �������� ����� ���� ������� - ��������� ��� ��������
	if (FarHwnd)
		ConEmuHwnd = GetAncestor(FarHwnd, GA_PARENT);
	if (ConEmuHwnd != NULL)
	{
		// ������������, ���� ��� ������ SetParent - �� � �������� ���� � ����������, �� ��� ��� �����. ��������
		ConEmuHwnd = CheckConEmuChild(ConEmuHwnd, &nChk);
	}


	if (!ConEmuHwnd) {
		char className[100];
		if (GetEnvironmentVariableA("ConEmuHWND", className, 99)) {
			if (className[0]==L'0' && className[1]==L'x') {
				// �������� ����� ���� ConEmu �� ���������� �����
				ConEmuHwnd = AtoH(className+2, 8);
				
				if (ConEmuHwnd) {
					
					GetClassNameA(ConEmuHwnd, className, 100);
					if (lstrcmpiA(className, VirtualConsoleClass) != 0)
					{
						ConEmuHwnd = NULL;
					} else {
						// ����� ���������, ��� ������� ����������� �������� ConEmu
						DWORD dwEmuPID = 0;
						GetWindowThreadProcessId(ConEmuHwnd, &dwEmuPID);
						if (dwEmuPID == 0) {
							// ������, �� ������� �������� ��� �������� ConEmu, ������� ���������� �����
						} else {
							DWORD *ProcList = (DWORD*)calloc(1000,sizeof(DWORD));
							DWORD dwCount = 0;
							FGetConsoleProcessList fGetConsoleProcessList = 
								(FGetConsoleProcessList)GetProcAddress(
									GetModuleHandleA("kernel32.dll"), "GetConsoleProcessList");
	                        if (!fGetConsoleProcessList)
		                        dwCount = 1001;
		                    else
                                dwCount = fGetConsoleProcessList(ProcList,1000);
                                
							if(dwCount>1000) {
								// ������, � ������� ������� ����� ���������, ������� ���������� �����
							} else {
								DWORD dw=0;
								for (dw=0; dw<dwCount; dw++) {
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
				// ��������� "������������" ������
				ConEmuHwnd = CheckConEmuChild(ConEmuHwnd, &nChk);
				// ���� �� ����� ����, ������ ������� �� �������� ����, � ���������� ����� 4
				if (nChk==3) nChk=4;
				else if (nChk==1 || nChk==2) nChk=0;
			}
		}
	}

	if (!ConEmuHwnd && FarHwnd) {
		while((ConEmuHwnd = FindWindowExA(NULL, ConEmuHwnd, VirtualConsoleClassMain, NULL))!=NULL)
		{
			HWND hCheck = (HWND)GetWindowLong(ConEmuHwnd, GWL_USERDATA);
			if (hCheck == FarHwnd) {
				// ����� ������ ConEmu
				// ��������� "������������" ������
				ConEmuHwnd = CheckConEmuChild(ConEmuHwnd, &nChk);
				// ���� �� ����� ����, ������ ������� �� �������� ����, � ���������� ����� 4
				if (nChk==3) nChk=4;
				else if (nChk==1 || nChk==2) nChk=0;
				break;
			}
		}
		
	}

    if (abRoot && ConEmuHwnd && (nChk>=3))
	    ConEmuRoot = GetAncestor(ConEmuHwnd, GA_PARENT);
	
	if (pnConsoleIsChild) *pnConsoleIsChild = nChk;
	
	if (abRoot)
		return ConEmuRoot;
	return ConEmuHwnd;
}

HWND AtoH(char *Str, int Len)
{
  DWORD Ret=0;
  for (; Len && *Str; Len--, Str++)
  {
    if (*Str>='0' && *Str<='9')
      Ret = (Ret<<4)+(*Str-'0');
    else if (*Str>='a' && *Str<='f')
      Ret = (Ret<<4)+(*Str-'a'+10);
    else if (*Str>='A' && *Str<='F')
      Ret = (Ret<<4)+(*Str-'A'+10);
  }
  return (HWND)Ret;
}
