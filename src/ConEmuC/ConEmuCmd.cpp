
#include "ConEmuC.h"

bool GetAliases(wchar_t* asExeName, wchar_t** rsAliases, LPDWORD rnAliasesSize);


int ComspecInit()
{
    TODO("���������� ��� ������������� ��������, � ���� ��� FAR - ��������� ��� (��� ����������� � ����� �������)");
    TODO("������ �������� �� GUI, ���� ��� ����, ����� - �� ���������");
    TODO("GUI ����� ��������������� ������ � ������ ������ ���������");
    
    WARNING("CreateFile(CONOUT$) �� ���� ���������� ������� ScreenBuffer. ����� ��� ����� ���������� � ComspecDone");
    // ������ ����� ���������, ��� ��� ���������� � ghConOut.Close(),... 

    // ������ ������ ������ ��� GUI, ����� ��������� ConEmuC!
    #ifdef SHOW_STARTED_MSGBOX
    MessageBox(GetConsoleWindow(), L"ConEmuC (comspec mode) is about to START", L"ConEmuC.ComSpec", 0);
    #endif


    //int nNewBufferHeight = 0;
    //COORD crNewSize = {0,0};
    //SMALL_RECT rNewWindow = cmd.sbi.srWindow;
    BOOL lbSbiRc = FALSE;
    
	gbRootWasFoundInCon = 2; // �� ��������� � "Press Enter to close console" - "or wait"

    // ��� �������� � �� �����, ������ ��� ����������...
    lbSbiRc = MyGetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cmd.sbi);
    
    
    // ���� �� �������� ���� ��� ������ -new_console
    // � ���� ������ ����� ��������� ���� ��������� � ��������� � ConEmu ����� �������
    if (cmd.bNewConsole) {
        PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
        STARTUPINFOW si; memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW|STARTF_USECOUNTCHARS;
        si.dwXCountChars = cmd.sbi.dwSize.X;
        si.dwYCountChars = cmd.sbi.dwSize.Y;
        si.wShowWindow = SW_HIDE;
        
        PRINT_COMSPEC(L"Creating new console for:\n%s\n", gpszRunCmd);
    
        // CREATE_NEW_PROCESS_GROUP - ����, ��������� �������� Ctrl-C
        BOOL lbRc = CreateProcessW(NULL, gpszRunCmd, NULL,NULL, TRUE, 
                NORMAL_PRIORITY_CLASS|CREATE_NEW_CONSOLE, 
                NULL, NULL, &si, &pi);
        DWORD dwErr = GetLastError();
        if (!lbRc)
        {
            _printf ("Can't create process, ErrCode=0x%08X! Command to be executed:\n", dwErr, gpszRunCmd);
            return CERR_CREATEPROCESS;
        }
        //delete psNewCmd; psNewCmd = NULL;
        AllowSetForegroundWindow(pi.dwProcessId);
        PRINT_COMSPEC(L"New console created. PID=%i. Exiting...\n", pi.dwProcessId);
        SafeCloseHandle(pi.hProcess); SafeCloseHandle(pi.hThread);
        gbAlwaysConfirmExit = FALSE;
		srv.nProcessStartTick = GetTickCount() - 2*CHECK_ROOTSTART_TIMEOUT;
        return CERR_RUNNEWCONSOLE;
    }
    
    
    // ���� ���������� ComSpecC - ������ ConEmuC ������������� ����������� ComSpec
    // ������ ���
    wchar_t szComSpec[MAX_PATH+1], *pszComSpecName;
    if (GetEnvironmentVariable(L"ComSpecC", szComSpec, MAX_PATH) && szComSpec[0] != 0)
    {
        // ������ ���� ��� (��������) �� conemuc.exe
        wchar_t* pwszCopy = (wchar_t*)PointToName(szComSpec); //wcsrchr(szComSpec, L'\\'); 
        //if (!pwszCopy) pwszCopy = szComSpec;
        #pragma warning( push )
        #pragma warning(disable : 6400)
        if (lstrcmpiW(pwszCopy, L"ConEmuC")==0 || lstrcmpiW(pwszCopy, L"ConEmuC.exe")==0)
            szComSpec[0] = 0;
        #pragma warning( pop )
        if (szComSpec[0]) {
	        SetEnvironmentVariable(L"ComSpec", szComSpec);
	        SetEnvironmentVariable(L"ComSpecC", NULL);
        }
        pszComSpecName = (wchar_t*)PointToName(szComSpec);
    } else {
    	pszComSpecName = L"cmd.exe";
    }
    lstrcpyn(cmd.szComSpecName, pszComSpecName, countof(cmd.szComSpecName));


    if (pszComSpecName) {
    	wchar_t szSelf[MAX_PATH+1];
		if (GetModuleFileName(NULL, szSelf, MAX_PATH)) {
			lstrcpyn(cmd.szSelfName, (wchar_t*)PointToName(szSelf), countof(cmd.szSelfName));

			if (!GetAliases(cmd.szSelfName, &cmd.pszPreAliases, &cmd.nPreAliasSize)) {
				if (cmd.pszPreAliases) {
					wprintf(cmd.pszPreAliases);
					free(cmd.pszPreAliases);
					cmd.pszPreAliases = NULL;
				}
			}
		}
    }
    
    SendStarted();
    
    //ghConOut.Close();
    
    return 0;
}


