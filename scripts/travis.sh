#! /usr/bin/env bash

set -euo pipefail

main() {
    rm -rf build && \
    ./scripts/build.sh "$TEST_TYPE" 1 && \
    (cd build && ctest -L UNITTEST) && \
    ./scripts/run_benchmarks.sh "$TEST_TYPE"
}

main
