#! /usr/bin/env bash

set -euo pipefail

main() {
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          -Hsrc -Bbuild/xcode -G Xcode
}

main "$@"
