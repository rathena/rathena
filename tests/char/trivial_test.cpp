/*#include "gtest/gtest-spi.h"
#include "gtest/gtest.h"

namespace testing {

class TrivialTest : public ::testing::Test
{
protected:
	TrivialTest() {}
	virtual ~TrivialTest() {}
	virtual void SetUp() {}
	virtual void TearDown() {}
};

TEST(TrivialTest, Trival)
{
	EXPECT_TRUE(true);
}
} /* testing */
