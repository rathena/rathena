FROM alpine:3.11
WORKDIR /rathena
RUN apk add --no-cache wget git cmake make gcc g++ gdb zlib-dev mariadb-dev ca-certificates linux-headers bash valgrind netcat-openbsd
RUN wget https://raw.githubusercontent.com/eficode/wait-for/v2.2.2/wait-for -O /bin/wait-for && chmod +x /bin/wait-for
ENTRYPOINT [  ]
