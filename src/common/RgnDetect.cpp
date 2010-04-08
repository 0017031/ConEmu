
#include <windows.h>
#include "common.hpp"
#include "RgnDetect.h"
#include "UnicodeChars.h"
#include "farcolor.hpp"

CRgnDetect::CRgnDetect()
{
	mb_SelfBuffers = FALSE;
	mn_DetectCallCount = 0;
	memset(&m_DetectedDialogs, 0, sizeof(m_DetectedDialogs));
	mp_FarInfo = NULL;
}

CRgnDetect::~CRgnDetect()
{
}

int CRgnDetect::GetDetectedDialogs(int anMaxCount, SMALL_RECT* rc, bool* rb)
{
	if (!this) return 0;
	int nCount = min(anMaxCount,m_DetectedDialogs.Count);
	if (nCount>0) {
		if (rc)
			memmove(rc, m_DetectedDialogs.Rects, nCount*sizeof(SMALL_RECT));
		if (rb)
			memmove(rb, m_DetectedDialogs.bWasFrame, nCount*sizeof(bool));
	}
	return nCount;
}






/* ****************************************** */
/* ����� �������� � ������� "����������" ���� */
/* ****************************************** */

static bool gbInTransparentAssert = false;

// ����� ������ �������
bool CRgnDetect::FindFrameRight_ByTop(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight)
{
	wchar_t wcMostRight = 0;
	int n;
	int nShift = nWidth*nFromY;

	wchar_t wc = pChar[nShift+nFromX];

	nMostRight = nFromX;

	if (wc != ucBoxSinglDownRight && wc != ucBoxDblDownRight) {
		// ������������� �������� - �������� ������ �� ������ ��������
		int nMostTop = nFromY;
		if (FindFrameTop_ByLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostTop))
			nFromY = nMostTop; // ���� ������ ������������ ������
	}

	if (wc != ucBoxSinglDownRight && wc != ucBoxDblDownRight) {
		wchar_t c;
		// �������� ����������� ���� ������ �� �������
		if (wc == ucBoxSinglVert || wc == ucBoxSinglVertRight) {
			while (++nMostRight < nWidth) {
				c = pChar[nShift+nMostRight];
				if (c == ucBoxSinglVert || c == ucBoxSinglVertLeft) {
					nMostRight++; break;
				}
			}

		} else if (wc == ucBoxDblDownRight) {
			while (++nMostRight < nWidth) {
				c = pChar[nShift+nMostRight];
				if (c == ucBoxDblVert || c == ucBoxDblVertLeft || c == ucBoxDblVertSinglLeft) {
					nMostRight++; break;
				}
			}

		}

	} else {
		if (wc == ucBoxSinglDownRight) {
			wcMostRight = ucBoxSinglDownLeft;
		} else if (wc == ucBoxDblDownRight) {
			wcMostRight = ucBoxDblDownLeft;
		}

		// ����� ������ �������
		while (++nMostRight < nWidth) {
			n = nShift+nMostRight;
			//if (pAttr[n].crBackColor != nBackColor)
			//	break; // ����� ����� ���� �������
			if (pChar[n] == wcMostRight) {
				nMostRight++;
				break; // ����������� ������� �����
			}
		}
	}

	nMostRight--;
	_ASSERTE(nMostRight<nWidth);
	return (nMostRight > nFromX);
}

// ����� ����� �������
bool CRgnDetect::FindFrameLeft_ByTop(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostLeft)
{
	wchar_t wcMostLeft;
	int n;
	int nShift = nWidth*nFromY;

	wchar_t wc = pChar[nShift+nFromX];

	nMostLeft = nFromX;

	if (wc != ucBoxSinglDownLeft && wc != ucBoxDblDownLeft) {
		// ������������� �������� - �������� ������ �� ������ ��������
		int nMostTop = nFromY;
		if (FindFrameTop_ByRight(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostTop))
			nFromY = nMostTop; // ���� ������ ������������ ������
	}

	if (wc != ucBoxSinglDownLeft && wc != ucBoxDblDownLeft) {
		wchar_t c;
		// �������� ����������� ���� ������ �� �������
		if (wc == ucBoxSinglVert || wc == ucBoxSinglVertLeft) {
			while (--nMostLeft >= 0) {
				c = pChar[nShift+nMostLeft];
				if (c == ucBoxSinglVert || c == ucBoxSinglVertRight) {
					nMostLeft--; break;
				}
			}

		} else if (wc == ucBoxDblDownRight) {
			while (--nMostLeft >= 0) {
				c = pChar[nShift+nMostLeft];
				if (c == ucBoxDblVert || c == ucBoxDblVertLeft || c == ucBoxDblVertSinglLeft) {
					nMostLeft--; break;
				}
			}

		}

	} else {
		if (wc == ucBoxSinglDownLeft) {
			wcMostLeft = ucBoxSinglDownRight;
		} else if (wc == ucBoxDblDownLeft) {
			wcMostLeft = ucBoxDblDownRight;
		} else {
			_ASSERTE(wc == ucBoxSinglDownLeft || wc == ucBoxDblDownLeft);
			return false;
		}

		// ����� ����� �������
		while (--nMostLeft >= 0) {
			n = nShift+nMostLeft;
			//if (pAttr[n].crBackColor != nBackColor)
			//	break; // ����� ����� ���� �������
			if (pChar[n] == wcMostLeft) {
				nMostLeft--;
				break; // ����������� ������� �����
			}
		}
	}

	nMostLeft++;
	_ASSERTE(nMostLeft>=0);
	return (nMostLeft < nFromX);
}

bool CRgnDetect::FindFrameRight_ByBottom(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight)
{
	wchar_t wcMostRight;
	int n;
	int nShift = nWidth*nFromY;

	wchar_t wc = pChar[nShift+nFromX];

	nMostRight = nFromX;


	if (wc == ucBoxSinglUpRight || wc == ucBoxSinglHorz || wc == ucBoxSinglUpHorz) {
		wcMostRight = ucBoxSinglUpLeft;
	} else if (wc == ucBoxDblUpRight || wc == ucBoxSinglUpDblHorz || wc == ucBoxDblHorz) {
		wcMostRight = ucBoxDblUpLeft;
	} else {
		return false; // ����� ������ ������ ���� �� �� ������
	}

	// ����� ������ �������
	while (++nMostRight < nWidth) {
		n = nShift+nMostRight;
		//if (pAttr[n].crBackColor != nBackColor)
		//	break; // ����� ����� ���� �������
		if (pChar[n] == wcMostRight) {
			nMostRight++;
			break; // ����������� ������� �����
		}
	}

	nMostRight--;
	_ASSERTE(nMostRight<nWidth);
	return (nMostRight > nFromX);
}

