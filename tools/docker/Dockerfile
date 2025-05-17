ARG UBUNTU_VERSION=22.04 # lts
ARG BUILDER_CONFIGURE="--enable-packetver=20220328"

FROM ubuntu:$UBUNTU_VERSION AS build

ARG DOCKER=1
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y git make libmariadb-dev libmariadb-dev-compat gcc g++ zlib1g-dev libpcre3-dev \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -pv rathena

WORKDIR /rathena

COPY . /rathena

RUN ./configure $BUILDER_CONFIGURE
RUN make clean server
RUN chmod a+x login-server \
    && chmod a+x char-server \
    && chmod a+x map-server \
    && chmod a+x web-server


###
# Login Server
###

FROM build AS login-server

CMD ["./login-server"]


###
# Char Server
###

FROM build AS char-server

CMD ["./char-server"]


###
# Map Server
###

FROM build AS map-server

CMD ["./map-server"]


###
# Web Server
###

FROM build AS web-server

CMD ["./web-server"]
