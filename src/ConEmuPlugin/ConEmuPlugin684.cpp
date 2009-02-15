#include <windows.h>
#include "..\common\common.hpp"
#include "..\common\pluginW684.hpp"
#include "PluginHeader.h"

#ifndef FORWARD_WM_COPYDATA
#define FORWARD_WM_COPYDATA(hwnd, hwndFrom, pcds, fn) \
    (BOOL)(UINT)(DWORD)(fn)((hwnd), WM_COPYDATA, (WPARAM)(hwndFrom), (LPARAM)(pcds))
#endif

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



struct PluginStartupInfo *InfoW684=NULL;
struct FarStandardFunctions *FSFW684=NULL;


void ProcessDragFrom684()
{
	WindowInfo WInfo;				
    WInfo.Pos=0;
	InfoW684->AdvControl(InfoW684->ModuleNumber, ACTL_GETSHORTWINDOWINFO, (void*)&WInfo);
	if (!WInfo.Current)
	{
		//InfoW684->AdvControl(InfoW684->ModuleNumber, ACTL_FREEWINDOWINFO, (void*)&WInfo);
		int ItemsCount=0;
		//WriteFile(hPipe, &ItemsCount, sizeof(int), &cout, NULL);				
		OutDataAlloc(sizeof(ItemsCount));
		OutDataWrite(&ItemsCount,sizeof(ItemsCount));
		return;
	}
	//InfoW684->AdvControl(InfoW684->ModuleNumber, ACTL_FREEWINDOWINFO, (void*)&WInfo);

	PanelInfo PInfo;
	InfoW684->Control(PANEL_ACTIVE, FCTL_GETPANELINFO, &PInfo);
	if ((PInfo.PanelType == PTYPE_FILEPANEL || PInfo.PanelType == PTYPE_TREEPANEL) && PInfo.Visible)
	{
		int nDirLen=0, nDirNoSlash=0;
		if (PInfo.lpwszCurDir && *PInfo.lpwszCurDir)
		{
			nDirLen=lstrlen(PInfo.lpwszCurDir);
			if (nDirLen>0)
				if (PInfo.lpwszCurDir[nDirLen-1]!=L'\\')
					nDirNoSlash=1;
		}

		OutDataAlloc(sizeof(int)+PInfo.SelectedItemsNumber*((MAX_PATH+2)+sizeof(int)));

		//Maximus5 - ����� ������ ��������
		int nNull=0;
		//WriteFile(hPipe, &nNull/*ItemsCount*/, sizeof(int), &cout, NULL);
		OutDataWrite(&nNull/*ItemsCount*/, sizeof(int));
		
		if (PInfo.SelectedItemsNumber>0)
		{
			int ItemsCount=PInfo.SelectedItemsNumber, i;

			int nMaxLen=MAX_PATH+1, nWholeLen=1;

			// ������� ��������� ������������ ����� ������
			for (i=0;i<ItemsCount;i++)
			{
				int nLen=nDirLen+nDirNoSlash;
				nLen += lstrlen(PInfo.SelectedItems[i]->FindData.lpwszFileName);
				if (nLen>nMaxLen)
					nMaxLen = nLen;
				nWholeLen += (nLen+1);
			}
			OutDataWrite(&nWholeLen, sizeof(int));

			WCHAR* Path=new WCHAR[nMaxLen+1];

			for (i=0;i<ItemsCount;i++)
			{
				//WCHAR Path[MAX_PATH+1];
				//ZeroMemory(Path, MAX_PATH+1);
				//Maximus5 - ������ � ������ ����� � ������������ overflow
				//wsprintf(Path, L"%s\\%s", PInfo.lpwszCurDir, PInfo.SelectedItems[i]->FindData.lpwszFileName);
				Path[0]=0;

				int nLen=nDirLen+nDirNoSlash;
				nLen += lstrlen(PInfo.SelectedItems[i]->FindData.lpwszFileName);

				if (nDirLen>0) {
					lstrcpy(Path, PInfo.lpwszCurDir);
					if (nDirNoSlash) {
						Path[nDirLen]=L'\\';
						Path[nDirLen+1]=0;
					}
				}
				lstrcpy(Path+nDirLen+nDirNoSlash, PInfo.SelectedItems[i]->FindData.lpwszFileName);

				nLen++;
				OutDataWrite(&nLen, sizeof(int));
				OutDataWrite(Path, sizeof(WCHAR)*nLen);
			}

			delete Path; Path=NULL;

			// ����� ������
			OutDataWrite(&nNull/*ItemsCount*/, sizeof(int));
		}
	}
	else
	{
		int ItemsCount=0;
		OutDataWrite(&ItemsCount, sizeof(int));
		OutDataWrite(&ItemsCount, sizeof(int)); // ����� �������
	}
	InfoW684->Control(PANEL_ACTIVE, FCTL_FREEPANELINFO, &PInfo);
}

