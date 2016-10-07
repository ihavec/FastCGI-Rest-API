# FastCGI Rest API written in C
## Goals
This library can be used to write fastcgi rest apis with mariadb (mysql) and libcurl support.

## Example how to run the application

Nginx:
```
upstream backend {
	ip_hash;

	server localhost:3000;
	server localhost:3001;
	server localhost:3002;
	server localhost:3003;
	server localhost:3004;
}

server {
	server_name api.example.com;

	listen       443;

	ssl    on;
	ssl_certificate    /etc/nginx/ssl/default.crt;
	ssl_certificate_key    /etc/nginx/ssl/default.key;


	location / {
		fastcgi_pass backend;
		fastcgi_param  SCRIPT_FILENAME  /scripts$fastcgi_script_name;
		fastcgi_param  GATEWAY_INTERFACE  CGI/1.1;
		fastcgi_param  SERVER_SOFTWARE    nginx;
		fastcgi_param  QUERY_STRING       $query_string;
		fastcgi_param  REQUEST_METHOD     $request_method;
		fastcgi_param  CONTENT_TYPE       $content_type;
		fastcgi_param  CONTENT_LENGTH     $content_length;
		fastcgi_param  SCRIPT_FILENAME    $document_root$fastcgi_script_name;
		fastcgi_param  SCRIPT_NAME        $fastcgi_script_name;
		fastcgi_param  REQUEST_URI        $request_uri;
		fastcgi_param  DOCUMENT_URI       $document_uri;
		fastcgi_param  DOCUMENT_ROOT      $document_root;
		fastcgi_param  SERVER_PROTOCOL    $server_protocol;
		fastcgi_param  REMOTE_ADDR        $remote_addr;
		fastcgi_param  REMOTE_PORT        $remote_port;
		fastcgi_param  SERVER_ADDR        $server_addr;
		fastcgi_param  SERVER_PORT        $server_port;
		fastcgi_param  SERVER_NAME        $server_name;
	}
}
```
Spawning the processes for this nginx config:
```bash
spawn-fcgi -p 3000 -n /path/to/your/binary
spawn-fcgi -p 3001 -n /path/to/your/binary
spawn-fcgi -p 3002 -n /path/to/your/binary
spawn-fcgi -p 3003 -n /path/to/your/binary
spawn-fcgi -p 3004 -n /path/to/your/binary
```

## Example how to test your app with a docker container
To build the app in the container just run `./build.sh`. If you want to use another development machine just use `make`. You must have curl-dev, mariadb-dev, fcgi-dev installed on the development machine if you won't be using the docker container, else the container takes care of all that...
## Docker container for easier local libfra development
Build and run with:
```bash
docker-compose build
docker-compose up
```
Restart with:
```bash
docker-compose restart
```
Delete with:
```bash
docker-compose down
```

After docker fills up your disk run:
```bash
docker rmi $(docker images --filter="dangling=true" -q --no-trunc)
```
to save yourself :)

After you have enough of the "docker way" run:
```bash
docker exec -ti fra-server ash
```
to access the ash console of your container (beware no changes will be saved between rebuilds... !!!)

