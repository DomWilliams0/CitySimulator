#include <unordered_set>
#include "world.hpp"
#include "utils.hpp"
#include "service/logging_service.hpp"
#include "service/render_service.hpp"

Tileset::Tileset() : converted(false)
{
}

Tileset::~Tileset()
{
	delete points;
}

void Tileset::load(const std::string &path)
{
	// load image
	image = new sf::Image;
	if (!image->loadFromFile(path))
		error("Could not load tileset '%1%'", path);

	size = image->getSize();
	size.x /= Constants::tilesetResolution;
	size.y /= Constants::tilesetResolution;

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

sf::Texture *Tileset::getTexture() const
{
	if (!converted)
		throw std::runtime_error("Tileset has not yet been converted to a texture");

	return const_cast<sf::Texture *>(&texture);
}

sf::Image *Tileset::getImage() const
{
	if (converted)
		throw std::runtime_error("Tileset has already been converted to a texture");
	return image;
}

sf::Vector2u Tileset::getSize() const
{
	return size;
}

void Tileset::convertToTexture(const std::vector<int> &flippedGIDs)
{
	// resize image
	int totalBlockTypes = BLOCK_UNKNOWN + flippedGIDs.size();
	int rowsRequired = totalBlockTypes / size.x;

	if (totalBlockTypes % size.x != 0)
		rowsRequired += 1;

	// transfer to new image
	sf::Image newImage;
	newImage.create(size.x * Constants::tilesetResolution, rowsRequired * Constants::tilesetResolution);
	newImage.copy(*image, 0, 0);

	// update size
	size.y = rowsRequired;
	generatePoints();

	// render flipped blocktypes
	int currentBlockType(BLOCK_UNKNOWN);
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

	// write to texture
	if (!texture.loadFromImage(newImage))
		throw std::runtime_error("Could not render tileset");
	texture.setSmooth(false);

	converted = true;
	delete image;
}

sf::IntRect Tileset::getTileRect(unsigned blockType)
{
	int tileX = blockType % size.x;
	int tileY = blockType / size.x;
	return sf::IntRect(tileX * Constants::tilesetResolution,
					   tileY * Constants::tilesetResolution,
					   Constants::tilesetResolution,
					   Constants::tilesetResolution);
}

void Tileset::createTileImage(sf::Image *image, unsigned blockType)
{
	if (converted)
		throw std::runtime_error("Tileset has already been converted to a texture");

	image->create(Constants::tilesetResolution, Constants::tilesetResolution);
	image->copy(*this->image, 0, 0, getTileRect(blockType));
}

void Tileset::addPoint(int x, int y)
{
	points[getIndex(x, y)] = sf::Vector2f(x * Constants::tilesetResolution,
										  y * Constants::tilesetResolution);
}

void Tileset::generatePoints()
{
	points = new sf::Vector2f[(size.x + 1) * (size.y + 1)];
	for (size_t y = 0; y <= size.y; y++)
		for (size_t x = 0; x <= size.x; x++)
			addPoint(x, y);
}

int Tileset::getIndex(int x, int y) const
{
	return x + (size.x + 1) * y;
}


WorldTerrain::WorldTerrain(World *container) : BaseWorld(container)
{
	tileVertices.setPrimitiveType(sf::Quads);
	overLayerVertices.setPrimitiveType(sf::Quads);
}

WorldTerrain::~WorldTerrain()
{
}

int WorldTerrain::getBlockIndex(const sf::Vector2i &pos, LayerType layerType)
{
	int index = (pos.x + pos.y * container->tileSize.x);
	index += layers.at(layerType).depth * container->tileSize.x * container->tileSize.y;
	index *= 4;

	return index;
}


int WorldTerrain::getVertexIndex(const sf::Vector2i &pos, LayerType layerType)
{
	int index = (pos.x + pos.y * container->tileSize.x);
	int depth = layers.at(layerType).depth;
	if (isOverLayer(layerType))
	{
		int diff = tileLayerCount - overLayerCount;
		depth -= diff;
	}

	index += depth * container->tileSize.x * container->tileSize.y;
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

void WorldTerrain::positionVertices(sf::Vertex *quad, const sf::Vector2f &pos, int delta)
{
	quad[0].position = sf::Vector2f(pos.x, pos.y);
	quad[1].position = sf::Vector2f(pos.x + delta, pos.y);
	quad[2].position = sf::Vector2f(pos.x + delta, pos.y + delta);
	quad[3].position = sf::Vector2f(pos.x, pos.y + delta);
}


sf::VertexArray &WorldTerrain::getVertices(const LayerType &layerType)
{
	return isOverLayer(layerType) ? overLayerVertices : tileVertices;
}

void WorldTerrain::resizeVertices()
{
	sf::Vector2i tilesetResolution(container->getTileSize());
	const int sizeMultiplier = tilesetResolution.x * tilesetResolution.y * 4;

	blockTypes.resize(tileLayerCount * sizeMultiplier);

	tileVertices.resize((tileLayerCount - overLayerCount) * sizeMultiplier);
	overLayerVertices.resize(overLayerCount * sizeMultiplier);
}

void WorldTerrain::registerLayer(LayerType layerType, int depth)
{
	layers.emplace_back(layerType, depth);
	Logger::logDebuggier(format("Found %3%layer type %1% at depth %2%", _str(layerType), _str(depth),
								isOverLayer(layerType) ? "overterrain " : ""));
}

void WorldTerrain::setBlockType(const sf::Vector2i &pos, BlockType blockType, LayerType layer, int rotationAngle,
								int flipGID)
{
	int vertexIndex = getVertexIndex(pos, layer);
	sf::VertexArray &vertices = getVertices(layer);
	auto size = vertices.getVertexCount();
	sf::Vertex *quad = &vertices[vertexIndex];

	positionVertices(quad, static_cast<sf::Vector2f>(pos), 1);
	tileset.textureQuad(quad, blockType, rotationAngle, flipGID);

	blockTypes[getBlockIndex(pos, layer)] = blockType;
}

void WorldTerrain::addObject(const sf::Vector2f &pos, BlockType blockType, float rotationAngle, int flipGID)
{
	// TODO: simply append object vertices to world vertices; remember order of objects so vertices can be referenced in the future

	std::vector<sf::Vertex> quad(4);
	sf::Vector2f adjustedPos = sf::Vector2f(pos.x / Constants::tilesetResolution,
											(pos.y - Constants::tilesetResolution) / Constants::tilesetResolution);

	positionVertices(&quad[0], adjustedPos, 1);
	tileset.textureQuad(&quad[0], blockType, 0, flipGID);

	if (rotationAngle != 0)
		rotateObject(&quad[0], rotationAngle, adjustedPos);

	sf::VertexArray &vertices = getVertices(LAYER_OBJECTS);
	for (int i = 0; i < 4; ++i)
		vertices.append(quad[i]);

	objects.emplace_back(blockType, rotationAngle, Utils::toTile(pos));
}

const std::vector<WorldObject> &WorldTerrain::getObjects()
{
	return objects;
}

const std::vector<WorldLayer> &WorldTerrain::getLayers()
{
	return layers;
}

void WorldTerrain::discoverLayers(std::vector<TMX::Layer *> &layers, std::vector<LayerType> &layerTypes)
{
	auto layerIt = layers.begin();
	int depth(0);
	tileLayerCount = 0;
	overLayerCount = 0;
	while (layerIt != layers.end())
	{
		auto layer = *layerIt;

		// unknown layer type
		LayerType layerType = layerTypeFromString(layer->name);
		if (layerType == LAYER_UNKNOWN)
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

		if (isTileLayer(layerType))
			++tileLayerCount;
		if (isOverLayer(layerType))
			++overLayerCount;

		// add layer
		registerLayer(layerType, depth);
		layerTypes.push_back(layerType);

		++depth;
		++layerIt;
	}
}

void WorldTerrain::discoverFlippedTiles(const std::vector<TMX::Layer *> &layers, std::vector<int> &flippedGIDs)
{
	std::unordered_set<int> flipped;

	for (TMX::Layer *layer : layers)
	{
		for (TMX::Tile *tile : layer->items)
		{
			// doesn't exist or isn't flipped
			if (tile == nullptr || !tile->isFlipped())
				continue;

			if (tile->getGID() == BLOCK_BLANK)
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

void WorldTerrain::addTiles(const std::vector<TMX::Layer *> &layers, const std::vector<LayerType> &types)
{
	sf::Vector2i tileSize = container->tileSize;
	sf::Vector2i pos;
	int layerIndex(0);

	for (TMX::Layer *layer : layers)
	{
		LayerType layerType = types[layerIndex++];

		if (layerType == LAYER_OBJECTS)
		{
			// objects
			for (TMX::Tile *item : layer->items)
			{
				BlockType blockType = static_cast<BlockType>(item->getGID());
				if (blockType == BLOCK_BLANK)
					continue;

				TMX::Object *object = dynamic_cast<TMX::Object *>(item);
				addObject(object->position, blockType, object->rotationAnglef, item->getFlipGID());
			}
		}

		else if (isTileLayer(layerType))
		{
			// tiles
			for (int y = 0; y < tileSize.y; ++y)
			{
				for (int x = 0; x < tileSize.x; ++x)
				{
					TMX::Tile *tile = layer->items[x + y * tileSize.x];
					if (tile == nullptr)
						continue;

					BlockType blockType = static_cast<BlockType>(tile->getGID());
					if (blockType == BLOCK_BLANK)
						continue;

					pos.x = x;
					pos.y = y;

					setBlockType(pos, blockType, layerType, tile->getRotationAngle(), tile->getFlipGID());
				}
			}
		}
	}
}

void WorldTerrain::render(sf::RenderTarget &target, sf::RenderStates &states, bool overLayers) const
{
	states.texture = tileset.getTexture();
	target.draw(overLayers ? overLayerVertices : tileVertices, states);
}

void WorldTerrain::load(const TMX::TileMap *tileMap, const std::string &tilesetPath)
{
	// find layer count and depths
	std::vector<TMX::Layer *> layers = tileMap->layers;
	std::vector<LayerType> types;
	discoverLayers(layers, types);

	Logger::logDebug(format("Discovered %1% tile layer(s), of which %2% is/are overlayer(s)",
							_str(tileLayerCount), _str(overLayerCount)));

	// resize vertex array to accommodate for layer count
	resizeVertices();

	// update tileset with flipped textures
	std::vector<int> flippedGIDs;
	discoverFlippedTiles(layers, flippedGIDs);
	tileset.load(tilesetPath);
	tileset.convertToTexture(flippedGIDs);

	// add tiles to terrain
	addTiles(layers, types);
}

RenderService::RenderService(sf::RenderWindow *renderWindow) : window(renderWindow)
{
}

void RenderService::onEnable()
{
}

sf::RenderWindow *RenderService::getWindow()
{
	return window;
}

void RenderService::render(const World &world)
{
	limitView(world);

	window->setView(*view);
	window->draw(world);
}

sf::Vector2f RenderService::mapScreenToWorld(const sf::Vector2i &screenPos)
{
	return window->mapPixelToCoords(screenPos, *view);
}

void RenderService::limitView(const World &world)
{
	const sf::Vector2i &worldSize = world.getPixelSize();
	bool update = false;

	sf::Vector2f centre = view->getCenter();
	sf::Vector2f size = view->getSize();

	// horizontal
	if (size.x > worldSize.x)
	{
		update = true;
		centre.x = worldSize.x / 2;
	}
	else
	{

		float leftBound = centre.x - size.x / 2.f;
		if (leftBound < 0)
		{
			update = true;
			centre.x -= leftBound;
		}
		else
		{
			float rightBound = centre.x + size.x / 2.f;
			if (rightBound > worldSize.x)
			{
				update = true;
				centre.x -= (rightBound - worldSize.x);
			}
		}
	}

	// vertical
	if (size.y > worldSize.y)
	{
		update = true;
		centre.y = worldSize.y / 2;
	}
	else
	{
		float topBound = centre.y - size.y / 2.f;
		if (topBound < 0)
		{
			update = true;
			centre.y -= topBound;
		}
		else
		{
			float bottomBound = centre.y + size.y / 2.f;
			if (bottomBound > worldSize.y)
			{
				update = true;
				centre.y -= (bottomBound - worldSize.y);
			}
		}
	}

	if (update)
		view->setCenter(centre);
}

