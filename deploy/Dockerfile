# Build context is the repo root (see docker-compose.yml: context: ..)
# so this always compiles the checked-out commit of this fork, not a
# separate git clone.
FROM ubuntu:24.04 AS build
RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates make gcc g++ zlib1g-dev libpcre2-dev libmariadb-dev libmariadb-dev-compat \
    && rm -rf /var/lib/apt/lists/*
ARG PACKETVER=20250716
COPY . /rathena
WORKDIR /rathena
RUN ./configure --enable-prere=yes --enable-64bit --enable-packetver=${PACKETVER} \
 && make server -j2

FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends \
      libmariadb3 zlib1g libpcre2-8-0 mariadb-client netcat-openbsd \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /rathena /rathena
COPY deploy/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
WORKDIR /rathena
ENTRYPOINT ["/entrypoint.sh"]
