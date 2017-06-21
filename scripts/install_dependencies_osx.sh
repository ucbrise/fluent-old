#! /usr/bin/env bash

set -euo pipefail

install_misc() {
    brew install libtool
}

install_cmake() {
    brew install cmake
}

install_redis() {
    brew install libhiredis libev
}

main() {
    set -x
    brew update
    install_misc
    install_cmake
    install_redis
    set +x
}

main
