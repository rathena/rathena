#!/bin/bash

#title          :script
#description    :this script only demonstrate usage of ls-config 
#author         :Łukasz A. Grabowski <www@lucas.net.pl>
#date           :20130928
#version        :1.0.3
#notes          :This only read one value from configuration file
#               :this script MUST be run from subdirectory of dir
#               :where ls-config (bin) and lslib-core are stored
#               :to place script in other places You must reconfigure paths
#bash_version   :4.2.37(1)-release
#copywrite      :Copyright (C) 2013 Łukasz A. Grabowski
#license        :This program is free software: you can redistribute 
#               :it and/or modify it under the terms of the GNU General
#               :Public License as published by the Free Software
#               :Foundation, either version 2 of the License or 
#               :any later version.
#               :
#               :This program is distributed in the hope that it will
#               :be useful, but WITHOUT ANY WARRANTY; without even the
#               :implied warranty of MERCHANTABILITY or FITNESS FOR
#               :A PARTICULAR PURPOSE. See the GNU General Public
#               :License for more details.
#               :
#               :You should have received a copy of the GNU General
#               :Public License along with this program. If not, see 
#               :http://www.gnu.org/licenses/.
#=======================================================================


#set app flag
LS_EXEC=1

#set configuration directory
CFGD="./"

#source bash library to manipulate config
source ../lslib-core

#path direcrories onlny for this sample
PACD="../"
LIBD=""
#end path

#read data from configuration file
TEST=$(cfg_f_g "config" "info")
ERR="$?"

#show data
echo "Info value: $TEST"
echo "Reading error: $ERR"

#show other method of reading value:

#output space and info
echo ""
echo "Reading using binary:"

#read data from configuration file
TEST=$(${PACD}ls-config -f config --get="info" -vq)
ERR="$?"

#show data
echo "Info value: $TEST"
echo "Reading error: $ERR"

exit 0;
