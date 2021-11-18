#include "util.h"

//Encodes string using key
std::string encode(std::string key, std::string clear) {
	std::string enc = "";
	char key_c;
	std::string enc_c;
	int placeholder;

	for (UINT i = 0; i < clear.length(); i++) {
		key_c = key[i % key.length()];
		placeholder = int(clear[i]) + int(key_c);
		enc_c = (placeholder % 127);
		enc.append(enc_c);
	}
	return enc;
}


//Decodes string using key
std::string decode(std::string key, std::string enc) {
	std::string dec = "";
	char key_c;
	std::string dec_c;
	int placeholder;

	for (UINT i = 0; i < enc.length(); i++) {
		key_c = key[i % key.length()];
		placeholder = 127 + int(enc[i]) - int(key_c);
		dec_c = (placeholder % 127);
		dec.append(dec_c);
	}
	return dec;
}