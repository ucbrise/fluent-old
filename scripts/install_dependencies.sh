#! /usr/bin/env bash

set -euo pipefail

install_misc() {
    sudo apt-get install libtool
}

install_clang() {
    sudo bash -c "echo '' >> /etc/apt/sources.list"
    sudo bash -c "echo '# http://apt.llvm.org/' >> /etc/apt/sources.list"
    sudo bash -c "echo 'deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-4.0 main' >> /etc/apt/sources.list"
    sudo bash -c "echo 'deb-src http://apt.llvm.org/trusty/ llvm-toolchain-trusty-4.0 main' >> /etc/apt/sources.list"
    wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
    sudo apt-get -y update

    sudo apt-get install -y \
        clang-4.0 clang-format-4.0 clang-tidy-4.0 lldb-4.0
    sudo ln -s "$(which clang-4.0)" /usr/bin/clang
    sudo ln -s "$(which clang++-4.0)" /usr/bin/clang++
    sudo ln -s "$(which clang-format-4.0)" /usr/bin/clang-format
    sudo ln -s "$(which clang-tidy-4.0)" /usr/bin/clang-tidy
    sudo ln -s "$(which lldb-4.0)" /usr/bin/lldb
    sudo ln -s "$(which lldb-server-4.0)" /usr/bin/lldb-server
}

install_gpp() {
    sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
    sudo apt-get update
    sudo apt-get install -y g++-6
    sudo ln -sf "$(which /usr/bin/g++-6)" /usr/bin/g++
    sudo ln -sf "$(which /usr/bin/gcc-6)" /usr/bin/gcc
}

install_cmake() {
    wget 'https://cmake.org/files/v3.6/cmake-3.6.2-Linux-x86_64.sh'
    sh cmake-3.6.2-Linux-x86_64.sh --skip-license
    echo 'export PATH="$PATH:$HOME/cmake-3.6.2-Linux-x86_64/bin"' >> ~/.bash_path
}

install_postgres() {
    sudo bash -c 'echo "deb http://apt.postgresql.org/pub/repos/apt/ trusty-pgdg main" > /etc/apt/sources.list.d/pgdg.list'
    wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | sudo apt-key add -
    sudo apt-get update
    sudo apt-get install -y postgresql-9.6 postgresql-server-dev-9.6 python-dev
}

install_redis() {
    sudo apt-get install -y libhiredis-dev libev-dev
}

install_cassandra_deps() {
    sudo apt-get install -y libuv-dev libssl-dev
}

main() {
    set -x
    sudo apt-get -y update
    install_misc
    install_clang
    install_gpp
    install_cmake
    install_redis
    install_cassandra_deps
    set +x
}

main