void ProcessDragTo684()
{
	WindowInfo WInfo;				
    WInfo.Pos=0;
	InfoW684->AdvControl(InfoW684->ModuleNumber, ACTL_GETSHORTWINDOWINFO, (void*)&WInfo);
	if (!WInfo.Current)
	{
		//InfoW684->AdvControl(InfoW684->ModuleNumber, ACTL_FREEWINDOWINFO, (void*)&WInfo);
		int ItemsCount=0;
		//WriteFile(hPipe, &ItemsCount, sizeof(int), &cout, NULL);				
		OutDataAlloc(sizeof(ItemsCount));
		OutDataWrite(&ItemsCount,sizeof(ItemsCount));
		return;
	}
	//InfoW684->AdvControl(InfoW684->ModuleNumber, ACTL_FREEWINDOWINFO, (void*)&WInfo);
	
	PanelInfo PAInfo, PPInfo;
	ForwardedPanelInfo *pfpi=NULL;
	int nStructSize = sizeof(ForwardedPanelInfo)+4; // ����� �������� �� ����� �����
	//ZeroMemory(&fpi, sizeof(fpi));
	BOOL lbAOK=FALSE, lbPOK=FALSE;
	
	//Maximus5 - � ���������, FCTL_GETPANELSHORTINFO �� ���������� lpwszCurDir :-(

	//if (!(lbAOK=InfoW684->Control(PANEL_ACTIVE, FCTL_GETPANELSHORTINFO, &PAInfo)))
	lbAOK=InfoW684->Control(PANEL_ACTIVE, FCTL_GETPANELINFO, &PAInfo);
	if (lbAOK && PAInfo.lpwszCurDir)
		nStructSize += (lstrlen(PAInfo.lpwszCurDir))*sizeof(WCHAR);

	//if (!(lbPOK=InfoW684->Control(PANEL_PASSIVE, FCTL_GETPANELSHORTINFO, &PPInfo)))
	lbPOK=InfoW684->Control(PANEL_PASSIVE, FCTL_GETPANELINFO, &PPInfo);
	if (lbPOK && PPInfo.lpwszCurDir)
		nStructSize += (lstrlen(PPInfo.lpwszCurDir))*sizeof(WCHAR); // ������ WCHAR! �� TCHAR

	pfpi = (ForwardedPanelInfo*)calloc(nStructSize,1);


	pfpi->ActivePathShift = sizeof(ForwardedPanelInfo);
	pfpi->pszActivePath = (WCHAR*)(((char*)pfpi)+pfpi->ActivePathShift);

	pfpi->PassivePathShift = pfpi->ActivePathShift+2; // ���� ActivePath ���������� - ��������

	if (lbAOK)
	{
		pfpi->ActiveRect=PAInfo.PanelRect;
		if ((PAInfo.PanelType == PTYPE_FILEPANEL || PAInfo.PanelType == PTYPE_TREEPANEL) && PAInfo.Visible)
		{
			if (PAInfo.lpwszCurDir != NULL) {
				lstrcpyW(pfpi->pszActivePath, PAInfo.lpwszCurDir);
				pfpi->PassivePathShift += lstrlenW(pfpi->pszActivePath)*2;
			}
		}
	}

	pfpi->pszPassivePath = (WCHAR*)(((char*)pfpi)+pfpi->PassivePathShift);
	if (lbPOK)
	{
		pfpi->PassiveRect=PPInfo.PanelRect;
		if ((PPInfo.PanelType == PTYPE_FILEPANEL || PPInfo.PanelType == PTYPE_TREEPANEL) && PPInfo.Visible)
		{
			if (PPInfo.lpwszCurDir != NULL)
				lstrcpyW(pfpi->pszPassivePath, PPInfo.lpwszCurDir);
		}
	}

	// ����������, ��������� ����������
	//WriteFile(hPipe, &nStructSize, sizeof(nStructSize), &cout, NULL);
	//WriteFile(hPipe, pfpi, nStructSize, &cout, NULL);
	OutDataAlloc(nStructSize+4);
	OutDataWrite(&nStructSize, sizeof(nStructSize));
	OutDataWrite(pfpi, nStructSize);

	free(pfpi); pfpi=NULL;
	InfoW684->Control(PANEL_ACTIVE, FCTL_FREEPANELINFO, &PAInfo);
	InfoW684->Control(PANEL_ACTIVE, FCTL_FREEPANELINFO, &PPInfo);
}