bool CRgnDetect::FindFrameLeft_ByBottom(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostLeft)
{
	wchar_t wcMostLeft;
	int n;
	int nShift = nWidth*nFromY;

	wchar_t wc = pChar[nShift+nFromX];

	nMostLeft = nFromX;


	if (wc == ucBoxSinglUpLeft || wc == ucBoxSinglHorz || wc == ucBoxSinglUpHorz) {
		wcMostLeft = ucBoxSinglUpRight;
	} else if (wc == ucBoxDblUpLeft || wc == ucBoxSinglUpDblHorz || wc == ucBoxDblHorz) {
		wcMostLeft = ucBoxDblUpRight;
	} else {
		return false; // ����� ����� ������ ���� �� �� ������
	}

	// ����� ����� �������
	while (--nMostLeft >= 0) {
		n = nShift+nMostLeft;
		//if (pAttr[n].crBackColor != nBackColor)
		//	break; // ����� ����� ���� �������
		if (pChar[n] == wcMostLeft) {
			nMostLeft--;
			break; // ����������� ������� �����
		}
	}

	nMostLeft++;
	_ASSERTE(nMostLeft>=0);
	return (nMostLeft < nFromX);
}

// ������ ��� ���������. ��� ������ - ���� �� �����
bool CRgnDetect::FindDialog_TopLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder)
{
	bMarkBorder = TRUE;
	int nShift = nWidth*nFromY;
	int nMostRightBottom;

	// ����� ������ ������� �� �����
	nMostRight = nFromX;
	FindFrameRight_ByTop(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight);
	_ASSERTE(nMostRight<nWidth);

	// ����� ������ �������
	nMostBottom = nFromY;
	FindFrameBottom_ByLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostBottom);
	_ASSERTE(nMostBottom<nHeight);

	// ����� ������ ������� �� ������ �������
	nMostRightBottom = nFromY;
	if (FindFrameBottom_ByRight(pChar, pAttr, nWidth, nHeight, nMostRight, nFromY, nMostRightBottom)) {
		_ASSERTE(nMostRightBottom<nHeight);
		// ����������� ������� - ���������� ������
		if (nMostRightBottom > nMostBottom)
			nMostBottom = nMostRightBottom;
	}

	return true;
}

// ������ ��� ���������. ��� ������ - ���� �� �����
bool CRgnDetect::FindDialog_TopRight(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder)
{
	bMarkBorder = TRUE;
	int nShift = nWidth*nFromY;
	int nX;

	nMostRight = nFromX;
	nMostBottom = nFromY;

	// ����� ����� ������� �� �����
	if (FindFrameLeft_ByTop(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nX)) {
		_ASSERTE(nX>=0);
		nFromX = nX;
	}

	// ����� ������ �������
	nMostBottom = nFromY;
	FindFrameBottom_ByRight(pChar, pAttr, nWidth, nHeight, nMostRight, nFromY, nMostBottom);
	_ASSERTE(nMostBottom<nHeight);

	// ����� ����� ������� �� ����
	if (FindFrameLeft_ByBottom(pChar, pAttr, nWidth, nHeight, nMostRight, nMostBottom, nX)) {
		_ASSERTE(nX>=0);
		if (nFromX > nX) nFromX = nX;
	}

	return true;
}

// ��� ����� ���� ������ ����� �������
// ����� ������� �����
// ������ ��� ���������, �� ���������� �� � ����
bool CRgnDetect::FindDialog_Left(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder)
{
	bMarkBorder = TRUE;
	//nBackColor = pAttr[nFromX+nWidth*nFromY].crBackColor; // �� �� ������ ������ �������� ���� ���� ��� ���������
	wchar_t wcMostRight, wcMostBottom, wcMostRightBottom, wcMostTop, wcNotMostBottom1, wcNotMostBottom2;
	int nShift = nWidth*nFromY;
	int nMostTop, nY, nX;

	wchar_t wc = pChar[nShift+nFromX];

	if (wc == ucBoxSinglVert || wc == ucBoxSinglVertRight) {
		wcMostRight = ucBoxSinglUpLeft; wcMostBottom = ucBoxSinglUpRight; wcMostRightBottom = ucBoxSinglUpLeft; wcMostTop = ucBoxSinglDownLeft;
		// ���������� �� ������������ ����� �� ������
		if (wc == ucBoxSinglVert) {
			wcNotMostBottom1 = ucBoxSinglUpHorz; wcNotMostBottom2 = ucBoxSinglUpDblHorz;
			nMostBottom = nFromY;
			while (++nMostBottom < nHeight) {
				wc = pChar[nFromX+nMostBottom*nWidth];
				if (wc == wcNotMostBottom1 || wc == wcNotMostBottom2)
					return false;
			}
		}
	} else {
		wcMostRight = ucBoxDblUpLeft; wcMostBottom = ucBoxDblUpRight; wcMostRightBottom = ucBoxDblUpLeft; wcMostTop = ucBoxDblDownLeft;
	}

	// ���������� ��������� �����, ����� ���� ���-���� ����?
	if (FindFrameTop_ByLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nY)) {
		_ASSERTE(nY >= 0);
		nFromY = nY;
	}

	// ����� ������ �������
	nMostBottom = nFromY;
	FindFrameBottom_ByLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostBottom);
	_ASSERTE(nMostBottom<nHeight);

	// ����� ������ ������� �� �����
	nMostRight = nFromX;
	if (FindFrameRight_ByTop(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nX)) {
		_ASSERTE(nX<nWidth);
		nMostRight = nX;
	}
	// ����������� ����� ������ ������� �� ����
	if (FindFrameRight_ByBottom(pChar, pAttr, nWidth, nHeight, nFromX, nMostBottom, nX)) {
		_ASSERTE(nX<nWidth);
		if (nX > nMostRight) nMostRight = nX;
	}
	_ASSERTE(nMostRight>=0);

	// ���������� ��������� ����� �� ������ �������?
	if (FindFrameTop_ByRight(pChar, pAttr, nWidth, nHeight, nMostRight, nFromY, nMostTop)) {
		_ASSERTE(nMostTop>=0);
		nFromY = nMostTop;
	}

	return true;
}

bool CRgnDetect::FindFrameTop_ByLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostTop)
{
	wchar_t c;
	// ���������� ��������� ����� ����� ������ ����� �� ����
	int nY = nFromY;
	while (nY > 0)
	{
		c = pChar[(nY-1)*nWidth+nFromX];
		if (c == ucBoxDblDownRight || c == ucBoxSinglDownRight // ������� � ��������� ���� (����� �������)
			|| c == ucBoxDblVertRight || c == ucBoxDblVertSinglRight || c == ucBoxSinglVertRight
			|| c == ucBoxDblVert || c == ucBoxSinglVert
			) // ����������� (������ �������)
		{
			nY--; continue;
		}
		// ������ ������� �����������
		break;
	}
	_ASSERTE(nY >= 0);
	nMostTop = nY;
	return (nMostTop < nFromY);
}

