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

TEST_F(SimpleWorldTest, CollisionBoxes)
{
	b2World *bw = world->getBox2DWorld();
	ASSERT_NE(bw, nullptr);

	EXPECT_EQ(bw->GetBodyCount(), 1);


	b2Body &body = bw->GetBodyList()[0];

	std::size_t count = 0;
	for (b2Fixture *f = body.GetFixtureList(); f; f = f->GetNext())
		count++;

	// 4 borders
	// 3 water
	// 3 trees
	EXPECT_EQ(count, 10);
}

TEST_F(SimpleWorldTest, BlockSetting)
{
	WorldTerrain *terrain = world->getTerrain();
	ASSERT_NE(terrain, nullptr);

	EXPECT_EQ(terrain->getBlockType({0, 0}), BLOCK_GRASS);
	EXPECT_ANY_THROW(terrain->getBlockType({500, 0}));

	terrain->setBlockType({0, 0}, BLOCK_SAND);
	EXPECT_EQ(terrain->getBlockType({0, 0}), BLOCK_SAND);
}
