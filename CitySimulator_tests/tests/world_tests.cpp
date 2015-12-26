#include "gtest/gtest.h"
#include "world.hpp"

class WorldTest : public ::testing::Test
{
protected:
	World world;

	virtual void SetUp() override
	{
		world.loadFromFile("data/test_world.tmx", "data/test_tileset.png");
	}

	virtual void TearDown() override
	{
	}
};
