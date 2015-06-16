#include <SFML\System\Vector2.hpp>
#include <boost\shared_ptr.hpp>
#include <exception>
#include "maploader.hpp"
#include "world.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include <unordered_set>

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
	return LT_ERROR;
}

Tileset::Tileset(const std::string &filename) : converted(false)
{
	// load image
	image = new sf::Image;
	if (!image->loadFromFile(Utils::searchForFile(filename, "res/world")))
		throw std::runtime_error("Could not load tileset");

	size = image->getSize();
	size.x /= Constants::tileSize;
	size.y /= Constants::tileSize;

	generatePoints();
}

void Tileset::textureQuad(sf::Vertex *quad, const BlockType &blockType, int rotationAngle, int flipGID)
{
	int blockID;
	auto flipResult = flippedBlockTypes.find(flipGID);

	// not flipped
	if (flipGID == 0 || flipResult == flippedBlockTypes.end())
		blockID = blockType;

	// flipped
	else
		blockID = flipResult->second;


	int row = blockID % size.x;
	int col = blockID / size.x;

	// rotating
	int offset = 0;
	if (rotationAngle == -90)
		offset = 1;
	else if (rotationAngle == 90)
		offset = 3;

	quad[(0 + offset) % 4].texCoords = points[getIndex(row, col)];
	quad[(1 + offset) % 4].texCoords = points[getIndex(row + 1, col)];
	quad[(2 + offset) % 4].texCoords = points[getIndex(row + 1, col + 1)];
	quad[(3 + offset) % 4].texCoords = points[getIndex(row, col + 1)];
}

void Tileset::convertToTexture(const std::vector<int> &flippedGIDs)
{
	// resize image
	int totalBlockTypes = LAST + flippedGIDs.size();
	int rowsRequired = totalBlockTypes / size.x;

	if (totalBlockTypes % size.x != 0)
		rowsRequired += 1;

	// transfer to new image
	sf::Image newImage;
	newImage.create(size.x * Constants::tileSize, rowsRequired * Constants::tileSize);
	newImage.copy(*image, 0, 0);

	// update size
	size.y = rowsRequired;
	generatePoints();

	// render flipped blocktypes
	int currentBlockType(LAST);
	for (int flippedGID : flippedGIDs)
	{
		std::bitset<3> flips;
		int blockType(TMX::stripFlip(flippedGID, flips));

		// create image
		sf::Image flippedImage;
		flippedImage.create(16, 16);
		createTileImage(&flippedImage, blockType);

		// flip
		if (flips[0])
			flippedImage.flipHorizontally();
		if (flips[1])
			flippedImage.flipVertically();

		// copy to tileset
		sf::IntRect rect = getTileRect(currentBlockType);
		newImage.copy(flippedImage, rect.left, rect.top);

		// remember this new blocktype
		flippedBlockTypes.insert(std::make_pair(flippedGID, currentBlockType));

		++currentBlockType;
	}

	// debug
	newImage.saveToFile("new_tileset.png");

	// write to texture
	if (!texture.loadFromImage(newImage))
		throw std::exception("Could not render tileset");

	converted = true;
	delete image;
}

sf::IntRect Tileset::getTileRect(unsigned blockType)
{
	int tileX = blockType % size.x;
	int tileY = blockType / size.x;
	return sf::IntRect(tileX * Constants::tileSize,
	                   tileY * Constants::tileSize,
	                   Constants::tileSize,
	                   Constants::tileSize);
}

void Tileset::createTileImage(sf::Image *image, unsigned blockType)
{
	if (converted)
		throw std::exception("Tileset has already been converted to a texture");

	image->create(Constants::tileSize, Constants::tileSize);
	image->copy(*this->image, 0, 0, getTileRect(blockType));
}

void Tileset::generatePoints()
{
	points = new sf::Vector2f[(size.x + 1) * (size.y + 1)];
	for (size_t x = 0; x <= size.x; x++)
		for (size_t y = 0; y <= size.y; y++)
			addPoint(x, y);
}

