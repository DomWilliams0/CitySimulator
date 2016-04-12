#include "world.hpp"
#include "service/logging_service.hpp"

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

bool isTileLayer(LayerType layerType)
{
	return layerType == LAYER_UNDERTERRAIN || layerType == LAYER_TERRAIN || layerType == LAYER_OVERTERRAIN;
}

bool isOverLayer(LayerType layerType)
{
	return layerType == LAYER_OVERTERRAIN;
}

WorldTerrain::WorldTerrain(World *container, const sf::Vector2i &size) : 
	BaseWorld(container), size(size), collisionMap(container)
{
	tileVertices.setPrimitiveType(sf::Quads);
	overLayerVertices.setPrimitiveType(sf::Quads);
}

int WorldTerrain::getBlockIndex(const sf::Vector2i &pos, LayerType layerType)
{
	int index = (pos.x + pos.y * size.x);
	index += getDepth(layerType) * size.x * size.y;
	index *= 4;

	return index;
}


int WorldTerrain::getVertexIndex(const sf::Vector2i &pos, LayerType layerType)
{
	int index = (pos.x + pos.y * size.x);
	int depth = getDepth(layerType);
	if (isOverLayer(layerType))
	{
		int diff = tileLayerCount - overLayerCount;
		depth -= diff;
	}

	index += depth * size.x * size.y;
	index *= 4;

	return index;
}


void WorldTerrain::rotateObject(sf::Vertex *quad, float degrees, const sf::Vector2f &pos)
{
	sf::Vector2f origin(pos.x, pos.y + 1);

	float radians(degrees * Math::degToRad);
	const float c(cos(radians));
	const float s(sin(radians));

	for (int i = 0; i < 4; ++i)
	{
		sf::Vector2f vPos(quad[i].position);
		vPos -= origin;

		sf::Vector2f rotated(vPos);
		rotated.x = vPos.x * c - vPos.y * s;
		rotated.y = vPos.x * s + vPos.y * c;

		rotated += origin;

		quad[i].position = rotated;
	}
}

void WorldTerrain::positionVertices(sf::Vertex *quad, const sf::Vector2i &pos, int delta)
{
	positionVertices(quad, sf::Vector2f(pos.x, pos.y), delta);
}

void WorldTerrain::positionVertices(sf::Vertex *quad, const sf::Vector2f &pos, int delta)
{
	quad[0].position = sf::Vector2f(pos.x, pos.y);
	quad[1].position = sf::Vector2f(pos.x + delta, pos.y);
	quad[2].position = sf::Vector2f(pos.x + delta, pos.y + delta);
	quad[3].position = sf::Vector2f(pos.x, pos.y + delta);
}

int WorldTerrain::getDepth(LayerType layerType) const
{
	auto it = layerDepths.find(layerType);
	if (it == layerDepths.end())
		error("Cannot get depth for invalid layer of type %d", _str(layerType));
	return it->second;
}


sf::VertexArray &WorldTerrain::getVertices(LayerType layerType)
{
	return isOverLayer(layerType) ? overLayerVertices : tileVertices;
}

void WorldTerrain::resizeVertices()
{
	const int sizeMultiplier = size.x * size.y * 4;

	blockTypes.resize(tileLayerCount * sizeMultiplier);

	tileVertices.resize((tileLayerCount - overLayerCount) * sizeMultiplier);
	overLayerVertices.resize(overLayerCount * sizeMultiplier);
}

void WorldTerrain::setBlockType(const sf::Vector2i &pos, BlockType blockType, LayerType layer, int rotationAngle,
                                int flipGID)
{
	int vertexIndex = getVertexIndex(pos, layer);
	sf::VertexArray &vertices = getVertices(layer);
	sf::Vertex *quad = &vertices[vertexIndex];

	positionVertices(quad, pos, 1);
	tileset->textureQuad(quad, blockType, rotationAngle, flipGID);

	blockTypes[getBlockIndex(pos, layer)] = blockType;
}

BlockType WorldTerrain::getBlockType(const sf::Vector2i &tile, LayerType layer)
{
	return blockTypes.at(getBlockIndex(tile, layer));
}

