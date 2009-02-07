#include <shlobj.h>
#include "Header.h"

CDragDrop::CDragDrop(HWND hWnd)
{
	m_hWnd=hWnd;
	m_lRefCount = 1;
	pfpi = NULL;
	RegisterDragDrop(hWnd, this);
}

CDragDrop::~CDragDrop()
{
	if (pfpi) free(pfpi); pfpi=NULL;
}

HRESULT STDMETHODCALLTYPE CDragDrop::Drop (IDataObject * pDataObject,DWORD grfKeyState,POINTL pt,DWORD * pdwEffect)
{
	WCHAR szStr[MAX_PATH];
	STGMEDIUM stgMedium;
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	HRESULT hr = pDataObject->GetData(&fmtetc, &stgMedium);

	HDROP hDrop = (HDROP)stgMedium.hGlobal;

	int iQuantity = DragQueryFile(hDrop,0xFFFFFFFF,NULL,NULL);
	ZeroMemory(szStr,sizeof(WCHAR)*MAX_PATH);

	if ((grfKeyState & 32/*MK_XBUTTON1*/) == 32/*MK_XBUTTON1*/)
	{
/*		IShellLink* pLink;
		if(SUCCEEDED(CoInitialize(NULL)))
		{
			if(SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL,
										  CLSCTX_INPROC_SERVER,
										  IID_IShellLink, (void **) &pLink)))
			{
				TCHAR curr[MAX_PATH+1];

				for ( int i = 0 ; i < iQuantity; i++ )
				{
					DragQueryFile(hDrop,i,curr,MAX_PATH);
					pLink->SetPath(curr);
		//			pLink->SetDescription("Woo hoo, look at Homer's shortcut");
					pLink->SetShowCmd(SW_SHOW);

					if(SUCCEEDED(pLink->QueryInterface(IID_IPersistFile,
													   (void **)&pPersistFile)))
					{

						WideString strShortCutLocation(DesktopDir);
//						strShortCutLocation += "\\bcbshortcut.lnk";
//						pPersistFile->Save(strShortCutLocation.c_bstr(), TRUE);
						pPersistFile->Release();
					}
				}
				pLink->Release();
			}
			CoUninitialize();
		}*/
	}
	else
	{
		SHFILEOPSTRUCT fop;

		if ((grfKeyState & MK_CONTROL) && gConEmu.isDragProcessed) {
			// ��������� ������� ��� ������� ��������, ���� ����� � ������ ������
			// �� ��������, ����� �� � ������ ������ ��������� (Alt, Shift,...)
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		} else
		if ((grfKeyState & MK_CONTROL)==0 || gConEmu.isDragProcessed) {
			if (gSet.isDefCopy)
				fop.wFunc=FO_COPY;
			else
				fop.wFunc=FO_MOVE;
		} else {
			if (gSet.isDefCopy)
				fop.wFunc=FO_MOVE;
			else
				fop.wFunc=FO_COPY;
		}

		fop.hwnd=m_hWnd;
		fop.pTo=NULL; //new TCHAR[MAX_PATH+3]; -- Maximus5

		ScreenToClient(m_hWnd, (LPPOINT)&pt);
		pt.x/=gSet.LogFont.lfWidth;
		pt.y/=gSet.LogFont.lfHeight;

		if (pfpi==NULL) {
			//delete fop.pTo;
			return S_OK; //1;
		} else if (pt.x>pfpi->ActiveRect.left && pt.x<pfpi->ActiveRect.right && pt.y>pfpi->ActiveRect.top && pt.y<pfpi->ActiveRect.bottom && pfpi->pszActivePath[0]) 
		{
			if (gConEmu.isDragProcessed) {
				*pdwEffect = DROPEFFECT_NONE;
				return S_OK; // ����� ������ ����� ����� FAR � �������� �� ��������, �.�. ������ �� ���������
			}
				
			if (!*pfpi->pszActivePath)
				return S_OK;
			if (pfpi->pszActivePath[lstrlen(pfpi->pszActivePath)-1]==_T('\\'))
				pfpi->pszActivePath[lstrlen(pfpi->pszActivePath)-1] = 0;
			fop.pTo=new WCHAR[lstrlenW(pfpi->pszActivePath)+3];
			wsprintf((LPWSTR)fop.pTo, _T("%s\\\0\0"), pfpi->pszActivePath);
		}
		else if (pt.x>pfpi->PassiveRect.left && pt.x<pfpi->PassiveRect.right && pt.y>pfpi->PassiveRect.top && pt.y<pfpi->PassiveRect.bottom && pfpi->pszPassivePath[0])
		{
			// ���� ���������...
			/*if (isDragProcessed) {
				WPARAM vk = (fop.wFunc==FO_COPY) ? VK_F5 : VK_F6;

				if ((grfKeyState & MK_CONTROL)==MK_CONTROL) // "������" ������� Ctrl
				{
					//keybd_event ( VK_CONTROL, 0, KEYEVENTF_KEYUP, 0 );
					*pdwEffect = DROPEFFECT_NONE;
					return S_OK;
				}
				PostMessage(ghWnd, WM_KEYDOWN, vk, 0x01510001); // ���� Send

				//SendMessage(hConWnd, WM_KEYUP, VK_NEXT, 0xc1510001);
				return S_OK; // ����� ������ ����
			}*/
			if (!*pfpi->pszPassivePath) return 1;
			if (pfpi->pszPassivePath[lstrlen(pfpi->pszPassivePath)-1]==_T('\\'))
				pfpi->pszPassivePath[lstrlen(pfpi->pszPassivePath)-1] = 0;
			fop.pTo=new WCHAR[lstrlenW(pfpi->pszPassivePath)+3];
			wsprintf((LPWSTR)fop.pTo, _T("%s\\\0\0"), pfpi->pszPassivePath);
		}
		else 
		{
			//delete fop.pTo;
			return S_OK; //1;
		}
	//	wsprintf((LPWSTR)fop.pTo, _T("%s\\\0\0"), _T("c:\\temp\\far"));

		fop.fFlags=FOF_SIMPLEPROGRESS;
		fop.pFrom=new WCHAR[MAX_DROP_PATH*iQuantity+iQuantity+1];
		ZeroMemory((void*)fop.pFrom,sizeof(WCHAR)*MAX_DROP_PATH*iQuantity+iQuantity+1);

		WCHAR *curr=(WCHAR *)fop.pFrom;

		for ( int i = 0 ; i < iQuantity; i++ )
		{
			DragQueryFile(hDrop,i,curr,MAX_DROP_PATH);
			curr+=wcslen(curr)+1;
		}
		SHFileOperation(&fop);
		if (fop.pTo) //Maximus5
			delete fop.pTo;
	}
	return S_OK; //1;
}

