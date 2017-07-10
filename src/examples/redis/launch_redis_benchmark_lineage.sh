#! /usr/bin/env bash

set -euo pipefail

main() {
    # http://stackoverflow.com/a/13864829/3187068
    if [[ -z ${TMUX+dummy} ]]; then
        echo "ERROR: you must run this script while in tmux."
        return 1
    fi

    # Make sure to start a redis server first.

    session="$(tmux display-message -p '#S')"
    tmux new-window -t "$session" -n "redis"
    tmux split-window -h
    tmux split-window -h
    tmux select-layout even-vertical
    tmux select-pane -t 1

    glog="GLOG_logtostderr=1"
    db_config="localhost 5432 vagrant vagrant vagrant"
    redis_config="localhost 6379"
    server_addr="tcp://0.0.0.0:9000"
    client_addr="tcp://0.0.0.0:9001"
    bindir="./build/Release/bin"
    server="$bindir/examples_redis_server_benchmark_lineage"
    client="$bindir/examples_redis_client_benchmark"

    tmux send-keys -t 2 "psql -f ./scripts/reset_database.sql" C-m

    tmux send-keys -t 0 "sleep 1" C-m
    tmux send-keys -t 0 "$glog $server $db_config $redis_config $server_addr" C-m

    tmux send-keys -t 1 "sleep 2" C-m
    tmux send-keys -t 1 "$glog $client $server_addr $client_addr joe" C-m
}

main
