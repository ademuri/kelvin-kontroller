#!/bin/bash

set -euo pipefail

if [ ! -f src/esp32/constants.h ]; then
  cp src/esp32/constants.sample.h src/esp32/constants.h
fi
