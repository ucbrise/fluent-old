#! /usr/bin/env bash

set -euo pipefail

main() {
    # http://stackoverflow.com/a/13864829/3187068
    if [[ -z ${TMUX+dummy} ]]; then
        echo "ERROR: you must run this script while in tmux."
        return 1
    fi

    tmux new-window -n "s3_benchmark"
    tmux split-window -v -p 99
    tmux select-layout even-vertical
    tmux select-pane -t 1

    glog="GLOG_logtostderr=1"
    server_addr="tcp://0.0.0.0:9000"
    client_addr="tcp://0.0.0.0:9001"
    bindir="./build/Release/bin"
    server="$bindir/examples_s3_server_benchmark"
    client="$bindir/examples_s3_client_benchmark"

    tmux send-keys -t 0 "sleep 0.5" C-m
    tmux send-keys -t 0 "$glog $server $server_addr" C-m

    tmux send-keys -t 1 "sleep 1" C-m
    tmux send-keys -t 1 "$glog $client $server_addr $client_addr" C-m
}

main
