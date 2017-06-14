#! /usr/bin/env bash

set -euo pipefail

install_misc() {
    brew install libtool
}

# install_clang() {
#     sudo bash -c "echo '' >> /etc/apt/sources.list"
#     sudo bash -c "echo '# http://apt.llvm.org/' >> /etc/apt/sources.list"
#     sudo bash -c "echo 'deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-3.9 main' >> /etc/apt/sources.list"
#     sudo bash -c "echo 'deb-src http://apt.llvm.org/trusty/ llvm-toolchain-trusty-3.9 main' >> /etc/apt/sources.list"
#     wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
#     sudo apt-get -y update

#     sudo apt-get install -y \
#         clang-3.9 clang-format-3.9 clang-tidy-3.9 lldb-3.9
#     sudo ln -s "$(which clang-3.9)" /usr/bin/clang
#     sudo ln -s "$(which clang++-3.9)" /usr/bin/clang++
#     sudo ln -s "$(which clang-format-3.9)" /usr/bin/clang-format
#     sudo ln -s "$(which clang-tidy-3.9)" /usr/bin/clang-tidy
#     sudo ln -s "$(which lldb-3.9)" /usr/bin/lldb
#     sudo ln -s "$(which lldb-server-3.9)" /usr/bin/lldb-server
# }

# install_gpp() {
#     sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
#     sudo apt-get update
#     sudo apt-get install -y g++-6
#     sudo ln -sf "$(which /usr/bin/g++-6)" /usr/bin/g++
#     sudo ln -sf "$(which /usr/bin/gcc-6)" /usr/bin/gcc
# }

install_cmake() {
    brew install cmake
}

# install_postgres() {
#     sudo bash -c 'echo "deb http://apt.postgresql.org/pub/repos/apt/ trusty-pgdg main" > /etc/apt/sources.list.d/pgdg.list'
#     wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | sudo apt-key add -
#     sudo apt-get update
#     sudo apt-get install -y postgresql-9.6 postgresql-server-dev-9.6 python-dev
# }

install_redis() {
    brew install libhiredis libev
}

main() {
    set -x
    brew update
    install_misc
    # install_clang
    # install_gpp
    install_cmake
    install_redis
    set +x
}

main
