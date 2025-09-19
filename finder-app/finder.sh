#! /bin/sh

# Verify usage - two command line args.
if [ $# -ne 2 ]; then
    echo "Usage: $0 <filesdir> <searchstr>"
    exit 1
fi

# Extract args
filesdir=$1
searchstr=$2

# Verify filesdir exists
if [ ! -d "$filesdir" ]; then
    echo "Directory does not exist"
    exit 1
fi

# Get list of files in directory
file_count=$(find "$filesdir" -type f | wc -l)

# Grep for search string in all files
match_count=$(grep -r "$searchstr" "$filesdir" | wc -l)

# Print number of files and number of matches
echo "The number of files are $file_count and the number of matching lines are $match_count"
exit 0

