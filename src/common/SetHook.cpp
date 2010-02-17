
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

#define DROP_SETCP_ON_WIN2K3R2

// ����� �� ����������� GetConsoleAliases (���� �� ������ ���� �������� � Win2k)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include "..\common\common.hpp"
#include "..\common\ConEmuCheck.h"
#include <Tlhelp32.h>
#include "SetHook.h"

#define DebugString(x) // OutputDebugString(x)

static HMODULE hOurModule = NULL; // ����� ����� dll'�� (����� ���� �� ��������)
static DWORD   nMainThreadId = 0;

extern CESERVER_REQ_CONINFO_HDR *gpConsoleInfo;
extern HWND ConEmuHwnd; // �������� ����� ���� ���������. ��� �������� ����.
extern HWND FarHwnd;
extern BOOL gbFARuseASCIIsort;
extern DWORD gdwServerPID;

static const wchar_t kernel32[] = L"kernel32.dll";
static const wchar_t user32[]   = L"user32.dll";
static const wchar_t shell32[]  = L"shell32.dll";

// Forward declarations of the hooks
static HMODULE WINAPI OnLoadLibraryW( const WCHAR* lpFileName );
static HMODULE WINAPI OnLoadLibraryA( const char* lpFileName );
static HMODULE WINAPI OnLoadLibraryExW( const WCHAR* lpFileName, HANDLE hFile, DWORD dwFlags );
static HMODULE WINAPI OnLoadLibraryExA( const char* lpFileName, HANDLE hFile, DWORD dwFlags );
static FARPROC WINAPI OnGetProcAddress( HMODULE hModule, LPCSTR lpProcName );
static BOOL WINAPI OnTrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y, int nReserved, HWND hWnd, CONST RECT * prcRect);
static BOOL WINAPI OnTrackPopupMenuEx(HMENU hmenu, UINT fuFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm);
static BOOL WINAPI OnShellExecuteExA(LPSHELLEXECUTEINFOA lpExecInfo);
static BOOL WINAPI OnShellExecuteExW(LPSHELLEXECUTEINFOW lpExecInfo);
static HINSTANCE WINAPI OnShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);
static HINSTANCE WINAPI OnShellExecuteW(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd);
static BOOL WINAPI OnFlashWindow(HWND hWnd, BOOL bInvert);
static BOOL WINAPI OnFlashWindowEx(PFLASHWINFO pfwi);
static BOOL WINAPI OnSetForegroundWindow(HWND hWnd);
int WINAPI OnCompareStringW(LCID Locale, DWORD dwCmpFlags, LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2, int cchCount2);
static DWORD WINAPI OnGetConsoleAliasesW(LPWSTR AliasBuffer, DWORD AliasBufferLength, LPWSTR ExeName);
static BOOL WINAPI OnGetNumberOfConsoleInputEvents(HANDLE hConsoleInput, LPDWORD lpcNumberOfEvents);
static BOOL WINAPI OnPeekConsoleInputA(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
static BOOL WINAPI OnPeekConsoleInputW(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
static BOOL WINAPI OnReadConsoleInputA(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
static BOOL WINAPI OnReadConsoleInputW(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
static HANDLE WINAPI OnCreateConsoleScreenBuffer(DWORD dwDesiredAccess, DWORD dwShareMode, const SECURITY_ATTRIBUTES *lpSecurityAttributes, DWORD dwFlags, LPVOID lpScreenBufferData);
static BOOL WINAPI OnAllocConsole(void);
static BOOL WINAPI OnFreeConsole(void);
static HWND WINAPI OnGetConsoleWindow(void); // � ���� ������ � ������ ������� ���� �������
static BOOL WINAPI OnWriteConsoleOutputA(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
static BOOL WINAPI OnWriteConsoleOutputW(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);



#define MAX_HOOKED_PROCS 50
static HookItem Hooks[MAX_HOOKED_PROCS] = {
	/* ***** MOST CALLED ***** */
//	{(void*)OnGetConsoleWindow,     "GetConsoleWindow",     kernel32}, -- ���� ������ ���. ������� ��� �� �� ������ ��������
	{(void*)OnWriteConsoleOutputW,  "WriteConsoleOutputW",  kernel32},
	{(void*)OnWriteConsoleOutputA,  "WriteConsoleOutputA",  kernel32},
	/* ************************ */
    {(void*)OnLoadLibraryA,			"LoadLibraryA",			kernel32},
	{(void*)OnLoadLibraryW,			"LoadLibraryW",			kernel32},
    {(void*)OnLoadLibraryExA,		"LoadLibraryExA",		kernel32},
	{(void*)OnLoadLibraryExW,		"LoadLibraryExW",		kernel32},
	{(void*)OnGetProcAddress,		"GetProcAddress",		kernel32},
	/* ************************ */
	{(void*)OnPeekConsoleInputW,	"PeekConsoleInputW",	kernel32},
	{(void*)OnPeekConsoleInputA,	"PeekConsoleInputA",	kernel32},
	{(void*)OnReadConsoleInputW,	"ReadConsoleInputW",	kernel32},
	{(void*)OnReadConsoleInputA,	"ReadConsoleInputA",	kernel32},
	{(void*)OnGetConsoleAliasesW,	"GetConsoleAliasesW",	kernel32},
	{(void*)OnAllocConsole,			"AllocConsole",			kernel32},
	{(void*)OnFreeConsole,			"FreeConsole",			kernel32},
	{(void*)OnGetNumberOfConsoleInputEvents,
									"GetNumberOfConsoleInputEvents",
															kernel32},
	{(void*)OnCreateConsoleScreenBuffer,
									"CreateConsoleScreenBuffer",
															kernel32},
	/* ************************ */
	{(void*)OnTrackPopupMenu,      "TrackPopupMenu",		user32},
	{(void*)OnTrackPopupMenuEx,    "TrackPopupMenuEx",		user32},
	{(void*)OnFlashWindow,         "FlashWindow",			user32},
	{(void*)OnFlashWindowEx,       "FlashWindowEx",			user32},
	{(void*)OnSetForegroundWindow, "SetForegroundWindow",	user32},
	/* ************************ */
	{(void*)OnShellExecuteExA,     "ShellExecuteExA",		shell32},
	{(void*)OnShellExecuteExW,     "ShellExecuteExW",		shell32},
	{(void*)OnShellExecuteA,       "ShellExecuteA",			shell32},
	{(void*)OnShellExecuteW,       "ShellExecuteW",			shell32},
	/* ************************ */
	{0}
};


#define MAX_EXCLUDED_MODULES 20
static const wchar_t* ExcludedModules[MAX_EXCLUDED_MODULES] = {
    L"kernel32.dll",
    // � user32.dll �� �����?
    0
};


// ������������ � ��� ������, ���� ��������� ��������� ������������ �������, ��� ����� �������
// ������ � OnPeekConsoleInputW
void* GetOriginalAddress( void* OurFunction, void* DefaultFunction, BOOL abAllowModified, HookItem** ph )
{
	for( int i = 0; Hooks[i].NewAddress; i++ ) {
		if ( Hooks[i].NewAddress == OurFunction) {
			*ph = &(Hooks[i]);
			return (abAllowModified && Hooks[i].ExeOldAddress) ? Hooks[i].ExeOldAddress : Hooks[i].OldAddress;
		}
	}
    _ASSERT(FALSE); // ���� �� �������� �� ������
    return DefaultFunction;
}

class CInFuncCall
{
public:
	int *mpn_Counter;
public:
	CInFuncCall() {
		mpn_Counter = NULL;
	}
	
	BOOL Inc(int* pnCounter) {
		BOOL lbFirstCall = FALSE;
		mpn_Counter = pnCounter;
		if (mpn_Counter) {
			lbFirstCall = (*mpn_Counter) == 0;
			(*mpn_Counter)++;
		}
		return lbFirstCall;
	}

	~CInFuncCall() {
		if (mpn_Counter && (*mpn_Counter)>0) (*mpn_Counter)--;
	}
};

#define ORIGINAL(n) \
	static HookItem *ph = NULL; \
	BOOL bMainThread = (GetCurrentThreadId() == nMainThreadId); \
	void* f##n = NULL; /* static ���� - ������� ����� ����������� �� ������� */ \
	static int nMainThCounter = 0; CInFuncCall CounterLocker; \
	BOOL bAllowModified = bMainThread; \
	if (bMainThread) bAllowModified = CounterLocker.Inc(&nMainThCounter); \
	if (bAllowModified) { \
		static void* f##n##Mod = NULL; \
		if ((f##n##Mod)==NULL) f##n##Mod = (void*)GetOriginalAddress((void*)(On##n) , (void*)n , TRUE, &ph); \
		f##n = f##n##Mod; \
	} else { \
		static void* f##n##Org = NULL; \
		if ((f##n##Org)==NULL) f##n##Org = (void*)GetOriginalAddress((void*)(On##n) , (void*)n , FALSE, &ph); \
		f##n = f##n##Org; \
	} \
	_ASSERTE((void*)(On##n)!=(void*)(f##n) && (void*)(f##n)!=NULL);
	
#define ORIGINALFAST(n) \
	static HookItem *ph = NULL; \
	static void* f##n = NULL; \
	if ((f##n)==NULL) f##n = (void*)GetOriginalAddress((void*)(On##n) , (void*)n , FALSE, &ph); \
	_ASSERTE((void*)(On##n)!=(void*)(f##n) && (void*)(f##n)!=NULL);

#define F(n) ((On##n##_t)f##n)


#define SETARGS(r) HookCallbackArg args = {bMainThread}; args.lpResult = (LPVOID)(r)
#define SETARGS1(r,a1) SETARGS(r); args.lArguments[0] = (DWORD_PTR)(a1)
#define SETARGS2(r,a1,a2) SETARGS1(r,a1); args.lArguments[1] = (DWORD_PTR)(a2)
#define SETARGS3(r,a1,a2,a3) SETARGS2(r,a1,a2); args.lArguments[2] = (DWORD_PTR)(a3)
#define SETARGS4(r,a1,a2,a3,a4) SETARGS3(r,a1,a2,a3); args.lArguments[3] = (DWORD_PTR)(a4)
#define SETARGS5(r,a1,a2,a3,a4,a5) SETARGS4(r,a1,a2,a3,a4); args.lArguments[4] = (DWORD_PTR)(a5)
#define SETARGS6(r,a1,a2,a3,a4,a5,a6) SETARGS5(r,a1,a2,a3,a4,a5); args.lArguments[5] = (DWORD_PTR)(a6)
#define SETARGS7(r,a1,a2,a3,a4,a5,a6,a7) SETARGS6(r,a1,a2,a3,a4,a5,a6); args.lArguments[6] = (DWORD_PTR)(a7)
	

// ��������� ���� HookItem.OldAddress (�������� ��������� �� ������� ���������)
// apHooks->Name && apHooks->DllName MUST be static for a lifetime
bool __stdcall InitHooks( HookItem* apHooks )
{
	int i, j;
	bool skip;
	
    if( apHooks )
    {
    	for (i = 0; apHooks[i].NewAddress; i++) {
    		if (apHooks[i].Name==NULL || apHooks[i].DllName==NULL) {
    			_ASSERTE(apHooks[i].Name!=NULL && apHooks[i].DllName!=NULL);
    			break;
    		}
    		skip = false;

    		for (j = 0; Hooks[j].NewAddress; j++) {
    			if (Hooks[j].NewAddress == apHooks[i].NewAddress) {
    				skip = true; break;
    			}
    		}
    		if (skip) continue;

    		j = 0; // using while, because of j
    		while (Hooks[j].NewAddress) {
    			if (!lstrcmpiA(Hooks[j].Name, apHooks[i].Name)
    				&& !lstrcmpiW(Hooks[j].DllName, apHooks[i].DllName))
				{
					Hooks[j].NewAddress = apHooks[i].NewAddress;
					skip = true; break;
				}
				j++;
    		}
    		if (skip) continue;
    		
    		if (j >= 0) {
    			if ((j+1) >= MAX_HOOKED_PROCS) {
    				// ��������� ���������� ����������
    				_ASSERTE((j+1) < MAX_HOOKED_PROCS);
    				continue; // ����� ����� ������ ���� ������� ��������, � �� ��������
    			}
    			Hooks[j].Name = apHooks[i].Name;
    			Hooks[j].DllName = apHooks[i].DllName;
				Hooks[j].NewAddress = apHooks[i].NewAddress;
				Hooks[j+1].Name = NULL; // �� ������
				Hooks[j+1].NewAddress = NULL; // �� ������
    		}
    	}
    }

    for (i = 0; Hooks[i].NewAddress; i++)
    {
        if( !Hooks[i].OldAddress )
        {
        	// ������ - �� ���������
            HMODULE mod = GetModuleHandle( Hooks[i].DllName );
            if( mod ) {
                Hooks[i].OldAddress = (void*)GetProcAddress( mod, Hooks[i].Name );
                Hooks[i].hDll = mod;
            }
        }
    }
    
    return true;
}

bool __stdcall SetHookCallbacks( const char* ProcName, const wchar_t* DllName,
								 HookItemPreCallback_t PreCallBack, HookItemPostCallback_t PostCallBack )
{
	_ASSERTE(ProcName!=NULL && DllName!=NULL);
	_ASSERTE(ProcName[0]!=0 && DllName[0]!=0);
	
	bool bFound = false;
	for (int i = 0; i<MAX_HOOKED_PROCS && Hooks[i].NewAddress; i++)
	{
		if (!lstrcmpA(Hooks[i].Name, ProcName) && !lstrcmpW(Hooks[i].DllName,DllName)) {
			Hooks[i].PreCallBack = PreCallBack;
			Hooks[i].PostCallBack = PostCallBack;
			bFound = true;
			break;
		}
	}
	return bFound;
}

static bool IsModuleExcluded( HMODULE module )
{
	if (module == hOurModule)
		return true;

    for( int i = 0; ExcludedModules[i]; i++ )
        if( module == GetModuleHandle( ExcludedModules[i] ) )
            return true;
	
    return false;
}


#define GetPtrFromRVA(rva,pNTHeader,imageBase) (PVOID)((imageBase)+(rva))

// ��������� ������������� ������� � ������
static bool SetHook( HMODULE Module, BOOL abExecutable )
{
    IMAGE_IMPORT_DESCRIPTOR* Import = 0;
    DWORD Size = 0;
    HMODULE hExecutable = GetModuleHandle( 0 );
    if( !Module )
        Module = hExecutable;
    BOOL bExecutable = (Module == hExecutable);
    IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)Module;
	IMAGE_NT_HEADERS* nt_header = NULL;
    if( dos_header->e_magic == IMAGE_DOS_SIGNATURE /*'ZM'*/ )
    {
        nt_header = (IMAGE_NT_HEADERS*)((char*)Module + dos_header->e_lfanew);
        if( nt_header->Signature != 0x004550 )
            return false;
        else
        {
            Import = (IMAGE_IMPORT_DESCRIPTOR*)((char*)Module +
                                         (DWORD)(nt_header->OptionalHeader.
                                         DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].
                                         VirtualAddress));
            Size = nt_header->OptionalHeader.
                                         DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
        }
    }
    else
        return false;

    // if wrong module or no import table
    if( Module == INVALID_HANDLE_VALUE || !Import )
        return false;

#ifdef _DEBUG
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);
#endif

	#ifdef _WIN64
	_ASSERTE(sizeof(DWORD_PTR)==8);
	#else
	_ASSERTE(sizeof(DWORD_PTR)==4);
	#endif

	#ifdef _WIN64
		#define TOP_SHIFT 60
	#else
		#define TOP_SHIFT 28
	#endif

	TODO("!!! ��������� ORDINAL �������� !!!");

    bool res = false, bHooked = false;
	int i;
	int nCount = Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
	//_ASSERTE(Size == (nCount * sizeof(IMAGE_IMPORT_DESCRIPTOR))); -- ����� ���� �� �������
    for( i = 0; i < nCount; i++ )
    {
		if (Import[i].Name == 0)
			break;
        //DebugString( ToTchar( (char*)Module + Import[i].Name ) );
		#ifdef _DEBUG
		char* mod_name = (char*)Module + Import[i].Name;
		#endif

		DWORD_PTR rvaINT = Import[i].OriginalFirstThunk;
		DWORD_PTR rvaIAT = Import[i].FirstThunk;
		if ( rvaINT == 0 )   // No Characteristics field?
		{
			// Yes! Gotta have a non-zero FirstThunk field then.
			rvaINT = rvaIAT;
			if ( rvaINT == 0 ) {  // No FirstThunk field?  Ooops!!!
				_ASSERTE(rvaINT!=0);
				break;
			}
		}

		//PIMAGE_IMPORT_BY_NAME pOrdinalName = NULL, pOrdinalNameO = NULL;
		PIMAGE_IMPORT_BY_NAME pOrdinalNameO = NULL;
		//IMAGE_IMPORT_BY_NAME** byname = (IMAGE_IMPORT_BY_NAME**)((char*)Module + rvaINT);
        //IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)((char*)Module + rvaIAT);
		IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)GetPtrFromRVA( rvaIAT, nt_header, (PBYTE)Module );
		IMAGE_THUNK_DATA* thunkO = (IMAGE_THUNK_DATA*)GetPtrFromRVA( rvaINT, nt_header, (PBYTE)Module );
		if (!thunk ||  !thunkO) {
			_ASSERTE(thunk && thunkO);
			continue;
		}

		int f = 0;
        for(f = 0 ; thunk->u1.Function; thunk++, thunkO++, f++)
        {
			const char* pszFuncName = NULL;
			ULONGLONG ordinalO = -1;

			if (thunk->u1.Function!=thunkO->u1.Function)
			{
				// Ordinal � ��� ���� �� ������������
				if ( IMAGE_SNAP_BY_ORDINAL(thunkO->u1.Ordinal) ) {
					ordinalO = IMAGE_ORDINAL(thunkO->u1.Ordinal);
					pOrdinalNameO = NULL;
				}
				TODO("�������� ����� ������ ��� ������� �� ������ ��� EXE, �� � ��� ���� dll");
				if (bExecutable) {
					if (!IMAGE_SNAP_BY_ORDINAL(thunkO->u1.Ordinal)) {
						pOrdinalNameO = (PIMAGE_IMPORT_BY_NAME)GetPtrFromRVA(thunkO->u1.AddressOfData, nt_header, (PBYTE)Module);
						BOOL lbValidPtr = !IsBadReadPtr(pOrdinalNameO, sizeof(IMAGE_IMPORT_BY_NAME));
						_ASSERTE(lbValidPtr);
						if (lbValidPtr) {
							lbValidPtr = !IsBadStringPtrA((LPCSTR)pOrdinalNameO->Name, 10);
							_ASSERTE(lbValidPtr);
							if (lbValidPtr)
								pszFuncName = (LPCSTR)pOrdinalNameO->Name;
						}
					}
				}
			}

			int j;
            for( j = 0; Hooks[j].Name; j++ )
			{
				if (Hooks[j].NewAddress == (void*)thunk->u1.Function) {
					res = true; // ��� ��� ��������
					break;
				}
			
				WARNING("??? �������� � ���� �������");
                if( !Hooks[j].OldAddress || (void*)thunk->u1.Function != Hooks[j].OldAddress )
                {
					if (!pszFuncName || !bExecutable) {
						continue;
					} else {
						if (strcmp(pszFuncName, Hooks[j].Name))
							continue;
					}
					// OldAddress ��� ����� ���������� �� ������������� �������� ����������
					// ��� ���������� �������� � PeekConsoleIntputW ��� ������� ������� Anamorphosis
					Hooks[j].ExeOldAddress = (void*)thunk->u1.Function;
				}
				if (Hooks[j].nOrdinal == 0 && ordinalO != -1)
					Hooks[j].nOrdinal = (DWORD)ordinalO;

				bHooked = true;
				DWORD old_protect = 0;
				VirtualProtect( &thunk->u1.Function, sizeof( thunk->u1.Function ),
					PAGE_READWRITE, &old_protect );
				thunk->u1.Function = (DWORD_PTR)Hooks[j].NewAddress;
				VirtualProtect( &thunk->u1.Function, sizeof( DWORD ), old_protect, &old_protect );
				#ifdef _DEBUG
				if (bExecutable)
					Hooks[j].ReplacedInExe = TRUE;
				#endif
				//DebugString( ToTchar( Hooks[j].Name ) );
				res = true;
				break;
			}
        }
    }

#ifdef _DEBUG
	if (bHooked) {
		wchar_t szDbg[MAX_PATH*3], szModPath[MAX_PATH*2]; szModPath[0] = 0;
		GetModuleFileName(Module, szModPath, MAX_PATH*2);
		lstrcpy(szDbg, L"  ## Hooks was set by conemu: ");
		lstrcat(szDbg, szModPath);
		lstrcat(szDbg, L"\n");
		OutputDebugStringW(szDbg);
	}
#endif

    return res;
}

// ��������� ������������� ������� �� ���� ������� ��������, ����������� �� conemu.dll
// *aszExcludedModules - ������ ��������� �� ����������� �������� (program lifetime)
bool __stdcall SetAllHooks( HMODULE ahOurDll, const wchar_t** aszExcludedModules /*= NULL*/ )
{
	// �.�. SetAllHooks ����� ���� ������ �� ������ dll - ���������� ����������
	if (!hOurModule) hOurModule = ahOurDll;
	
	InitHooks ( NULL );

#ifdef _DEBUG
	char szHookProc[128];
	for (int i = 0; Hooks[i].NewAddress; i++)
	{
		wsprintfA(szHookProc, "## %s -> 0x%08X (exe: 0x%X)\n", Hooks[i].Name, (DWORD)Hooks[i].NewAddress, (DWORD)Hooks[i].ExeOldAddress);
		OutputDebugStringA(szHookProc);
	}
#endif
	
	// ��������� aszExcludedModules
	if (aszExcludedModules) {
		int j;
		bool skip;
		for ( int i = 0; aszExcludedModules[i]; i++ ) {
			j = 0; skip = false;
			while (ExcludedModules[j]) {
				if (lstrcmpi(ExcludedModules[j], aszExcludedModules[i]) == 0) {
					skip = true; break;
				}
				j++;
			}
    		if (skip) continue;
    		
    		if (j > 0) {
    			if ((j+1) >= MAX_EXCLUDED_MODULES) {
    				// ��������� ���������� ����������
    				_ASSERTE((j+1) < MAX_EXCLUDED_MODULES);
    				continue;
    			}
    			ExcludedModules[j] = aszExcludedModules[i];
				ExcludedModules[j+1] = NULL; // �� ������
    		}
		}
	}
	
	// ��� ������������ ����� ����� ���� ������ �������������� inject-� (��������� � FAR)
    HMODULE hExecutable = GetModuleHandle( 0 );
    HANDLE snapshot;

    // ����� ID �������� ����
    if (!nMainThreadId) {
		DWORD dwPID = GetCurrentProcessId();
	    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwPID);
	    if(snapshot != INVALID_HANDLE_VALUE)
	    {
	    	THREADENTRY32 module = {sizeof(THREADENTRY32)};
			if (Thread32First(snapshot, &module)) {
				while (!nMainThreadId) {
					if (module.th32OwnerProcessID == dwPID) {
	    				nMainThreadId = module.th32ThreadID;
						break;
					}
					if (!Thread32Next(snapshot, &module))
						break;
				}
			}
	    	CloseHandle(snapshot);
		}
	}
	_ASSERTE(nMainThreadId!=0);

	// �������� ������ �� ���� ����������� (static linked) �������
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
    if(snapshot != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 module = {sizeof(MODULEENTRY32)};

        for(BOOL res = Module32First(snapshot, &module); res; res = Module32Next(snapshot, &module))
        {
            if(module.hModule && !IsModuleExcluded(module.hModule))
            {
                DebugString( module.szModule );
                SetHook( module.hModule, (module.hModule == hExecutable) );
            }
        }

        CloseHandle(snapshot);
    }

    return true;
}




// ��������� ������������� ������� � ������
static bool UnsetHook( HMODULE Module, BOOL abExecutable )
{
    IMAGE_IMPORT_DESCRIPTOR* Import = 0;
    DWORD Size = 0;

    _ASSERTE(Module!=NULL); 
    
    IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)Module;
    if( dos_header->e_magic == IMAGE_DOS_SIGNATURE /*'ZM'*/ )
    {
        IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)((char*)Module + dos_header->e_lfanew);
        if( nt_header->Signature != 0x004550 )
            return false;
        else
        {
            Import = (IMAGE_IMPORT_DESCRIPTOR*)((char*)Module +
                                         (DWORD)(nt_header->OptionalHeader.
                                         DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].
                                         VirtualAddress));
            Size = nt_header->OptionalHeader.
                                         DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
        }
    }
    else
        return false;

    // if wrong module or no import table
    if( Module == INVALID_HANDLE_VALUE || !Import )
        return false;

    bool res = false, bUnhooked = false;
	int i;
	int nCount = Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
	//_ASSERTE(Size == (nCount * sizeof(IMAGE_IMPORT_DESCRIPTOR))); -- ����� ���� �� �������
	for( i = 0; i < nCount; i++ )
	{
		if (Import[i].Name == 0)
			break;

		#ifdef _DEBUG
		char* mod_name = (char*)Module + Import[i].Name;
		#endif

		DWORD_PTR rvaINT = Import[i].OriginalFirstThunk;
		DWORD_PTR rvaIAT = Import[i].FirstThunk;
		if ( rvaINT == 0 )   // No Characteristics field?
		{
			// Yes! Gotta have a non-zero FirstThunk field then.
			rvaINT = rvaIAT;
			if ( rvaINT == 0 ) {  // No FirstThunk field?  Ooops!!!
				_ASSERTE(rvaINT!=0);
				break;
			}
		}

		//PIMAGE_IMPORT_BY_NAME pOrdinalName = NULL, pOrdinalNameO = NULL;
		PIMAGE_IMPORT_BY_NAME pOrdinalNameO = NULL;
		//IMAGE_IMPORT_BY_NAME** byname = (IMAGE_IMPORT_BY_NAME**)((char*)Module + rvaINT);
		//IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)((char*)Module + rvaIAT);
		IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)GetPtrFromRVA( rvaIAT, nt_header, (PBYTE)Module );
		IMAGE_THUNK_DATA* thunkO = (IMAGE_THUNK_DATA*)GetPtrFromRVA( rvaINT, nt_header, (PBYTE)Module );
		if (!thunk ||  !thunkO) {
			_ASSERTE(thunk && thunkO);
			continue;
		}

		int f = 0;
		for(f = 0 ; thunk->u1.Function; thunk++, thunkO++, f++)
		{
			const char* pszFuncName = NULL;
			//ULONGLONG ordinalO = -1;

			//if ( IMAGE_SNAP_BY_ORDINAL(thunkO->u1.Ordinal) ) {
			//	ordinalO = IMAGE_ORDINAL(thunkO->u1.Ordinal);
			//	pOrdinalNameO = NULL;
			//}
			if (thunk->u1.Function!=thunkO->u1.Function && !IMAGE_SNAP_BY_ORDINAL(thunkO->u1.Ordinal)) {
				pOrdinalNameO = (PIMAGE_IMPORT_BY_NAME)GetPtrFromRVA(thunkO->u1.AddressOfData, nt_header, (PBYTE)Module);
				BOOL lbValidPtr = !IsBadReadPtr(pOrdinalNameO, sizeof(IMAGE_IMPORT_BY_NAME));
				_ASSERTE(lbValidPtr);
				if (lbValidPtr) {
					lbValidPtr = !IsBadStringPtrA((LPCSTR)pOrdinalNameO->Name, 10);
					_ASSERTE(lbValidPtr);
					if (lbValidPtr)
						pszFuncName = (LPCSTR)pOrdinalNameO->Name;
				}
			}

			int j;
            for( j = 0; Hooks[j].Name; j++ )
            {
				if( !Hooks[j].OldAddress )
					continue; // ��� ������� �� ������������ (���� ������ ����?)

				// ����� ����� ������� (thunk) � Hooks ����� NewAddress ��� ���
				if ((void*)thunk->u1.Function != Hooks[j].NewAddress )
				{
					if (!pszFuncName) {
						continue;
					} else {
						if (strcmp(pszFuncName, Hooks[j].Name))
							continue;
					}
					// OldAddress ��� ����� ���������� �� ������������� �������� ����������
					// ��� ���� ������� �������� ��� ����� ���
				}

				// ���� �� ����� ���� - ������ ������� ������� (��� �� ������ ��� �� �����)
            	// BugBug: � ��������, ��� ������� ��� �������� � ������ ������ (��� ����� ���),
				// �� ����� ������� ������������, ��� ����� ���������
                DWORD old_protect;
				bUnhooked = true;
                VirtualProtect( &thunk->u1.Function, sizeof( thunk->u1.Function ),
                                PAGE_READWRITE, &old_protect );
				// BugBug: ExeOldAddress ����� ���������� �� �������������, ���� ������� ���� ����������� �� ���
				//if (abExecutable && Hooks[j].ExeOldAddress)
				//	thunk->u1.Function = (DWORD_PTR)Hooks[j].ExeOldAddress;
				//else
					thunk->u1.Function = (DWORD_PTR)Hooks[j].OldAddress;
                VirtualProtect( &thunk->u1.Function, sizeof( DWORD ), old_protect, &old_protect );
                //DebugString( ToTchar( Hooks[j].Name ) );
                res = true;
                break; // ������� � ���������� thunk-�
            }
        }
    }

#ifdef _DEBUG
	if (bUnhooked) {
		wchar_t szDbg[MAX_PATH*3], szModPath[MAX_PATH*2]; szModPath[0] = 0;
		GetModuleFileName(Module, szModPath, MAX_PATH*2);
		lstrcpy(szDbg, L"  ## Hooks was UNset by conemu: ");
		lstrcat(szDbg, szModPath);
		lstrcat(szDbg, L"\n");
		OutputDebugStringW(szDbg);
	}
#endif

    return res;
}

void __stdcall UnsetAllHooks( )
{
    HMODULE hExecutable = GetModuleHandle( 0 );

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
    if(snapshot != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 module = {sizeof(module)};

        for(BOOL res = Module32First(snapshot, &module); res; res = Module32Next(snapshot, &module))
        {
            if(module.hModule && !IsModuleExcluded(module.hModule))
            {
                DebugString( module.szModule );
                UnsetHook( module.hModule, (module.hModule == hExecutable) );
            }
        }

        CloseHandle(snapshot);
    }

#ifdef _DEBUG
	hExecutable = hExecutable;
#endif
}

static void TouchReadPeekConsoleInputs(int Peek = -1)
{
	if (!gpConsoleInfo) {
		_ASSERTE(gpConsoleInfo);
		return;
	}
	if (GetCurrentThreadId() != nMainThreadId)
		return;
		
	gpConsoleInfo->nFarReadIdx++;

#ifdef _DEBUG
	if (Peek == -1)
		return;
	if ((GetKeyState(VK_SCROLL)&1) == 0)
		return;
	static DWORD nLastTick;
	DWORD nCurTick = GetTickCount();
	DWORD nDelta = nCurTick - nLastTick;
	static CONSOLE_SCREEN_BUFFER_INFO sbi;
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (nDelta > 1000) {
		GetConsoleScreenBufferInfo(hOut, &sbi);
		nCurTick = nCurTick;
	}
	static wchar_t Chars[] = L"-\\|/-\\|/";
	int nNextChar = 0;
	if (Peek) {
		static int nPeekChar = 0;
		nNextChar = nPeekChar++;
		if (nPeekChar >= 8) nPeekChar = 0;
	} else {
		static int nReadChar = 0;
		nNextChar = nReadChar++;
		if (nReadChar >= 8) nReadChar = 0;
	}
	CHAR_INFO chi;
	chi.Char.UnicodeChar = Chars[nNextChar];
	chi.Attributes = 15;
	COORD crBufSize = {1,1};
	COORD crBufCoord = {0,0};
	SMALL_RECT rc = {sbi.srWindow.Left+(Peek?0:1),sbi.srWindow.Bottom,sbi.srWindow.Left+(Peek?0:1),sbi.srWindow.Bottom};
	WriteConsoleOutputW(hOut, &chi, crBufSize, crBufCoord, &rc);
#endif
}

static void GuiSetForeground(HWND hWnd)
{
	if (ConEmuHwnd) {
		CESERVER_REQ In, *pOut;
		ExecutePrepareCmd(&In, CECMD_SETFOREGROUND, sizeof(CESERVER_REQ_HDR)+sizeof(u64));
		In.qwData[0] = (u64)hWnd;
		HWND hConWnd = GetConsoleWindow();
		pOut = ExecuteGuiCmd(hConWnd, &In, hConWnd);
		if (pOut) ExecuteFreeResult(pOut);
	}
}

static void GuiFlashWindow(BOOL bSimple, HWND hWnd, BOOL bInvert, DWORD dwFlags, UINT uCount, DWORD dwTimeout)
{
	if (ConEmuHwnd) {
		CESERVER_REQ In, *pOut;
		ExecutePrepareCmd(&In, CECMD_FLASHWINDOW, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_FLASHWINFO));
		In.Flash.bSimple = bSimple;
		In.Flash.hWnd = hWnd;
		In.Flash.bInvert = bInvert;
		In.Flash.dwFlags = dwFlags;
		In.Flash.uCount = uCount;
		In.Flash.dwTimeout = dwTimeout;
		HWND hConWnd = GetConsoleWindow();
		pOut = ExecuteGuiCmd(hConWnd, &In, hConWnd);
		if (pOut) ExecuteFreeResult(pOut);
	}
}

static bool IsHandleConsole( HANDLE handle, bool output = true )
{
	// �������?
    if( ( (DWORD)handle & 0x10000003) != 3 )
        return false;

    // �������� ���� ����������� ������ (In/Out)
    DWORD num;
    if( !output )
        if( GetNumberOfConsoleInputEvents( handle, &num ) )
            return true;
        else
            return false;
    else
        if( GetNumberOfConsoleInputEvents( handle, &num ) )
            return false;
        else
            return true;
}


//static BOOL SrvSetConsoleCP(BOOL bSetOutputCP, DWORD nCP)
//{
//	_ASSERTE(ConEmuHwnd);
//
//	BOOL lbRc = FALSE;
//
//	if (gdwServerPID) {
//		// ��������� ������� ��������
//		HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, gdwServerPID);
//		if (hProcess) {
//			if (WaitForSingleObject(hProcess,0) == WAIT_OBJECT_0)
//				gdwServerPID = 0; // ������� ������� ����������
//			CloseHandle(hProcess);
//		} else {
//			gdwServerPID = 0;
//		}
//	}
//
//	if (gdwServerPID) {
//#ifndef DROP_SETCP_ON_WIN2K3R2
//		CESERVER_REQ In, *pOut;
//		ExecutePrepareCmd(&In, CECMD_SETCONSOLECP, sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_SETCONCP));
//		In.SetConCP.bSetOutputCP = bSetOutputCP;
//		In.SetConCP.nCP = nCP;
//		HWND hConWnd = GetConsoleWindow();
//		pOut = ExecuteSrvCmd(gdwServerPID, &In, hConWnd);
//		if (pOut) {
//			if (pOut->hdr.nSize >= In.hdr.nSize) {
//				lbRc = pOut->SetConCP.bSetOutputCP;
//				if (!lbRc)
//					SetLastError(pOut->SetConCP.nCP);
//			}
//			ExecuteFreeResult(pOut);
//		}
//#else
//		lbRc = TRUE;
//#endif
//	} else {
//		if (bSetOutputCP)
//			lbRc = SetConsoleOutputCP(nCP);
//		else
//			lbRc = SetConsoleCP(nCP);
//	}
//
//	return lbRc;
//}





/* **************************** *
 *                              *
 *  ����� ���� ���������� ����  *
 *                              *
 * **************************** */


// �������� � ������ Module �������������� ������� �� ����������� �������� �������
// �� ���������, �.�. � Win32 ���������� shell32 ����� ���� ��������� ����� conemu.dll
//   ��� ������� ������������ �������� �������,
// � � Win64 �������� ������ ������ ���� 64�������, � ��������� ������ ������ ������ 32������ ��������

static void PrepareNewModule ( HMODULE module )
{
    if( !module || IsModuleExcluded(module) )
        return;

    // ��������� ��������������� ���������� ����� ����
    // �� ��������� �� ����� ��������� �������������
    InitHooks( NULL );
    
    // ������� ������������� ������� � module
    SetHook( module, FALSE );
}


typedef HMODULE (WINAPI* OnLoadLibraryA_t)( const char* lpFileName );
static HMODULE WINAPI OnLoadLibraryA( const char* lpFileName )
{
	ORIGINAL(LoadLibraryA);

    HMODULE module = F(LoadLibraryA)( lpFileName );
    
    PrepareNewModule ( module );
    
    if (ph && ph->PostCallBack) {
    	SETARGS1(&module,lpFileName);
    	ph->PostCallBack(&args);
    	
    }
    
	return module;
}

typedef HMODULE (WINAPI* OnLoadLibraryW_t)( const wchar_t* lpFileName );
static HMODULE WINAPI OnLoadLibraryW( const wchar_t* lpFileName )
{
	ORIGINAL(LoadLibraryW);

	HMODULE module = F(LoadLibraryW)( lpFileName );
    
    PrepareNewModule ( module );
    
    if (ph && ph->PostCallBack) {
    	SETARGS1(&module,lpFileName);
    	ph->PostCallBack(&args);
    	
    }
    
	return module;
}

typedef HMODULE (WINAPI* OnLoadLibraryExA_t)( const char* lpFileName, HANDLE hFile, DWORD dwFlags );
static HMODULE WINAPI OnLoadLibraryExA( const char* lpFileName, HANDLE hFile, DWORD dwFlags )
{
	ORIGINAL(LoadLibraryExA);

	HMODULE module = F(LoadLibraryExA)( lpFileName, hFile, dwFlags );
    
    PrepareNewModule ( module );
    
    if (ph && ph->PostCallBack) {
    	SETARGS3(&module,lpFileName,hFile,dwFlags);
    	ph->PostCallBack(&args);
    	
    }
    
	return module;
}

typedef HMODULE (WINAPI* OnLoadLibraryExW_t)( const wchar_t* lpFileName, HANDLE hFile, DWORD dwFlags );
static HMODULE WINAPI OnLoadLibraryExW( const wchar_t* lpFileName, HANDLE hFile, DWORD dwFlags )
{
    ORIGINAL(LoadLibraryExW);

    HMODULE module = F(LoadLibraryExW)( lpFileName, hFile, dwFlags );
    
    PrepareNewModule ( module );
    
    if (ph && ph->PostCallBack) {
    	SETARGS3(&module,lpFileName,hFile,dwFlags);
    	ph->PostCallBack(&args);
    	
    }
    
	return module;
}

typedef FARPROC (WINAPI* OnGetProcAddress_t)(HMODULE hModule, LPCSTR lpProcName);
static FARPROC WINAPI OnGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	ORIGINAL(GetProcAddress);
	FARPROC lpfn = NULL;

	
	if (((DWORD_PTR)lpProcName) <= 0xFFFF) {
		TODO("!!! ������������ � ORDINAL values !!!");

	} else {
		for (int i = 0; Hooks[i].Name; i++)
		{
    		// The spelling and case of a function name pointed to by lpProcName must be identical
    		// to that in the EXPORTS statement of the source DLL's module-definition (.Def) file
			if (Hooks[i].hDll == hModule
        		&& strcmp(Hooks[i].Name, lpProcName) == 0)
			{
        		lpfn = (FARPROC)Hooks[i].NewAddress;
        		break;
			}
		}
	}
    
    if (!lpfn)
    	lpfn = F(GetProcAddress)(hModule, lpProcName);
    
    return lpfn;
}

TODO("�� �������� �� ��� � FreeDll ������ �����, ����� �� ������� �������� ������������ �������...");




typedef BOOL (WINAPI* OnTrackPopupMenu_t)(HMENU hMenu, UINT uFlags, int x, int y, int nReserved, HWND hWnd, CONST RECT * prcRect);
static BOOL WINAPI OnTrackPopupMenu(HMENU hMenu, UINT uFlags, int x, int y, int nReserved, HWND hWnd, CONST RECT * prcRect)
{
	ORIGINALFAST(TrackPopupMenu);
	
	#ifdef _DEBUG
		WCHAR szMsg[128]; wsprintf(szMsg, L"TrackPopupMenu(hwnd=0x%08X)\n", (DWORD)hWnd);
		DebugString(szMsg);
	#endif
	
	if (ConEmuHwnd) {
		GuiSetForeground(hWnd);
	}
	
	BOOL lbRc;
	
	lbRc = F(TrackPopupMenu)(hMenu, uFlags, x, y, nReserved, hWnd, prcRect);
	
	return lbRc;
}

typedef BOOL (WINAPI* OnTrackPopupMenuEx_t)(HMENU hmenu, UINT fuFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm);
static BOOL WINAPI OnTrackPopupMenuEx(HMENU hmenu, UINT fuFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm)
{
	ORIGINALFAST(TrackPopupMenuEx);
	
	#ifdef _DEBUG
		WCHAR szMsg[128]; wsprintf(szMsg, L"TrackPopupMenuEx(hwnd=0x%08X)\n", (DWORD)hWnd);
		DebugString(szMsg);
	#endif
	
	if (ConEmuHwnd) {
		GuiSetForeground(hWnd);
	}
	
	BOOL lbRc;
	
	lbRc = F(TrackPopupMenuEx)(hmenu, fuFlags, x, y, hWnd, lptpm);
	
	return lbRc;
}

typedef BOOL (WINAPI* OnShellExecuteExA_t)(LPSHELLEXECUTEINFOA lpExecInfo);
static BOOL WINAPI OnShellExecuteExA(LPSHELLEXECUTEINFOA lpExecInfo)
{
	ORIGINALFAST(ShellExecuteExA);
	
	if (ConEmuHwnd) {
		if (!lpExecInfo->hwnd || lpExecInfo->hwnd == GetConsoleWindow())
			lpExecInfo->hwnd = GetParent(ConEmuHwnd);
	}
	
	BOOL lbRc;
	
	lbRc = F(ShellExecuteExA)(lpExecInfo);
	
	return lbRc;
}
typedef BOOL (WINAPI* OnShellExecuteExW_t)(LPSHELLEXECUTEINFOW lpExecInfo);
static BOOL WINAPI OnShellExecuteExW(LPSHELLEXECUTEINFOW lpExecInfo)
{
	ORIGINALFAST(ShellExecuteExW);
	
	if (ConEmuHwnd) {
		if (!lpExecInfo->hwnd || lpExecInfo->hwnd == GetConsoleWindow())
			lpExecInfo->hwnd = GetParent(ConEmuHwnd);
	}
	
	BOOL lbRc;
	
	lbRc = F(ShellExecuteExW)(lpExecInfo);
	
	return lbRc;
}

typedef HINSTANCE (WINAPI* OnShellExecuteA_t)(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);
static HINSTANCE WINAPI OnShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
	ORIGINALFAST(ShellExecuteA);
	
	if (ConEmuHwnd) {
		if (!hwnd || hwnd == GetConsoleWindow())
			hwnd = GetParent(ConEmuHwnd);
	}
	
	HINSTANCE lhRc;
	
	lhRc = F(ShellExecuteA)(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
	
	return lhRc;
}
typedef HINSTANCE (WINAPI* OnShellExecuteW_t)(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd);
static HINSTANCE WINAPI OnShellExecuteW(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd)
{
	ORIGINALFAST(ShellExecuteW);
	
	if (ConEmuHwnd) {
		if (!hwnd || hwnd == GetConsoleWindow())
			hwnd = GetParent(ConEmuHwnd);
	}
	
	HINSTANCE lhRc;
	
	lhRc = F(ShellExecuteW)(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
	
	return lhRc;
}

typedef BOOL (WINAPI* OnFlashWindow_t)(HWND hWnd, BOOL bInvert);
static BOOL WINAPI OnFlashWindow(HWND hWnd, BOOL bInvert)
{
	ORIGINALFAST(FlashWindow);

	if (ConEmuHwnd) {
		GuiFlashWindow(TRUE, hWnd, bInvert, 0,0,0);
		return TRUE;
	}
	
	BOOL lbRc;
	
	lbRc = F(FlashWindow)(hWnd, bInvert);
	
	return lbRc;
}
typedef BOOL (WINAPI* OnFlashWindowEx_t)(PFLASHWINFO pfwi);
static BOOL WINAPI OnFlashWindowEx(PFLASHWINFO pfwi)
{
	ORIGINALFAST(FlashWindowEx);

	if (ConEmuHwnd) {
		GuiFlashWindow(FALSE, pfwi->hwnd, 0, pfwi->dwFlags, pfwi->uCount, pfwi->dwTimeout);
		return TRUE;
	}
	
	BOOL lbRc;
	
	lbRc = F(FlashWindowEx)(pfwi);
	
	return lbRc;
}

typedef BOOL (WINAPI* OnSetForegroundWindow_t)(HWND hWnd);
static BOOL WINAPI OnSetForegroundWindow(HWND hWnd)
{
	ORIGINALFAST(SetForegroundWindow);
	
	if (ConEmuHwnd) {
		GuiSetForeground(hWnd);
	}
	
	BOOL lbRc;
	
	lbRc = F(SetForegroundWindow)(hWnd);
	
	return lbRc;
}

#ifndef NORM_STOP_ON_NULL
#define NORM_STOP_ON_NULL 0x10000000
#endif

// ANSI ������ ������ ���, �.�. FAR 1.x ���������� ���
typedef int (WINAPI* OnCompareStringW_t)(LCID Locale, DWORD dwCmpFlags, LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2, int cchCount2);
int WINAPI OnCompareStringW(LCID Locale, DWORD dwCmpFlags, LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2, int cchCount2)
{
	ORIGINALFAST(CompareStringW);
	int nCmp = -1;
	
	if (gbFARuseASCIIsort)
	{
		if (dwCmpFlags == (NORM_IGNORECASE|NORM_STOP_ON_NULL|SORT_STRINGSORT)) {
			int n = 0;
			WCHAR ch1 = *lpString1++, /*ch10 = 0,*/ ch2 = *lpString2++ /*,ch20 = 0*/;
			int n1 = (cchCount1==cchCount2) ? cchCount1
				: (cchCount1!=-1 && cchCount2!=-1) ? -1 
					: (cchCount1!=-1) ? cchCount2 
						: (cchCount2!=-1) ? cchCount1 : min(cchCount1,cchCount2);
			while (!n && /*(ch1=*lpString1++) && (ch2=*lpString2++) &&*/ n1--)
			{
				if (ch1==ch2) {
					if (!ch1) break;
					// n = 0;
				} else if (ch1<0x80 && ch2<0x80) {
					if (ch1>=L'A' && ch1<=L'Z') ch1 |= 0x20;
					if (ch2>=L'A' && ch2<=L'Z') ch2 |= 0x20;
					n = (ch1==ch2) ? 0 : (ch1<ch2) ? -1 : 1;
				} else {
					n = CompareStringW(Locale, dwCmpFlags, &ch1, 1, &ch2, 1)-2;
				}
				if (!ch1 || !ch2 || !n1) break;
				ch1 = *lpString1++;
				ch2 = *lpString2++;
			}

			nCmp = (n<0) ? 1 : (n>0) ? 3 : 2;
		}
	}
	
	if (nCmp == -1)
		nCmp = F(CompareStringW)(Locale, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2);

	return nCmp;
}

typedef DWORD (WINAPI* OnGetConsoleAliasesW_t)(LPWSTR AliasBuffer, DWORD AliasBufferLength, LPWSTR ExeName);
static DWORD WINAPI OnGetConsoleAliasesW(LPWSTR AliasBuffer, DWORD AliasBufferLength, LPWSTR ExeName)
{
	ORIGINALFAST(GetConsoleAliasesW);
	DWORD nError = 0;
	DWORD nRc = F(GetConsoleAliasesW)(AliasBuffer,AliasBufferLength,ExeName);
	if (!nRc) {
		nError = GetLastError();
		// ���� �����
		if (nError == ERROR_NOT_ENOUGH_MEMORY && gdwServerPID) {
			CESERVER_REQ_HDR In;
			ExecutePrepareCmd((CESERVER_REQ*)&In, CECMD_GETALIASES,sizeof(CESERVER_REQ_HDR));
			CESERVER_REQ* pOut = ExecuteSrvCmd(gdwServerPID, (CESERVER_REQ*)&In, GetConsoleWindow());
			if (pOut) {
				DWORD nData = min(AliasBufferLength,(pOut->hdr.nSize-sizeof(pOut->hdr)));
				if (nData) {
					memmove(AliasBuffer, pOut->Data, nData);
					nRc = TRUE;
				}
				ExecuteFreeResult(pOut);
			}
			if (!nRc)
				SetLastError(nError); // �������, ����� ����� ������� ��� ��������
		}
	}
	return nRc;
}

//static BOOL WINAPI OnSetConsoleCP(UINT wCodePageID)
//{
//	_ASSERTE(OnSetConsoleCP!=SetConsoleCP);
//	TODO("������ � 2k3R2 ��� 'chcp 866 <enter> chcp 20866 <enter>");
//	BOOL lbRc = FALSE;
//	if (gdwServerPID) {
//		lbRc = SrvSetConsoleCP(FALSE/*bSetOutputCP*/, wCodePageID);
//	} else {
//		lbRc = SetConsoleCP(wCodePageID);
//	}
//	return lbRc;
//}
//
//static BOOL WINAPI OnSetConsoleOutputCP(UINT wCodePageID)
//{
//	_ASSERTE(OnSetConsoleOutputCP!=SetConsoleOutputCP);
//	BOOL lbRc = FALSE;
//	if (gdwServerPID) {
//		lbRc = SrvSetConsoleCP(TRUE/*bSetOutputCP*/, wCodePageID);
//	} else {
//		lbRc = SetConsoleOutputCP(wCodePageID);
//	}
//	return lbRc;
//}

typedef BOOL (WINAPI* OnGetNumberOfConsoleInputEvents_t)(HANDLE hConsoleInput, LPDWORD lpcNumberOfEvents);
static BOOL WINAPI OnGetNumberOfConsoleInputEvents(HANDLE hConsoleInput, LPDWORD lpcNumberOfEvents)
{
	ORIGINAL(GetNumberOfConsoleInputEvents);

	if (gpConsoleInfo && bMainThread)	
		TouchReadPeekConsoleInputs();
		
	BOOL lbRc = F(GetNumberOfConsoleInputEvents)(hConsoleInput, lpcNumberOfEvents);
	return lbRc;
}

typedef BOOL (WINAPI* OnPeekConsoleInputA_t)(HANDLE,PINPUT_RECORD,DWORD,LPDWORD);
static BOOL WINAPI OnPeekConsoleInputA(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead)
{
	ORIGINAL(PeekConsoleInputA);
	
	if (gpConsoleInfo && bMainThread)
		TouchReadPeekConsoleInputs(1);
		
	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack) {
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
		if (!ph->PreCallBack(&args))
			return FALSE;
	}

	lbRc = F(PeekConsoleInputA)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);
	
	if (ph && ph->PostCallBack) {
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
		ph->PostCallBack(&args);
	}
	
	return lbRc;
}

typedef BOOL (WINAPI* OnPeekConsoleInputW_t)(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
static BOOL WINAPI OnPeekConsoleInputW(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead)
{
	ORIGINAL(PeekConsoleInputW);
	
	if (gpConsoleInfo && bMainThread)
		TouchReadPeekConsoleInputs(1);
		
	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack) {
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
		if (!ph->PreCallBack(&args))
			return FALSE;
	}

	lbRc = F(PeekConsoleInputW)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);

	if (ph && ph->PostCallBack) {
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
		ph->PostCallBack(&args);
	}

	return lbRc;
}

typedef BOOL (WINAPI* OnReadConsoleInputA_t)(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
static BOOL WINAPI OnReadConsoleInputA(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead)
{
	ORIGINAL(ReadConsoleInputA);
	
	if (gpConsoleInfo&& bMainThread)
		TouchReadPeekConsoleInputs(0);
		
	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack) {
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
		if (!ph->PreCallBack(&args))
			return FALSE;
	}

	lbRc = F(ReadConsoleInputA)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);

	if (ph && ph->PostCallBack) {
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
		ph->PostCallBack(&args);
	}

	return lbRc;
}

typedef BOOL (WINAPI* OnReadConsoleInputW_t)(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
static BOOL WINAPI OnReadConsoleInputW(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead)
{
	ORIGINAL(ReadConsoleInputW);
	
	if (gpConsoleInfo && bMainThread)
		TouchReadPeekConsoleInputs(0);
		
	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack) {
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
		if (!ph->PreCallBack(&args))
			return FALSE;
	}

	lbRc = F(ReadConsoleInputW)(hConsoleInput, lpBuffer, nLength, lpNumberOfEventsRead);

	if (ph && ph->PostCallBack) {
		SETARGS4(&lbRc,hConsoleInput,lpBuffer,nLength,lpNumberOfEventsRead);
		ph->PostCallBack(&args);
	}

	return lbRc;
}

typedef HANDLE (WINAPI* OnCreateConsoleScreenBuffer_t)(DWORD dwDesiredAccess, DWORD dwShareMode, const SECURITY_ATTRIBUTES *lpSecurityAttributes, DWORD dwFlags, LPVOID lpScreenBufferData);
static HANDLE WINAPI OnCreateConsoleScreenBuffer(DWORD dwDesiredAccess, DWORD dwShareMode, const SECURITY_ATTRIBUTES *lpSecurityAttributes, DWORD dwFlags, LPVOID lpScreenBufferData)
{
	ORIGINALFAST(CreateConsoleScreenBuffer);
	
	if ((dwShareMode & (FILE_SHARE_READ|FILE_SHARE_WRITE)) != (FILE_SHARE_READ|FILE_SHARE_WRITE))
		dwShareMode |= (FILE_SHARE_READ|FILE_SHARE_WRITE);

	HANDLE h;
	h = F(CreateConsoleScreenBuffer)(dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwFlags, lpScreenBufferData);
	return h;
}

typedef BOOL (WINAPI* OnAllocConsole_t)(void);
static BOOL WINAPI OnAllocConsole(void)
{
	ORIGINALFAST(AllocConsole);
	BOOL bMainThread = (GetCurrentThreadId() == nMainThreadId);

	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack) {
		SETARGS(&lbRc);
		if (!ph->PreCallBack(&args))
			return FALSE;
	}

	lbRc = F(AllocConsole)();

	if (ph && ph->PostCallBack) {
		SETARGS(&lbRc);
		ph->PostCallBack(&args);
	}

	return lbRc;
}

