#!/usr/bin/env bash

set -e

reader="$1"
writer="$2"

"$reader" &
reader_pid=$!

sleep 0.1

"$writer"

wait "$reader_pid"