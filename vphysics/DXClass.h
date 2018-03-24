#pragma once

#include "stdafx.h"
#include "WindowClass.h"

class DXClass
{
private:
	const int framebuffercount;
	int frameIndex;
	int rtvDescSize;

	HANDLE fenceEvent;
	ID3D12Device* device;
	IDXGISwapChain3* swapChain;
	ID3D12CommandQueue* cmdQueue;
	ID3D12DescriptorHeap* rtvDescHeap;
	ID3D12GraphicsCommandList* cmdList;
	
	ID3D12Resource** renderTargets;
	ID3D12CommandAllocator** cmdAllocator;
	ID3D12Fence** fence;
	UINT64* fenceValue;
	
	WindowClass& WndClass;

public:
	DXClass(WindowClass&);
	DXClass(WindowClass&, int f);
	~DXClass();

	bool InitD3D();
	void Update();
	bool UpdatePipeLine();
	bool Render();
	void Cleanup();
	bool WaitForPreviousFrame();
};
