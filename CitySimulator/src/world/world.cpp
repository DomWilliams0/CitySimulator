#include "world.hpp"
#include "service/locator.hpp"

WorldService::WorldService(const std::string &mainWorldPath, const std::string &tilesetPath)
		: mainWorldPath(mainWorldPath), tilesetPath(tilesetPath)
{
}

void WorldService::onEnable()
{
	std::set<std::string> worldsToLoad;
	worldsToLoad.insert(mainWorldPath);
	lastID = 0;

	std::vector<int> flippedGIDs;

	while(!worldsToLoad.empty())
	{
		std::vector<std::string> loadingNow(worldsToLoad.begin(), worldsToLoad.end());
		worldsToLoad.clear();

		for(std::string &worldName : loadingNow)
		{
			int id = lastID++;

			World w(id, tileset);
			w.loadFromFile(worldName, flippedGIDs, worldsToLoad);
			worlds.insert({id, std::move(w)});
		}
	}

	tileset.load(tilesetPath);
	tileset.convertToTexture(flippedGIDs);

	mainWorld = &worlds.at(0);
}

void WorldService::onDisable()
{
}


World &WorldService::getWorld()
{
	return *mainWorld;
}

bool isCollidable(BlockType blockType)
{
	static const std::set<BlockType> collidables(
			{BLOCK_WATER, BLOCK_TREE, BLOCK_BUILDING_WALL, BLOCK_BUILDING_EDGE, BLOCK_BUILDING_ROOF,
			 BLOCK_BUILDING_ROOF_CORNER});
	return collidables.find(blockType) != collidables.end();
}

bool isInteractable(BlockType blockType)
{
	static const std::set<BlockType> interactables(
			{BLOCK_SLIDING_DOOR});
	return interactables.find(blockType) != interactables.end();
}

LayerType layerTypeFromString(const std::string &s)
{
	if (s == "underterrain")
		return LAYER_UNDERTERRAIN;
	if (s == "terrain")
		return LAYER_TERRAIN;
	if (s == "overterrain")
		return LAYER_OVERTERRAIN;
	if (s == "objects")
		return LAYER_OBJECTS;
	if (s == "collisions")
		return LAYER_COLLISIONS;
	if (s == "buildings")
		return LAYER_BUILDINGS;

	Logger::logWarning("Unknown LayerType: " + s);
	return LAYER_UNKNOWN;
}

bool isTileLayer(const LayerType &layerType)
{
	return layerType == LAYER_UNDERTERRAIN || layerType == LAYER_TERRAIN || layerType == LAYER_OVERTERRAIN;
}

bool isOverLayer(const LayerType &layerType)
{
	return layerType == LAYER_OVERTERRAIN;
}

World::World(int id, Tileset &tileset) : terrain(this, tileset), collisionMap(this), buildingMap(this), id(id)
{
	transform.scale(Constants::tileSizef, Constants::tileSizef);
}

void World::loadFromFile(const std::string &filename,
						 std::vector<int> &flippedGIDs, std::set<std::string> &worldsToLoad)
{
	Logger::logDebug(format("Began loading world %1%", filename));
	Logger::pushIndent();

	std::string path(Utils::joinPaths(Config::getResource("world.root"), filename));

	TMX::TileMap tmx;
	tmx.load(path);
	resize(tmx.size);

	terrain.load(tmx, flippedGIDs);
	buildingMap.load(tmx, worldsToLoad);
	collisionMap.load();

	Logger::popIndent();
	Logger::logInfo(format("Loaded world %1%", filename));
}

void World::resize(sf::Vector2i size)
{
	tileSize = size;
	pixelSize = Utils::toPixel(size);
}

WorldTerrain &World::getTerrain()
{
	return terrain;
}

CollisionMap &World::getCollisionMap()
{
	return collisionMap;
}

BuildingMap &World::getBuildingMap()
{
	return buildingMap;
}

b2World *World::getBox2DWorld()
{
	return &collisionMap.world;
}

sf::Vector2i World::getPixelSize() const
{
	return pixelSize;
}

sf::Vector2i World::getTileSize() const
{
	return tileSize;
}

sf::Transform World::getTransform() const
{
	return transform;
}

BlockType World::getBlockAt(const sf::Vector2i &tile, LayerType layer)
{
	int index = terrain.getBlockIndex(tile, layer);
	return terrain.blockTypes[index];
}

int World::getID() const
{
	return id;
}

void World::tick(float delta)
{
	// todo fixed time step
	getBox2DWorld()->Step(delta, 6, 2);
}

void World::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	states.transform *= transform;

	// terrain
	terrain.render(target, states, false);

	// entities
	Locator::locate<EntityService>()->renderSystems();

	// overterrain
	terrain.render(target, states, true);

}
