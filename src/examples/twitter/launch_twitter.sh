#! /usr/bin/env bash

set -euo pipefail

main() {
    # http://stackoverflow.com/a/13864829/3187068
    if [[ -z ${TMUX+dummy} ]]; then
        echo "ERROR: you must run this script while in tmux."
        return 1
    fi

    session="$(tmux display-message -p '#S')"
    tmux new-window -t "$session" -n "twitter"
    tmux split-window -h
    tmux split-window -h
    tmux split-window -h
    tmux split-window -h
    tmux split-window -h
    tmux next-layout
    tmux next-layout
    tmux next-layout
    tmux next-layout
    tmux next-layout

    glog="GLOG_logtostderr=1"
    db_config="vagrant vagrant vagrant"
    redis_addr="tcp://0.0.0.0:8000"
    server_addr="tcp://0.0.0.0:9000"

    redis=""
    redis+="$glog "
    redis+="./build/bin/examples_black_boxes_redis_server "
    redis+="$db_config "
    redis+="0.0.0.0 6379 "
    redis+="$redis_addr"

    server=""
    server+="$glog "
    server+="./build/bin/examples_twitter_server "
    server+="$db_config "
    server+="$redis_addr "
    server+="$server_addr"

    client=""
    client+="$glog "
    client+="./build/bin/examples_twitter_client "
    client+="$db_config "
    client+="$server_addr"

    tmux send-keys -t 0 "psql -f scripts/reset_database.sql" C-m

    tmux send-keys -t 2 "sleep 0.5" C-m
    tmux send-keys -t 2 "$redis" C-m

    tmux send-keys -t 4 "sleep 1" C-m
    tmux send-keys -t 4 "$server" C-m

    tmux send-keys -t 1 "sleep 1.5" C-m
    tmux send-keys -t 1 "$client tcp://0.0.0.0:9001 alice" C-m

    tmux send-keys -t 3 "sleep 1.5" C-m
    tmux send-keys -t 3 "$client tcp://0.0.0.0:9002 bob" C-m

    tmux send-keys -t 5 "sleep 1.5" C-m
    tmux send-keys -t 5 "$client tcp://0.0.0.0:9003 joe" C-m
}

main
