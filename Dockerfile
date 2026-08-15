FROM alpine:3.23 AS builder

WORKDIR /rathena

RUN apk add --no-cache \
        bash \
        build-base \
        cmake \
        linux-headers \
        mariadb-dev \
        zlib-dev

COPY . .

# Keep rAthena's checked-in default PACKETVER for the initial bootable image.
# Client/server packet alignment will be configured in a later step.
# Build rapidyaml serially first because its generated Makefile can race while
# creating nested object directories under a parallel top-level build.
RUN ./configure \
    && make -j1 rapidyaml \
    && make -j"$(nproc)" login char map import


FROM alpine:3.23 AS runtime

RUN apk add --no-cache \
        ca-certificates \
        libstdc++ \
        mariadb-connector-c \
        netcat-openbsd \
        tini \
        zlib \
    && addgroup -S rathena \
    && adduser -S -D -H -G rathena rathena \
    && mkdir -p /rathena/log \
    && chown -R rathena:rathena /rathena

WORKDIR /rathena

USER rathena

STOPSIGNAL SIGTERM

ENTRYPOINT ["/sbin/tini", "--"]


FROM runtime AS login

COPY --from=builder --chown=rathena:rathena /rathena/login-server ./login-server
COPY --from=builder --chown=rathena:rathena /rathena/conf ./conf

EXPOSE 6900

CMD ["./login-server"]


FROM runtime AS char

COPY --from=builder --chown=rathena:rathena /rathena/char-server ./char-server
COPY --from=builder --chown=rathena:rathena /rathena/conf ./conf
COPY --from=builder --chown=rathena:rathena /rathena/db ./db

EXPOSE 6121

CMD ["./char-server"]


FROM runtime AS map

COPY --from=builder --chown=rathena:rathena /rathena/map-server ./map-server
COPY --from=builder --chown=rathena:rathena /rathena/conf ./conf
COPY --from=builder --chown=rathena:rathena /rathena/db ./db
COPY --from=builder --chown=rathena:rathena /rathena/npc ./npc

EXPOSE 5121

CMD ["./map-server"]