bool CRgnDetect::FindFrameTop_ByRight(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostTop)
{
	wchar_t c;
	// ���������� ��������� ����� ����� ������ ����� �� ����
	int nY = nFromY;
	while (nY > 0)
	{
		c = pChar[(nY-1)*nWidth+nFromX];
		if (c == ucBoxDblDownLeft || c == ucBoxSinglDownLeft // ������� � ��������� ���� (������ �������)
			|| c == ucBoxDblVertLeft || c == ucBoxDblVertSinglLeft || c == ucBoxSinglVertLeft // ����������� (������ �������)
			|| c == ucBoxDblVert || c == ucBoxSinglVert
			|| (c >= ucBox25 && c <= ucBox75) || c == ucUpScroll || c == ucDnScroll) // ������ ��������� ����� ���� ������ ������
		{
			nY--; continue;
		}
		// ������ ������� �����������
		break;
	}
	_ASSERTE(nY >= 0);
	nMostTop = nY;
	return (nMostTop < nFromY);
}

bool CRgnDetect::FindFrameBottom_ByRight(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostBottom)
{
	// ���������� ���������� ����� ������ ����� �� ����
	int nY = nFromY;
	int nEnd = nHeight - 1;
	wchar_t c; //, cd = ucBoxDblVert;
	//// � ������ � ������ ������� � ������ ������� ���� ����� ���� ����, � �� ���������� ����
	//// "<=1" �.�. ���� ������ ����������� ����������� ����
	//c = pChar[nY*nWidth+nFromX];
	//if (nFromY <= 1 && nFromX == (nWidth-1)) {
	//	if (isDigit(c)) {
	//		cd = c;
	//	}
	//}
	while (nY < nEnd)
	{
		c = pChar[(nY+1)*nWidth+nFromX];
		if (c == ucBoxDblUpLeft || c == ucBoxSinglUpLeft // ������� � ��������� ���� (������ ������)
			//|| c == cd // ������ ���� �� ������ ������
			|| c == ucBoxDblVertLeft || c == ucBoxDblVertSinglLeft || c == ucBoxSinglVertLeft // ����������� (������ �������)
			|| c == ucBoxDblVert || c == ucBoxSinglVert
			|| (c >= ucBox25 && c <= ucBox75) || c == ucUpScroll || c == ucDnScroll) // ������ ��������� ����� ���� ������ ������
		{
			nY++; continue;
		}
		// ������ ������� �����������
		break;
	}
	_ASSERTE(nY < nHeight);
	nMostBottom = nY;
	return (nMostBottom > nFromY);
}

bool CRgnDetect::FindFrameBottom_ByLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostBottom)
{
	// ���������� ���������� ����� ����� ����� �� ����
	int nY = nFromY;
	int nEnd = nHeight - 1;
	wchar_t c;
	while (nY < nEnd)
	{
		c = pChar[(nY+1)*nWidth+nFromX];
		if (c == ucBoxDblUpRight || c == ucBoxSinglUpRight // ������� � ��������� ���� (����� ������)
			|| c == ucBoxDblVertRight || c == ucBoxDblVertSinglRight || c == ucBoxSinglVertRight
			|| c == ucBoxDblVert || c == ucBoxSinglVert
			) // ����������� (������ �������)
		{
			nY++; continue;
		}
		// ������ ������� �����������
		break;
	}
	_ASSERTE(nY < nHeight);
	nMostBottom = nY;
	return (nMostBottom > nFromY);
}

// �� �� ������ ������� �����. ����� ������ �����
bool CRgnDetect::FindDialog_Right(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder)
{
	bMarkBorder = TRUE;
	int nY = nFromY;
	int nX = nFromX;
	nMostRight = nFromX;
	nMostBottom = nFromY; // ����� ��������

	// ���������� ��������� ����� ����� ������ ����� �� ����
	if (FindFrameTop_ByRight(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nY))
		nFromY = nY;

	// ���������� ���������� ����� ������ ����� �� ����
	if (FindFrameBottom_ByRight(pChar, pAttr, nWidth, nHeight, nFromX, nMostBottom, nY))
		nMostBottom = nY;


	// ������ ����� ������ ������
	
	// �� �����
	if (FindFrameLeft_ByTop(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nX))
		nFromX = nX;

	// �� ����
	if (FindFrameLeft_ByBottom(pChar, pAttr, nWidth, nHeight, nFromX, nMostBottom, nX))
		if (nX < nFromX) nFromX = nX;

	_ASSERTE(nFromX>=0 && nFromY>=0);

	return true;
}

// ������ ����� ���� ��� �����, ��� � ������ �� ������������ �����
bool CRgnDetect::FindDialog_Any(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder)
{
	wchar_t c;
	wchar_t wc = pChar[nFromY*nWidth+nFromX];
	// ��� ��� ������� ����� ��������� (��� ����������) �� ����� �� ����
	for (int ii = 0; ii <= 1; ii++)
	{
		int nY = nFromY;
		int nYEnd = (!ii) ? -1 : nHeight;
		int nYStep = (!ii) ? -1 : 1;
		wchar_t wcCorn1 = (!ii) ? ucBoxSinglDownLeft : ucBoxSinglUpLeft;
		wchar_t wcCorn2 = (!ii) ? ucBoxDblDownLeft : ucBoxDblUpLeft;
		wchar_t wcCorn3 = (!ii) ? ucBoxDblDownRight : ucBoxDblUpRight;
		wchar_t wcCorn4 = (!ii) ? ucBoxSinglDownRight : ucBoxSinglUpRight;

		// TODO: ���� ����� - ��������� ����� ���� �� ���� (�������/���������)
		
		// �������
		while (nY != nYEnd)
		{
			c = pChar[nY*nWidth+nFromX];
			if (c == wcCorn1 || c == wcCorn2 // ������� � ��������� ���� (������ �������/������)
				|| c == ucBoxDblVertLeft || c == ucBoxDblVertSinglLeft || c == ucBoxSinglVertLeft // ����������� (������ �������)
				|| (c >= ucBox25 && c <= ucBox75) || c == ucUpScroll || c == ucDnScroll) // ������ ��������� ����� ���� ������ ������
			{
				if (FindDialog_Right(pChar, pAttr, nWidth, nHeight, nFromX, nY, nMostRight, nMostBottom, bMarkBorder)) {
					nFromY = nY;
					return true;
				}
				return false; // ��������� ���...
			}
			if (c == wcCorn3 || c == wcCorn4 // ������� � ��������� ���� (����� �������/������)
				|| c == ucBoxDblVertRight || c == ucBoxDblVertSinglRight || c == ucBoxSinglVertRight) // ����������� (������ �������)
			{
				if (FindDialog_Left(pChar, pAttr, nWidth, nHeight, nFromX, nY, nMostRight, nMostBottom, bMarkBorder)) {
					nFromY = nY;
					return true;
				}
				return false; // ��������� ���...
			}
			if (c != wc) {
				// ������ ������� �����������
				break;
			}

			// ���������� (����� ��� ����)
			nY += nYStep;
		}
	}
	return false;
}

