#include "gtest/gtest-spi.h"
#include "gtest/gtest.h"
#include <string>

#include "../../common/database.hpp"

struct s_test_val {
		uint16_t integer;
		std::string str;
};

class TestImplDB : public TypesafeYamlDatabase<uint16_t, s_test_val> {
public:
	TestImplDB(const std::string type, uint16_t version, uint16_t minVersion, std::string path) :
		TypesafeYamlDatabase(type, version, minVersion), path(path)
	{
	}

	const std::string getDefaultLocation() { return path; };
	uint64_t parseBodyNode(const YAML::Node& node) { return 1; };

private:
	std::string path;
};


namespace testing {

class DatabaseTest : public ::testing::Test
{
protected:
	DatabaseTest() {}
	virtual ~DatabaseTest() {}
	virtual void SetUp() {}
	virtual void TearDown() {}
};

TEST_F(DatabaseTest, SimpleValid)
{
	auto ydb = TestImplDB("Test", 1, 1, "src/tests/common/yaml_files/simple.yml");
  ASSERT_TRUE(ydb.load());	
}

TEST_F(DatabaseTest, SimpleInvalid)
{
	auto ydb = TestImplDB("Test", 1, 1, "src/tests/common/yaml_files/empty.yml");
	ASSERT_FALSE(ydb.load());
}

TEST_F(DatabaseTest, InvalidPath)
{
	auto ydb = TestImplDB("Test", 1, 1, "invalid path");
	ASSERT_FALSE(ydb.load());
}
} /* testing */
