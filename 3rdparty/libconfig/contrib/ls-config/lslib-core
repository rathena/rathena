#!/bin/bash

#title          :lslib-core
#description    :core library for LS scripts 
#author         :Łukasz A. Grabowski <www@lucas.net.pl>
#date           :20130928
#version        :1.0.3
#notes          : 
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

#################
# Configuration #
#################

PACD="/usr/share/ls" #main directory for LS scripts
LIBD="lib"           #library dir for LS scripts

#######################
# Predefined and init #
#######################

if [ -z "$LS_EXEC" ]; then
    echo "This script are only for inclusion in LS packet scripts. Don't use it itself." 1>&2
    exit 1
fi

##############################
# configuration read / write #
##############################

#get configuration
# cfg_g PATH
cfg_g() {
    local PTH=""
    if [ $# -gt 0 ]; then
	local PTH="$1";
    fi;
    local DAT
    local ERR
    DAT="$($PACD/$LIBD/ls-config -f "$CFGFN" -qv --get="$PTH")"
    ERR=$?
    echo "$DAT"
    return $ERR
}

#get configuration type
# cfg_t PATH
cfg_t() {
    local PTH=""
    if [ $# -gt 0 ]; then
	local PTH="$1";
    fi;
    local DAT
    local ERR
    DAT="$($PACD/$LIBD/ls-config -f "$CFGFN" -qt --get="$PTH")"
    ERR=$?
    echo "$DAT"
    return $ERR
}

#get configuration items count
# cfg_c PATH
cfg_c() {
    local PTH=""
    if [ $# -gt 0 ]; then
	local PTH="$1";
    fi;
    local DAT
    local ERR
    DAT="$($PACD/$LIBD/ls-config -f "$CFGFN" -qc --get="$PTH")"
    ERR=$?
    echo "$DAT"
    return $ERR
}

#set configuration
# cfg_s PATH DATA [TYPE=string]
cfg_s() {
    local PTH=""
    if [ $# -gt 0 ]; then
	local PTH="$1";
    fi;
    local DATA=""
    if [ $# -gt 1 ]; then
	local DATA="$2";
    fi;
    local TYPE="string"
    if [ $# -gt 2 ]; then
	local TYPE="$3";
    fi;
    local DAT
    local ERR
    DAT="$($PACD/$LIBD/ls-config -f "$CFGFN" -q --set="$PTH" --data="$DATA" --type="$TYPE")"
    ERR=$?
    echo "$DAT"
    return $ERR
}

#remove configuration
# cfg_u PATH 
cfg_u() {
    local PTH=""
    if [ $# -gt 0 ]; then
	local PTH="$1";
    fi;
    local DAT
    local ERR
    DAT="$($PACD/$LIBD/ls-config -f "$CFGFN" -q --set="$PTH" --unset)"
    ERR=$?
    return $ERR
}

cfg_f_g() {
    local BCFN="$CFGFN"
    local EX
    CFGFN="$1"
    shift
    cfg_g "$@"
    EX=$?
    CFGFN="$BCFN"
    return $EX
}

cfg_f_t() {
    local BCFN="$CFGFN"
    local EX
    CFGFN="$1"
    shift
    cfg_t "$@"
    EX=$?
    CFGFN="$BCFN"
    return $EX
}

cfg_f_c() {
    local BCFN="$CFGFN"
    local EX
    CFGFN="$1"
    shift
    cfg_c "$@"
    EX=$?
    CFGFN="$BCFN"
    return $EX
}

cfg_f_s() {
    local BCFN="$CFGFN"
    local EX
    CFGFN="$1"
    shift
    cfg_s "$@"
    EX=$?
    CFGFN="$BCFN"
    return $EX
}

cfg_f_u() {
    local BCFN="$CFGFN"
    local EX
    CFGFN="$1"
    shift
    cfg_u "$@"
    EX=$?
    CFGFN="$BCFN"
    return $EX
}


######################
# base variable init #
######################

#package name
SCRFN="$(basename "$0")"

#configuriation directory and file
if [ -z "$CFGD" ]; then
    CFGD="/etc/ls"
fi;
CFGFN="$CFGD/$SCRFN"


