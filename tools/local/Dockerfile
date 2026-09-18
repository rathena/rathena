FROM alpine:3.23
RUN apk add --no-cache bash make gcc g++ zlib-dev mariadb-dev linux-headers
WORKDIR /rathena
COPY . .
ARG PACKETVER=20211103
ARG BUILD_JOBS=2
RUN sh ./configure --enable-packetver=${PACKETVER} --disable-debug --enable-64bit \
    && make -j${BUILD_JOBS} login char map import
COPY tools/local/conf/ conf/import/
CMD ["./login-server"]
