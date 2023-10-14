FROM nginx
COPY proxy.conf /etc/nginx/conf.d/default.conf
RUN mkdir -p /etc/nginx/certs \
&& openssl req -x509 -nodes -days 365 -newkey rsa:4096 -subj "/C=US" -keyout /etc/nginx/certs/key.key -out /etc/nginx/certs/cert.crt 