L_SRV=login-server
C_SRV=char-server
M_SRV=map-server
W_SRV=web-server
INST_PATH=/opt
PKG=rathena
PKG_PATH="${INST_PATH}/${PKG}"

check_files() {
    for i in ${L_SRV} ${C_SRV} ${M_SRV} ${W_SRV}
    do
        if [ ! -f ./$i ]; then
            echo "$i does not exist... exiting..."
            exit 1;
        fi
    done
}

check_inst_right(){
    if [ ! -w "${INST_PATH}" ]; then echo "You must have sudo right to use this install (write/read permission in ${INST_PATH}/ )" && exit; fi
}

inst_launch_workaround(){
  if [ -d "${PKG_PATH}" ]; then
    if [ "$(pwd)" != "${PKG_PATH}" ]; then cd "${PKG_PATH}"; fi
  fi
}
