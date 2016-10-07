FROM alpine:latest
RUN apk add --update mariadb-client && apk add --update curl && apk add --update fcgi && apk add --update spawn-fcgi

# this line is only useful for development
RUN apk add --update g++ && apk add --update make && apk add --update mariadb-dev && apk add --update curl-dev && apk add --update fcgi-dev

RUN rm -rf /var/cache/apk/*
ENTRYPOINT ["/fra-server/container_run.sh"]
