#include <gtest/gtest.h>
#include "random.hpp"
#include <vector>
#include <algorithm>

TEST(RandomTest, RndValueDeterministic) {
    // Set a fixed seed for deterministic results
    std::mt19937 old_gen = generator;
    generator.seed(42);

    int v1 = rnd_value<int>(1, 10);
    int v2 = rnd_value<int>(1, 10);
    generator.seed(42);
    int v1_repeat = rnd_value<int>(1, 10);
    int v2_repeat = rnd_value<int>(1, 10);

    EXPECT_EQ(v1, v1_repeat);
    EXPECT_EQ(v2, v2_repeat);

    generator = old_gen; // Restore
}

TEST(RandomTest, RndChance) {
    generator.seed(123);
    int true_count = 0;
    int false_count = 0;
    for (int i = 0; i < 1000; ++i) {
        if (rnd_chance<int>(50, 100)) true_count++;
        else false_count++;
    }
    // Should be roughly 50/50
    EXPECT_NEAR(true_count, 500, 50);
    EXPECT_NEAR(false_count, 500, 50);
}

TEST(RandomTest, RndChanceOfficial) {
    generator.seed(456);
    int true_count = 0;
    for (int i = 0; i < 1000; ++i) {
        if (rnd_chance_official<int>(100, 200)) true_count++;
    }
    // Should be roughly 50%
    EXPECT_NEAR(true_count, 500, 50);
}

TEST(RandomTest, RndVectorOrder) {
    generator.seed(789);
    std::vector<int> vec = {1, 2, 3, 4, 5};
    rnd_vector_order(vec);
    // Should be a permutation of 1-5
    std::vector<int> sorted = vec;
    std::sort(sorted.begin(), sorted.end());
    EXPECT_EQ(sorted, (std::vector<int>{1,2,3,4,5}));
}