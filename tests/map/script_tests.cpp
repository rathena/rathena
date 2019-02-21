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
	virtual void SetUp() {
		//do_init_script();  requires more mocks
	}
	virtual void TearDown() {
		script_free_mock();
	}
};

TEST_F(ScriptTest, Trival)
{
	EXPECT_TRUE(true);
}

TEST_F(ScriptTest, Simple)
{
	std::unique_ptr<map_mock> mmap(new map_mock());
	std::unique_ptr<clif_mock> mclif(new clif_mock());

	EXPECT_CALL(*mmap, id2sd(_)).Times(1).WillOnce(Return((struct map_session_data*)0x1));
	EXPECT_CALL(*mclif, scriptmes(_, _, _));
	
	script_set_clif(std::move(mclif));
	script_set_map(std::move(mmap));

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
