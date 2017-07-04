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
    tmux split-window -h -p 99
    tmux split-window -h -p 99
    tmux split-window -h -p 99
    tmux split-window -h -p 99
    tmux split-window -h -p 99
    tmux select-layout tiled
    tmux select-pane -t 0

    glog="GLOG_logtostderr=1"
    bin="./build/Debug/bin"
    server="$bin/examples_distributed_kvs_server"
    client="$bin/examples_distributed_kvs_command_line_client"
    addr9000="0.0.0.0:9000"
    addr9001="0.0.0.0:9001"
    addr9002="0.0.0.0:9002"

    tmux send-keys -t 1 "$glog $server ${addr9000} ${addr9001} ${addr9002}" C-m
    tmux send-keys -t 3 "$glog $server ${addr9001} ${addr9002} ${addr9000}" C-m
    tmux send-keys -t 5 "$glog $server ${addr9002} ${addr9000} ${addr9001}" C-m
    tmux send-keys -t 0 "$glog $client ${addr9000}" C-m
    tmux send-keys -t 2 "$glog $client ${addr9001}" C-m
    tmux send-keys -t 4 "$glog $client ${addr9002}" C-m
}

main
