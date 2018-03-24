#include "Main.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
	WindowClass WndClass(L"vPhysics", 1600, 900);
	if (!WndClass.initWindow(hInstance, lpCmdLine, nCmdShow))
		return 1;
	DXClass dx(WndClass, 3);
	if (!dx.InitD3D())
	{
		dx.Cleanup();
		return 1;
	}
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		else
		{
			dx.Update();
			dx.Render();
		}
		Sleep(1);
	}
	dx.WaitForPreviousFrame();
	dx.Cleanup();
	return 0;
}

