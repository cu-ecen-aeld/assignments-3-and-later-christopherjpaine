#! /bin/bash

# Verify usage - two command line args.
if [ $# -ne 2 ]; then
    echo "Usage: $0 <writefile> <writestr>"
    exit 1
fi

# Extract args
writefile=$1
writestr=$2

# Check directory for writefile exists
writedir=$(dirname "$writefile")
if [ ! -d "$writedir" ]; then   
    mkdir -p "$writedir"
fi

# Create a new writefile containing writestr - overwriting if it already exists.
echo "$writestr" > "$writefile"

# Check echo operation succeeded
if [ $? -ne 0 ]; then
    echo "Unable to create file" # (ie writefile is a directory)
    exit 1
fi