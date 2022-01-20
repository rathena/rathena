#!/usr/bin/env bash

set -x

# input environment variables:
# OS: the operating system
# CXX_: the compiler version. eg, g++-9 or clang++-6.0
# BT: the build type
# VG: whether to install valgrind
# ARM: whether to arm cross-compiler and emulator
# GITHUB_WORKFLOW: when run from github
# API: whether to install swig
# CMANY: whether to install cmany



#-------------------------------------------------------------------------------

function c4_install_test_requirements()
{
    os=$1
    case "$os" in
        ubuntu*)
            c4_install_test_requirements_ubuntu
            return 0
            ;;
        macos*)
            c4_install_test_requirements_macos
            return 0
            ;;
        win*)
            c4_install_test_requirements_windows
            return 0
            ;;
        *)
            return 0
            ;;
    esac
}

function c4_install_test_requirements_windows()
{
    if [ "$CMANY" == "ON" ] ; then
        pip install cmany
    fi
    if [ "$API" == "ON" ] ; then
        choco install swig
        which swig
    fi
    # ensure chocolatey does not override cmake's cpack
    which cpack
    choco_cpack="/c/ProgramData/Chocolatey/bin/cpack.exe"
    if [ -f $choco_cpack ] ; then
        newname=$(echo $choco_cpack | sed 's:cpack:choco-cpack:')
        mv -vf $choco_cpack $newname
    fi
    which cpack
}

function c4_install_test_requirements_macos()
{
    if [ "$CMANY" == "ON" ] ; then
        sudo pip3 install cmany
    fi
}

function c4_install_test_requirements_ubuntu()
{
    APT_PKG=""  # all
    PIP_PKG=""
    c4_gather_test_requirements_ubuntu
    echo "apt packages: $APT_PKG"
    echo "pip packages: $PIP_PKG"
    c4_install_test_requirements_ubuntu_impl
    echo 'INSTALL COMPLETE!'
}


function c4_install_all_possible_requirements_ubuntu()
{
    export CXX_=all
    export BT=Coverage
    APT_PKG=""  # all
    PIP_PKG=""
    sudo dpkg --add-architecture i386
    c4_gather_test_requirements_ubuntu
    _c4_add_arm_compilers
    echo "apt packages: $APT_PKG"
    echo "pip packages: $PIP_PKG"
    c4_install_test_requirements_ubuntu_impl
    echo 'INSTALL COMPLETE!'
}


function c4_gather_test_requirements_ubuntu()
{
    if [ "$GITHUB_WORKFLOW" != "" ] ; then
        sudo dpkg --add-architecture i386
    else
        _add_apt build-essential
        _add_apt cmake
    fi

    _add_apt linux-libc-dev:i386
    _add_apt libc6:i386
    _add_apt libc6-dev:i386
    _add_apt libc6-dbg:i386
    _c4_addlibcxx

    _c4_gather_compilers "$CXX_"

    _add_apt python3-setuptools
    _add_apt python3-pip

    #_add_apt iwyu
    #_add_apt cppcheck
    #_add_pip cpplint
    # oclint?
    if [ "$VG" == "ON" ] ; then
        _add_apt valgrind
    fi

    if [ "$BT" == "Coverage" ]; then
        _add_apt lcov
        _add_apt libffi-dev
        _add_apt libssl-dev
        _add_pip requests[security]
        _add_pip pyopenssl
        _add_pip ndg-httpsclient
        _add_pip pyasn1
        _add_pip cpp-coveralls
    fi

    if [ "$CMANY" != "" ] ; then
        _add_pip cmany
    fi

    case "$CXX_" in
        arm*)
            _c4_add_arm_compilers
            ;;
    esac
}


function c4_install_test_requirements_ubuntu_impl()
{
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key 2>/dev/null | sudo apt-key add -
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | sudo apt-key add -
    sudo -E apt-add-repository --yes 'deb https://apt.kitware.com/ubuntu/ bionic main'
    sudo -E add-apt-repository --yes ppa:ubuntu-toolchain-r/test

    if [ "$APT_PKG" != "" ] ; then
        #sudo -E apt-get clean
        sudo -E apt-get update
        sudo -E apt-get install -y --force-yes $APT_PKG
    fi

    if [ "$PIP_PKG" != "" ]; then
        sudo pip3 install $PIP_PKG
    fi
}


#-------------------------------------------------------------------------------

