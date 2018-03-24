#include "DXClass.h"

DXClass::DXClass(WindowClass& inW) : framebuffercount(2), WndClass(inW)
{
	renderTargets = new ID3D12Resource*[framebuffercount];
	cmdAllocator = new ID3D12CommandAllocator*[framebuffercount];
	fence = new ID3D12Fence*[framebuffercount];
	fenceValue = new UINT64[framebuffercount];
	for (int i = 0; i < framebuffercount; i++)
	{
		renderTargets[i] = nullptr;
		cmdAllocator[i] = nullptr;
		fence[i] = nullptr;
	}
	fenceEvent = nullptr;
	device = nullptr;
	swapChain = nullptr;
	cmdQueue = nullptr;
	rtvDescHeap = nullptr;
	cmdList = nullptr;
}

DXClass::DXClass(WindowClass& inW, int in) : framebuffercount(in), WndClass(inW)
{
	renderTargets = new ID3D12Resource*[framebuffercount];
	cmdAllocator = new ID3D12CommandAllocator*[framebuffercount];
	fence = new ID3D12Fence*[framebuffercount];
	fenceValue = new UINT64[framebuffercount];
	for (int i = 0; i < framebuffercount; i++)
	{
		renderTargets[i] = nullptr;
		cmdAllocator[i] = nullptr;
		fence[i] = nullptr;
	}
	fenceEvent = nullptr;
	device = nullptr;
	swapChain = nullptr;
	cmdQueue = nullptr;
	rtvDescHeap = nullptr;
	cmdList = nullptr;
}

DXClass::~DXClass()
{
	delete[] renderTargets;
	delete[] cmdAllocator;
	delete[] fence;
	delete[] fenceValue;
}

bool DXClass::InitD3D()
{
	try
	{
		// ----------- Create D3DDevice -----------

		IDXGIFactory4* dxgiFactory;
		HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
		if (FAILED(hr))
			throw(L"CreateDXGIFactory1");
		
		IDXGIAdapter1* adapter;
		int adapterIndex = 0;
		bool adapterFound = false;
		while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				adapterIndex++;
				continue;
			}
			hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
			if (SUCCEEDED(hr))
			{
				adapterFound = true;
				break;
			}
			adapterIndex++;
		}
		if (!adapterFound)
			throw(tstring(L"No device supports DirectX 12!"));

		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
		if (FAILED(hr))
			throw(L"D3D12CreateDevice");

		// ----------- Create Command Queue -----------

		D3D12_COMMAND_QUEUE_DESC cqDesc = {};
		hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&cmdQueue));
		if (FAILED(hr))
			throw(L"device->CreateCommandQueue");

		// ----------- Create Swap Chain -----------

		DXGI_SAMPLE_DESC dsd = { 1, 0 };
		DXGI_MODE_DESC dmd = { WndClass.width, WndClass.height };
		dmd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_SWAP_CHAIN_DESC dscd = {dmd, dsd, DXGI_USAGE_RENDER_TARGET_OUTPUT, 
									 framebuffercount, WndClass.hWnd, TRUE, DXGI_SWAP_EFFECT_FLIP_DISCARD};
		IDXGISwapChain* tmpswapchain;
		hr = dxgiFactory->CreateSwapChain(cmdQueue, &dscd, &tmpswapchain);
		if (FAILED(hr))
			throw(L"dxgiFactory->CreateSwapChain");
		swapChain = static_cast<IDXGISwapChain3*>(tmpswapchain);
		frameIndex = swapChain->GetCurrentBackBufferIndex();
		if (frameIndex >= framebuffercount || frameIndex < 0)
			throw(L"swapChain->GetCurrentBackBufferIndex");
		// ----------- Create Descriptor Heap -----------

		D3D12_DESCRIPTOR_HEAP_DESC ddhd = { D3D12_DESCRIPTOR_HEAP_TYPE_RTV, framebuffercount,
											D3D12_DESCRIPTOR_HEAP_FLAG_NONE};
		hr = device->CreateDescriptorHeap(&ddhd, IID_PPV_ARGS(&rtvDescHeap));
		if (FAILED(hr))
			throw(L"device->CreateDescriptorHeap");

		// ----------- Get RTV Descriptor Size -----------

		rtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// ----------- Get Handle to First CPU Descriptor -----------

		CD3DX12_CPU_DESCRIPTOR_HANDLE hRtv (rtvDescHeap->GetCPUDescriptorHandleForHeapStart());

		for (int i = 0; i < framebuffercount; i++)
		{
			// -----------Create an RTV for Each Buffer -----------
			
			hr = swapChain->GetBuffer(i, IID_PPV_ARGS(renderTargets + i));
			if (FAILED(hr))
				throw(L"swapChain->GetBuffer");
			device->CreateRenderTargetView(renderTargets[i], nullptr, hRtv);
			hRtv.Offset(1, rtvDescSize);

			// ----------- Create Command Allocators -----------

			hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdAllocator + i));
			if (FAILED(hr))
				throw(L"device->CreateCommandAllocator");
			
			// ---------- Create Fences -----------

			hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence + i));
			if (FAILED(hr))
				throw(L"device->CreateFence");
			fenceValue[i] = 0;
		}

		// ----------- Create Fence Event -----------

		fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (fenceEvent == nullptr)
			throw(L"CreateEvent");

		// ----------- Create Command List -----------

		hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator[0], NULL, IID_PPV_ARGS(&cmdList));
		if (FAILED(hr))
			throw(L"device->CreateCommandList");
		cmdList->Close();

		return true;
	}
	catch (LPCTSTR funcname)
	{
		tstring message(L"Failed at DXClass::InitD3D : ");
		message.append(funcname);
		MessageBox(WndClass.hWnd, message.c_str(), L"Failed", MB_OK | MB_ICONERROR);
		return false;
	}
	catch (tstring message)
	{
		MessageBox(WndClass.hWnd, message.c_str(), L"Failed", MB_OK | MB_ICONERROR);
		return false;
	}
}

