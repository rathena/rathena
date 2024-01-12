# Runsheet

## Steps

```sh
# Install dependencies
apt install -y git make gcc libmysqlclient-dev zlib1g-dev libpcre3-dev build-essential mysql-client

# apt install mysql-server

# Install Docker
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
```

### Setup mysql:8.2 with docker-compose.yaml

```yaml
# Use root/example as user/password credentials
version: '3.9'

services:
  ro2024db:
    image: mysql:8.2
    # NOTE: use of "mysql_native_password" is not recommended: https://dev.mysql.com/doc/refman/8.0/en/upgrading-from-previous-series.html#upgrade-caching-sha2-password
    # (this is just an example, not intended to be a production configuration)
    command: --default-authentication-plugin=mysql_native_password
    restart: always
    ports:
      - 3306:3306
    environment:
      MYSQL_ROOT_PASSWORD: quidapchai
      MYSQL_DATABASE: ro2024
      MYSQL_USER: ro2024username
      MYSQL_PASSWORD: ro2024password
```

```bash
docker compose up -d
```

### Import SQL files to docker DB container

```bash
cd sql-files/

cat *.sql > all-files.sql

docker exec -i $(docker ps --filter "name=ro2024" --format "{{.ID}}") mysql -p'quidapchai' ro2024 < all-files.sql
docker exec -i $(docker ps --filter "name=rathena_db" --format "{{.ID}}") mysql -p'quidapchai' ro2024 < tbro_custom_1.sql
```

## Setup rathena server

```bash
./configure --enable-packetver=20211103

make clean && make server
```

## Setup .conf/import/inter_conf.txt

```txt
login_server_id: root
login_server_pw: quidapchai
login_server_db: ro2024

ipban_db_id: root
ipban_db_pw: quidapchai
ipban_db_db: ro2024

// MySQL Character server
char_server_id: root
char_server_pw: quidapchai
char_server_db: ro2024

// MySQL Map Server
map_server_id: root
map_server_pw: quidapchai
map_server_db: ro2024

// MySQL Web Server
web_server_id: root
web_server_pw: quidapchai
web_server_db: ro2024

// MySQL Log Database
log_db_id: root
log_db_pw: quidapchai
log_db_db: ro2024
```