bool CRgnDetect::FindDialog_Inner(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY)
{
	// ���������� �� ������������ ����� �� ������
	int nShift = nWidth*nFromY;
	wchar_t wc = pChar[nShift+nFromX];

	if (wc != ucBoxSinglVert) {
		_ASSERTE(wc == ucBoxSinglVert);
		return false;
	}

	// ������ ������ ����� ���� ������... ������ �� ��� ���������

	int nY = nFromY;
	while (++nY < nHeight) {
		wc = pChar[nFromX+nY*nWidth];
		switch (wc)
		{
		// �� ������� ������������ ����� ����� ����������� '}' (����� ��� ����� � ������� �� �������)
		case ucBoxSinglVert:
		case L'}':
			continue;

		// ���� �� ���������� �� ������� �������� ������ - ������ ��� ����� �������. �������
		case ucBoxSinglUpRight:
		case ucBoxSinglUpLeft:
		case ucBoxSinglVertRight:
		case ucBoxSinglVertLeft:
			return false;

		// �������� ���� ������
		case ucBoxSinglUpHorz:
		case ucBoxSinglUpDblHorz:
			nY++; // �������� ��� ������ (�������)
		// ����� - �������� ����� � �������� ��� ������ (�� �������)
		default:
			nY--;
			{
				// �������� ��� ����� �� ����� (���������� ���������� �������) ��� ����� �����
				CharAttr* p = pAttr+(nWidth*nY+nFromX);
				while (nY-- >= nFromY) {
					//_ASSERTE(p->bDialog);
					_ASSERTE(p >= pAttr);
					p->bDialogVBorder = true;
					p -= nWidth;
				}
				// �� ����� ������ �� � ����� ������
				while (nY >= 0) {
					wc = pChar[nFromX+nY*nWidth];
					if (wc != ucBoxSinglVert && wc != ucBoxSinglDownHorz && wc != ucBoxSinglDownDblHorz)
						break;
					//_ASSERTE(p->bDialog);
					_ASSERTE(p >= pAttr);
					p->bDialogVBorder = true;
					p -= nWidth;
					nY --;
				}
			}
			return true;
		}
	}

	return false;
}

// ���������� ����� �����?
bool CRgnDetect::FindFrame_TopLeft(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nFrameX, int &nFrameY)
{
	// ���������� ����� �����?
	nFrameX = -1; nFrameY = -1;
	int nShift = nWidth*nFromY;
	int nFindFrom = nShift+nFromX;
	int nMaxAdd = min(5,(nWidth - nFromX - 1));
	wchar_t wc;
	// � ���� �� ������
	for (int n = 1; n <= nMaxAdd; n++) {
		wc = pChar[nFindFrom+n];
		if (wc == ucBoxSinglDownRight || wc == ucBoxDblDownRight) {
			nFrameX = nFromX+n; nFrameY = nFromY;
			return true;
		}
	}
	if (nFrameY == -1) {
		// ������� ����
		nFindFrom = nShift+nWidth+nFromX;
		for (int n = 0; n <= nMaxAdd; n++) {
			wc = pChar[nFindFrom+n];
			if (wc == ucBoxSinglDownRight || wc == ucBoxDblDownRight) {
				nFrameX = nFromX+n; nFrameY = nFromY+1;
				return true;
			}
		}
	}
	return false;
}


bool CRgnDetect::ExpandDialogFrame(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int nFrameX, int nFrameY, int &nMostRight, int &nMostBottom)
{
	bool bExpanded = false;
	int nStartRight = nMostRight;
	int nStartBottom = nMostBottom;
	// ������ ��������� nMostRight & nMostBottom �� ���������
	int n, nShift = nWidth*nFromY;
	wchar_t wc = pChar[nShift+nFromX];
	DWORD nColor = pAttr[nShift+nFromX].crBackColor;

	if (nFromX == nFrameX && nFromY == nFrameY) {
		if (wc != ucBoxDblDownRight && wc != ucBoxSinglDownRight)
			return false;

		//������� ����� ������ ����� � �����
		if (nFromY) { // ������� �����
			n = (nFromY-1)*nWidth+nFromX;
			if (pAttr[n].crBackColor == nColor && (pChar[n] == L' ' || pChar[n] == ucNoBreakSpace)) {
				nFromY--; bExpanded = true;
			}
		}
		if (nFromX) { // ������� �����
			int nMinMargin = nFromX-3; if (nMinMargin<0) nMinMargin = 0;
			n = nFromY*nWidth+nFromX;
			while (nFromX > nMinMargin) {
				n--;
				if (pAttr[n].crBackColor == nColor && (pChar[n] == L' ' || pChar[n] == ucNoBreakSpace)) {
					nFromX--;
				} else {
					break;
				}
			}
			bExpanded = (nFromX<nFrameX);
		}
		_ASSERTE(nFromX>=0 && nFromY>=0);
	} else {
		if (wc != ucSpace && wc != ucNoBreakSpace)
			return false;
	}

	if (nMostRight < (nWidth-1)) {
		int nMaxMargin = 3+(nFrameX - nFromX);
		if (nMaxMargin > nWidth) nMaxMargin = nWidth;
		int nFindFrom = nShift+nWidth+nMostRight+1;
		n = 0;
		wc = pChar[nShift+nFromX];

		while (n < nMaxMargin) {
			if (pAttr[nFindFrom].crBackColor != nColor || (pChar[nFindFrom] != L' ' && pChar[nFindFrom] != ucNoBreakSpace))
				break;
			n++; nFindFrom++;
		}
		if (n) {
			nMostRight += n;
			bExpanded = true;
		}
	}
	_ASSERTE(nMostRight<nWidth);

	// nMostBottom
	if (nFrameY > nFromY && nMostBottom < (nHeight-1)) {
		n = (nMostBottom+1)*nWidth+nFrameX;
		if (pAttr[n].crBackColor == nColor && (pChar[n] == L' ' || pChar[n] == ucNoBreakSpace)) {
			nMostBottom ++; bExpanded = true;
		}
	}
	_ASSERTE(nMostBottom<nHeight);

	return bExpanded;
}

