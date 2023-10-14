Proxies traffic to C2 

To generate certificate: 
mkdir -p /etc/nginx/certs && openssl req -x509 -nodes -days 365 -newkey rsa:4096 -subj "/C=US" -keyout /etc/nginx/certs/key.key -out /etc/nginx/certs/cert.crt 