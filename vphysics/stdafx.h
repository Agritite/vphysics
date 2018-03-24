#pragma once
#define SAFE_RELEASE(p) { if ( (p != nullptr) ) { (p)->Release(); (p) = 0; } }

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' \
language='*'\"")

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif 

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <Windows.h>
#include <string>
#include <vector>

#ifdef _UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif