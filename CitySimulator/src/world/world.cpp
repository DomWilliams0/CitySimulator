#include "world.hpp"
#include "bodydata.hpp"
#include "service/locator.hpp"

WorldService::WorldService(const std::string &worldPath, const std::string &tilesetPath)
		: worldPath(worldPath), tilesetPath(tilesetPath)
{
}

void WorldService::onEnable()
{
	std::vector<std::string> worldsToLoad;
	world.loadFromFile(worldPath, tilesetPath, worldsToLoad);
	// todo clear worldsToLoad and keep loading until it's empty
}

void WorldService::onDisable()
{
}


World &WorldService::getWorld()
{
	return world;
}

BodyData *WorldService::getSharedBodyDataForBlockType(BlockType blockType)
{
	auto it = bodyDataCache.find(blockType);
	if (it != bodyDataCache.end())
		return &it->second;

	// create new data
	if (blockType == BLOCK_SLIDING_DOOR)
	{
		BodyData data;
		data.type = BODYDATA_BLOCK;
		data.blockInteraction.callback = &interactWithSlidingDoor;

		return &bodyDataCache.insert({blockType, data}).first->second;
	}

	Logger::logError(format("Cannot create body data for blocktype %1%", _str(blockType)));
	return nullptr;
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

World::World() : terrain(this), collisionMap(this), buildingMap(this)
{
	transform.scale(Constants::tileSizef, Constants::tileSizef);
}

void World::loadFromFile(const std::string &filename,
						 const std::string &tileset, std::vector<std::string> &worldsToLoad)
{
	Logger::logDebug(format("Began loading world %1%", filename));
	Logger::pushIndent();

	std::string path(Utils::joinPaths(Config::getResource("world.root"), filename));
	TMX::TileMap *tmx = TMX::TileMap::load(path);

	// failure
	if (tmx == nullptr)
		throw std::runtime_error("Could not load world from null TileMap");

	sf::Vector2i size(tmx->width, tmx->height);
	resize(size);

	// terrain
	terrain.load(tmx, tileset);
	collisionMap.load();
	buildingMap.load(*tmx, worldsToLoad);

	Logger::popIndent();
	Logger::logInfo(format("Loaded world %1%", filename));
	delete tmx;
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

void World::getSurroundingTiles(const sf::Vector2i &tilePos, std::set<sf::FloatRect> &ret)
{
	return collisionMap.getSurroundingTiles(tilePos, ret);
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
