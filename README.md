# 476final
A standard HTTP Command and Control server with agents for both Windows and Linux targets.  

Capabilities include: command execution, file upload/download, process list, system information list, screenshot (Windows only), process kill  
## Packages
### 476test (Windows Client)
WinInet used for C2 communication. 
### bot_server (C2)
Flask HTTP server. Uses SQLite database for backend storage.

Contains dockerfile for simple deployment (docker build/docker run -p 5000:5000)

Simple front-end for a cleaner red team experience. 
![image](https://user-images.githubusercontent.com/16729369/133910072-6327415b-8aed-4283-b6c7-31411f524f2b.png)

### linux (Linux Client)
LibCurl is used for C2 communication. 
sudo ./a.out > /dev/null 2>&1 &

ZIP Password: infected
