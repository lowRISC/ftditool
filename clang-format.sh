#! /usr/bin/env bash
set -e
[[ "$1" == "check" ]] && operation="-n" || operation="-i"

find  -name "*.cc" -print0  | xargs -0 clang-format $operation
find  -name "*.hh" -print0  | xargs -0 clang-format $operation

echo "$1 done"