// � ������ - ���� �������� �� ������. ��� ����� ���� ��� ����� �������
// ������ ����� �������� ��� ������, ��� �������� �� ������� ������
bool CRgnDetect::FindByBackground(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int &nFromX, int &nFromY, int &nMostRight, int &nMostBottom, bool &bMarkBorder)
{
	// �������� ���� ������ �� ����� ����
	// ��� ����� ���� ������, ����� �������� ������� ������ ��������, 
	// ��� ������ ����� �������, � �������� ����� ������ ����� �����
	DWORD nBackColor = pAttr[nFromX+nWidth*nFromY].crBackColor;
	int n, nMostRightBottom, nShift = nWidth*nFromY;
	// ����� ������ �������
	nMostRight = nFromX;
	while (++nMostRight < nWidth) {
		n = nShift+nMostRight;
		if (pAttr[n].crBackColor != nBackColor)
			break; // ����� ����� ���� �������
	}
	nMostRight--;
	_ASSERTE(nMostRight<nWidth);

	wchar_t wc = pChar[nFromY*nWidth+nMostRight];
	if (wc >= ucBoxSinglHorz && wc <= ucBoxDblVertHorz)
	{
		switch (wc)
		{
		case ucBoxSinglDownRight: case ucBoxDblDownRight:
		case ucBoxSinglUpRight: case ucBoxDblUpRight:
		case ucBoxSinglDownLeft: case ucBoxDblDownLeft:
		case ucBoxSinglUpLeft: case ucBoxDblUpLeft:
		case ucBoxDblVert: case ucBoxSinglVert:
			{
				DetectDialog(pChar, pAttr, nWidth, nHeight, nMostRight, nFromY);
				if (pAttr[nShift+nFromX].bDialog)
					return false; // ��� ��� ����������
			}
		}
	} else if (nMostRight && ((wc >= ucBox25 && wc <= ucBox75) || wc == ucUpScroll || wc == ucDnScroll)) {
		int nX = nMostRight;
		if (FindDialog_Right(pChar, pAttr, nWidth, nHeight, nX, nFromY, nMostRight, nMostBottom, bMarkBorder)) {
			nFromX = nX;
			return false;
		}
	}

	// ����� ������ �������
	nMostBottom = nFromY;
	while (++nMostBottom < nHeight) {
		n = nFromX+nMostBottom*nWidth;
		if (pAttr[n].crBackColor != nBackColor)
			break; // ����� ����� ���� �������
	}
	nMostBottom--;
	_ASSERTE(nMostBottom<nHeight);

	// ����� ������ ������� �� ������ �������
	nMostRightBottom = nFromY;
	while (++nMostRightBottom < nHeight) {
		n = nMostRight+nMostRightBottom*nWidth;
		if (pAttr[n].crBackColor != nBackColor)
			break; // ����� ����� ���� �������
	}
	nMostRightBottom--;
	_ASSERTE(nMostRightBottom<nHeight);

	// ����������� ������� - ���������� ������
	if (nMostRightBottom > nMostBottom)
		nMostBottom = nMostRightBottom;

	return true;
}

void CRgnDetect::DetectDialog(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int nFromX, int nFromY, int *pnMostRight, int *pnMostBottom)
{
	if (nFromX >= nWidth || nFromY >= nHeight)
	{
		_ASSERTE(nFromX<nWidth);
		_ASSERTE(nFromY<nHeight);
		return;
	}

#ifdef _DEBUG
	if (nFromX == 79 && nFromY == 1) {
		nFromX = nFromX;
	}
#endif

	// ������ �� ������������ ����� (���� �� ������)
	if (mn_DetectCallCount >= 3) {
		gbInTransparentAssert = true;
		_ASSERTE(mn_DetectCallCount<3);
		gbInTransparentAssert = false;
		return;
	}
	
	
	/* *********************************************** */
	/* ����� ���� ������ 'return' ������������ ������! */
	/* *********************************************** */
	mn_DetectCallCount++;

	wchar_t wc; //, wcMostRight, wcMostBottom, wcMostRightBottom, wcMostTop, wcNotMostBottom1, wcNotMostBottom2;
	int nMostRight, nMostBottom; //, nMostRightBottom, nMostTop, nShift, n;
	//DWORD nBackColor;
	bool bMarkBorder = false;

	// ����� ��������� - ������ �������, ������� �������� �������� ������ ��������

	int nShift = nWidth*nFromY;
	wc = pChar[nShift+nFromX];


	WARNING("�������� detect");
	/*
	���� ������-����� ���� ������� �� ����� - �� ����� ���� ������ ������ ��������?
	���������� ����� ������-������ ����?
	*/

	/*
	���� � ������� ������� �������� ������ ����� (���� ��������) �� �������� ���������,
	��� ��� ���� ������� ������. � ���� ����� �� ����� ����������.
	*/


	if (wc >= ucBoxSinglHorz && wc <= ucBoxDblVertHorz)
	{
		switch (wc)
		{
			// ������� ����� ����?
			case ucBoxSinglDownRight: case ucBoxDblDownRight:
			{
				// ������ ��� ���������. ��� ������ - ���� �� �����
				if (!FindDialog_TopLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder))
					goto fin;
				goto done;
			}
			// ������ ����� ����?
			case ucBoxSinglUpRight: case ucBoxDblUpRight:
			{
				// ������� ����� ����� ��������� �� ����� �����
				if (!FindDialog_TopLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder))
					goto fin;
				goto done;
			}

			// ������� ������ ����?
			case ucBoxSinglDownLeft: case ucBoxDblDownLeft:
			{
				// ������ ��� ���������. ��� ������ - ���� �� �����
				if (!FindDialog_TopRight(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder))
					goto fin;
				goto done;
			}
			// ������ ������ ����?
			case ucBoxSinglUpLeft: case ucBoxDblUpLeft:
			{
				// ������� ����� ����� ��������� �� ����� �����
				if (!FindDialog_Right(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder))
					goto fin;
				goto done;
			}

			case ucBoxDblVert: case ucBoxSinglVert:
			{
				// ���������� �� ������������ ����� �� ������
				if (wc == ucBoxSinglVert) {
					if (FindDialog_Inner(pChar, pAttr, nWidth, nHeight, nFromX, nFromY))
						goto fin;
				}

				// ������ ����� ���� ��� �����, ��� � ������ �� ������������ �����
				if (FindDialog_Any(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder))
					goto done;
			}
		}
	}
	
	if (wc == ucSpace || wc == ucNoBreakSpace)
	{
		// ���������� ����� �����?
		int nFrameX = -1, nFrameY = -1;
		if (FindFrame_TopLeft(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nFrameX, nFrameY))
		{
			// ���� ���� ����� - ���� ����� �� ����� :)
			DetectDialog(pChar, pAttr, nWidth, nHeight, nFrameX, nFrameY, &nMostRight, &nMostBottom);
			//// ������ ��������� nMostRight & nMostBottom �� ���������
			//ExpandDialogFrame(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nFrameX, nFrameY, nMostRight, nMostBottom);
			//
			goto done;
		}
	}
	

	// �������� ���� ������ �� ����� ����
	// ��� ����� ���� ������, ����� �������� ������� ������ ��������, 
	// ��� ������ ����� �������, � �������� ����� ������ ����� �����
	if (!FindByBackground(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder))
		goto fin; // ������ ��� ��� ��������, ��� ������� ���



