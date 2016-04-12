#include "test_helpers.hpp"
#include "world.hpp"

#define DECLARE_WORLD_TEST(fixtureName, worldName) \
class fixtureName : public ::testing::Test\
{\
protected:\
	WorldService *ws;\
	World *world;\
\
	virtual void SetUp() override\
	{\
		Locator::provide(SERVICE_RENDER, new RenderService(nullptr));\
		Locator::provide(SERVICE_CONFIG,\
		                 new ConfigService(DATA_ROOT, "test_reference_config.json", "test_config.json"));\
\
		ws = new WorldService(worldName, "data/test_tileset.png");\
		Locator::provide(SERVICE_WORLD, ws);\
		world = Locator::locate<WorldService>()->getMainWorld();\
	}\
\
	virtual void TearDown() override\
	{\
	}\
};\

DECLARE_WORLD_TEST(SimpleWorldTest, "tiny")
DECLARE_WORLD_TEST(ConnectionLookupTest, "hub")

TEST_F(SimpleWorldTest, WorldService)
{
	ASSERT_NE(ws, nullptr);

	EXPECT_EQ(ws->getMainWorld(), world);
	EXPECT_EQ(ws->getWorld(world->getID()), world);
}

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

/**
 * Ensures that every connection found in the source world points to a single
 * world in the given list. If any connections to worlds that aren't in the given
 * list are found, or any expected connections are not found, an exception is
 * thrown
 */
void testWorldConnections(WorldID src, const std::string &expectedWorldName,
		std::vector<std::string> expectedConnections)
{
	WorldService *ws = Locator::locate<WorldService>();
	ASSERT_NE(ws, nullptr);

	World *srcWorld = ws->getWorld(src);
	ASSERT_NE(srcWorld, nullptr);
	if (srcWorld->getName() != expectedWorldName)
		error("World ID %1%'s name is not '%2%' as expected, but '%3%'", 
				_str(srcWorld->getID()), expectedWorldName, srcWorld->getName());

	sf::Vector2i size = srcWorld->getTileSize();
	for (int x = 0; x < size.x; ++x)
	{
		for (int y = 0; y < size.y; ++y)
		{
			// get connection
			Location out;
			if (!ws->getConnectionDestination(Location(srcWorld->getID(), x, y), out))
				continue;

			// get world from connection
			World *dstWorld = ws->getWorld(out.world);
			ASSERT_NE(dstWorld, nullptr);

			auto index = std::find(expectedConnections.cbegin(),
					expectedConnections.cend(), dstWorld->getName());
			if (index == expectedConnections.cend())
			{
				std::stringstream ss;
				ss << "Unexpected connection from world " << srcWorld->getID() << " to " <<
					out.world << " (coordinates " << x << ", " << y << " to " << 
					out.x << ", " << out.y << ")" << std::endl;

				throw std::runtime_error(ss.str());
			}
			expectedConnections.erase(index);
		}
	}

	if (!expectedConnections.empty())
		error("%1% expected connections from world %2% not found: ",
				_str(expectedConnections.size()), _str(srcWorld->getID()));
}

TEST_F(ConnectionLookupTest, ConnectionLookupTesterTest)
{
	// incorrect set
	EXPECT_THROW(testWorldConnections(0, "hub", {}), std::runtime_error);
	EXPECT_THROW(testWorldConnections(0, "hub", {"none-test"}), std::runtime_error);
	EXPECT_THROW(testWorldConnections(0, "hub", {"none-test", "single-test", "multiple-test", "extra"}),
			std::runtime_error);

	// order doesn't matter
	EXPECT_NO_THROW(testWorldConnections(0, "hub", {"none-test", "single-test", "multiple-test"}));
	EXPECT_NO_THROW(testWorldConnections(0, "hub", {"single-test", "multiple-test", "none-test"}));
	EXPECT_NO_THROW(testWorldConnections(0, "hub", {"multiple-test", "none-test", "single-test"}));
	EXPECT_THROW(testWorldConnections(0, "uh oh", {"none-test", "single-test", "multiple-test"}),
			std::runtime_error);
}

TEST_F(ConnectionLookupTest, ConnectionLookup)
{
	Location out;
	EXPECT_TRUE(ws->getConnectionDestination({0, 1, 3}, out));
	EXPECT_EQ(out, Location(1, 1, 5));

	// 0 tested in tester test
	EXPECT_NO_THROW(testWorldConnections(1, "none-test", {"hub"}));
	EXPECT_NO_THROW(testWorldConnections(2, "single-test", {"hub", "none-test"}));
	EXPECT_NO_THROW(testWorldConnections(3, "multiple-test", 
				{"hub", "single-test", "none-test", "none-double-test", "none-double-test"}));
}
