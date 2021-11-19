Proxies traffic to C2 

To generate cert/key:
.\openssl.exe req -x509 -nodes -days 365 -newkey rsa:4096 -keyout key.pem -out cert.crt