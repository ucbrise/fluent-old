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
    # if [[ ! -z $(ccm list) ]]; then
        # ccm remove
    # fi
    # ccm create test_cluster --install-dir=$HOME/cassandra -n 3 -s
    cqlsh -f src/examples/cassandra/reset_database.cql
    #
    # first.

    tmux new-window -n "cassandra_benchmark"
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
    bin="./build/Release/bin"
    lineagedb_config="localhost 5432 vagrant vagrant vagrant"
    replica_addrs="$addr8000 $addr8001 $addr8002"

    server="$bin/examples_cassandra_server"
    client_getter="$bin/examples_cassandra_benchmark_client_getter"
    client_setter="$bin/examples_cassandra_benchmark_client_setter"

    tmux send-keys -t 6 "psql -f scripts/reset_database.sql" C-m

    tmux send-keys -t 0 "sleep 2" C-m
    tmux send-keys -t 1 "sleep 2" C-m
    tmux send-keys -t 2 "sleep 2" C-m
    tmux send-keys -t 0 "$glog $server $lineagedb_config $addr1 1000 0 $replica_addrs" C-m
    tmux send-keys -t 1 "$glog $server $lineagedb_config $addr2 1000 1 $replica_addrs" C-m
    tmux send-keys -t 2 "$glog $server $lineagedb_config $addr3 1000 2 $replica_addrs" C-m

    tmux send-keys -t 3 "sleep 3.2" C-m
    tmux send-keys -t 4 "sleep 3.1" C-m
    tmux send-keys -t 5 "sleep 3.0" C-m
    tmux send-keys -t 3 "$glog $client_getter $lineagedb_config $addr8000 $addr9000 "$1" ZIPFIAN 10" C-m
    tmux send-keys -t 4 "$glog $client_setter $lineagedb_config $addr8001 $addr9001 "$1" ZIPFIAN 10" C-m
    tmux send-keys -t 5 "$glog $client_setter $lineagedb_config $addr8002 $addr9002 "$1" ZIPFIAN 10" C-m
}

main "$@"
