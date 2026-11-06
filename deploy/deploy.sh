#!/bin/bash
# Aethera - deploy/redeploy from this git checkout.
# Usage: sudo bash deploy/deploy.sh [IP_PUBLIQUE]
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"

if [ ! -f .env ]; then
  PUBLIC_IP="${1:-$(curl -4 -s https://ifconfig.me)}"
  GM_PASS="$(head -c 200 /dev/urandom | tr -dc 'A-Za-z0-9' | head -c 12)"
  DB_PASS="$(head -c 200 /dev/urandom | tr -dc 'A-Za-z0-9' | head -c 20)"
  cat > .env <<EOF
PUBLIC_IP=$PUBLIC_IP
PACKETVER=20250716
DB_PASS=$DB_PASS
GM_PASS=$GM_PASS
EOF
  chmod 600 .env
  echo ">> .env généré (compte GM: bastien / $GM_PASS)"
else
  echo ">> .env existant conservé"
fi

echo ">> Build de l'image (5 à 15 min)..."
docker compose build
docker compose up -d

echo ">> Attente du map-server..."
until docker compose logs map 2>/dev/null | grep -q "Server is 'ready'"; do sleep 5; done

# shellcheck disable=SC1091
source .env
cat <<EOF

=========================================
 Aethera est en ligne
 IP client  : $PUBLIC_IP:6900
 Identifiants : voir deploy/.env (GM_PASS, DB_PASS)
 Pare-feu Hetzner : ouvrir TCP 6900, 6121, 5121
=========================================
EOF
