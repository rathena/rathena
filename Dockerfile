FROM ubuntu:focal as build-base
RUN apt-get update
RUN apt-get -y install build-essential
RUN apt-get -y install git make libmysqlclient-dev zlib1g-dev libpcre3-dev

from build-base as build
WORKDIR /rathena
COPY / .
RUN ./configure
RUN make server
RUN chmod a+x login-server && chmod a+x char-server && chmod a+x map-server

from ubuntu:focal as runtime-base
RUN apt-get update
RUN apt-get -y install libmysqlclient21

from runtime-base as runtime
WORKDIR /rathena
COPY --from=build /rathena .
ENTRYPOINT ["./athena-start", "watch"]