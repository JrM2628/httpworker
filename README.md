# HTTPworker
A standard HTTP Command and Control server with agents for both Windows and Linux targets.  

Capabilities include: command execution, file upload/download, process list, system information list, screenshot (Windows only), process kill  
## Packages
### Client
##### Builder
Searches the executable for the magic string "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" and replaces that with a multi-byte XOR-encoded configuration file (written in JSON)
##### Windows
WinInet used for C2 communication
##### Linux
LibCurl used for C2 communication

sudo ./a.out > /dev/null 2>&1 &

### Server
Flask HTTP server. Uses SQLite database for backend storage.

Contains docker-compose file for simplified deployment

Simple front-end for a cleaner red team experience (work in progress!) 
![image](https://user-images.githubusercontent.com/16729369/133910072-6327415b-8aed-4283-b6c7-31411f524f2b.png)


