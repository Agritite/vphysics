#pragma once
#include "stdafx.h"

class WindowClass
{
private:
	int width;
	int height;
	int x;
	int y;
	bool isfullscreen;
	DWORD WndStyle;

	HWND hWnd;
	WNDCLASSEX wc;
	tstring windowname;
public:
	WindowClass();
	WindowClass(LPCTSTR);
	WindowClass(int, int);
	WindowClass(LPCTSTR, int, int);

	bool initWindow(HINSTANCE, LPCSTR, int);
	
	friend class DXClass;
	friend void GetFullscreenMetrics(int&, int&, int&, int&, bool&, DWORD&);
};
