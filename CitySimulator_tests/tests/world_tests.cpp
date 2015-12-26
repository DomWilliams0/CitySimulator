#include "gtest/gtest.h"
#include "world.hpp"

class WorldTest : public ::testing::Test
{
protected:
	World world;

	virtual void SetUp() override
	{
		world.loadFromFile("data/test_world.tmx");
	}

	virtual void TearDown() override
	{
	}
};

TEST_F(WorldTest, EmptyTest)
{
}
