#!/bin/bash

CLANG_FORMAT_BIN=${CLANG_FORMAT_BIN:-clang-format}

# So we know what clang-format version CI is using
VERSION=$($CLANG_FORMAT_BIN --version | rev | cut -d' ' -f 1 | rev | cut -d'.' -f 1)
if [ "$VERSION" -lt 15 ]; then
    echo "clang-format version 15 or higher is required"
    exit 1
else
    echo "clang-format version is $($CLANG_FORMAT_BIN --version)"
fi


SHOULD_EXIT=0

# only check files in common for now
for FILE in $(ls src/common/*.hpp src/common/*.cpp)
do
    CHANGES=$(diff -u <(cat "$FILE") <($CLANG_FORMAT_BIN -Werror "$FILE"))
    if [ -n "$CHANGES" ]; then
        echo "File $FILE is not formatted correctly"
        echo "$CHANGES"
        SHOULD_EXIT=1
    fi
done

if [ $SHOULD_EXIT -eq 1 ]; then
    echo "Some files are not formatted correctly, please run $CLANG_FORMAT_BIN"
    exit 1
fi
echo "All files are formatted correctly"