void ComspecDone(int aiRc)
{
    //WARNING("������� � GUI CONEMUCMDSTOPPED");

	// ��� ���������� ������, �.�. ��� ����� ������ (SetConsoleActiveScreenBuffer) �����������,
	// ���������� ����� �������, ����� conhost ����� �� ������� ���������� �����
	//ghConOut.Close();

    // ��������� �������
    if (cmd.szComSpecName[0] && cmd.szSelfName[0]) {
    	// ����������� ������ �� cmd.exe � conemuc.exe
		wchar_t *pszPostAliases = NULL;
		DWORD nPostAliasSize;
		BOOL lbChanged = (cmd.pszPreAliases == NULL);

		if (!GetAliases(cmd.szComSpecName, &pszPostAliases, &nPostAliasSize)) {
			if (pszPostAliases) wprintf(pszPostAliases);
		} else {
			if (!lbChanged) {
				lbChanged = (cmd.nPreAliasSize!=nPostAliasSize);
			}
			if (!lbChanged && cmd.nPreAliasSize && cmd.pszPreAliases && pszPostAliases) {
				lbChanged = memcmp(cmd.pszPreAliases,pszPostAliases,cmd.nPreAliasSize)!=0;
			}
			if (lbChanged) {
				if (cmd.dwSrvPID) {
					MCHKHEAP;
					CESERVER_REQ* pIn = ExecuteNewCmd(CECMD_SAVEALIASES,sizeof(CESERVER_REQ_HDR)+nPostAliasSize);
					if (pIn) {
						MCHKHEAP;
						memmove(pIn->Data, pszPostAliases, nPostAliasSize);
						MCHKHEAP;
						CESERVER_REQ* pOut = ExecuteSrvCmd(cmd.dwSrvPID, pIn, GetConsoleWindow());
						MCHKHEAP;
						if (pOut) ExecuteFreeResult(pOut);
						ExecuteFreeResult(pIn);
						MCHKHEAP;
					}
				}

				wchar_t *pszNewName = pszPostAliases, *pszNewTarget, *pszNewLine;
				while (pszNewName && *pszNewName) {
					pszNewLine = pszNewName + lstrlen(pszNewName);
					pszNewTarget = wcschr(pszNewName, L'=');
					if (pszNewTarget) {
						*pszNewTarget = 0;
						pszNewTarget++;
					}
					if (*pszNewTarget == 0) pszNewTarget = NULL;
					AddConsoleAlias(pszNewName, pszNewTarget, cmd.szSelfName);
					pszNewName = pszNewLine+1;
				}
			}
		}
		if (pszPostAliases) { free(pszPostAliases); pszPostAliases = NULL; }
    }
    

    //TODO("��������� ������ ����� ���� (���� �������� - FAR) ��� ������� ��������. ������ ������ ������� � ��������� ���������� ������� � ������ ����� ������� ���������� � ConEmuC!");

	DWORD dwErr1 = 0, dwErr2 = 0;
	HANDLE hOut1 = NULL, hOut2 = NULL;
    BOOL lbRc1 = FALSE, lbRc2 = FALSE;
    CONSOLE_SCREEN_BUFFER_INFO sbi1 = {{0,0}}, sbi2 = {{0,0}};
	#ifdef _DEBUG
	HWND hWndCon = GetConsoleWindow();
	#endif
    // ��� ����� ��������, � �� ����������������� ����������!
	if (!cmd.bNonGuiMode) {
		// ���� GUI �� ������ ����� ������ ������� ������ ������ - ��� ����� ������� ���!
        lbRc1 = GetConsoleScreenBufferInfo(hOut1 = GetStdHandle(STD_OUTPUT_HANDLE), &sbi1);
		if (!lbRc1)
			dwErr1 = GetLastError();
	}


    //PRAGMA_ERROR("������ ������ ���������� ��� GUI, ����� ��������� ConEmuC!");
    #ifdef SHOW_STARTED_MSGBOX
    MessageBox(GetConsoleWindow(), L"ConEmuC (comspec mode) is about to TERMINATE", L"ConEmuC.ComSpec", 0);
    #endif
    
    if (!cmd.bNonGuiMode)
    {
        //// ������� ������ ������ (������ � ������)
        //if (cmd.sbi.dwSize.X && cmd.sbi.dwSize.Y) {
        //    SMALL_RECT rc = {0};
        //    SetConsoleSize(0, cmd.sbi.dwSize, rc, "ComspecDone");
        //}
        
        //ghConOut.Close();

        CESERVER_REQ *pIn = NULL, *pOut = NULL;
        int nSize = sizeof(CESERVER_REQ_HDR)+sizeof(CESERVER_REQ_STARTSTOP);
        pIn = ExecuteNewCmd(CECMD_CMDSTARTSTOP,nSize);
        if (pIn) {
			pIn->StartStop.nStarted = 3; // Cmd ����� ��������
			pIn->StartStop.hWnd = ghConWnd;
			pIn->StartStop.dwPID = gnSelfPID;
			pIn->StartStop.nSubSystem = gnImageSubsystem;
			// �� MyGet..., � �� ����� ���������������...
			// ghConOut ����� ���� NULL, ���� ������ ��������� �� ����� ������� ����������
			lbRc2 = GetConsoleScreenBufferInfo(hOut2 = GetStdHandle(STD_OUTPUT_HANDLE), &pIn->StartStop.sbi);
			if (!lbRc2)
				dwErr2 = GetLastError();

            PRINT_COMSPEC(L"Finalizing comspec mode (ExecuteGuiCmd started)\n",0);
            pOut = ExecuteGuiCmd(ghConWnd, pIn, ghConWnd);
            PRINT_COMSPEC(L"Finalizing comspec mode (ExecuteGuiCmd finished)\n",0);
            if (pOut) {
				if (!pOut->StartStopRet.bWasBufferHeight) {
					//cmd.sbi.dwSize = pIn->StartStop.sbi.dwSize;
					lbRc1 = FALSE; // ���������� ���������� �������������� �������� �������� �����. �� ���������...
				} else {
					lbRc1 = TRUE;
				}
                ExecuteFreeResult(pOut); pOut = NULL;
            }
			ExecuteFreeResult(pIn); pIn = NULL; // �� �������������
        }

        lbRc2 = GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &sbi2);
		#ifdef _DEBUG
		if (sbi2.dwSize.Y > 200) {
			wchar_t szTitle[128]; wsprintfW(szTitle, L"ConEmuC (PID=%i)", GetCurrentProcessId());
			MessageBox(NULL, L"BufferHeight was not turned OFF", szTitle, MB_SETFOREGROUND|MB_SYSTEMMODAL);
		}
		#endif
        if (lbRc1 && lbRc2 && sbi2.dwSize.Y == sbi1.dwSize.Y) {
            // GUI �� ���� ������� ������ ������... 
            // ��� �����, �.�. ��� ������ ������ �� ������ � ����� ������ ������� �� N ������ �����...
            if (sbi2.dwSize.Y != cmd.sbi.dwSize.Y) {
            	PRINT_COMSPEC(L"Error: BufferHeight was not changed from %i\n", sbi2.dwSize.Y);
                SMALL_RECT rc = {0};
                sbi2.dwSize.Y = cmd.sbi.dwSize.Y;
                SetConsoleSize(0, sbi2.dwSize, rc, "ComspecDone.Force");
            }
        }
    }

    if (cmd.pszPreAliases) { free(cmd.pszPreAliases); cmd.pszPreAliases = NULL; }
    
    //SafeCloseHandle(ghCtrlCEvent);
    //SafeCloseHandle(ghCtrlBreakEvent);
}

