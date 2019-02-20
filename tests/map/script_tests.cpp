#include "gtest/gtest-spi.h"
#include "gtest/gtest.h"
#include "../../src/map/script.hpp"
#include "../../src/map/storage.hpp"
#include "map_mock.hpp"
#include "clif_mock.hpp"

namespace testing {

class ScriptTest : public ::testing::Test
{
protected:
	ScriptTest() {}
	virtual ~ScriptTest() {}
	virtual void SetUp() {}
	virtual void TearDown() {}
	std::shared_ptr<map_mock> mmap;
	std::shared_ptr<clif_mock> mclif;
};

TEST_F(ScriptTest, Trival)
{
	EXPECT_TRUE(true);
}

TEST_F(ScriptTest, Simple)
{
	mmap = std::shared_ptr<map_mock>(new map_mock());
	mclif = std::shared_ptr<clif_mock>(new clif_mock());

	script_set_map(mmap);
	script_set_clif(mclif);
	EXPECT_CALL(*mmap, id2sd(_)).Times(1).WillOnce(Return((struct map_session_data*)0x1));//.WillByDefault(Return((struct map_session_data*)0x1));
	/* prepare script_state */
	script_data sc_data = {
		C_FUNC,
		{.str = "Test"},
		NULL
	};

	script_stack stack = {
		3,
		3,
		0,
		&sc_data,
		NULL
	};

	script_state st = {
		&stack,
		0, 0,
		0,
		RUN,
		0, 0,
		NULL,
		{ 0, 0, 0 },
		NULL,
		0,
		0,
		0,
		0,
		0,
		"Test",
		0
		};

	/**
	 * get the function from buildin_function 
	 * 0: mes
	 **/
	int (*script_func)(struct script_state *st);
	script_func = get_func_ptr(0);
	
	/* execute function */
	script_func(&st);
}
} /* testing */
