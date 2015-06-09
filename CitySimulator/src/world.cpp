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

void Tileset::textureQuad(sf::Vertex *quad, const BlockType &blockType, int fullGID)
{
	int blockID;

	// rotated
	auto newBlockID = rotatedBlockTypes.find(fullGID);
	if (newBlockID != rotatedBlockTypes.end())
		blockID = newBlockID->second;

	// normal
	else
		blockID = blockType;

	int row = blockID % size.x;
	int col = blockID / size.x;

	quad[0].texCoords = points[getIndex(row, col)];
	quad[1].texCoords = points[getIndex(row + 1, col)];
	quad[2].texCoords = points[getIndex(row + 1, col + 1)];
	quad[3].texCoords = points[getIndex(row, col + 1)];
}

void Tileset::convertToTexture(std::vector<std::tuple<int, unsigned, std::shared_ptr<sf::Image>>> rotatedImages, int totalBlockTypeCount)
{
	// resize image to fit all blocktypes
	int rowsRequired = totalBlockTypeCount / size.x;
	if (totalBlockTypeCount % size.x != 0)
		rowsRequired += 1;

	// transfer to new image
	sf::Image newImage;
	newImage.create(size.x * Constants::tilesetResolution, rowsRequired * Constants::tilesetResolution);
	newImage.copy(*image, 0, 0);
	delete image;

	// update size
	size.y = rowsRequired;
	generatePoints();

	// render extra blocktypes
	for (auto tuple : rotatedImages)
	{
		int newID = std::get<0>(tuple);
		int tileGID = std::get<1>(tuple);
		std::shared_ptr<sf::Image> image = std::get<2>(tuple);

		int tileX = newID % size.x;
		int tileY = newID / size.x;

		newImage.copy(*image, tileX * Constants::tilesetResolution, tileY * Constants::tilesetResolution);
		image.reset();

		// fullGID: new blockID
		rotatedBlockTypes.insert(std::make_pair(tileGID, newID));
	}

	//	newImage.saveToFile("new_tileset.png");

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

void BaseWorld::setBlockType(const sf::Vector2i &pos, BlockType blockType, LayerType layer, int fullGID)
{
	int index = getBlockIndex(pos, layer);

	sf::Vertex *quad = &vertices[index];
	tileset->textureQuad(quad, blockType, fullGID);

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

	// objects
	for (Object o : debugObjects)
		target.draw(o);
}

std::pair<int, int> BaseWorld::getRotation(TMX::Tile *tile)
{
	int rotation(0);
	int flip(0);

	bool h = tile->rotation[0];
	bool v = tile->rotation[1];
	bool d = tile->rotation[2];

	if (h)
		flip |= TMX::Rotation::HORIZONTAL;
	if (v)
		flip |= TMX::Rotation::HORIZONTAL;
	if (d)
	{
		if (h && v)
		{
			rotation = 90;
			flip ^= TMX::Rotation::HORIZONTAL;
		}

		else if (h)
		{
			rotation = -90;
			flip ^= TMX::Rotation::HORIZONTAL;
		}

		else if (v)
		{
			rotation = 90;
			flip ^= TMX::Rotation::HORIZONTAL;
		}

		else
		{
			rotation = -90;
			flip ^= TMX::Rotation::HORIZONTAL;
		}
	}

	return std::make_pair(rotation, flip);
}

void BaseWorld::createRotatedImage(std::shared_ptr<sf::Image> image, TMX::Tile *tile, BlockType blockType)
{
	auto rotFlip = getRotation(tile);
	int rotation = rotFlip.first;
	int flip = rotFlip.second;

	// create image from tile
	tileset->createTileImage(image.get(), blockType);

	// transform
	if ((flip & TMX::Rotation::HORIZONTAL) != 0)
		image->flipHorizontally();
	if ((flip & TMX::Rotation::VERTICAL) != 0)
		image->flipVertically();
	if (rotation != 0)
	{
		sf::Image *rot = new sf::Image;
		*rot = Utils::rotateImage(*image, rotation);
		image.reset(rot);
	}
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

void BaseWorld::discoverTiles(std::vector<TMX::Layer*> layers, LayerType *types, int &lastBlockType,
                              std::vector<std::tuple<sf::Vector2i, LayerType, TMX::Tile*>> &tiles,
                              std::vector<std::tuple<int, unsigned, std::shared_ptr<sf::Image>>> &rotatedImages)
{
	int i(0);
	for (TMX::Layer *layer : layers)
	{
		LayerType layerType = types[i++];

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

				sf::Vector2i pos(x, y);

				// objects don't need a new rotated texture as they're sprites
				if (layerType == OBJECTS)
				{
					TMX::Object *object = dynamic_cast<TMX::Object*>(tile);
					pos = object->position;
				}

				// rotated tiles
				else if (tile->isRotated())
				{
					auto rotTexture = rotations.find(tile->rotGid);

					// not found: create
					if (rotTexture == rotations.end())
					{
						std::shared_ptr<sf::Image> image(new sf::Image);
						createRotatedImage(image, tile, blockType);

						auto newBlockIDRotGidImage = std::make_tuple(lastBlockType++, tile->rotGid, image);
						rotatedImages.push_back(newBlockIDRotGidImage);
						rotations.insert(tile->rotGid);
					}
				}

				// add to list of tiles to add to world
				tiles.push_back(std::make_tuple(pos, layerType, tile));
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

	// resize vertex array
	world->resizeVertexArray();

	// discover extra (rotated) blocktypes and collect all tiles
	int lastBlockType(LAST);
	std::vector<std::tuple<sf::Vector2i, LayerType, TMX::Tile*>> tiles;
	std::vector<std::tuple<int, unsigned, std::shared_ptr<sf::Image>>> rotatedImages;
	world->discoverTiles(layers, types, lastBlockType, tiles, rotatedImages);

	// update world and tileset to accomodate for new textures/blocktypes
	world->tileset->convertToTexture(rotatedImages, lastBlockType);
	world->blockTypes.resize(size.x * size.y * layers.size() * 4);

	// add blocks and objects to world
	for (auto posLayerTypeTile : tiles)
	{
		sf::Vector2i pos = std::get<0>(posLayerTypeTile);
		LayerType layerType = std::get<1>(posLayerTypeTile);
		TMX::Tile *tile = std::get<2>(posLayerTypeTile);
		BlockType blockType = static_cast<BlockType>(tile->gid);

		// tiles on the grid
		if (tile->isTile())
			world->setBlockType(pos, blockType, layerType, tile->rotGid);

		// objects
		else
		{
			TMX::Object *o = dynamic_cast<TMX::Object*>(tile);

			Object object(world, blockType);
			object.setRotation(o->rotationAngle);
			world->addObject(object, static_cast<sf::Vector2f>(pos));
		}
	}

	delete tmx;
	return world;
}

void BaseWorld::addObject(Object object, sf::Vector2f pos)
{
	pos.x *= Constants::tileScale;

	pos.y -= object.getLocalBounds().height;
	pos.y *= Constants::tileScale;

	object.setPosition(pos);
	debugObjects.push_back(object);
}
