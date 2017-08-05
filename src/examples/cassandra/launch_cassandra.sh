#! /usr/bin/env bash

set -euo pipefail

main() {
    # http://stackoverflow.com/a/13864829/3187068
    if [[ -z ${TMUX+dummy} ]]; then
        echo "ERROR: you must run this script while in tmux."
        return 1
    fi

    # Make sure to run
    #
    #   ccm create test_cluster --install-dir=$HOME/cassandra -n 3 -s
    #   cqlsh -f reset_database.cql
    #
    # first.

    tmux new-window -n "cassandra"
    tmux split-window -v -p 99
    tmux split-window -v -p 99
    tmux split-window -v -p 99
    tmux split-window -v -p 99
    tmux split-window -v -p 99
    tmux split-window -v -p 99
    tmux select-layout tiled
    tmux select-pane -t 6

    addr1="127.0.0.1"
    addr2="127.0.0.2"
    addr3="127.0.0.3"
    addr8000="tcp://0.0.0.0:8000"
    addr8001="tcp://0.0.0.0:8001"
    addr8002="tcp://0.0.0.0:8002"
    addr9000="tcp://0.0.0.0:9000"
    addr9001="tcp://0.0.0.0:9001"
    addr9002="tcp://0.0.0.0:9002"

    glog="GLOG_logtostderr=1"
    bin="./build/Debug/bin"
    lineagedb_config="localhost 5432 vagrant vagrant vagrant"
    replica_addrs="$addr8000 $addr8001 $addr8002"

    server="$bin/examples_cassandra_server"
    client="$bin/examples_cassandra_client"

    tmux send-keys -t 6 "psql -f scripts/reset_database.sql" C-m

    tmux send-keys -t 0 "sleep 2" C-m
    tmux send-keys -t 1 "sleep 2" C-m
    tmux send-keys -t 2 "sleep 2" C-m
    tmux send-keys -t 0 "$glog $server $lineagedb_config $addr1 0 $replica_addrs" C-m
    tmux send-keys -t 1 "$glog $server $lineagedb_config $addr2 1 $replica_addrs" C-m
    tmux send-keys -t 2 "$glog $server $lineagedb_config $addr3 2 $replica_addrs" C-m

    tmux send-keys -t 3 "sleep 3" C-m
    tmux send-keys -t 4 "sleep 3" C-m
    tmux send-keys -t 5 "sleep 3" C-m
    tmux send-keys -t 3 "$glog $client $lineagedb_config $addr8000 $addr9000" C-m
    tmux send-keys -t 4 "$glog $client $lineagedb_config $addr8001 $addr9001" C-m
    tmux send-keys -t 5 "$glog $client $lineagedb_config $addr8002 $addr9002" C-m
}

main
