#include "D3DHook.h"
#include "PolyHook\PolyHook.h"

bool Edo::IsD3D9Loaded()
{
	HMODULE module;
	return GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, TEXT("d3d9"), &module) != 0;
}

bool Edo::IsD3D11Loaded()
{
	HMODULE module;
	return GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, TEXT("d3d11"), &module) != 0;
}

void Edo::CreateD3D9DummyDevice(HWND window, LPDIRECT3DDEVICE9* ppDevice)
{
	//Create IDirect3d* object.
	LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	//Get the adapter display mode.
	D3DDISPLAYMODE d3ddm;
	ZeroMemory(&d3ddm, sizeof(D3DDISPLAYMODE));
	ThrowIfFailed(pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm));

	//Get present parameters.
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = d3ddm.Format;

	//Create device.
	HRESULT res = pD3D->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX,
		&d3dpp, ppDevice);

	if (res != D3D_OK)
	{
		if (res == D3DERR_DEVICELOST)
			throw std::exception("D3DERR_DEVICELOST");

		if (res == D3DERR_INVALIDCALL)
			throw std::exception("D3DERR_INVALIDCALL");

		if (res == D3DERR_NOTAVAILABLE)
			throw std::exception("D3DERR_NOTAVAILABLE");

		if (res == D3DERR_OUTOFVIDEOMEMORY)
			throw std::exception("D3DERR_OUTOFVIDEOMEMORY");

		ThrowIfFailed(res);
	}

	//Release.
	pD3D->Release();
}

uintptr_t* Edo::GetD3D9DeviceVTable(HWND window)
{
	IDirect3DDevice9* pDevice;

	//Create dummy device.
	CreateD3D9DummyDevice(window, &pDevice);

	//Get the VTable.
	uintptr_t* vTable = (uintptr_t*)*(uintptr_t*)pDevice;

	//Release.
	pDevice->Release();

	return vTable;
}

uintptr_t Edo::GetD3D9DeviceVFunc(HWND window, D3D9DeviceFunction offset)
{
	return GetD3D9DeviceVTable(window)[(int)offset];
}

void Edo::CreateD3D11DummyDeviceAndSwapchain(HWND window, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext, IDXGISwapChain** ppSwapchain)
{
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapchainDesc;
	ZeroMemory(&swapchainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	//Set swap chain description.
	swapchainDesc.BufferCount = 1;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapchainDesc.OutputWindow = window;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.Windowed = TRUE;

	//Create device, swapchain and devicecontext.
	ThrowIfFailed(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapchainDesc, ppSwapchain, ppDevice, NULL, ppContext));
}

void Edo::GetD3D11DeviceAndSwapchainVTable(HWND window, uintptr_t** ppDeviceVTable, uintptr_t** ppContextVTable, uintptr_t** ppSwapchainVTable)
{
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pContext;
	IDXGISwapChain* pSwapchain;

	//Create dummy devices.
	CreateD3D11DummyDeviceAndSwapchain(window, &pDevice, &pContext, &pSwapchain);

	//Set the vtable pointers
	if(ppDeviceVTable)
		*ppDeviceVTable = (uintptr_t*)*(uintptr_t*)pDevice;

	if(ppContextVTable)
		*ppContextVTable = (uintptr_t*)*(uintptr_t*)pContext;

	if(ppSwapchainVTable)
		*ppSwapchainVTable = (uintptr_t*)*(uintptr_t*)pSwapchain;

	//Cleanup.
	pDevice->Release();
	pContext->Release();
	pSwapchain->Release();
}

uintptr_t Edo::GetD3D11DeviceVFunc(HWND window, D3D11DeviceFunction offset)
{
	uintptr_t* vtable;
	GetD3D11DeviceAndSwapchainVTable(window, &vtable, NULL, NULL);

	return vtable[(int)offset];
}

uintptr_t Edo::GetD3D11DeviceContextVFunc(HWND window, D3D11DeviceContextFunction offset)
{
	uintptr_t* vtable;
	GetD3D11DeviceAndSwapchainVTable(window, NULL, &vtable, NULL);

	return vtable[(int)offset];
}

uintptr_t Edo::GetDXGISwapchainVFunc(HWND window, DXGISwapchainFunction offset)
{
	uintptr_t* vtable;
	GetD3D11DeviceAndSwapchainVTable(window, NULL, NULL, &vtable);

	return vtable[(int)offset];
}