HRESULT STDMETHODCALLTYPE CDragDrop::DragOver(DWORD grfKeyState,POINTL pt,DWORD * pdwEffect)
{
	if (!gSet.isDnD)
		return -1;

	ScreenToClient(m_hWnd, (LPPOINT)&pt);
	pt.x/=gSet.LogFont.lfWidth;
	pt.y/=gSet.LogFont.lfHeight;

	if (pfpi==NULL)
		*pdwEffect=DROPEFFECT_NONE;
	else
	if ((pt.x>pfpi->ActiveRect.left && pt.x<pfpi->ActiveRect.right && pt.y>pfpi->ActiveRect.top && pt.y<pfpi->ActiveRect.bottom && pfpi->pszActivePath[0] && !selfdrag) ||
		(pt.x>pfpi->PassiveRect.left && pt.x<pfpi->PassiveRect.right && pt.y>pfpi->PassiveRect.top && pt.y<pfpi->PassiveRect.bottom && pfpi->pszPassivePath[0]))
	{
/*		if ((grfKeyState & 32) == 32)
			*pdwEffect=DROPEFFECT_LINK;
		else if ((grfKeyState & MK_CONTROL)==MK_CONTROL)
			*pdwEffect=DROPEFFECT_COPY;
		else
			*pdwEffect=DROPEFFECT_MOVE;*/

/*		if ((grfKeyState & MK_CONTROL)==MK_CONTROL)
			if (gSet.isDefCopy)
				*pdwEffect=DROPEFFECT_MOVE;
			else
				*pdwEffect=DROPEFFECT_COPY;
		else
			if (gSet.isDefCopy)
				*pdwEffect=DROPEFFECT_COPY;
			else
				*pdwEffect=DROPEFFECT_MOVE;*/
		if ((grfKeyState & MK_CONTROL) && gConEmu.isDragProcessed) {
			// ��������� ������� ��� ������� ��������, ���� ����� � ������ ������
			// �� ��������, ����� �� � ������ ������ ��������� (Alt, Shift,...)
			*pdwEffect = DROPEFFECT_NONE;
		} else
		if ((grfKeyState & MK_CONTROL)==0 || gConEmu.isDragProcessed) {
			if (gSet.isDefCopy)
				*pdwEffect=DROPEFFECT_COPY;
			else
				*pdwEffect=DROPEFFECT_MOVE;
		} else {
			if (gSet.isDefCopy)
				*pdwEffect=DROPEFFECT_MOVE;
			else
				*pdwEffect=DROPEFFECT_COPY;
		}

	}
	else
		*pdwEffect=DROPEFFECT_NONE;
	return 0;
}

HRESULT STDMETHODCALLTYPE CDragDrop::DragEnter(IDataObject * pDataObject,DWORD grfKeyState,POINTL pt,DWORD * pdwEffect)
{
	if (gSet.isDnD)
	{
		selfdrag=(pDataObject == this->pDataObject);
		PipeCmd cmd=DragTo;
		DWORD cbWritten=0;
		WriteFile(gConEmu.hPipe, &cmd, sizeof(cmd), &cbWritten, NULL); 
		SetEvent(gConEmu.hPipeEvent);

		DWORD cbBytesRead=0;
		int cbStructSize=0;
		if (pfpi) {free(pfpi); pfpi=NULL;}
		ReadFile(gConEmu.hPipe, &cbStructSize, sizeof(int), &cbBytesRead, NULL);
		if (cbStructSize>sizeof(ForwardedPanelInfo)) {
			pfpi = (ForwardedPanelInfo*)calloc(cbStructSize, 1);

			ReadFile(gConEmu.hPipe, pfpi, cbStructSize, &cbBytesRead, NULL); 

			pfpi->pszActivePath = (WCHAR*)(((char*)pfpi)+pfpi->ActivePathShift);
			pfpi->pszPassivePath = (WCHAR*)(((char*)pfpi)+pfpi->PassivePathShift);
		}
	}
	return 0;
}

