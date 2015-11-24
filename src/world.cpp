#include <SFML/System/Vector2.hpp>
#include <boost/shared_ptr.hpp>
#include "maploader.hpp"
#include "world.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include <unordered_set>
#include <boost/format.hpp>
#include <map>
#include <set>

bool isCollidable(BlockType blockType)
{
	static const std::set<BlockType> collidables({WATER, TREE, BUILDING_WALL, BUILDING_EDGE, BUILDING_ROOF, BUILDING_ROOF_CORNER});
	return collidables.find(blockType) != collidables.end();
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

	Logger::logWarning("Unknown LayerType: " + s);
	return LAYER_COUNT;
}

bool isTileLayer(LayerType &layerType)
{
	return layerType == LAYER_UNDERTERRAIN || layerType == LAYER_TERRAIN || layerType == LAYER_OVERTERRAIN;
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

Tileset::~Tileset()
{
	delete points;
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

sf::Texture* Tileset::getTexture()
{
	if (!converted)
		throw std::runtime_error("Tileset has not yet been converted to a texture");

	return &texture;
}

sf::Image* Tileset::getImage() const
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

	// write to texture
	if (!texture.loadFromImage(newImage))
		throw std::runtime_error("Could not render tileset");

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
		throw std::runtime_error("Tileset has already been converted to a texture");

	image->create(Constants::tileSize, Constants::tileSize);
	image->copy(*this->image, 0, 0, getTileRect(blockType));
}

void Tileset::addPoint(int x, int y)
{
	points[getIndex(x, y)] = sf::Vector2f(x * Constants::tileSizef,
	                                      y * Constants::tileSizef);
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

WorldTerrain::WorldTerrain(World *container) : BaseWorld(container), tileset(new Tileset("tileset.png"))
{
	vertices.setPrimitiveType(sf::Quads);
}

WorldTerrain::~WorldTerrain()
{
	delete tileset;
}

int WorldTerrain::getBlockIndex(const sf::Vector2i &pos, LayerType layerType)
{
	auto depth = layerDepths[layerType];
	if (depth != 0)
		depth -= 1;

	int index = (pos.x + pos.y * container->tileSize.x);
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

void WorldTerrain::resize(const int &layerCount)
{
	auto tileSize = container->getTileSize();
	const int size = tileSize.x * tileSize.y * layerCount * 4;
	vertices.resize(size);
	blockTypes.resize(size);
}

void WorldTerrain::registerLayer(LayerType layerType, int depth)
{
	layerDepths.insert(std::make_pair(layerType, depth));
}

void WorldTerrain::setBlockType(const sf::Vector2i &pos, BlockType blockType, LayerType layer, int rotationAngle, int flipGID)
{
	int index = getBlockIndex(pos, layer);
	sf::Vertex *quad = &vertices[index];

	positionVertices(quad, static_cast<sf::Vector2f>(pos), 1);
	tileset->textureQuad(quad, blockType, rotationAngle, flipGID);

	blockTypes[index] = blockType;
}

void WorldTerrain::addObject(const sf::Vector2f &pos, BlockType blockType, LayerType layer, float rotationAngle, int flipGID)
{
	// TODO: simply append object vertices to world vertices; remember order of objects so vertices can be referenced in the future

	std::vector<sf::Vertex> quad(4);
	sf::Vector2f adjustedPos = sf::Vector2f(pos.x / Constants::tileSize,
	                                        (pos.y - Constants::tileSize) / Constants::tileSize);

	positionVertices(&quad[0], adjustedPos, 1);
	tileset->textureQuad(&quad[0], blockType, 0, flipGID);

	if (rotationAngle != 0)
		rotateObject(&quad[0], rotationAngle, adjustedPos);

	for (int i = 0; i < 4; ++i)
		vertices.append(quad[i]);
}

Tileset* WorldTerrain::getTileset() const
{
	return tileset;
}

int WorldTerrain::discoverLayers(std::vector<TMX::Layer*> &layers, std::vector<LayerType> &layerTypes)
{
	auto layerIt = layers.begin();
	int depth(1);
	int tileLayerCount(0);
	while (layerIt != layers.end())
	{
		auto layer = *layerIt;

		// unknown layer type
		LayerType layerType = layerTypeFromString(layer->name);
		if (layerType == LAYER_COUNT)
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

		// add layer
		registerLayer(layerType, depth);
		layerTypes.push_back(layerType);

		++depth;
		++layerIt;
	}

	return tileLayerCount;
}

void WorldTerrain::discoverFlippedTiles(const std::vector<TMX::Layer*> &layers, std::vector<int> &flippedGIDs)
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

void WorldTerrain::addTiles(const std::vector<TMX::Layer*> &layers, const std::vector<LayerType> &types)
{
	sf::Vector2i tileSize = container->tileSize;
	sf::Vector2i pos;
	int layerIndex(0);

	for (TMX::Layer *layer : layers)
	{
		LayerType layerType = types[layerIndex++];

		for (int y = 0; y < tileSize.y; ++y)
		{
			for (int x = 0; x < tileSize.x; ++x)
			{
				TMX::Tile *tile = layer->items[x + y * tileSize.x];
				if (tile == nullptr)
					continue;

				BlockType blockType = static_cast<BlockType>(tile->gid);
				if (blockType == BLANK)
					continue;

				// objects are not stuck to the grid
				if (layerType == LAYER_OBJECTS)
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

void WorldTerrain::render(sf::RenderTarget &target, sf::RenderStates &states) const
{
	states.texture = tileset->getTexture();
	target.draw(vertices, states);
}

void WorldTerrain::load(const TMX::TileMap *tileMap)
{
	// find layer count and depths
	auto layers = tileMap->layers;
	std::vector<LayerType> types;
	int tileLayerCount = discoverLayers(layers, types);

	Logger::logDebug(FORMAT("Discovered %1% tile layer(s)", tileLayerCount));

	// resize vertex array to accomodate for layer count
	resize(tileLayerCount);

	// update tileset with flipped textures
	std::vector<int> flippedGIDs;
	discoverFlippedTiles(layers, flippedGIDs);
	tileset->convertToTexture(flippedGIDs);

	// add tiles to terrain
	addTiles(layers, types);
}

bool compareRectsHorizontally(const sf::FloatRect &a, const sf::FloatRect &b)
{
	if (a.top < b.top) return true;
	if (b.top < a.top) return false;

	if (a.left < b.left) return true;
	if (b.left < a.left) return false;

	return false;
}

bool compareRectsVertically(const sf::FloatRect &a, const sf::FloatRect &b)
{
	if (a.left < b.left) return true;
	if (b.left < a.left) return false;

	if (a.top < b.top) return true;
	if (b.top < a.top) return false;

	return false;
}

void CollisionMap::findCollidableTiles(std::vector<sf::FloatRect> &rectsRet) const
{
	sf::Vector2i worldTileSize = container->getTileSize();

	// find collidable tiles
	for (auto y = 0; y < worldTileSize.y; ++y)
	{
		for (auto x = 0; x < worldTileSize.x; ++x)
		{
			BlockType bt = container->getBlockAt({x, y}, LAYER_TERRAIN); // the only collidable tile layer
			if (!isCollidable(bt))
				continue;

			sf::Vector2f pos(static_cast<float>(x) * Constants::tileSizef, static_cast<float>(y) * Constants::tileSizef);
			sf::Vector2f size(Constants::tileSizef, Constants::tileSizef); // todo: assuming all tiles are the same size

			sf::FloatRect rect(pos, size);
			rectsRet.push_back(rect);

			// todo
			// fill every tile with a single tile sized rectangle
			// attempt to merge every rectangle with its adjacent rectangles (smallest/largest area first?)
			// vertically, then horizontally, then choose the set with the least rectangles
		}
	}

	// todo repeat for object layer
}

void CollisionMap::mergeAdjacentTiles(std::vector<sf::Rect<float>> &rects)
{
	// join individual rects
	sort(rects.begin(), rects.end(), compareRectsHorizontally);
	mergeHelper(rects, true);

	// join rows together
	sort(rects.begin(), rects.end(), compareRectsVertically);
	mergeHelper(rects, false);
}

void CollisionMap::mergeHelper(std::vector<sf::FloatRect> &rects, bool moveOnIfFar)
{
	bool (*nextRowFunc)(const sf::FloatRect *last, const sf::FloatRect *current);
	if (moveOnIfFar)
	{
		nextRowFunc = [](const sf::FloatRect *lastRect, const sf::FloatRect *rect)
			{
				return powf(rect->left - lastRect->left, 2.f) + powf(rect->top - lastRect->top, 2.f) > Constants::tileSizef * Constants::tileSizef;
			};;
	}
	else
	{
		nextRowFunc = [](const sf::FloatRect *lastRect, const sf::FloatRect *rect)
			{
				// adjacent and same dimensions
				return !(lastRect->left <= rect->left + rect->width &&
					rect->left <= lastRect->left + lastRect->width &&
					lastRect->top <= rect->top + rect->height &&
					rect->top <= lastRect->top + lastRect->height &&
					lastRect->width == rect->width && lastRect->height == rect->height);
			};
	}


	std::vector<sf::FloatRect> rectsCopy(rects.begin(), rects.end());
	rects.clear();

	sf::FloatRect *current = nullptr;
	sf::FloatRect *lastRect = nullptr;

	rectsCopy.push_back(sf::FloatRect(-100.f, -100.f, 0.f, 0.f)); // to ensure the last rect is included

	for (size_t i = 0; i < rectsCopy.size(); ++i)
	{
		sf::FloatRect *rect = &rectsCopy[i];

		// no current rect expanding
		if (current == nullptr)
		{
			current = lastRect = rect;
			continue;
		}

		if ((nextRowFunc)(lastRect, rect))
		{
			rects.push_back(*current);
			current = lastRect = rect;
			continue;
		}

		// stretch current
		current->left = std::min(current->left, rect->left);
		current->top = std::min(current->top, rect->top);
		current->width = std::max(current->left + current->width, rect->left + rect->width) - current->left;
		current->height = std::max(current->top + current->height, rect->top + rect->height) - current->top;

		lastRect = rect;
	}
}

CollisionMap::~CollisionMap()
{
	if (worldBody != nullptr)
		world.DestroyBody(worldBody);
}

void CollisionMap::getSurroundingTiles(const sf::Vector2i &tilePos, std::set<sf::Rect<float>> &ret)
{
	const static int edge = 1; // todo dependent on entity size

	// gather all (unique) rects in the given range
	sf::FloatRect rect;
	for (int y = -edge; y <= edge; ++y)
	{
		for (int x = -edge; x <= edge; ++x)
		{
			sf::Vector2i offsetTile(tilePos.x + x, tilePos.y + y);
			if (getRectAt(offsetTile, rect))
				ret.insert(rect);
		}
	}
}

bool CollisionMap::getRectAt(const sf::Vector2i &tilePos, sf::FloatRect &ret)
{
	auto result(cellGrid.find(Utils::toPixel(tilePos)));
	if (result == cellGrid.end())
		return false;

	ret = result->second;
	return true;
}

void CollisionMap::load()
{
	std::vector<sf::FloatRect> rects;

	// gather all collidable tiles
	findCollidableTiles(rects);

	// merge adjacents
	mergeAdjacentTiles(rects);

	// debug drawing
	world.SetDebugDraw(&b2Renderer);
	b2Renderer.SetFlags(b2Draw::e_shapeBit);

	// create world body
	b2BodyDef worldBodyDef;
	worldBodyDef.type = b2_staticBody;
	worldBody = world.CreateBody(&worldBodyDef);

	// collision fixtures
	b2FixtureDef fixDef;
	b2PolygonShape box;
	fixDef.shape = &box;
	for (auto &unscaledRect : rects)
	{
		auto rect = Utils::scaleToBox2D(unscaledRect);

		box.SetAsBox(rect.width / 2, rect.height / 2, b2Vec2(rect.left / 2 + rect.width / 2, rect.top + rect.height / 2), 0.f);
		Debug::printVector(sf::Vector2f(rect.left, rect.top), "pos ");
		Debug::printVector(sf::Vector2f(rect.width, rect.height), "size ");
		worldBody->CreateFixture(&fixDef);
	}
}

World::World(): terrain(this), collisionMap(this)
{
	transform.scale(Constants::tileSizef, Constants::tileSizef);
}

void World::loadFromFile(const std::string &filename)
{
	Logger::logDebug(str(boost::format("Began loading world %1%") % filename));
	Logger::pushIndent();

	TMX::TileMap *tmx = TMX::TileMap::load(filename);

	// failure
	if (tmx == nullptr)
		throw std::runtime_error("Could not load world from null TileMap");

	sf::Vector2i size(tmx->width, tmx->height);
	resize(size);

	// terrain
	terrain.load(tmx);
	collisionMap.load();

	Logger::popIndent();
	Logger::logDebug(str(boost::format("Loaded world %1%") % filename));
	delete tmx;
}

void World::resize(sf::Vector2i size)
{
	tileSize = size;
	pixelSize = Utils::toPixel(size);
}

WorldTerrain& World::getTerrain()
{
	return terrain;
}

CollisionMap& World::getCollisionMap()
{
	return collisionMap;
}

b2World* World::getBox2DWorld()
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
	terrain.render(target, states);
}
