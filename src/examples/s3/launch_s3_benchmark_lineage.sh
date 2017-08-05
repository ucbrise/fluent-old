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
    tmux split-window -v -p 99
    tmux select-layout even-vertical
    tmux select-pane -t 1

    glog="GLOG_logtostderr=1"
    db_config="localhost 5432 vagrant vagrant vagrant"
    server_addr="tcp://0.0.0.0:9000"
    client_addr="tcp://0.0.0.0:9001"
    bindir="./build/Release/bin"
    server="$bindir/examples_s3_server_benchmark_lineage"
    client="$bindir/examples_s3_client_benchmark"

    tmux send-keys -t 2 "psql -f ./scripts/reset_database.sql" C-m

    tmux send-keys -t 0 "sleep 0.5" C-m
    tmux send-keys -t 0 "$glog $server $db_config $server_addr" C-m

    tmux send-keys -t 1 "sleep 1" C-m
    tmux send-keys -t 1 "$glog $client $server_addr $client_addr" C-m
}

main
