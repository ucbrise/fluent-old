#! /usr/bin/env bash

set -euo pipefail

main() {
    # http://stackoverflow.com/a/13864829/3187068
    if [[ -z ${TMUX+dummy} ]]; then
        echo "ERROR: you must run this script while in tmux."
        return 1
    fi

    session="$(tmux display-message -p '#S')"
    tmux new-window -t "$session" -n "kvs"
    tmux split-window -h
    tmux select-pane -t 0
    tmux split-window -v

    glog="GLOG_logtostderr=1"
    db_config="vagrant vagrant vagrant"
    server_addr="tcp://0.0.0.0:9000"

    lineage_server=""
    lineage_server+="$glog "
    lineage_server+="./build/Debug/bin/examples_fluent_kvs_server "
    lineage_server+="$db_config "
    lineage_server+="$server_addr"

    lineage_client=""
    lineage_client+="$glog "
    lineage_client+="./build/Debug/bin/examples_fluent_kvs_client "
    lineage_client+="$db_config "
    lineage_client+="$server_addr"

    tmux send-keys -t 2 "psql -f scripts/reset_database.sql" C-m

    tmux send-keys -t 0 "sleep 0.5" C-m
    tmux send-keys -t 0 "$lineage_server" C-m

    tmux send-keys -t 1 "sleep 1" C-m
    tmux send-keys -t 1 "$lineage_client tcp://0.0.0.0:9001" C-m
}

main
