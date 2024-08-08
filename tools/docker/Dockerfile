ARG ENABLE_PRERE="no"
ARG ENABLE_VIP="no"
ARG ENABLE_BUILDBOT="yes"
ARG ENABLE_LTO="yes"
ARG PACKETVER="20211103"

FROM alpine:3.11 as base-builder
WORKDIR /rathena
RUN apk add --no-cache make gcc g++ zlib-dev mariadb-dev linux-headers

FROM base-builder as dev
RUN apk add --no-cache git cmake gdb ca-certificates bash valgrind wget netcat-openbsd
RUN wget https://raw.githubusercontent.com/eficode/wait-for/v2.2.2/wait-for -O /bin/wait-for && chmod +x /bin/wait-for
ENTRYPOINT [ ]

FROM base-builder as builder
ARG ENABLE_PRERE
ARG ENABLE_VIP
ARG ENABLE_BUILDBOT
ARG ENABLE_LTO
COPY . .
RUN ./configure --enable-prere=${ENABLE_PRERE} --enable-vip=${ENABLE_VIP} --enable-buildbot=${ENABLE_BUILDBOT} --enable-lto=${ENABLE_LTO} --enable-packetver=${PACKETVER}
RUN make clean server

FROM alpine:3.11 as base-runtime
WORKDIR /rathena
RUN apk add --no-cache libgcc libstdc++ gcompat mariadb-connector-c
ENTRYPOINT ["/rathena/tools/docker/entrypoint.sh"]
CMD ["./athena-start", "watch"]

FROM base-runtime as runtime
COPY --from=builder /rathena /rathena
