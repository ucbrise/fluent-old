#! /usr/bin/env bash

set -euo pipefail

main() {
    set -x

    echo "$CC"
    echo "$CXX"
    echo "$PATH"
    which clang
    which clang++
    clang --version
    clang++ --version
    ls -l $(which clang)
    ls -l $(which clang++)
    ls /usr/bin
    ls /usr/local
    ls /usr/local/bin

    set +x
}

main
