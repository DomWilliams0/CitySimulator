#include "test_helpers.hpp"
#include "world.hpp"

class SimpleWorldTest : public ::testing::Test
{
protected:
	World *world;

	virtual void SetUp() override
	{
		Locator::provide(SERVICE_RENDER, new RenderService(nullptr));
		Locator::provide(SERVICE_CONFIG,
		                 new ConfigService(DATA_ROOT, "test_reference_config.json", "test_config.json"));

		Locator::provide(SERVICE_WORLD, new WorldService("tiny", "data/test_tileset.png"));
		world = Locator::locate<WorldService>()->getMainWorld();
	}

	virtual void TearDown() override
	{
	}
};

TEST_F(SimpleWorldTest, Size)
{
	auto realSize = sf::Vector2i(6, 6);
	EXPECT_EQ(world->getTileSize(), realSize);
	EXPECT_EQ(world->getPixelSize(), Utils::toPixel(realSize));
}


TEST_F(SimpleWorldTest, Properties)
{
	EXPECT_EQ(world->getName(), "tiny");	
	EXPECT_EQ(world->getID(), 0);
	EXPECT_TRUE(world->isOutside());
}

TEST_F(SimpleWorldTest, LayerLoad)
{
	const std::vector<WorldObject> &objects = world->getTerrain()->getObjects();
	EXPECT_EQ(objects.size(), 3);

	for (const WorldObject &obj : objects)
		EXPECT_EQ(obj.type, BLOCK_TREE);

}