void WorldTerrain::addObject(const sf::Vector2f &pos, BlockType blockType, float rotationAngle, int flipGID)
{
	std::vector<sf::Vertex> quad(4);
	sf::Vector2f adjustedPos = sf::Vector2f(pos.x / Constants::tilesetResolution,
	                                        (pos.y - Constants::tilesetResolution) / Constants::tilesetResolution);

	positionVertices(&quad[0], adjustedPos, 1);
	tileset->textureQuad(&quad[0], blockType, 0, flipGID);

	if (rotationAngle != 0)
		rotateObject(&quad[0], rotationAngle, adjustedPos);

	sf::VertexArray &vertices = getVertices(LAYER_OBJECTS);
	for (int i = 0; i < 4; ++i)
		vertices.append(quad[i]);

	objects.emplace_back(blockType, rotationAngle, Utils::toTile(pos));
}

const std::vector<WorldObject> &WorldTerrain::getObjects() const
{
	return objects;
}

const std::map<LayerType, int> &WorldTerrain::getLayerDepths() const
{
	return layerDepths;
}

void WorldTerrain::discoverLayers(std::vector<TMX::Layer> &tmxLayers)
{
	int depth = 0;
	tileLayerCount = 0;
	overLayerCount = 0;

	auto layerIt = tmxLayers.cbegin();
	while (layerIt != tmxLayers.cend())
	{
		const TMX::Layer &layer = *layerIt;

		// unknown layer type
		LayerType layerType = layerTypeFromString(layer.name);
		if (layerType == LAYER_UNKNOWN)
		{
			Logger::logError("Invalid layer name: " + layer.name);
			layerIt = tmxLayers.erase(layerIt);
			continue;
		}

		// invisible layer
		if (!layer.visible)
		{
			layerIt = tmxLayers.erase(layerIt);
			continue;
		}

		if (isTileLayer(layerType))
			++tileLayerCount;
		if (isOverLayer(layerType))
			++overLayerCount;

		layerDepths.insert({layerType, depth});
		Logger::logDebuggier(format("Found layer type %1% at depth %2%", _str(layerType), _str(depth)));

		++depth;
		++layerIt;
	}
}

void WorldTerrain::discoverFlippedTiles(const std::vector<TMX::Layer> &layers, std::unordered_set<int> &flippedGIDs)
{
	for (const TMX::Layer &layer : layers)
	{
		for (const TMX::TileWrapper &tile : layer.items)
		{
			if (!tile.tile.isFlipped() || tile.tile.getGID() == BLOCK_BLANK)
				continue;

			int flipGID = tile.tile.getFlipGID();

			// already done
			if (flippedGIDs.find(flipGID) == flippedGIDs.end())
				flippedGIDs.insert(flipGID);
		}
	}
}

void WorldTerrain::applyTiles(Tileset &tileset)
{
	int layerIndex(0);
	this->tileset = &tileset;

	auto &layers = tmx->layers;
	for (const TMX::Layer &layer : layers)
	{
		LayerType layerType = layerTypeFromString(layer.name);

		if (layerType == LAYER_OBJECTS)
		{
			// objects
			for (const TMX::TileWrapper &tile : layer.items)
			{
				BlockType blockType = static_cast<BlockType>(tile.tile.getGID());
				if (blockType == BLOCK_BLANK)
					continue;

				addObject(tile.tile.position, blockType, tile.objectRotation, tile.tile.getFlipGID());
			}
		}

		else if (isTileLayer(layerType))
		{
			sf::Vector2i tempPos;

			// tiles
			for (const TMX::TileWrapper &tile : layer.items)
			{
				BlockType blockType = static_cast<BlockType>(tile.tile.getGID());
				if (blockType == BLOCK_BLANK)
					continue;

				tempPos.x = (int) tile.tile.position.x;
				tempPos.y = (int) tile.tile.position.y;

				setBlockType(tempPos, blockType, layerType, tile.tile.getRotationAngle(), tile.tile.getFlipGID());
			}
		}
	}

	tmx = nullptr;
	collisionMap.load();
}

void WorldTerrain::render(sf::RenderTarget &target, sf::RenderStates &states, bool overLayers) const
{
	states.texture = tileset->getTexture();
	target.draw(overLayers ? overLayerVertices : tileVertices, states);
}

void WorldTerrain::loadFromTileMap(TMX::TileMap &tileMap, std::unordered_set<int> &flippedGIDs)
{
	tmx = &tileMap;

	// find layer count and depths
	discoverLayers(tileMap.layers);

	Logger::logDebug(format("Discovered %1% tile layer(s), of which %2% are overterrain",
				_str(tileLayerCount), _str(overLayerCount)));

	// resize vertex array to accommodate for layer count
	resizeVertices();

	// collect any gids that need flipping
	discoverFlippedTiles(tileMap.layers, flippedGIDs);
}


CollisionMap *WorldTerrain::getCollisionMap()
{
	return &collisionMap;
}
