#!/bin/sh

lcov -c -b . -d . -o coverage.info
lcov -r coverage.info '*/usr/*' -o coverage_filtered.info
lcov -r coverage_filtered.info '*/install/*' -o coverage_filtered.info
lcov -r coverage_filtered.info '*/tests/*' -o coverage_filtered.info
lcov -r coverage_filtered.info '*/deps/*' -o coverage_filtered.info
lcov --list coverage_filtered.info
