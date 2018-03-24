#include "WindowClass.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

inline void GetFullscreenMetrics(_In_ int &x, _In_  int &y, _In_  int &width, _In_  int &height, _In_  bool &is, _In_  DWORD& style)
{
	HMONITOR hMon = MonitorFromWindow(NULL, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = { sizeof(mi) };
	if (!GetMonitorInfo(hMon, &mi))
	{
		MessageBox(NULL, L"Failed to initialize fullscreen. Using default windows size.", L"Info", MB_OK | MB_ICONEXCLAMATION);
		x = y = CW_USEDEFAULT;
		width = 800;
		height = 600;
		is = false;
		style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		return;
	}
	x = mi.rcMonitor.left;
	y = mi.rcMonitor.top;
	width = mi.rcMonitor.right - mi.rcMonitor.left;
	height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	is = true;
	style = WS_POPUP | WS_VISIBLE;
}

WindowClass::WindowClass()
{
	windowname.append(L"New Window");
	GetFullscreenMetrics(x, y, width, height, isfullscreen, WndStyle);
}

WindowClass::WindowClass(LPCTSTR in)
{
	windowname.append(in);
	GetFullscreenMetrics(x, y, width, height, isfullscreen, WndStyle);
}

WindowClass::WindowClass(int ix, int iy)
{
	windowname.append(L"New Window");
	x = y = CW_USEDEFAULT;
	width = ix;
	height = iy;
	WndStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	isfullscreen = false;
}

WindowClass::WindowClass(LPCTSTR in, int ix, int iy)
{
	windowname.append(in);
	x = y = CW_USEDEFAULT;
	width = ix;
	height = iy;
	WndStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	isfullscreen = false;
}

bool WindowClass::initWindow(_In_ HINSTANCE hInst, _In_ LPCSTR lpCmdLine, _In_  int nCmdShow)
{
	try
	{
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = NULL;
		wc.cbWndExtra = NULL;
		wc.hInstance = hInst;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = reinterpret_cast <HBRUSH> (COLOR_WINDOW + 2);
		wc.lpszClassName = windowname.c_str();
		wc.lpszMenuName = NULL;

		if (!RegisterClassEx(&wc))
			throw(L"RegisterClassEx");
		hWnd = CreateWindowEx(NULL, windowname.c_str(), windowname.c_str(), WndStyle, x, y, width, height, NULL, NULL, hInst, NULL);
		if (!hWnd)
			throw(L"CreateWindowEx");
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);

		return true;
	}
	catch (LPCTSTR funcname)
	{
		tstring message(L"Failed at WindowClass::initWindow : ");
		message.append(funcname);
		MessageBox(NULL, message.c_str(), L"Failed", MB_OK | MB_ICONERROR);
		return false;
	}
}