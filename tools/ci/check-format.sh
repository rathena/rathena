#!/bin/bash

SHOULD_EXIT=0

# only check files in common for now
for FILE in $(ls src/common/*.hpp src/common/*.cpp)
do
    CHANGES=$(diff -u <(cat $FILE) <(clang-format $FILE))
    if [ -n "$CHANGES" ]; then
        echo "File $FILE is not formatted correctly"
        echo "$CHANGES"
        SHOULD_EXIT=1
    fi
done

if [ $SHOULD_EXIT -eq 1 ]; then
    echo "Some files are not formatted correctly, please run clang-format"
    exit 1
fi
echo "All files are formatted correctly"
