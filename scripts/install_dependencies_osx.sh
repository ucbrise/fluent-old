#! /usr/bin/env bash

set -euo pipefail

install_gtools() {
	brew install gflags glog protobuf
}
main() {
    set -x
    brew update
    brew install boost
    brew install cereal
    brew install zmq
    brew install cereal
    brew install redis
    install_gtools
    set +x
}

main