bool GetAliases(wchar_t* asExeName, wchar_t** rsAliases, LPDWORD rnAliasesSize)
{
	bool lbRc = false;
	DWORD nAliasRC, nAliasErr, nAliasAErr = 0, nSizeA = 0;

	_ASSERTE(asExeName && rsAliases && rnAliasesSize);
	_ASSERTE(*rsAliases == NULL);

	*rnAliasesSize = GetConsoleAliasesLength(asExeName);
	if (*rnAliasesSize == 0) {
		lbRc = true;
	} else {
		*rsAliases = (wchar_t*)calloc(*rnAliasesSize+2,1);
		nAliasRC = GetConsoleAliases(*rsAliases,*rnAliasesSize,asExeName);
		if (nAliasRC) {
			lbRc = true;
		} else {
			nAliasErr = GetLastError();
			if (nAliasErr == ERROR_NOT_ENOUGH_MEMORY) {
				// ����������� ANSI �������
				UINT nCP = CP_OEMCP;
				char szExeName[MAX_PATH+1];
				char *pszAliases = NULL;
				WideCharToMultiByte(nCP,0,asExeName,-1,szExeName,MAX_PATH+1,0,0);
				nSizeA = GetConsoleAliasesLengthA(szExeName);
				if (nSizeA) {
					pszAliases = (char*)calloc(nSizeA+1,1);
					nAliasRC = GetConsoleAliasesA(pszAliases,nSizeA,szExeName);
					if (nAliasRC) {
						lbRc = true;
						MultiByteToWideChar(nCP,0,pszAliases,nSizeA,*rsAliases,((*rnAliasesSize)/2)+1);
					} else {
						nAliasAErr = GetLastError();
					}
					free(pszAliases);
				}
			}
			if (!nAliasRC) {
				if ((*rnAliasesSize) < 255) { free(*rsAliases); *rsAliases = (wchar_t*)calloc(128,2); }
				wsprintf(*rsAliases, L"\nConEmuC: GetConsoleAliases failed, ErrCode=0x%08X(0x%08X), AliasesLength=%i(%i)\n\n", nAliasErr, nAliasAErr, *rnAliasesSize, nSizeA);
			}
		}
	}
	return lbRc;
}
