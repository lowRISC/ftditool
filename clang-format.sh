#! /usr/bin/env bash

find  -name "*.cc" -print0  | xargs -0 clang-format -i
find  -name "*.hh" -print0  | xargs -0 clang-format -i
echo "done"