BaseWorld::BaseWorld(const sf::Vector2i &size) : tileSize(size), pixelSize(Utils::toPixel(tileSize)),
                                                 tileset(new Tileset("tileset.png"))
{
	vertices.setPrimitiveType(sf::Quads);
	transform.scale(Constants::tileSizef, Constants::tileSizef);
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

void BaseWorld::rotateObject(sf::Vertex *quad, float degrees, const sf::Vector2f &pos)
{
	sf::Vector2f origin(pos.x, pos.y + 1);

	float radius(degrees * Constants::degToRad);
	const float c(cos(radius));
	const float s(sin(radius));

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


void BaseWorld::positionVertices(sf::Vertex *quad, const sf::Vector2f &pos, int delta)
{
	quad[0].position = sf::Vector2f(pos.x, pos.y);
	quad[1].position = sf::Vector2f(pos.x + delta, pos.y);
	quad[2].position = sf::Vector2f(pos.x + delta, pos.y + delta);
	quad[3].position = sf::Vector2f(pos.x, pos.y + delta);
}

void BaseWorld::setBlockType(const sf::Vector2i &pos, BlockType blockType, LayerType layer, int rotationAngle, int flipGID)
{
	int index = getBlockIndex(pos, layer);
	sf::Vertex *quad = &vertices[index];

	positionVertices(quad, static_cast<sf::Vector2f>(pos), 1);
	tileset->textureQuad(quad, blockType, rotationAngle, flipGID);

	blockTypes[index] = blockType;
}

void BaseWorld::addObject(const sf::Vector2f &pos, BlockType blockType, LayerType layer, float rotationAngle, int flipGID)
{
	// TODO: simply append object vertices to world vertices; remember order of objects so vertices can be referenced in the future

	std::vector<sf::Vertex> quad(4);
	sf::Vector2f adjustedPos = sf::Vector2f(pos.x / Constants::tileSize,
	                                        (pos.y - Constants::tileSize) / Constants::tileSize);

	positionVertices(&quad[0], adjustedPos, 1);
	tileset->textureQuad(&quad[0], blockType, rotationAngle, flipGID);

	if (rotationAngle != 0)
		rotateObject(&quad[0], rotationAngle, adjustedPos);

	for (int i = 0; i < 4; ++i)
		vertices.append(quad[i]);
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
		if (layerType == LT_ERROR)
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

void BaseWorld::discoverFlippedTiles(const std::vector<TMX::Layer*> &layers, std::vector<int> &flippedGIDs)
{
	std::unordered_set<int> flipped;

	for (TMX::Layer *layer : layers)
	{
		for (TMX::Tile *tile : layer->items)
		{
			// doesn't exist or isn't flipped
			if (tile == nullptr || !tile->isFlipped())
				continue;

			if (tile->gid == BLANK)
				continue;

			int flipGID = tile->getFlipGID();

			// already done
			if (flipped.find(flipGID) != flipped.end())
				continue;

			flippedGIDs.push_back(flipGID);
			flipped.insert(flipGID);
		}
	}
}

void BaseWorld::addTiles(std::vector<TMX::Layer*> layers, LayerType *types)
{
	sf::Vector2i pos;
	int layerIndex(0);
	for (TMX::Layer *layer : layers)
	{
		LayerType layerType = types[layerIndex++];

		for (int x = 0; x < tileSize.x; ++x)
		{
			for (int y = 0; y < tileSize.y; ++y)
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
					addObject(object->position, blockType, layerType, object->rotationAnglef, tile->getFlipGID());
				}

				// tiles
				else
				{
					pos.x = x;
					pos.y = y;

					setBlockType(pos, blockType, layerType, tile->getRotationAngle(), tile->getFlipGID());
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
	LayerType types[LayerType::LT_ERROR];
	world->discoverLayers(layers, types);

	// resize vertex array to accomodate for layer count
	world->resizeVertexArray();

	// update tileset with flipped textures
	std::vector<int> flippedGIDs;
	world->discoverFlippedTiles(layers, flippedGIDs);
	world->tileset->convertToTexture(flippedGIDs);

	// add tiles to world
	world->addTiles(layers, types);

	delete tmx;
	return world;
}