HRESULT STDMETHODCALLTYPE CDragDrop::DragLeave(void)
{
	return 0;
}

void CDragDrop::Drag()
{
	if (!gSet.isDnD /*|| isInDrag */|| gConEmu.isDragProcessed)
		return;

	//isInDrag=true; // return � ���� �� ��������� - ����� �������� � �����
	gConEmu.isDragProcessed=true; // ����� �� ��������� ��� ���� �� ���� ����

	PipeCmd cmd=DragFrom;
	DWORD cbWritten=0;
	WriteFile(gConEmu.hPipe, &cmd, sizeof(cmd), &cbWritten, NULL); 
	SetEvent(gConEmu.hPipeEvent);
	DWORD cbBytesRead=0;
	int nWholeSize=0;
	ReadFile(gConEmu.hPipe, &nWholeSize, sizeof(nWholeSize), &cbBytesRead, NULL); 
	if (nWholeSize==0) // ������ ����� �������
		ReadFile(gConEmu.hPipe, &nWholeSize, sizeof(nWholeSize), &cbBytesRead, NULL); 
	else
		nWholeSize=0;

	if (nWholeSize>0)
	{
		wchar_t *szDraggedPath=NULL; //ASCIIZZ
		szDraggedPath=new wchar_t[nWholeSize/*(MAX_PATH+1)*FilesCount*/+1];	
		ZeroMemory(szDraggedPath, /*((MAX_PATH+1)*FilesCount+1)*/(nWholeSize+1)*sizeof(wchar_t));
		wchar_t  *curr=szDraggedPath;
		
		for (;;)
		{
			int nCurSize=0;
			ReadFile(gConEmu.hPipe, &nCurSize, sizeof(nCurSize), &cbBytesRead, NULL); 
			if (nCurSize==0) break;

			ReadFile(gConEmu.hPipe, curr, sizeof(WCHAR)*nCurSize, &cbBytesRead, NULL); 

			curr+=wcslen(curr)+1;
		}
	//		int size = MakeDropWord(szDraggedPath);// ������ null,null ������
	//		if (size <= 1) return;
		int size=(curr-szDraggedPath)*sizeof(wchar_t)+2;
	    
	    
		IDropSource *pDropSource;

		DROPFILES drop_struct = { sizeof(drop_struct), { 0, 0 }, 0, 1 };
	    
		HGLOBAL drop_data = GlobalAlloc(0, size+sizeof(drop_struct));
		ZeroMemory(drop_data, size+sizeof(drop_struct));

		wchar_t* clip_data = reinterpret_cast<wchar_t*>(drop_data);
		memcpy(drop_data, &drop_struct, sizeof(drop_struct));

		memcpy((byte*)drop_data + sizeof(drop_struct), szDraggedPath, size);
		//Maximus5 - � ������ ����������� ��� �����?
		delete szDraggedPath; szDraggedPath=NULL;

		DWORD           dwEffect;
		DWORD           dwResult;
		FORMATETC       fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM       stgmed = { TYMED_HGLOBAL, { 0 }, 0 };
		stgmed.hGlobal = drop_data ;
	//		stgmed.lpszFileName=szDraggedPath;
		CreateDropSource(&pDropSource);
		CreateDataObject(&fmtetc, &stgmed, 1, &pDataObject) ;//   |   ���������� ����... 
		DWORD dwAllowedEffects = DROPEFFECT_LINK;
		unsigned short stateControl = GetAsyncKeyState(VK_CONTROL);
		if (gSet.isDefCopy==1) {
			// ����������� �� ���������
			if (stateControl & 0x8000) // �� ����� Ctrl
				dwAllowedEffects |= DROPEFFECT_MOVE;
			else
				dwAllowedEffects |= DROPEFFECT_COPY;
		} else if (gSet.isDefCopy==0) {
			// ����������� �� ���������
			if (stateControl & 0x8000) // �� ����� Ctrl
				dwAllowedEffects |= DROPEFFECT_COPY;
			else
				dwAllowedEffects |= DROPEFFECT_MOVE;
		} else {
			// "�����������" ���������
			dwAllowedEffects |= DROPEFFECT_COPY|DROPEFFECT_MOVE;
		}
		dwResult = DoDragDrop(pDataObject, pDropSource, dwAllowedEffects, &dwEffect);
		pDataObject->Release();
		pDropSource->Release();		
		//isLBDown=false; -- � ReleaseCapture ��� ����� ������?
	}

	//isInDrag=false;
	//isDragProcessed=false; -- ����� ��� �������� � ��������� ������ ������� ������ ���� ����� ��������� ��� ���???
}