#include <windows.h>

#define CONEMUTABMAX 0x400
struct ConEmuTab
{
	int  Pos;
	int  Current;
	int  Type;
	int  Modified;
	wchar_t Name[CONEMUTABMAX];
//	int  Modified;
//	int isEditor;
};

struct ForwardedPanelInfo
{
	RECT ActiveRect;
	RECT PassiveRect;
	int ActivePathShift; // ����� � ���� ��������� � ������
	int PassivePathShift; // ����� � ���� ��������� � ������
	WCHAR* pszActivePath/*[MAX_PATH+1]*/;
	WCHAR* pszPassivePath/*[MAX_PATH+1]*/;
};

struct FarVersion {
	union {
		DWORD dwVer;
		struct {
			WORD dwVerMinor;
			WORD dwVerMajor;
		};
	};
	DWORD dwBuild;
};

struct ForwardedFileInfo
{
	WCHAR Path[MAX_PATH+1];
};

enum PipeCmd
{
	SetTabs,
	DragFrom,
	DragTo,
	SetConEmuHwnd
};

// ConEmu.dll ������������ ��������� �������
//HWND WINAPI GetFarHWND();
//void WINAPI _export GetFarVersion ( FarVersion* pfv );
