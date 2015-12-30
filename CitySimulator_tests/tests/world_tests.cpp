#include "test_helpers.hpp"
#include "world.hpp"

class WorldTest : public ::testing::Test
{
protected:
	World world;

	virtual void SetUp() override
	{
		Locator::provide(SERVICE_RENDER, new RenderService(nullptr));
		Locator::provide(SERVICE_CONFIG, new ConfigService("data/test_reference_config.json",
		                                                   "data/test_config.json"));

		world.loadFromFile("data/test_world.tmx", "data/test_tileset.png");
	}

	virtual void TearDown() override
	{
	}
};

TEST_F(WorldTest, Size)
{
	auto realSize = sf::Vector2i(6, 6);
	EXPECT_EQ(world.getTileSize(), realSize);
	EXPECT_EQ(world.getPixelSize(), Utils::toPixel(realSize));
}