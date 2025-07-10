#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Create and navigate to the build directory
mkdir -p build
cd build

# Run CMake configuration
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build .

# Run the executable (optional)
cd ..
./tinyrenderer obj/diablo3_pose/diablo3_pose.obj
