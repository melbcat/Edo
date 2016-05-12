#ifndef _D3DHOOK_H
#define _D3DHOOK_H

#include <edo_embedded.h>
#include "PolyHook\PolyHook.h"

//DirectX
//#include <d3dx9.h> //Is this even used?
#include <d3d9.h>
#include <d3d11.h>

//#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3d11.lib")

namespace Edo
{
	//Function signature declarations.
	//Start with D3D9
	using hkD3D9DeviceEndScene = HRESULT (__stdcall*)(LPDIRECT3DDEVICE9);

	//Then D3D11
	using hkDXGISwapchainPresent = void (__stdcall*)(IDXGISwapChain*, UINT, UINT);
	using hkD3D11DeviceContextDrawIndexed = void (__stdcall*)(ID3D11DeviceContext*, UINT, UINT, INT);

	//Function enumerations.
	enum class D3D9DeviceFunction : int
	{
		EndScene = 42,
	};

	enum class D3D11DeviceFunction : int
	{

	};
	
	enum class D3D11DeviceContextFunction : int
	{
		DrawIndexed = 12,
	};

	enum class DXGISwapchainFunction : int
	{
		Present = 8,
	};

	//Functions that check whether or not specific directx versions are in use.
	bool IsD3D9Loaded();
	bool IsD3D11Loaded();

	//Creates a D3D9 dummy device for use in vtable hooking.
	void CreateD3D9DummyDevice(HWND window, LPDIRECT3DDEVICE9* ppDevice);

	//Shortcut methods.
	uintptr_t* GetD3D9DeviceVTable(HWND window);
	uintptr_t GetD3D9DeviceVFunc(HWND window, D3D9DeviceFunction offset);

	//Creates a D3D11 dummy device, context and swapchain for use in vtable hooking.
	void CreateD3D11DummyDeviceAndSwapchain(HWND window, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext, IDXGISwapChain** ppSwapchain);
	void GetD3D11DeviceAndSwapchainVTable(HWND window, uintptr_t** ppDeviceVTable, uintptr_t** ppContextVTable, uintptr_t** ppSwapchainVTable);

	uintptr_t GetD3D11DeviceVFunc(HWND window, D3D11DeviceFunction offset);
	uintptr_t GetD3D11DeviceContextVFunc(HWND window, D3D11DeviceContextFunction offset);
	uintptr_t GetDXGISwapchainVFunc(HWND window, DXGISwapchainFunction offset);
}
#endif