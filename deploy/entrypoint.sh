#!/bin/bash
set -e
cd /rathena
DB="mysql -h db -uragnarok -p${DB_PASS} ragnarok"
cp -f /conf-import/*.txt conf/import/ 2>/dev/null || true

for k in login_server char_server map_server web_server log_db ipban_db; do
  echo "${k}_ip: db"; echo "${k}_pw: ${DB_PASS}"
done > conf/import/inter_conf.txt
printf 'server_name: Aethera\nlogin_ip: login\nchar_ip: %s\n' "$PUBLIC_IP" > conf/import/char_conf.txt
printf 'char_ip: char\nmap_ip: %s\n' "$PUBLIC_IP" > conf/import/map_conf.txt

until mysqladmin ping -h db -uragnarok -p"${DB_PASS}" --silent; do sleep 2; done

case "$1" in
  initdb)
    if $DB -e "SELECT 1 FROM login LIMIT 1" >/dev/null 2>&1; then echo "Base déjà initialisée"; exit 0; fi
    for f in main logs item_db item_db2 item_db_equip item_db_etc item_db_usable \
             mob_db mob_db2 mob_skill_db mob_skill_db2 roulette_default_data web; do
      echo "Import $f"; $DB < sql-files/$f.sql
    done
    $DB -e "INSERT INTO login (account_id,userid,user_pass,sex,email,group_id) VALUES (2000000,'bastien','${GM_PASS}','M','a@a.com',99);"
    ;;
  login) exec ./login-server ;;
  char)  until nc -z login 6900; do sleep 2; done; exec ./char-server ;;
  map)   until nc -z char 6121;  do sleep 2; done; exec ./map-server ;;
  *) exec "$@" ;;
esac
