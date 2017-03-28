# Fluent: An Asynchronous Dataflow Framework
Fluent = [Bloom][bloom_paper] + C++.

## Getting Started
There are three things you'll have to install for yourself:

1. A C++ compiler that supports C++14. If you're trying to install an
   up-to-date g++ on linux, [this tutorial][gpp_tutorial] might be helpful. If
   you're tying to install an up-to-date clang++ on linux, [this
   site](http://apt.llvm.org/) might be useful.
2. [CMake](https://cmake.org/download) version 3.0 or better.
3. [PostgreSQL and libpq](https://www.postgresql.org/download/).

Once you have these installed, then just build the code! The build process will
install and build the rest of the dependencies for you!

```bash
# Building
./scripts/build_debug.sh   # build the code in debug mode
./scripts/build_release.sh # build the code in release mode

# Testing
./build/ra/ra_all_test     # run a test
(cd build && make test)    # run all the tests

# Run a chat server listening on port 8000.
./build/examples/chat/example_chat_server 'tcp://*:8000'

# Run a chat client on port 8001 with nickname 'zardoz'
./build/examples/chat/example_chat_client \
    'tcp://0.0.0.0:8000' 'tcp://0.0.0.0:8001' 'zardoz'
```

## Tour
| Directory                  | Description                             |
| -------------------------- | --------------------------------------- |
| [`common`](src/common)     | Helpful functions, macros, and whatnot. |
| [`testing`](src/testing)   | Testing utilities.                      |
| [`zmq_util`](src/zmq_util) | ZeroMQ utilities.                       |
| [`ra`](src/ra)             | Relational algebra implementation.      |
| [`fluent`](src/fluent)     | Main fluent code.                       |
| [`examples`](src/examples) | Example fluent programs.                |

[bloom_paper]: https://scholar.google.com/scholar?cluster=9165311711752272482
[gpp_tutorial]: http://scholtyssek.org/blog/2015/06/11/install-gcc-with-c14-support-on-ubuntumint
