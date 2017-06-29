#! /usr/bin/env bash

set -euo pipefail

main() {
    # http://stackoverflow.com/a/13864829/3187068
    if [[ -z ${TMUX+dummy} ]]; then
        echo "ERROR: you must run this script while in tmux."
        return 1
    fi

    session="$(tmux display-message -p '#S')"
    tmux new-window -t "$session" -n "dkvs"
    tmux split-window -h
    tmux split-window -h
    tmux split-window -h
    tmux split-window -h
    tmux select-pane -t 0
    tmux split-window -h
    tmux split-window -h
    tmux split-window -h
    tmux split-window -h
    tmux split-window -v
    tmux select-layout tiled
    tmux select-pane -t 9

    glog="GLOG_logtostderr=1 GLOG_v=1"
    bin="./build/Debug/bin"

    server="$bin/examples_distributed_key_value_store_server"
    addr9000="0.0.0.0:9000"
    addr9001="0.0.0.0:9001"
    addr9002="0.0.0.0:9002"

    fluent_server="$bin/examples_distributed_key_value_store_fluent_server"
    lineagedb_config="vagrant vagrant vagrant"
    addr8000="tcp://0.0.0.0:8000"
    addr8001="tcp://0.0.0.0:8001"
    addr8002="tcp://0.0.0.0:8002"
    replica_addrs="$addr8000 $addr8001 $addr8002"

    fluent_client="$bin/examples_distributed_key_value_store_fluent_client"
    addr10000="tcp://0.0.0.0:10000"
    addr10001="tcp://0.0.0.0:10001"
    addr10002="tcp://0.0.0.0:10002"

    tmux send-keys -t 9 "psql -f scripts/reset_database.sql" C-m

    tmux send-keys -t 0 "sleep 1" C-m
    tmux send-keys -t 1 "sleep 1" C-m
    tmux send-keys -t 2 "sleep 1" C-m
    tmux send-keys -t 0 "$glog $server $addr9000 $addr9001 $addr9002" C-m
    tmux send-keys -t 1 "$glog $server $addr9001 $addr9002 $addr9000" C-m
    tmux send-keys -t 2 "$glog $server $addr9002 $addr9000 $addr9001" C-m

    tmux send-keys -t 3 "sleep 2" C-m
    tmux send-keys -t 4 "sleep 2" C-m
    tmux send-keys -t 5 "sleep 2" C-m
    tmux send-keys -t 3 "$glog $fluent_server $lineagedb_config $addr9000 0 $replica_addrs" C-m
    tmux send-keys -t 4 "$glog $fluent_server $lineagedb_config $addr9001 1 $replica_addrs" C-m
    tmux send-keys -t 5 "$glog $fluent_server $lineagedb_config $addr9002 2 $replica_addrs" C-m

    tmux send-keys -t 6 "sleep 3" C-m
    tmux send-keys -t 7 "sleep 3" C-m
    tmux send-keys -t 8 "sleep 3" C-m
    tmux send-keys -t 6 "$glog $fluent_client $lineagedb_config $addr8000 $addr10000" C-m
    tmux send-keys -t 7 "$glog $fluent_client $lineagedb_config $addr8001 $addr10001" C-m
    tmux send-keys -t 8 "$glog $fluent_client $lineagedb_config $addr8002 $addr10002" C-m
}

main
