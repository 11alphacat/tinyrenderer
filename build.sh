#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Create and navigate to the build directory
mkdir -p build
cd build

# Run CMake configuration
cmake .. -DCMAKE_BUILD_TYPE=Release

# cmake .. -DCMAKE_BUILD_TYPE=Profile

# Build the project
# cmake --build .
make -j$(nproc)

# Run the executable (optional)
cd ..
./tinyrenderer obj/diablo3_pose/diablo3_pose.obj
# ./tinyrenderer obj/african_head/african_head.obj
