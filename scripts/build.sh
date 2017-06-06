#! /usr/bin/env bash

set -euo pipefail

usage() {
    echo "build.sh <Debug|Release> [num_jobs]"
    exit 1
}

main() {
    if [[ $# -ne 1 && $# -ne 2 ]]; then
        usage
    fi

    build_type="$1"
    if [[ "$build_type" != "Debug" && "$build_type" != "Release" ]]; then
        usage
    fi

    if [[ $# -eq 1 ]]; then
        j=1
    elif [[ $# -eq 2 ]]; then
        j="$2"
    else
        usage
    fi

    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          -DCMAKE_BUILD_TYPE="$build_type" \
          -Hsrc -Bbuild
    cmake --build build -- -j "$j"
}

main "$@"
