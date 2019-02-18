#include "gtest/gtest-spi.h"
#include "gtest/gtest.h"
#include "../../src/map/script.hpp"
#include "../fff.h"

DEFINE_FFF_GLOBALS;
FAKE_VALUE_FUNC(struct map_session_data*, map_id2sd, int);

namespace testing {

class ScriptTest : public ::testing::Test
{
protected:
	ScriptTest() {}
	virtual ~ScriptTest() {}
	virtual void SetUp() {}
	virtual void TearDown() {}
};

TEST(ScriptTest, Trival)
{
	EXPECT_TRUE(true);
}

TEST(ScriptTest, Simple)
{
	map_id2sd_fake.return_val = (map_session_data*)0x1;	
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
