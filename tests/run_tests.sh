#!/usr/bin/env bash
set -e

KOFI="./kofi"
if [ -f "./kofi.exe" ]; then
    KOFI="./kofi.exe"
fi

echo "Running tests in tests/pass/..."
for test in tests/pass/*.kf; do
    echo "  Testing $test"
    $KOFI "$test" > /dev/null
    ./output > /dev/null || ./output.exe > /dev/null
done

echo "Running tests in tests/fail/..."
for test in tests/fail/*.kf; do
    echo "  Testing (should fail) $test"
    if $KOFI "$test" > /dev/null 2>&1; then
        echo "Error: $test was expected to fail, but succeeded."
        exit 1
    fi
done

echo "All tests passed successfully!"
