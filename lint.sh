#!/usr/bin/env bash

set -euo pipefail

if [ $# -ne 1 ]; then
    echo "Expected 1 command"
    echo "Usage: $0 check|format"
    exit 1
fi

FILES=$(find \
lib \
src \
test \
-not -path build \
-iname *.h \
-o -iname *.cc \
-o -iname *.ino \
)

if [ $1 = "check" ]; then
    clang-format --dry-run --Werror $FILES
elif [ $1 = "format" ]; then
    clang-format --style google -i $FILES
else
    echo "Unrecognized command: $1"
    echo "Usage: $0 check|format"
    exit 1
fi
