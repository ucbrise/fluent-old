# Fluent [![Build Status](https://travis-ci.org/ucbrise/fluent.svg?branch=master)](https://travis-ci.org/ucbrise/fluent)

## Getting Started
There are a handful of things you'll have to install for yourself including
clang, g++, cmake, boost, etc. See
[`scripts/install_dependencies.sh`](scripts/install_dependencies.sh) if you're
on Ubuntu and
[`scripts/install_dependencies_osx.sh`](scripts/install_dependencies_osx.sh) if
your on macOS.

Once you have these installed, then just build the code! The build process will
install and build the rest of the dependencies for you!

```bash
# Building
export CC=clang CXX=clang++  # Use clang++
export CC=gcc CXX=g++        # OR, use g++
./scripts/build.sh Debug     # build the code in debug mode
./scripts/build.sh Release   # build the code in release mode
./scripts/build.sh Debug 4   # build the code in debug mode with 4 cores
./scripts/build.sh Release 4 # build the code in release mode with 4 cores

# Testing
./build/Debug/bin/ra_logical_cross_test # run a test
./build/Debug/bin/common_macros_test    # run another test
(cd build/Debug && ctest -L UNITTEST)   # run all the tests

# Benchmarking
./build/Debug/bin/ra_physical_iterable_bench # run a benchmark
(cd build/Debug && ctest -L BENCHMARK)       # run all the benchmarks

# Generating Tags
./scripts/generate_tags.sh

# Everything
./scripts/build.sh Debug && \
    ./scripts/generate_tags.sh && \
    (cd build/Debug && make test)
```

## Design Documents
For more detailed documentation describing Fluent, refer to the following
documents:

- [Fluent and lineage overview.][doc_lineage_overview]
- [Black box lineage design.][doc_black_boxes]

[gpp_tutorial]: http://scholtyssek.org/blog/2015/06/11/install-gcc-with-c14-support-on-ubuntumint
[doc_lineage_overview]: https://docs.google.com/document/d/1ykhcDQv8h9Eiymt47N7kx7oWlAmRbRsuA7EMZTIndNs/edit?usp=sharing
[doc_black_boxes]: https://docs.google.com/document/d/1bEMB0LiDQlCbGVSf2t2vo7RvJc3Dnh1WfRHTPsV0ehE/edit?usp=sharing
