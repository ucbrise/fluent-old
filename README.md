# Fluent [![Build Status](https://travis-ci.org/ucbrise/fluent.svg?branch=master)](https://travis-ci.org/ucbrise/fluent)
Fluent = [Bloom][bloom_paper] + C++.

## Getting Started
There are a handful of things you'll have to install for yourself:

1. Some miscellaneous dependencies: `libtool`.
2. A C++ compiler that supports C++14. If you're trying to install an
   up-to-date g++ on linux, [this tutorial][gpp_tutorial] might be helpful. If
   you're tying to install an up-to-date clang++ on linux, [this
   site](http://apt.llvm.org/) might be useful.
3. [CMake](https://cmake.org/download) version 3.0 or better.
4. [PostgreSQL and libpq](https://www.postgresql.org/download/).
5. [Redis](https://redis.io/topics/quickstart), libhiredis (`sudo apt-get
   install libhiredis-dev`), and libev (`sudo apt-get install libev-dev`).

Once you have these installed, then just build the code! The build process will
install and build the rest of the dependencies for you!

```bash
# Building
./scripts/build.sh Debug     # build the code in debug mode
./scripts/build.sh Release   # build the code in release mode
./scripts/build.sh Debug 4   # build the code in debug mode with 4 cores
./scripts/build.sh Release 4 # build the code in release mode with 4 cores

# Testing
./build/bin/ra_logical_cross_test # run a test
./build/bin/common_macros_test    # run another test
(cd build && ctest -L UNITTEST)   # run all the tests

# Benchmarking
./build/bin/ra_physical_iterable_bench # run a benchmark
(cd build && ctest -L BENCHMARK)       # run all the benchmarks

# Generating Tags
./scripts/generate_tags.sh

# Everything
./scripts/build.sh Debug 4 && \
    ./scripts/generate_tags.sh && \
    (cd build && make test)

# Run a chat server listening on port 8000.
./build/bin/examples_chat_noop_server 'tcp://*:8000'

# Run a chat client on port 8001 with nickname "zardoz"
./build/bin/examples_chat_noop_client \
    tcp://0.0.0.0:8000 tcp://0.0.0.0:8001 zardoz
```

## Design Documents
For more detailed documentation describing Fluent, refer to the following
documents:

- [Fluent and lineage overview.][doc_lineage_overview]
- [Black box lineage design.][doc_black_boxes]

[bloom_paper]: https://scholar.google.com/scholar?cluster=9165311711752272482
[gpp_tutorial]: http://scholtyssek.org/blog/2015/06/11/install-gcc-with-c14-support-on-ubuntumint
[doc_lineage_overview]: https://docs.google.com/document/d/1ykhcDQv8h9Eiymt47N7kx7oWlAmRbRsuA7EMZTIndNs/edit?usp=sharing
[doc_black_boxes]: https://docs.google.com/document/d/1bEMB0LiDQlCbGVSf2t2vo7RvJc3Dnh1WfRHTPsV0ehE/edit?usp=sharing
