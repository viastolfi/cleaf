#!/usr/bin/env bash
# Integration tests for `cleaf build`: each subdirectory of
# test/integration_case/ is a standalone multi-module project. An
# `expect.txt` file (key=value, one per line) declares the expected
# outcome:
#
#   build_exit=<n>   required — expected exit code of `cleaf build`
#   run_exit=<n>      optional — if set, the produced build/a.out is
#                      executed afterwards and its exit code checked
#
# Usage: test/integration_test.sh <path-to-cleaf-binary>

set -u

CLEAF_BIN="${1:-build/cleaf}"
CLEAF_BIN="$(cd "$(dirname "$CLEAF_BIN")" && pwd)/$(basename "$CLEAF_BIN")"

CASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/integration_case" && pwd)"

fail=0
total=0

for dir in "$CASE_DIR"/*/; do
  name="$(basename "$dir")"
  expect_file="$dir/expect.txt"

  if [ ! -f "$expect_file" ]; then
    continue
  fi

  total=$((total + 1))

  expected_build_exit=$(grep '^build_exit=' "$expect_file" | cut -d= -f2)
  expected_run_exit=$(grep '^run_exit=' "$expect_file" | cut -d= -f2)

  (
    cd "$dir" || exit 1
    rm -rf build a.out
    "$CLEAF_BIN" build > /tmp/cleaf_integration_${name}.log 2>&1
  )
  actual_build_exit=$?

  if [ "$actual_build_exit" != "$expected_build_exit" ]; then
    echo "[FAIL] $name: expected build exit $expected_build_exit, got $actual_build_exit"
    echo "       see /tmp/cleaf_integration_${name}.log"
    fail=$((fail + 1))
    continue
  fi

  if [ -n "$expected_run_exit" ]; then
    (cd "$dir" && ./build/a.out)
    actual_run_exit=$?
    if [ "$actual_run_exit" != "$expected_run_exit" ]; then
      echo "[FAIL] $name: expected run exit $expected_run_exit, got $actual_run_exit"
      fail=$((fail + 1))
      continue
    fi
  fi

  echo "[ OK ] $name"
done

echo ""
echo "$((total - fail))/$total integration test(s) passed"

exit $([ "$fail" -eq 0 ] && echo 0 || echo 1)
