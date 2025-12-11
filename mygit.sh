#!/bin/bash

# Check if the executable path is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <executable_path> <command> [args...]"
    exit 1
fi

# Get the executable path from the first argument
EXECUTABLE="$1"

# Shift the positional parameters to get the command and any additional arguments
shift

# Run the executable with the remaining arguments
"$EXECUTABLE" "$@"
