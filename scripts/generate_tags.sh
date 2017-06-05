#! /usr/bin/env bash

set -euo pipefail

main() {
    cd src
    ctags --c++-kinds=+p --fields=+iaS --extra=+q --language-force=C++ -R .
}

main
