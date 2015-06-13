#include <SFML\System\Vector2.hpp>
#include <boost\shared_ptr.hpp>
#include <exception>
#include "maploader.hpp"
#include "world.hpp"
#include "utils.hpp"
#include "logger.hpp"

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

Tileset::Tileset(const std::string &filename) : converted(false)
{
	// load image
	image = new sf::Image;
	if (!image->loadFromFile(Utils::searchForFile(filename, "res/world")))
		throw std::runtime_error("Could not load tileset");

	size = image->getSize();
	size.x /= Constants::tilesetResolution;
	size.y /= Constants::tilesetResolution;

	generatePoints();
}

void Tileset::textureQuad(sf::Vertex *quad, const BlockType &blockType, int rotationAngle, int flipFlags)
{
	int row = blockType % size.x;
	int col = blockType / size.x;

	// rotating
	int offset = 0;
	if (rotationAngle == -90)
		offset = 1;
	else if (rotationAngle == 90)
		offset = 3;

	// flipping TODO (shader?)

	quad[(0 + offset) % 4].texCoords = points[getIndex(row, col)];
	quad[(1 + offset) % 4].texCoords = points[getIndex(row + 1, col)];
	quad[(2 + offset) % 4].texCoords = points[getIndex(row + 1, col + 1)];
	quad[(3 + offset) % 4].texCoords = points[getIndex(row, col + 1)];
}

void Tileset::convertToTexture()
{
	// resize image
	int rowsRequired = LAST / size.x;
	if (LAST % size.x != 0)
		rowsRequired += 1;

	// transfer to new image
	sf::Image newImage;
	newImage.create(size.x * Constants::tilesetResolution, rowsRequired * Constants::tilesetResolution);
	newImage.copy(*image, 0, 0);
	delete image;

	// update size
	size.y = rowsRequired;
	generatePoints();

	// write to texture
	if (!texture.loadFromImage(newImage))
		throw std::exception("Could not render tileset");

	converted = true;
}

sf::IntRect Tileset::getTileRect(unsigned blockType)
{
	int tileX = (blockType % size.x);
	int tileY = (blockType / size.x);
	return sf::IntRect(tileX * Constants::tilesetResolution,
	                   tileY * Constants::tilesetResolution,
	                   Constants::tilesetResolution,
	                   Constants::tilesetResolution);
}

void Tileset::createTileImage(sf::Image *image, unsigned blockType)
{
	if (converted)
		throw std::exception("Tileset has already been converted to a texture");

	image->create(Constants::tilesetResolution, Constants::tilesetResolution);
	image->copy(*this->image, 0, 0, getTileRect(blockType));
}

void Tileset::generatePoints()
{
	points = new sf::Vector2f[(size.x + 1) * (size.y + 1)];
	for (size_t x = 0; x <= size.x; x++)
		for (size_t y = 0; y <= size.y; y++)
			points[getIndex(x, y)] = sf::Vector2f(x * Constants::tilesetResolution, y * Constants::tilesetResolution);
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

int BaseWorld::getBlockIndex(const sf::Vector2i &pos, LayerType layerType)
{
	auto depth = layerDepths[layerType];
	if (depth != 0)
		depth -= 1;

	int index = (pos.x + pos.y * tileSize.x);
	index += depth * tileSize.x * tileSize.y;
	index *= 4;

	return index;
}

void BaseWorld::setBlockType(const sf::Vector2i &pos, BlockType blockType, LayerType layer, int rotationAngle, int flipFlags)
{
	int index = getBlockIndex(pos, layer);

	sf::Vertex *quad = &vertices[index];
	tileset->textureQuad(quad, blockType, rotationAngle, flipFlags);

	quad[0].position = sf::Vector2f(pos.x, pos.y);
	quad[1].position = sf::Vector2f(pos.x + 1, pos.y);
	quad[2].position = sf::Vector2f(pos.x + 1, pos.y + 1);
	quad[3].position = sf::Vector2f(pos.x, pos.y + 1);

	blockTypes[index] = blockType;
}

void BaseWorld::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	states.transform *= transform;

	// tile drawing
	states.texture = tileset->getTexture();
	target.draw(vertices, states);
}

void BaseWorld::discoverLayers(std::vector<TMX::Layer*> layers, LayerType *layerTypes)
{
	auto layerIt = layers.begin();
	int depth(1);
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
		registerLayer(layerType, depth);
		layerTypes[depth - 1] = layerType;

		++depth;
		++layerIt;
	}
}

void BaseWorld::addTiles(std::vector<TMX::Layer*> layers, LayerType *types)
{
	sf::Vector2i pos;
	int layerIndex(0);
	for (TMX::Layer *layer : layers)
	{
		LayerType layerType = types[layerIndex++];

		for (size_t x = 0; x < tileSize.x; ++x)
		{
			for (size_t y = 0; y < tileSize.y; ++y)
			{
				TMX::Tile *tile = layer->items[x + y * tileSize.x];
				if (tile == nullptr)
					continue;

				BlockType blockType = static_cast<BlockType>(tile->gid);
				if (blockType == BLANK)
					continue;

				// objects are not stuck to the grid
				if (layerType == OBJECTS)
				{
					TMX::Object *object = dynamic_cast<TMX::Object*>(tile);
					pos = object->position;

					// TODO: add to world
				}

				// tiles
				else
				{
					pos.x = x;
					pos.y = y;

					setBlockType(pos, blockType, layerType, tile->getRotationAngle(), tile->getFlipFlags());
				}
			}
		}
	}
}

BaseWorld* BaseWorld::loadWorld(const std::string &filename)
{
	TMX::TileMap *tmx = TMX::TileMap::load(filename);

	// failure
	if (tmx == nullptr)
		return nullptr;

	sf::Vector2i size(tmx->width, tmx->height);
	BaseWorld *world = new BaseWorld(size);

	// find layer depths
	auto layers = tmx->layers;
	LayerType types[LayerType::ERROR];
	world->discoverLayers(layers, types);

	// resize vertex array to accomodate for layer count
	world->resizeVertexArray();
	world->tileset->convertToTexture();

	// add tiles to world
	world->addTiles(layers, types);

	delete tmx;
	return world;
}


