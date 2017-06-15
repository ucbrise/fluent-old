#! /usr/bin/env bash

set -euo pipefail

usage() {
    echo "run_benchmarks.sh <Debug|Release>"
    exit 1
}

main() {
	if [[ $# -ne 1 && $# -ne 2 ]]; then
		echo "Wrong count" $#
        usage
    fi

    build_type="$1"
    if [[ "$build_type" != "Debug" && "$build_type" != "Release" ]]; then
        usage
    fi

    first=true
    for bench in build/${build_type}/bin/*_bench; do
        if "$first"; then
            "$bench" --benchmark_format=csv 2>/dev/null
            first=false
        else
            "$bench" --benchmark_format=csv 2>/dev/null | tail -n +2
        fi
    done
}

main "$@"