done:
#ifdef _DEBUG
	if (nFromX<0 || nFromX>=nWidth || nMostRight<nFromX || nMostRight>=nWidth
		|| nFromY<0 || nFromY>=nHeight || nMostBottom<nFromY || nMostBottom>=nHeight)
	{
		//_ASSERT(FALSE);
		// ��� ����������, ���� ���������� ���������� ������� ��������� ��
		// ���������� ��������� �������� (������� ���� �������, ��������, � �.�.)
		goto fin;
	}
#endif
	// ������ ��������
	MarkDialog(pChar, pAttr, nWidth, nHeight, nFromX, nFromY, nMostRight, nMostBottom, bMarkBorder);

	// ������� �������, ���� �������
	if (pnMostRight) *pnMostRight = nMostRight;
	if (pnMostBottom) *pnMostBottom = nMostBottom;
fin:
	mn_DetectCallCount--;
	_ASSERTE(mn_DetectCallCount>=0);
	return;
}

void CRgnDetect::MarkDialog(wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight, int nX1, int nY1, int nX2, int nY2, bool bMarkBorder, bool bFindExterior /*= TRUE*/)
{
	if (nX1<0 || nX1>=nWidth || nX2<nX1 || nX2>=nWidth
		|| nY1<0 || nY1>=nHeight || nY2<nY1 || nY2>=nHeight)
	{
		_ASSERTE(nX1>=0 && nX1<nWidth);  _ASSERTE(nX2>=0 && nX2<nWidth);
		_ASSERTE(nY1>=0 && nY1<nHeight); _ASSERTE(nY2>=0 && nY2<nHeight);
		return;
	}

	TODO("������� ���������� � ����� ������ ���������������, ������������ � �������");
	if (m_DetectedDialogs.Count < MAX_DETECTED_DIALOGS) {
		int i = m_DetectedDialogs.Count++;
		m_DetectedDialogs.Rects[i].Left = nX1;
		m_DetectedDialogs.Rects[i].Top = nY1;
		m_DetectedDialogs.Rects[i].Right = nX2;
		m_DetectedDialogs.Rects[i].Bottom = nY2;
		m_DetectedDialogs.bWasFrame[i] = bMarkBorder;
	}

#ifdef _DEBUG
	if (nX1 == 57 && nY1 == 0) {
		nX2 = nX2;
	}
#endif

	if (bMarkBorder) {
		pAttr[nY1 * nWidth + nX1].bDialogCorner = TRUE;
		pAttr[nY1 * nWidth + nX2].bDialogCorner = TRUE;
		pAttr[nY2 * nWidth + nX1].bDialogCorner = TRUE;
		pAttr[nY2 * nWidth + nX2].bDialogCorner = TRUE;
	}

	for (int nY = nY1; nY <= nY2; nY++) {
		int nShift = nY * nWidth + nX1;

		if (bMarkBorder) {
			pAttr[nShift].bDialogVBorder = TRUE;
			pAttr[nShift+nX2-nX1].bDialogVBorder = TRUE;
		}

		for (int nX = nX1; nX <= nX2; nX++, nShift++) {
			if (nY > 0 && nX >= 58) {
				nX = nX;
			}
			pAttr[nShift].bDialog = TRUE;
			pAttr[nShift].bTransparent = FALSE;
		}

		//if (bMarkBorder)
		//	pAttr[nShift].bDialogVBorder = TRUE;
	}

	// ���� ���������� ������ �� ����� - ���������� ���������� ���������
	if (bFindExterior && bMarkBorder) {
		int nMostRight = nX2, nMostBottom = nY2;
		int nNewX1 = nX1, nNewY1 = nY1;
		if (ExpandDialogFrame(pChar, pAttr, nWidth, nHeight, nNewX1, nNewY1, nX1, nY1, nMostRight, nMostBottom)) {
			_ASSERTE(nNewX1>=0 && nNewY1>=0);
			//Optimize: �������� ����� ������ ��������� - ��� ������ ��� �������
			MarkDialog(pChar, pAttr, nWidth, nHeight, nNewX1, nNewY1, nMostRight, nMostBottom, TRUE, FALSE);
		}
	}
}

