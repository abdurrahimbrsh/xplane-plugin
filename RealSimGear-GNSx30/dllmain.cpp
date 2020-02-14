// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "util.h"

extern "C" __declspec(dllexport) void dummy_func() {}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		logMessage("DLL loaded");
		break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		logMessage("DLL unloading");
        break;
    }
    return TRUE;
}

