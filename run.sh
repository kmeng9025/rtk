#!/bin/bash

cleanup() {
    trap - EXIT INT TERM
    echo "Stopping..."

    if [[ -n "$NTRIP_PID" ]]; then
        kill "$NTRIP_PID" 2>/dev/null
    fi

    if [[ -n "$MAIN_PID" ]]; then
        kill "$MAIN_PID" 2>/dev/null
    fi

    wait 2>/dev/null
}

trap cleanup EXIT INT TERM

./str2str \
  -in ntrip://kmeng90251@gmail.com:none@3.143.243.81:2101/lptg-ringrocks-01 \
  -out serial://ttyS0:115200:8:n:1:off &

NTRIP_PID=$!

g++ ./main.cpp -o main || exit 1

./main &
MAIN_PID=$!

wait "$MAIN_PID"