void DXClass::Update()
{
}

bool DXClass::UpdatePipeLine()
{
	try
	{
		HRESULT hr;
		if (!WaitForPreviousFrame())
			return false;
		// ----------- Resetting Command Allocator and Command List -----------

		hr = cmdAllocator[frameIndex]->Reset();
		if (FAILED(hr))
			throw(L"commandAllocator[frameIndex]->Reset");
		hr = cmdList->Reset(cmdAllocator[frameIndex], NULL);
		if (FAILED(hr))
			throw(L"commandList->Reset");
		
		// ----------- Recording commands with command list -----------

		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex],
									 D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		CD3DX12_CPU_DESCRIPTOR_HANDLE hRtv(rtvDescHeap->GetCPUDescriptorHandleForHeapStart(),
										   frameIndex, rtvDescSize);
		cmdList->OMSetRenderTargets(1, &hRtv, FALSE, nullptr);
		const float clearcolor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		cmdList->ClearRenderTargetView(hRtv, clearcolor, 0, nullptr);
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex],
									 D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		hr = cmdList->Close();
		if (FAILED(hr))
			throw(L"commandList->Close");
		return true;
	}
	catch (LPCTSTR funcname)
	{
		tstring message(L"Failed at DXClass::UpdatePipeLine : ");
		message.append(funcname);
		MessageBox(WndClass.hWnd, message.c_str(), L"Failed", MB_OK | MB_ICONERROR);
		DestroyWindow(WndClass.hWnd);
		return false;
	}
}

bool DXClass::Render()
{
	try
	{
		HRESULT hr;
		if (!UpdatePipeLine())
			return false;
		ID3D12CommandList* ppCmdLists[] = { cmdList };
		cmdQueue->ExecuteCommandLists(_countof(ppCmdLists), ppCmdLists);
		hr = cmdQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
		if (FAILED(hr))
			throw(L"cmdQueue->Signal(...)");
		hr = swapChain->Present(0, 0);
		if (FAILED(hr))
			throw(L"swapChain->Present(...)");
		return true;
	}
	catch (LPCTSTR funcname)
	{
		tstring message(L"Failed at DXClass::Render : ");
		message.append(funcname);
		MessageBox(WndClass.hWnd, message.c_str(), L"Failed", MB_OK | MB_ICONERROR);
		DestroyWindow(WndClass.hWnd);
		return false;
	}
}

void DXClass::Cleanup()
{
	if (fenceEvent != nullptr)
		CloseHandle(fenceEvent);
	for (int i = 0; i < framebuffercount; i++)
	{
		frameIndex = i;
		WaitForPreviousFrame();
	}
	if(swapChain != nullptr)
	{
		BOOL fs = FALSE;
			swapChain->GetFullscreenState(&fs, NULL);
			if (fs)
				swapChain->SetFullscreenState(FALSE, NULL);
	}
	SAFE_RELEASE(device);
	SAFE_RELEASE(swapChain);
	SAFE_RELEASE(cmdQueue);
	SAFE_RELEASE(rtvDescHeap);
	SAFE_RELEASE(cmdList);
	for (int i = 0; i < framebuffercount; i++)
	{
		SAFE_RELEASE(renderTargets[i]);
		SAFE_RELEASE(cmdAllocator[i]);
		SAFE_RELEASE(fence[i]);
	}
	return;
}

bool DXClass::WaitForPreviousFrame()
{
	try
	{
		if (swapChain == nullptr)
			return true;
		HRESULT hr;
		frameIndex = swapChain->GetCurrentBackBufferIndex();
		if (frameIndex >= framebuffercount || frameIndex < 0)
			throw(L"swapChain->GetCurrentBackBufferIndex");
		if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
		{
			hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
			if (FAILED(hr))
				throw(L"fence[frameIndex]->SetEventOnCompletion");
			WaitForSingleObject(fenceEvent, INFINITE);
		}
		fenceValue[frameIndex]++;
		return true;
	}
	catch (LPCTSTR funcname)
	{
		tstring message(L"Failed at DXClass::WaitForPreviousFrame : ");
		message.append(funcname);
		MessageBox(WndClass.hWnd, message.c_str(), L"Failed", MB_OK | MB_ICONERROR);
		return false;
	}
}
