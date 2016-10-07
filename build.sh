#!/bin/bash


if [ ! -f container_nginx_ssl/default.crt ] || [ ! -f container_nginx_ssl/default.key ] ; then
	openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout container_nginx_ssl/default.key -out container_nginx_ssl/default.crt
fi

docker-compose build

docker-compose up -d

docker exec -ti fra-server ash -c "cd /fra-server/example && make ${1}"
docker-compose restart

