#!/bin/bashsh
#source var/function

set -o allexport && source .env && set +o allexport

./login-server --run-once
./char-server --run-once
./map-server --run-once