void CRgnDetect::PrepareTransparent(const CEFAR_INFO *apFarInfo, const COLORREF *apColors, const CONSOLE_SCREEN_BUFFER_INFO *apSbi,
									wchar_t* pChar, CharAttr* pAttr, int nWidth, int nHeight)
{
	mp_FarInfo = apFarInfo;
	mp_Colors = apColors;
	m_sbi = *apSbi;
	mb_BufferHeight = m_sbi.dwSize.Y > (m_sbi.srWindow.Bottom - m_sbi.srWindow.Top + 10);

	if (gbInTransparentAssert)
		return;

	m_DetectedDialogs.Count = 0;

	//if (!mp_ConsoleInfo || !gSet.NeedDialogDetect())
	//	return;

	// !!! � �������� ������ - ������� ������������, �� ������� - ��������, ���������� ��� ���������

	// � ����������-�������� ���� ����� ������
	//if (!mp_FarInfo->bFarPanelAllowed)
	//	return;
	//if (nCurFarPID && pRCon->mn_LastFarReadIdx != pRCon->mp_ConsoleInfo->nFarReadIdx) {
	//if (isPressed(VK_CONTROL) && isPressed(VK_SHIFT) && isPressed(VK_MENU))
	//	return;

	WARNING("���� ���������� � FarInfo �� ��������� - ����� ���������!");
	WARNING("��������� ����������� ������� ������� ����, ������� ������ 'R', � �� ��������, �/� ������");

	//COLORREF crColorKey = gSet.ColorKey;
	// �������� ����, �������� � ����
	int nUserBackIdx = (mp_FarInfo->nFarColors[COL_COMMANDLINEUSERSCREEN] & 0xF0) >> 4;
	COLORREF crUserBack = mp_Colors[nUserBackIdx];
	int nMenuBackIdx = (mp_FarInfo->nFarColors[COL_MENUTITLE] & 0xF0) >> 4;
	COLORREF crMenuTitleBack = mp_Colors[nMenuBackIdx];
	
	// ��� ������� ������� PanelTabs
	int nPanelTextBackIdx = (mp_FarInfo->nFarColors[COL_PANELTEXT] & 0xF0) >> 4;
	int nPanelTextForeIdx = mp_FarInfo->nFarColors[COL_PANELTEXT] & 0xF;
	
	// ��� bUseColorKey ���� ������ �������� (��� ������) �� 
	// 1. UserScreen ��� ��� ���������� �� crColorKey
	// 2. � ����� - �� �������
	// ��������� ������� KeyBar �� ���������� (Keybar + CmdLine)
	bool bShowKeyBar = (mp_FarInfo->nFarInterfaceSettings & 8/*FIS_SHOWKEYBAR*/) != 0;
	int nBottomLines = bShowKeyBar ? 2 : 1;
	// ��������� ������� MenuBar �� ����������
	// ��� ����� ���� ���� ������ ��������?
	// 1 - ��� ������� ������ ��� ��������� ����
	bool bAlwaysShowMenuBar = (mp_FarInfo->nFarInterfaceSettings & 0x10/*FIS_ALWAYSSHOWMENUBAR*/) != 0;
	int nTopLines = bAlwaysShowMenuBar ? 1 : 0;

	// �������� ������ � ������ ����� (�� �������), �� � ����� ������ ���� ����� ������� ��������...
	//// ��������, ��� ��� ����������
	//if (bShowKeyBar) {
	//	// � �����-������ ���� ������ ���� ����� 1
	//	if (pChar[nWidth*(nHeight-1)] != L'1')
	//		return;
	//	// ���������������� �����
	//	BYTE KeyBarNoColor = mp_FarInfo->nFarColors[COL_KEYBARNUM];
	//	if (pAttr[nWidth*(nHeight-1)].nBackIdx != ((KeyBarNoColor & 0xF0)>>4))
	//		return;
	//	if (pAttr[nWidth*(nHeight-1)].nForeIdx != (KeyBarNoColor & 0xF))
	//		return;
	//}

	// ������ ���������� � ������� ���������� ����������� ��������
	//if (mb_LeftPanel)
	//	MarkDialog(pAttr, nWidth, nHeight, mr_LeftPanelFull.left, mr_LeftPanelFull.top, mr_LeftPanelFull.right, mr_LeftPanelFull.bottom);
	//if (mb_RightPanel)
	//	MarkDialog(pAttr, nWidth, nHeight, mr_RightPanelFull.left, mr_RightPanelFull.top, mr_RightPanelFull.right, mr_RightPanelFull.bottom);

	// �������� ������������� ������
	RECT r;
	bool lbLeftVisible = false, lbRightVisible = false, lbFullPanel = false;

	// ���� ���������� � ������� ���������� ����������� ��������, �� ��� ����� � ���������,
	// �� � ��������� �� ������ ����� ��������� �����������

	//if (mb_LeftPanel) {
	if (mp_FarInfo->bFarLeftPanel) {
		lbLeftVisible = true;
		//r = mr_LeftPanelFull;
		r = mp_FarInfo->FarLeftPanel.PanelRect;
	} else
	// �� ���� ����� ������ ������ ��������� - ��� ������ ������ ��� �� ���������
	if (mp_FarInfo->bFarLeftPanel && mp_FarInfo->FarLeftPanel.Visible) {
		// � "��������" ������ ������ ������� ������� ������ ������
		lbLeftVisible = ConsoleRect2ScreenRect(mp_FarInfo->FarLeftPanel.PanelRect, &r);
	}
	if (lbLeftVisible) {
		if (r.right == (nWidth-1))
			lbFullPanel = true; // ������ ������ ���� �� �����
		MarkDialog(pChar, pAttr, nWidth, nHeight, r.left, r.top, r.right, r.bottom, TRUE);
		// ��� ������� ������� PanelTabs
		if (nHeight > (nBottomLines+r.bottom+1)) {
			int nIdx = nWidth*(r.bottom+1)+r.right-1;
			if (pChar[nIdx-1] == 9616 && pChar[nIdx] == L'+' && pChar[nIdx+1] == 9616
				&& pAttr[nIdx].nBackIdx == nPanelTextBackIdx
				&& pAttr[nIdx].nForeIdx == nPanelTextForeIdx)
			{
				MarkDialog(pChar, pAttr, nWidth, nHeight, r.left, r.bottom+1, r.right, r.bottom+1);
			}
		}
	}

	if (!lbFullPanel) {
		//if (mb_RightPanel) {
		if (mp_FarInfo->bFarRightPanel) {
			lbRightVisible = true;
			r = mp_FarInfo->FarRightPanel.PanelRect; // mr_RightPanelFull;
		} else
		// �� ���� ����� ������ ������ ��������� - ��� ������ ������ ��� �� ���������
		if (mp_FarInfo->bFarRightPanel && mp_FarInfo->FarRightPanel.Visible) {
			// � "��������" ������ ������ ������� ������� ������ ������
			lbRightVisible = ConsoleRect2ScreenRect(mp_FarInfo->FarRightPanel.PanelRect, &r);
		}
		if (lbRightVisible) {
			MarkDialog(pChar, pAttr, nWidth, nHeight, r.left, r.top, r.right, r.bottom, TRUE);
			// ��� ������� ������� PanelTabs
			if (nHeight > (nBottomLines+r.bottom+1)) {
				int nIdx = nWidth*(r.bottom+1)+r.right-1;
				if (pChar[nIdx-1] == 9616 && pChar[nIdx] == L'+' && pChar[nIdx+1] == 9616
					&& pAttr[nIdx].nBackIdx == nPanelTextBackIdx
					&& pAttr[nIdx].nForeIdx == nPanelTextForeIdx)
				{
					MarkDialog(pChar, pAttr, nWidth, nHeight, r.left, r.bottom+1, r.right, r.bottom+1);
				}
			}
		}
	}

	// ����� ���� ������ ������ - ����? ���������� ��� �������
	if (bAlwaysShowMenuBar // ������
		|| (pAttr[0].crBackColor == crMenuTitleBack
			&& (pChar[0] == L' ' && pChar[1] == L' ' && pChar[2] == L' ' && pChar[3] == L' ' && pChar[4] != L' '))
			)
	{
		MarkDialog(pChar, pAttr, nWidth, nHeight, 0, 0, nWidth-1, 0);
	}

	WARNING("!!! ��������� bTransparent ������ �� ������ ������, ����� ��� ������� ��� ���������� !!!");

	if (mn_DetectCallCount != 0) {
		gbInTransparentAssert = true;
		_ASSERT(mn_DetectCallCount == 0);
		gbInTransparentAssert = false;
	}

	wchar_t* pszDst = pChar;
	CharAttr* pnDst = pAttr;
	for (int nY = 0; nY < nHeight; nY++)
	{
		if (nY >= nTopLines && nY < (nHeight-nBottomLines))
		{
			// ! ������ cell - ����������� ��� �������/������ �������
			int nX1 = 0;
			int nX2 = nWidth-1; // �� ��������� - �� ��� ������

			//if (!mb_LeftPanel && mb_RightPanel) {
			if (!mp_FarInfo->bFarLeftPanel && !mp_FarInfo->bFarRightPanel) {
				// �������� ������ ����� ������
				nX2 = /*mr_RightPanelFull*/ mp_FarInfo->FarRightPanel.PanelRect.left-1;
			//} else if (mb_LeftPanel && !mb_RightPanel) {
			} else if (mp_FarInfo->bFarLeftPanel && !mp_FarInfo->bFarRightPanel) {
				// �������� ������ ������ ������
				nX1 = /*mr_LeftPanelFull*/ mp_FarInfo->FarLeftPanel.PanelRect.right+1;
			} else {
				//��������! ������ ����� ����, �� ��� ����� ���� ��������� PlugMenu!
			}

#ifdef _DEBUG
			if (nY == 16) {
				nY = nY;
			}
#endif

			int nShift = nY*nWidth+nX1;
			int nX = nX1;
			while (nX <= nX2)
			{
				// ���� ��� �� ��������� ��� ���� �������
				
				/*if (!pnDst[nX].bDialogVBorder) {
					if (pszDst[nX] == ucBoxSinglDownLeft || pszDst[nX] == ucBoxDblDownLeft) {
						// ��� ������ ����� �������, ������� �� ��������� ���� �� �����
						// �������� "������" �� ��� ����
						int nYY = nY;
						int nXX = nX;
						wchar_t wcWait = (pszDst[nX] == ucBoxSinglDownLeft) ? ucBoxSinglUpLeft : ucBoxDblUpLeft;
						while (nYY++ < nHeight) {
							if (!pnDst[nX].bDialog)
								break;
							pnDst[nXX].bDialogVBorder = TRUE;
							if (pszDst[nXX] == wcWait)
								break;
							nXX += nWidth;
						}
					}

					//Optimize:
					if (isCharBorderLeftVertical(pszDst[nX])) {
						DetectDialog(pChar, pAttr, nWidth, nHeight, nX, nY, &nX);
						nX++; nShift++;
						continue;
					}
				}*/
				if (!pnDst[nX].bDialog) {
					if (pnDst[nX].crBackColor != crUserBack) {
						DetectDialog(pChar, pAttr, nWidth, nHeight, nX, nY);
					}
				} else if (!pnDst[nX].bDialogCorner) {
					switch (pszDst[nX]) {
						case ucBoxSinglDownRight:
						case ucBoxSinglDownLeft:
						case ucBoxSinglUpRight:
						case ucBoxSinglUpLeft:
						case ucBoxDblDownRight:
						case ucBoxDblDownLeft:
						case ucBoxDblUpRight:
						case ucBoxDblUpLeft:
							// ��� ������ ����� �������, ������� �� ��������� ���� �� �����
							// �������� "������" �� ��� ����
							DetectDialog(pChar, pAttr, nWidth, nHeight, nX, nY);
					}
				}
				nX++; nShift++;
			}
		}
		pszDst += nWidth;
		pnDst += nWidth;
	}

	if (mn_DetectCallCount != 0) {
		_ASSERT(mn_DetectCallCount == 0);
	}


	if (mb_BufferHeight)
		return; // � �������� ������ - ������� ������������

	if (!lbLeftVisible && !lbRightVisible) {
		if (isPressed(VK_CONTROL) && isPressed(VK_SHIFT) && isPressed(VK_MENU))
			return; // �� CtrlAltShift - �������� UserScreen (�� ������ ��� ����������)
	}


	// 0x0 ������ ���� ������������
	//pAttr[0].bDialog = TRUE;

	pszDst = pChar;
	pnDst = pAttr;
	for (int nY = 0; nY < nHeight; nY++)
	{
		if (nY >= nTopLines && nY < (nHeight-nBottomLines))
		{
			// ! ������ cell - ����������� ��� �������/������ �������
			int nX1 = 0;
			int nX2 = nWidth-1; // �� ��������� - �� ��� ������

			// ���-����, ���� ������ ���������� - ������ UserScreen ����������
			// ���� �������� ����������� ���������� ��� �� CtrlAltShift
			
			//if (!mb_LeftPanel && mb_RightPanel) {
			//	// �������� ������ ����� ������
			//	nX2 = mr_RightPanelFull.left-1;
			//} else if (mb_LeftPanel && !mb_RightPanel) {
			//	// �������� ������ ������ ������
			//	nX1 = mr_LeftPanelFull.right+1;
			//} else {
			//	//��������! ������ ����� ����, �� ��� ����� ���� ��������� PlugMenu!
			//}

			WARNING("�� ����� ������� ��������� �������� - ���� �� �������� ������ - ���������� ����������");

			int nShift = nY*nWidth+nX1;
			int nX = nX1;
			while (nX <= nX2)
			{
				// ���� ��� �� ��������� ��� ���� �������
				if (!pnDst[nX].bDialog) {
					if (pnDst[nX].crBackColor == crUserBack) {
						// �������� ����������
						pnDst[nX].bTransparent = TRUE;
						//pnDst[nX].crBackColor = crColorKey;
						//pszDst[nX] = L' ';
					}
				}
				nX++; nShift++;
			}
		}
		pszDst += nWidth;
		pnDst += nWidth;
	}

	// ���������...
	//// 0x0 ������ ���� ������������
	//pAttr[0].bTransparent = FALSE;
}

