// dllmain.cpp : Defines the entry point for the DLL application.
#include "edo_scanner.h"
#include "D3DHook.h"

PLH::VFuncDetour d9;
PLH::VFuncDetour d11;
static bool first = true;

void mbox_debug(std::string msg)
{
	MessageBoxA(NULL, msg.c_str(), "DEBUG", MB_OK);
}

HRESULT __stdcall hkD3D9DeviceEndSceneImpl(LPDIRECT3DDEVICE9 pDevice)
{
	if (first)
	{
		first = false;
		mbox_debug("EndSceneImpl was hit!");
	}

	return d9.GetOriginal<Edo::hkD3D9DeviceEndScene>()(pDevice);
}

void __stdcall hkD3D11DeviceContextDrawIndexedImpl(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndex, INT BaseVertex)
{
	mbox_debug("DrawIndexedImpl was hit!");
}

void main(HMODULE module)
{
	//Setup D9
	try
	{
		//auto start = std::chrono::high_resolution_clock::now();
		//while ((std::chrono::high_resolution_clock::now() - start) < std::chrono::milliseconds(100))
		//{

		//}

		auto d3DeviceTable = Edo::GetD3D9DeviceVTable(GetForegroundWindow());
		d9.SetupHook((BYTE**)d3DeviceTable, (int)Edo::D3D9DeviceFunction::EndScene, (BYTE*)&hkD3D9DeviceEndSceneImpl);
		mbox_debug("SetupHook() completed.");
		if (!d9.Hook())
			mbox_debug("Failure.");
		mbox_debug("Hook() completed.");

		auto start = std::chrono::high_resolution_clock::now();
		while ((std::chrono::high_resolution_clock::now() - start) < std::chrono::seconds(3))
		{

		}

		mbox_debug("Finished.");

		d9.UnHook();
	}
	catch (std::exception& e)
	{
		mbox_debug(e.what());
	}

	FreeLibraryAndExitThread(module, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&main, hModule, NULL, NULL);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}