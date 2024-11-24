#!/bin/bash

# Helper script to easily build `pacific-scales-parser`
# Run this script from repo root

if [ ! -e CMakeLists.txt ] || [ ! -d .git ]; then
  echo "Please execute the script from the repo root dir"
  exit 1
fi

# Delete the existing build dir and create new one
rm -rf build && mkdir -p build

# Change to build dir and generate cmake config
pushd build
cmake ..
# Build the app
make -j pacific-parser
popd