// ������������� ���������� ���������� ������� � ���������� ������ ������
// (������� ����� ������� ������� ������ � ��������������� ������� ������)
bool CRgnDetect::ConsoleRect2ScreenRect(const RECT &rcCon, RECT *prcScr)
{
	if (!this) return false;
	*prcScr = rcCon;

	int nTopVisibleLine = m_sbi.srWindow.Top;
	//bool bBufferHeight = m_sbi.dwSize.Y > (m_sbi.srWindow.Bottom - m_sbi.srWindow.Top + 10);
	int nTextWidth = m_sbi.dwSize.X;
	int nTextHeight = mb_BufferHeight ? (m_sbi.srWindow.Bottom - m_sbi.srWindow.Top + 1) : m_sbi.dwSize.Y;

	if (mb_BufferHeight && nTopVisibleLine) {
		prcScr->top -= nTopVisibleLine;
		prcScr->bottom -= nTopVisibleLine;
	}

	bool lbRectOK = true;

	if (prcScr->left == 0 && prcScr->right >= nTextWidth)
		prcScr->right = nTextWidth - 1;
	if (prcScr->left) {
		if (prcScr->left >= nTextWidth)
			return false;
		if (prcScr->right >= nTextWidth)
			prcScr->right = nTextWidth - 1;
	}

	if (prcScr->bottom < 0) {
		lbRectOK = false; // ��������� ����� �� ������� �����
	} else if (prcScr->top >= nTextHeight) {
		lbRectOK = false; // ��������� ����� �� ������� ����
	} else {
		// ��������������� �� �������� ��������������
		if (prcScr->top < 0)
			prcScr->top = 0;
		if (prcScr->bottom >= nTextHeight)
			prcScr->bottom = nTextHeight - 1;
		lbRectOK = (prcScr->bottom > prcScr->top);
	}
	return lbRectOK;
}