void SetStartupInfoW684(void *aInfo)
{
	::InfoW684 = (PluginStartupInfo*)calloc(sizeof(PluginStartupInfo),1);
	::FSFW684 = (FarStandardFunctions*)calloc(sizeof(FarStandardFunctions),1);
	*::InfoW684 = *((struct PluginStartupInfo*)aInfo);
	*::FSFW684 = *((struct PluginStartupInfo*)aInfo)->FSF;
	::InfoW684->FSF = ::FSFW684;

	/*if (!FarHwnd)
		InitHWND((HWND)InfoW684->AdvControl(InfoW684->ModuleNumber, ACTL_GETFARHWND, 0));*/
}


// watch non-modified -> modified editor status change
int ProcessEditorInputW684(LPCVOID aRec)
{
	const INPUT_RECORD *Rec = (const INPUT_RECORD*)aRec;
	// only key events with virtual codes > 0 are likely to cause status change (?)
	if (Rec->EventType == KEY_EVENT && Rec->Event.KeyEvent.wVirtualKeyCode > 0  && Rec->Event.KeyEvent.bKeyDown)
	{
		EditorInfo ei;
		// ���� ��� PluginStartupInfo �� ������� �� �����������...
		InfoW684->EditorControl(ECTL_GETINFO, &ei);
		int currentModifiedState = ei.CurState == ECSTATE_MODIFIED ? 1 : 0;
		InfoW684->EditorControl(ECTL_FREEINFO, &ei);
		if (lastModifiedStateW != currentModifiedState)
		{
			// !!! ������ UpdateConEmuTabsW, ��� ������ !!!
			UpdateConEmuTabsW(0, false, false);
			lastModifiedStateW = currentModifiedState;
		}
	}
	return 0;
}

int ProcessEditorEventW684(int Event, void *Param)
{
	switch (Event)
	{
	case EE_CLOSE:
	case EE_GOTFOCUS:
	case EE_KILLFOCUS:
	case EE_SAVE:
	case EE_READ:
		{
			// !!! ������ UpdateConEmuTabsW, ��� ������ !!!
			UpdateConEmuTabsW(Event, Event == EE_KILLFOCUS, Event == EE_SAVE);
			break;
		}
	}
	return 0;
}

int ProcessViewerEventW684(int Event, void *Param)
{
	switch (Event)
	{
	case VE_CLOSE:
	case VE_READ:
	case VE_KILLFOCUS:
	case VE_GOTFOCUS:
		{
			// !!! ������ UpdateConEmuTabsW, ��� ������ !!!
			UpdateConEmuTabsW(Event, Event == VE_KILLFOCUS, false);
		}
	}

	return 0;
}


void UpdateConEmuTabsW684(int event, bool losingFocus, bool editorSave)
{
    BOOL lbCh = FALSE;
	WindowInfo WInfo;

	int windowCount = (int)InfoW684->AdvControl(InfoW684->ModuleNumber, ACTL_GETWINDOWCOUNT, NULL);
	lbCh = (lastWindowCount != windowCount);
	
	if (!CreateTabs ( windowCount ))
		return;

	EditorInfo ei;
	if (editorSave)
	{
		InfoW684->EditorControl(ECTL_GETINFO, &ei);
	}

	int tabCount = 1;
	for (int i = 0; i < windowCount; i++)
	{
		WInfo.Pos = i;
		InfoW684->AdvControl(InfoW684->ModuleNumber, ACTL_GETWINDOWINFO, (void*)&WInfo);
		if (WInfo.Type == WTYPE_EDITOR || WInfo.Type == WTYPE_VIEWER || WInfo.Type == WTYPE_PANELS)
			lbCh |= AddTab(tabCount, losingFocus, editorSave, 
				WInfo.Type, WInfo.Name, editorSave ? ei.FileName : NULL, 
				WInfo.Current, WInfo.Modified);
		InfoW684->AdvControl(InfoW684->ModuleNumber, ACTL_FREEWINDOWINFO, (void*)&WInfo);
	}
	
	if (editorSave) 
		InfoW684->EditorControl(ECTL_FREEINFO, &ei);

	if (lbCh)
		SendTabs(tabCount);
}

void ExitFARW684(void)
{
	if (InfoW684) {
		free(InfoW684);
		InfoW684=NULL;
	}
	if (FSFW684) {
		free(FSFW684);
		FSFW684=NULL;
	}
}

int ShowMessage684(int aiMsg, int aiButtons)
{
	if (!InfoW684 || !InfoW684->Message)
		return -1;
	return InfoW684->Message(InfoW684->ModuleNumber, FMSG_ALLINONE, NULL, 
		(const wchar_t * const *)InfoW684->GetMsg(InfoW684->ModuleNumber,aiMsg), 0, aiButtons);
}

void ReloadMacro684()
{
	if (!InfoW684 || !InfoW684->AdvControl)
		return;

	ActlKeyMacro command;
	command.Command=MCMD_LOADALL;
	InfoW684->AdvControl(InfoW684->ModuleNumber,ACTL_KEYMACRO,&command);
}
