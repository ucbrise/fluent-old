#! /usr/bin/env bash

set -euo pipefail

main() {
    first=true
    for bench in build/bin/*_bench; do
        if "$first"; then
            "$bench" --benchmark_format=csv 2>/dev/null
            first=false
        else
            "$bench" --benchmark_format=csv 2>/dev/null | tail -n +2
        fi
    done
}

main
