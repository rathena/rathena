#!/usr/bin/env bash

set -x

# https://askubuntu.com/questions/735201/installing-clang-3-8-on-ubuntu-14-04-3
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -

done=$(grep C4STL /etc/apt/sources.list)
if [ -z "$done" ] ; then
    cat >> /etc/apt/sources.list <<EOF

# C4STL
# http://apt.llvm.org/
#deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.7 main
#deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.8 main
deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.9 main
deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-4.0 main
#deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-5.0 main
EOF
fi

sudo -E apt-get install -y software-properties-common python-software-properties
sudo -E add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo -E add-apt-repository -y ppa:george-edison55/cmake-3.x
sudo -E apt-get -yq update

sudo -E apt-get install -yq --force-yes \
     build-essential \
     cmake \
     g++-5 \
     g++-5-multilib \
     g++-6 \
     g++-6-multilib \
     g++-7 \
     g++-7-multilib \
     g++-8 \
     g++-8-multilib \
     g++-9 \
     g++-9-multilib \
     g++-10 \
     g++-10-multilib \
     g++-11 \
     g++-11-multilib \
     clang-3.7 \
     clang-3.8 \
     clang-3.9 \
     clang-4.0 \
     swig3.0 \
     libssl-dev \
     zlib1g-dev \
     libbz2-dev \
     libreadline-dev \
     libsqlite3-dev \
     wget \
     curl \
     llvm \
     libncurses5-dev \
     libncursesw5-dev \
     xz-utils \
     tk-dev \
     libffi-dev \
     liblzma-dev \
     python-openssl \
     git \
     python3 \
     python3-pip \
     python3-venv

sudo -E pip install cmany

exit 0