function _c4_add_arm_compilers()
{
    # this is going to be deprecated:
    # https://askubuntu.com/questions/1243252/how-to-install-arm-none-eabi-gdb-on-ubuntu-20-04-lts-focal-fossa
    sudo -E add-apt-repository --yes ppa:team-gcc-arm-embedded/ppa

    _add_apt gcc-arm-embedded
    _add_apt g++-arm-linux-gnueabihf
    _add_apt g++-multilib-arm-linux-gnueabihf
    _add_apt qemu
}


function _c4_gather_compilers()
{
    cxx=$1
    case $cxx in
        g++-11     ) _c4_addgcc 11 ;;
        g++-10     ) _c4_addgcc 10 ;;
        g++-9      ) _c4_addgcc 9  ;;
        g++-8      ) _c4_addgcc 8  ;;
        g++-7      ) _c4_addgcc 7  ;;
        g++-6      ) _c4_addgcc 6  ;;
        g++-5      ) _c4_addgcc 5  ;;
        #g++-4.9    ) _c4_addgcc 4.9 ;;  # https://askubuntu.com/questions/1036108/install-gcc-4-9-at-ubuntu-18-04
        clang++-12 ) _c4_addclang 12  ;;
        clang++-11 ) _c4_addclang 11  ;;
        clang++-10 ) _c4_addclang 10  ;;
        clang++-9  ) _c4_addclang 9   ;;
        clang++-8  ) _c4_addclang 8   ;;
        clang++-7  ) _c4_addclang 7   ;;
        clang++-6.0) _c4_addclang 6.0 ;;
        clang++-5.0) _c4_addclang 5.0 ;;
        clang++-4.0) _c4_addclang 4.0 ;;
        clang++-3.9) _c4_addclang 3.9 ;;
        all)
            all="g++-11 g++-10 g++-9 g++-8 g++-7 g++-6 g++-5 clang++-12 clang++-11 clang++-10 clang++-9 clang++-8 clang++-7 clang++-6.0 clang++-5.0 clang++-4.0 clang++-3.9"
            echo "installing all compilers: $all"
            for cxx in $all ; do
                _c4_gather_compilers $cxx
            done
            ;;
        "")
            # use default compiler
            ;;
        arm*)
            ;;
        *)
            echo "unknown compiler: $cxx"
            exit 1
            ;;
    esac
}

# add a gcc compiler
function _c4_addgcc()
{
    gccversion=$1
    case $gccversion in
        5 )
            _add_apt gcc-5 "deb http://dk.archive.ubuntu.com/ubuntu/ xenial main"
            _add_apt gcc-5 "deb http://dk.archive.ubuntu.com/ubuntu/ xenial universe"
            ;;
        *)
            ;;
    esac
    _add_apt g++-$gccversion
    _add_apt g++-$gccversion-multilib
    _add_apt libstdc++-$gccversion-dev
    _add_apt lib32stdc++-$gccversion-dev
}

# add a clang compiler
function _c4_addclang()
{
    clversion=$1
    case $clversion in
        # in 18.04, clang9 and later require PPAs
        9 | 10 | 11 | 12 )
            _add_apt clang-$clversion "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-$clversion main"
            # libstdc++ is required
            _c4_addgcc 11
            _c4_addgcc 10
            _c4_addgcc 9
            ;;
        *)
            _add_apt clang-$clversion
            ;;
    esac
    _add_apt g++-multilib  # this is required for 32 bit https://askubuntu.com/questions/1057341/unable-to-find-stl-headers-in-ubuntu-18-04
    _add_apt clang-tidy-$clversion
}

# add libc++
function _c4_addlibcxx()
{
    _add_apt libc++1
    _add_apt libc++abi-dev
    _add_apt libc++-dev
    _add_apt libc++1:i386
    _add_apt libc++abi-dev:i386
    _add_apt libc++-dev:i386
}


#-------------------------------------------------------------------------------

# add a pip package to the list
function _add_pip()
{
    pkgs=$*
    PIP_PKG="$PIP_PKG $pkgs"
    echo "adding to pip packages: $pkgs"
}

# add a debian package to the list
function _add_apt()
{
    pkgs=$1
    sourceslist=$2
    APT_PKG="$APT_PKG $pkgs"
    echo "adding to apt packages: $pkgs"
    _add_src "$sourceslist" "# for packages: $pkgs"
}

# add an apt source
function _add_src()
{
    sourceslist=$1
    comment=$2
    if [ ! -z "$sourceslist" ] ; then
        echo "adding apt source: $sourceslist"
        sudo bash -c "cat >> /etc/apt/sources.list <<EOF
$comment
$sourceslist
EOF"
        #cat /etc/apt/sources.list
    fi
}
