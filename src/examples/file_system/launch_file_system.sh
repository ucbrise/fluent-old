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
    tmux split-window -v
    tmux select-pane -t 0
    tmux split-window -v

    glog="GLOG_logtostderr=1"
    db_config="vagrant vagrant vagrant"
    server_addr="tcp://0.0.0.0:9000"
    fluent_server_addr="tcp://0.0.0.0:9001"
    lineage_file="src/examples/file_system/lineage.py"

    server=""
    server+="$glog "
    server+="./build/bin/examples_file_system_server "
    server+="$server_addr"

    fluent_server=""
    fluent_server+="$glog "
    fluent_server+="./build/bin/examples_file_system_fluent_server "
    fluent_server+="$db_config "
    fluent_server+="$server_addr "
    fluent_server+="$fluent_server_addr "
    fluent_server+="$lineage_file "

    fluent_client=""
    fluent_client+="$glog "
    fluent_client+="./build/bin/examples_file_system_fluent_client "
    fluent_client+="$db_config "
    fluent_client+="$fluent_server_addr"

    tmux send-keys -t 4 "psql -f scripts/reset_database.sql" C-m

    tmux send-keys -t 0 "sleep 0.5" C-m
    tmux send-keys -t 0 "$server" C-m

    tmux send-keys -t 1 "sleep 1" C-m
    tmux send-keys -t 1 "$fluent_server" C-m

    tmux send-keys -t 2 "sleep 2" C-m
    tmux send-keys -t 2 "$fluent_client tcp://0.0.0.0:9002" C-m

    tmux send-keys -t 3 "sleep 2" C-m
    tmux send-keys -t 3 "$fluent_client tcp://0.0.0.0:9003" C-m
}

main
