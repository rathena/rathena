cd $SRC/libconfig

mkdir -p build
cmake -S . -B build \
        -DBUILD_FUZZERS=On \
        -DCMAKE_C_COMPILER_WORKS=1 \
        -DCMAKE_CXX_COMPILER_WORKS=1 \
        -DBUILD_SHARED_LIBS=Off \
        -DBUILD_TESTS=Off \
        -DBUILD_EXAMPLES=Off \
    && cmake --build build --target install

# Prepare corpora
zip -q $OUT/config_read_fuzzer_seed_corpus.zip fuzz/corpus/*