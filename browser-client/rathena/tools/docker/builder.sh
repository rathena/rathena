if [ ! -f /rathena/login-server ]; then
  export runBuild=1;
elif [ ! -f /rathena/char-server ]; then
  export runBuild=1;
elif [ ! -f /rathena/map-server ]; then
  export runBuild=1;
elif [ ! -f /rathena/web-server ]; then
  export runBuild=1;
else
  export runBuild=0;
fi

if [ "${runBuild}" -eq "1" ]; then
  ### checking that ./configure has ran by looking for make file
  if [ ! -f /rathena/make ]; then
    echo "Warning: ./configure will be executed with provided values";
    echo "Make sure you have set the variables you want in the docker-compose.yml file";
    echo $BUILDER_CONFIGURE
    ./configure $BUILDER_CONFIGURE
  fi
  
  mkdir -p 3rdparty/libconfig/obj  
  mkdir -p 3rdparty/rapidyaml/obj/src/c4/yml  
  mkdir -p 3rdparty/rapidyaml/obj/ext/c4core/src/c4  
  mkdir -p 3rdparty/yaml-cpp/obj/src  
  mkdir -p 3rdparty/yaml-cpp/obj/src/contrib 
  mkdir -p 3rdparty/httplib/obj  
  mkdir -p src/common/obj  
  mkdir -p src/login/obj  
  mkdir -p src/char/obj  
  mkdir -p src/map/obj 
  mkdir -p src/web/obj  
  mkdir -p src/tool/obj_all

  make clean server;
fi