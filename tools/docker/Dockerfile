FROM alpine:3.23 AS builder

WORKDIR /rathena

RUN apk add --no-cache \
    wget \
    git \
    cmake \
    make \
    gcc \
    g++ \
    zlib-dev \
    mariadb-dev \
    linux-headers \
    bash

# Optional helper (used at runtime, so downloaded here and copied later)
RUN wget https://raw.githubusercontent.com/eficode/wait-for/v2.2.4/wait-for \
    -O /bin/wait-for && chmod +x /bin/wait-for

COPY . .
RUN ls

ARG BUILDER_CONFIGURE

RUN ./configure ${BUILDER_CONFIGURE} \
 && make clean \
 && make server -j$(nproc)


FROM alpine:3.23

WORKDIR /rathena

RUN apk add --no-cache \
    ca-certificates \
    zlib \
    mariadb-connector-c \
    bash \
    libstdc++ \
    libgcc \
    netcat-openbsd

COPY --from=builder /rathena/map-server /rathena/map-server
COPY --from=builder /rathena/char-server /rathena/char-server
COPY --from=builder /rathena/login-server /rathena/login-server
COPY --from=builder /rathena/web-server /rathena/web-server

COPY --from=builder /bin/wait-for /bin/wait-for

RUN chmod +x \
    /rathena/map-server \
    /rathena/char-server \
    /rathena/login-server \
    /rathena/web-server \
    /bin/wait-for

ENTRYPOINT []
