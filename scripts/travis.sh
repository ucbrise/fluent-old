#! /usr/bin/env bash

set -euo pipefail

main() {
    # Remove the build directory just in case there's something there already.
    rm -rf build/"$TEST_TYPE";

    # Build the code.
    ./scripts/build.sh "$TEST_TYPE" 1;

    # Run all the unit tests.
    (cd build/"$TEST_TYPE" && ctest -L UNITTEST);

    # Run all the benchmarks.
    ./scripts/run_benchmarks.sh "$TEST_TYPE"
}

main
