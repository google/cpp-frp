#!/bin/bash

lcov --directory . --zerocounters
ctest
lcov --directory . --capture --output-file test-coverage.info
lcov --remove test-coverage.info 'test/include/*' 'test/src/*' 'googletest-src/*' '/usr/*' --output-file test-coverage.info
genhtml --output-directory test-coverage test-coverage.info