typedef BOOL (WINAPI* OnFreeConsole_t)(void);
static BOOL WINAPI OnFreeConsole(void)
{
	ORIGINALFAST(FreeConsole);
	BOOL bMainThread = (GetCurrentThreadId() == nMainThreadId);

	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack) {
		SETARGS(&lbRc);
		if (!ph->PreCallBack(&args))
			return FALSE;
	}

	lbRc = F(FreeConsole)();

	if (ph && ph->PostCallBack) {
		SETARGS(&lbRc);
		ph->PostCallBack(&args);
	}

	return lbRc;
}

typedef HWND (WINAPI* OnGetConsoleWindow_t)(void);
static HWND WINAPI OnGetConsoleWindow(void)
{
	ORIGINALFAST(GetConsoleWindow);

	if (ConEmuHwnd && FarHwnd) {
		return FarHwnd;
	}

	HWND h;

	h = F(GetConsoleWindow)();

	return h;
}

typedef BOOL (WINAPI* OnWriteConsoleOutputA_t)(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
static BOOL WINAPI OnWriteConsoleOutputA(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion)
{
	ORIGINAL(WriteConsoleOutputA);

	BOOL lbRc = FALSE;

	lbRc = F(WriteConsoleOutputA)(hConsoleOutput, lpBuffer, dwBufferSize, dwBufferCoord, lpWriteRegion);

	if (ph && ph->PostCallBack) {
		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
		ph->PostCallBack(&args);
	}

	return lbRc;
}

typedef BOOL (WINAPI* OnWriteConsoleOutputW_t)(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
static BOOL WINAPI OnWriteConsoleOutputW(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion)
{
	ORIGINAL(WriteConsoleOutputW);

	BOOL lbRc = FALSE;

	lbRc = F(WriteConsoleOutputW)(hConsoleOutput, lpBuffer, dwBufferSize, dwBufferCoord, lpWriteRegion);

	if (ph && ph->PostCallBack) {
		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
		ph->PostCallBack(&args);
	}

	return lbRc;
}
