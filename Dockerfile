FROM ubuntu:23.10 as build

RUN apt update -y && \
  apt install -y \
    make gcc zlib1g-dev libpcre3-dev build-essential

WORKDIR /rathena

RUN pwd
RUN ls -lanh

ADD ./* ./

RUN ls -lanh

RUN ./configure --enable-packetver=20211103 && make clean && make server
