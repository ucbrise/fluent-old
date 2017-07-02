#! /usr/bin/env bash

set -euo pipefail

main() {
    # Remove the build directory just in case there's something there already.
    rm -rf build/"$TEST_TYPE";

    # Build the code. Sometimes, building a fluent program can take a really,
    # really long time. This triggers travis to fail because the compiler runs
    # for quite a while with no ouput. We use the travis_wait command [1] to
    # prevent travis from prematurely killing our jobs.
    #
    # [1]: https://goo.gl/XAqkJg
    travis_wait 60 ./scripts/build.sh "$TEST_TYPE" 1;

    # Run all the unit tests.
    (cd build/"$TEST_TYPE" && ctest -L UNITTEST);

    # Run all the benchmarks.
    ./scripts/run_benchmarks.sh "$TEST_TYPE"
}

main
