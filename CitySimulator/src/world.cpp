#include <SFML\System\Vector2.hpp>
#include <boost\shared_ptr.hpp>
#include "maploader.hpp"
#include "world.hpp"
#include "utils.hpp"
#include <boost/make_shared.hpp>

LayerType layerTypeFromString(const std::string &s)
{
	if (s == "underterrain")
		return UNDERTERRAIN;
	if (s == "terrain")
		return TERRAIN;
	if (s == "overterrain")
		return OVERTERRAIN;
	if (s == "objects")
		return OBJECTS;
	if (s == "collisions")
		return COLLISIONS;

	Logger::logWarning("Unknown LayerType: " + s);
	return ERROR;
}

Tileset::Tileset(const std::string &filename)
{
	// load texture
	if (!texture.loadFromFile(Utils::searchForFile(filename, "res/world")))
		throw std::runtime_error("Could not load tileset");

	size = texture.getSize();
	size.x /= Constants::tilesetResolution;
	size.y /= Constants::tilesetResolution;

	texture.setSmooth(false);

	// generate points
	points = new sf::Vector2f[(size.x + 1) * (size.y + 1)];
	for (size_t x = 0; x <= size.x; x++)
		for (size_t y = 0; y <= size.y; y++)
			points[getIndex(x, y)] = sf::Vector2f(x * Constants::tilesetResolution, y * Constants::tilesetResolution);
}

void Tileset::textureQuad(sf::Vertex *quad, const BlockType &blockType)
{
	int blockId = blockType;
	int row = blockId % size.x;
	int col = blockId / size.y;

	quad[0].texCoords = points[getIndex(row, col)];
	quad[1].texCoords = points[getIndex(row + 1, col)];
	quad[2].texCoords = points[getIndex(row + 1, col + 1)];
	quad[3].texCoords = points[getIndex(row, col + 1)];
}

BaseWorld::BaseWorld(const sf::Vector2i &size) : tileSize(size), pixelSize(Utils::toPixel(tileSize)),
                                                 tileset(new Tileset("tileset.png"))
{
	vertices.setPrimitiveType(sf::Quads);
	transform.scale(Constants::tileSize, Constants::tileSize);
}

BaseWorld::~BaseWorld()
{
	delete tileset;
}

void BaseWorld::setBlockType(const sf::Vector2u &pos, BlockType blockType, LayerType layer)
{
	auto depth = layers[layer];
	auto index = (pos.x + pos.y * tileSize.x);
	index += (depth - 1) * tileSize.x * tileSize.y;
	index *= 4;

	sf::Vertex *quad = &vertices[index];

	tileset->textureQuad(quad, blockType);

	quad[0].position = sf::Vector2f(pos.x, pos.y);
	quad[1].position = sf::Vector2f(pos.x + 1, pos.y);
	quad[2].position = sf::Vector2f(pos.x + 1, pos.y + 1);
	quad[3].position = sf::Vector2f(pos.x, pos.y + 1);
}

void BaseWorld::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	states.transform *= transform;

	sf::Texture texture = tileset->getTexture();
	states.texture = &texture;

	target.draw(vertices, states);
}

boost::shared_ptr<BaseWorld> BaseWorld::loadWorld(const std::string &filename)
{
	TMX::TileMap *tmx = TMX::TileMap::load(filename);

	// failure
	if (tmx == nullptr)
		return nullptr;

	sf::Vector2i size(tmx->width, tmx->height);
	BaseWorld *world = new BaseWorld(size);

	// find layers
	int depth(1);

	auto layers = tmx->layers;
	LayerType types[LayerType::ERROR];

	auto layerIt = layers.begin();
	while (layerIt != layers.end())
	{
		auto layer = *layerIt;

		// invalid layer type
		LayerType layerType = layerTypeFromString(layer->name);
		if (layerType == ERROR)
		{
			Logger::logError("Invalid layer name: " + layer->name);
			layerIt = layers.erase(layerIt);
			continue;
		}

		// invisible layer
		if (!layer->visible)
		{
			layerIt = layers.erase(layerIt);
			continue;
		}

		// add layer
		world->registerLayer(layerType, depth);
		types[depth - 1] = layerType;

		++depth;
		++layerIt;
	}

	// resize vertex array
	world->resizeVertexArray();

	// set blocktypes in layers
	sf::Vector2u pos;
	int i(0);
	for (TMX::TileLayer *layer : layers)
	{
		LayerType layerType = types[i++];

		for (size_t x = 0; x < size.x; ++x)
		{
			for (size_t y = 0; y < size.y; ++y)
			{
				pos.x = x;
				pos.y = y;

				TMX::Tile tile = layer->items[x + y * size.x];
				BlockType blockType = static_cast<BlockType>(tile.gid);

				if (blockType == BLANK)
					continue;

				// TODO: rotation
				if (!tile.isRotated())
					world->setBlockType(pos, blockType, layerType);
			}
		}
	}


	delete tmx;
	return boost::shared_ptr<BaseWorld>(world);
}

boost::shared_ptr<BaseWorld> createShitWorld()
{
	int s(20);
	sf::Vector2i size(s, s);
	auto world(boost::make_shared<BaseWorld>(size));

	for (int x = 0; x < size.x; ++x)
	{
		for (int y = 0; y < size.y; ++y)
		{
			world->setBlockType(sf::Vector2u(x, y), static_cast<BlockType>((rand() % 6) + 1));
		}
	}

	return world;
}
