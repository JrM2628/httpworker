// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "ReflectiveLoader.h"
#include "../common/mainloop.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    HANDLE hThread;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)mainloop, NULL, 0, NULL);
        if(hThread)
            WaitForSingleObject(hThread, INFINITE);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

