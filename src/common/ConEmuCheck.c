
#include <windows.h>
#define _T(s) s
#include "ConEmuCheck.h"
#include "common.hpp"

typedef HWND (APIENTRY *FGetConsoleWindow)();
typedef DWORD (APIENTRY *FGetConsoleProcessList)(LPDWORD,DWORD);

WARNING("��� '�������' �������� ����� ������������ 'CallNamedPipe', ��� ���� ����� �������� �������� ������ ����");


//#if defined(__GNUC__)
//extern "C" {
//#endif
//	WINBASEAPI DWORD GetEnvironmentVariableW(LPCWSTR lpName,LPWSTR lpBuffer,DWORD nSize);
//#if defined(__GNUC__)
//};
//#endif

// Returns HWND of Gui console DC window
HWND GetConEmuHWND(BOOL abRoot)
{
	HWND FarHwnd=NULL, ConEmuHwnd=NULL, ConEmuRoot=NULL;
	FGetConsoleWindow fGetConsoleWindow = NULL;
	wchar_t szGuiPipeName[MAX_PATH];
	BOOL lbRc = FALSE;
	HANDLE hPipe = NULL; 
	CESERVER_REQ in = {0};
	BYTE cbReadBuf[64];
	BOOL fSuccess = FALSE;
	DWORD cbRead = 0, dwMode = 0, dwErr = 0;
	
	
	
	fGetConsoleWindow = (FGetConsoleWindow)GetProcAddress( GetModuleHandleA("kernel32.dll"), "GetConsoleWindow" );
	if (!fGetConsoleWindow) return NULL;
		
	FarHwnd = fGetConsoleWindow();
	if (!FarHwnd) return NULL;
	
	wsprintfW(szGuiPipeName, CEGUIPIPENAME, L".", (DWORD)FarHwnd);
	//

	// Try to open a named pipe; wait for it, if necessary. 
	while (1) 
	{ 
	  hPipe = CreateFile( 
		 szGuiPipeName,  // pipe name 
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
		return NULL;
	  }

	  // All pipe instances are busy, so wait for 1 second.
	  if (!WaitNamedPipe(szGuiPipeName, 1000) ) 
	  {
		return NULL;
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
	  return NULL;
	}

	in.nSize = 12;
	in.nVersion = CESERVER_REQ_VER;
	in.nCmd  = CECMD_GETGUIHWND;

	// Send a message to the pipe server and read the response. 
	fSuccess = TransactNamedPipe( 
	  hPipe,                  // pipe handle 
	  &in,                    // message to server
	  in.nSize,               // message length 
	  cbReadBuf,              // buffer to receive reply
	  sizeof(cbReadBuf),      // size of read buffer
	  &cbRead,                // bytes read
	  NULL);                  // not overlapped 

	dwErr = GetLastError();
	CloseHandle(hPipe);
	
	if (!fSuccess /*&& (dwErr != ERROR_MORE_DATA)*/)
	{
	  return NULL;
	}
	
	if (cbRead < (5*sizeof(DWORD)))
		return NULL;

	if (((DWORD*)cbReadBuf)[0] != (5*sizeof(DWORD)) ||
	    ((DWORD*)cbReadBuf)[1] != in.nCmd ||
	    ((DWORD*)cbReadBuf)[2] != CESERVER_REQ_VER)
		return NULL;

	ConEmuRoot = (HWND)(((DWORD*)cbReadBuf)[3]);
	ConEmuHwnd = (HWND)(((DWORD*)cbReadBuf)[4]);
	
	return (abRoot) ? ConEmuRoot : ConEmuHwnd;
}


// 0 -- All OK (ConEmu found, Version OK)
// 1 -- NO ConEmu (simple console mode)
// (obsolete) 2 -- ConEmu found, but old version
int ConEmuCheck(HWND* ahConEmuWnd)
{
    //int nChk = -1;
    HWND ConEmuWnd = NULL;
    
    ConEmuWnd = GetConEmuHWND(FALSE/*abRoot*/  /*, &nChk*/);

    // ���� ������ ������ ����� - ���������� ���
    if (ahConEmuWnd) *ahConEmuWnd = ConEmuWnd;
    
    if (ConEmuWnd == NULL) {
	    return 1; // NO ConEmu (simple console mode)
    } else {
	    //if (nChk>=3)
		//    return 2; // ConEmu found, but old version
		return 0;
    }
}


//// Returns HWND of Gui console DC window
////     pnConsoleIsChild [out]
////        -1 -- Non ConEmu mode
////         0 -- console is standalone window
////         1 -- console is child of ConEmu DC window
////         2 -- console is child of Main ConEmu window (why?)
////         3 -- same as 2, but ConEmu DC window - absent (old conemu version?)
////         4 -- same as 0, but ConEmu DC window - absent (old conemu version?)
//HWND CheckConEmuChild(HWND ConEmuHwnd, int* pnConsoleIsChild/*=NULL*/)
//{
//	char className[100];
//	HWND hParent=NULL;
//	
//	// ��������� �����
//	if (pnConsoleIsChild) *pnConsoleIsChild = -1;
//	if (!ConEmuHwnd) return NULL;
//
//	
//	GetClassNameA(ConEmuHwnd, className, 100);
//	if (lstrcmpiA(className, VirtualConsoleClass) != 0 &&
//		lstrcmpiA(className, VirtualConsoleClassMain) != 0)
//	{   // �������� ������� - �� ������
//		ConEmuHwnd = NULL; // ����� ���������� ��� ������
//	} else {
//		//bWasSetParent = TRUE;
//		if (pnConsoleIsChild) *pnConsoleIsChild = 2; // init
//		// ��������� �������� �������� �������
//		hParent = GetAncestor(ConEmuHwnd, GA_PARENT);
//		if (hParent) {
//		    // ������ ���� �����
//			GetClassNameA(hParent, className, 100);
//			if (lstrcmpiA(className, VirtualConsoleClassMain) == 0) {
//				if (pnConsoleIsChild) *pnConsoleIsChild = 1; // console is child of ConEmu DC window
//			} else {
//				hParent = NULL; // ���������, ��� ������ ������������, ������ ������� � �������� ���� ConEmu
//			}
//		}
//		if (!hParent) {
//			// 2 -- console is child of Main ConEmu window (why?)
//			if (pnConsoleIsChild) *pnConsoleIsChild = 2;
//			hParent = ConEmuHwnd;
//			ConEmuHwnd = FindWindowExA(hParent, NULL, VirtualConsoleClass, NULL);
//			if (ConEmuHwnd==NULL) {
//			    // same as 2, but ConEmu DC windows - absent (old conemu version?)
//				if (pnConsoleIsChild) *pnConsoleIsChild = 3;
//				ConEmuHwnd = hParent;
//			}
//		}
//	}
//	
//	return ConEmuHwnd;
//}
//
//// Returns HWND of Gui console DC window
////     pnConsoleIsChild [out]
////        -1 -- Non ConEmu mode
////         0 -- console is standalone window
////         1 -- console is child of ConEmu DC window
////         2 -- console is child of Main ConEmu window (why?)
////         3 -- same as 2, but ConEmu DC window - absent (old conemu version?)
////         4 -- same as 0, but ConEmu DC window - absent (old conemu version?)
//HWND GetConEmuHWND(BOOL abRoot, int* pnConsoleIsChild/*=NULL*/)
//{
//	HWND FarHwnd=NULL, ConEmuHwnd=NULL, ConEmuRoot=NULL;
//	FGetConsoleWindow fGetConsoleWindow = NULL;
//	int nChk = -1;
//	
//	fGetConsoleWindow = (FGetConsoleWindow)GetProcAddress( GetModuleHandleA("kernel32.dll"), "GetConsoleWindow" );
//
//	if (fGetConsoleWindow)
//		FarHwnd = fGetConsoleWindow();
//	// ��������� �����
//	if (pnConsoleIsChild) *pnConsoleIsChild = nChk;
//
//	// ���� ������� �������� ����� ���� ������� - ��������� ��� ��������
//	if (FarHwnd)
//		ConEmuHwnd = GetAncestor(FarHwnd, GA_PARENT);
//	if (ConEmuHwnd == (HWND)0x10014)
//		ConEmuHwnd = NULL; else
//	if (ConEmuHwnd != NULL)
//	{
//		// ������������, ���� ��� ������ SetParent - �� � �������� ���� � ����������, �� ��� ��� �����. ��������
//		ConEmuHwnd = CheckConEmuChild(ConEmuHwnd, &nChk);
//	}
//
//
//#ifndef CONMANSERVER
//	if (!ConEmuHwnd) {
//		char className[100];
//		if (GetEnvironmentVariableA("ConEmuHWND", className, 99)) {
//			if (className[0]==L'0' && className[1]==L'x') {
//				// �������� ����� ���� ConEmu �� ���������� �����
//				ConEmuHwnd = AtoH(className+2, 8);
//				
//				if (ConEmuHwnd) {
//					
//					GetClassNameA(ConEmuHwnd, className, 100);
//					if (lstrcmpiA(className, VirtualConsoleClass) != 0)
//					{
//						ConEmuHwnd = NULL;
//					} else {
//						// ����� ���������, ��� ������� ����������� �������� ConEmu
//						DWORD dwEmuPID = 0;
//						GetWindowThreadProcessId(ConEmuHwnd, &dwEmuPID);
//						if (dwEmuPID == 0) {
//							// ������, �� ������� �������� ��� �������� ConEmu, ������� ���������� �����
//						} else {
//							DWORD *ProcList = (DWORD*)calloc(1000,sizeof(DWORD));
//							DWORD dwCount = 0;
//							FGetConsoleProcessList fGetConsoleProcessList = 
//								(FGetConsoleProcessList)GetProcAddress(
//									GetModuleHandleA("kernel32.dll"), "GetConsoleProcessList");
//	                        if (!fGetConsoleProcessList)
//		                        dwCount = 1001;
//		                    else
//                                dwCount = fGetConsoleProcessList(ProcList,1000);
//                                
//							if(dwCount>1000) {
//								// ������, � ������� ������� ����� ���������, ������� ���������� �����
//							} else {
//								DWORD dw=0;
//								for (dw=0; dw<dwCount; dw++) {
//									if (dwEmuPID == ProcList[dw]) {
//										dwEmuPID = 0; break;
//									}
//								}
//								if (dwEmuPID)
//									ConEmuHwnd = NULL; // ������� �� ����������� ConEmu
//							}
//							free(ProcList);
//						}
//					}
//				}
//				// ��������� "������������" ������
//				ConEmuHwnd = CheckConEmuChild(ConEmuHwnd, &nChk);
//				// ���� �� ����� ����, ������ ������� �� �������� ����, � ���������� ����� 4
//				if (nChk==3) nChk=4;
//				else if (nChk==1 || nChk==2) nChk=0;
//			}
//		}
//	}
//#endif
//
//	if (!ConEmuHwnd && FarHwnd) {
//		while((ConEmuHwnd = FindWindowExA(NULL, ConEmuHwnd, VirtualConsoleClassMain, NULL))!=NULL)
//		{
//			HWND hCheck = (HWND)GetWindowLong(ConEmuHwnd, GWL_USERDATA);
//			if (hCheck == FarHwnd) {
//				// ����� ������ ConEmu
//				// ��������� "������������" ������
//				ConEmuHwnd = CheckConEmuChild(ConEmuHwnd, &nChk);
//				// ���� �� ����� ����, ������ ������� �� �������� ����, � ���������� ����� 4
//				if (nChk==3) nChk=4;
//				else if (nChk==1 || nChk==2) nChk=0;
//				break;
//			}
//		}
//		
//	}
//
//    if (abRoot && ConEmuHwnd && (nChk<3))
//	    ConEmuRoot = GetAncestor(ConEmuHwnd, GA_PARENT);
//	
//	if (pnConsoleIsChild) *pnConsoleIsChild = nChk;
//	
//	if (abRoot)
//		return ConEmuRoot;
//	return ConEmuHwnd;
//}

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
