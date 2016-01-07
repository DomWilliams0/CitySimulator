#include "world.hpp"
#include "services.hpp"

bool isCollidable(BlockType blockType)
{
	static const std::set<BlockType> collidables(
			{BLOCK_WATER, BLOCK_TREE, BLOCK_BUILDING_WALL, BLOCK_BUILDING_EDGE, BLOCK_BUILDING_ROOF, BLOCK_BUILDING_ROOF_CORNER});
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

void CollisionMap::findCollidableTiles(std::vector<CollisionRect> &rects) const
{
	sf::Vector2i worldTileSize = container->getTileSize();

	// find collidable tiles
	sf::Vector2f size(Constants::tileSizef, Constants::tileSizef); // todo: assuming all tiles are the same size
	for (auto y = 0; y < worldTileSize.y; ++y)
	{
		for (auto x = 0; x < worldTileSize.x; ++x)
		{
			BlockType bt = container->getBlockAt({x, y}, LAYER_TERRAIN); // the only collidable tile layer
			if (!isCollidable(bt))
				continue;

			sf::Vector2f pos(Utils::toPixel(sf::Vector2f(x, y)));
			rects.emplace_back(sf::FloatRect(pos, size), 0.f);
		}
	}

	// objects
	auto objects = container->getTerrain().getObjects();
	for (auto &obj : objects)
	{
		auto pos = obj.tilePos;
		pos.y -= 1 / Constants::scale;
		pos = Math::multiply(pos, Constants::tileScale);

		rects.emplace_back(sf::FloatRect(pos, size), obj.rotation);
	}
}

void CollisionMap::mergeAdjacentTiles(std::vector<CollisionRect> &rects, std::vector<sf::FloatRect> &ret)
{
	std::vector<sf::FloatRect> rectangles;

	auto it = rects.begin();
	while (it != rects.end())
	{
		if (it->rotation == 0.f)
		{
			rectangles.push_back(it->rect);
			it = rects.erase(it);
		}
		else
			++it;
	}

	// join individual rects
	sort(rectangles.begin(), rectangles.end(), compareRectsHorizontally);
	mergeHelper(rectangles, true);

	// join rows together
	sort(rectangles.begin(), rectangles.end(), compareRectsVertically);
	mergeHelper(rectangles, false);

	// add back to returning list
	for (auto &mergedRect : rectangles)
		rects.emplace_back(mergedRect, 0.f);
}

void CollisionMap::mergeHelper(std::vector<sf::FloatRect> &rects, bool moveOnIfFar)
{
	bool (*nextRowFunc)(const sf::FloatRect *last, const sf::FloatRect *current);
	if (moveOnIfFar)
	{
		nextRowFunc = [](const sf::FloatRect *lastRect, const sf::FloatRect *rect)
		{
			return powf(rect->left - lastRect->left, 2.f) + powf(rect->top - lastRect->top, 2.f) >
			       Constants::tileSizef * Constants::tileSizef;
		};
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
	std::vector<CollisionRect> rects;
	std::vector<sf::FloatRect> mergedRects;

	// gather all collidable tiles
	findCollidableTiles(rects);

	// merge adjacents
	mergeAdjacentTiles(rects, mergedRects);

	// debug drawing
	sf::RenderWindow *window = Locator::locate<RenderService>()->getWindow();
	if (window != nullptr)
	{
		b2Renderer.emplace(*window);
		world.SetDebugDraw(&*b2Renderer);
		b2Renderer->SetFlags(b2Draw::e_shapeBit);
	}

	// create world body
	b2BodyDef worldBodyDef;
	worldBodyDef.type = b2_staticBody;
	worldBody = world.CreateBody(&worldBodyDef);

	// world borders
	int borderThickness = Constants::tileSize;
	int padding = Constants::tileSize / 4;
	auto worldSize = container->pixelSize;
	rects.emplace_back(sf::FloatRect(-borderThickness - padding, 0, borderThickness, worldSize.y), 0.f);
	rects.emplace_back(sf::FloatRect(0, -borderThickness - padding, worldSize.x, borderThickness), 0.f);
	rects.emplace_back(sf::FloatRect(worldSize.x + padding, 0, borderThickness, worldSize.y), 0.f);
	rects.emplace_back(sf::FloatRect(0, worldSize.y + padding, worldSize.x, borderThickness), 0.f);

	// todo make big collision rectangles hollow to work better with box2d?

	// collision fixtures
	b2FixtureDef fixDef;
	b2PolygonShape box;
	fixDef.shape = &box;
	fixDef.friction = 0.1f;

	// rotated
	for (auto &collisionRect : rects)
	{
		sf::FloatRect aabb = Utils::scaleToBox2D(collisionRect.rect);
		sf::Vector2f size(aabb.width, aabb.height);

		if (collisionRect.rotation != 0.f)
		{
			sf::Transform transform;
			transform.rotate(collisionRect.rotation, aabb.left, aabb.top + aabb.height);
			aabb = transform.transformRect(aabb);
		}

		box.SetAsBox(
				size.x / 2, // half dimensions
				size.y / 2,
				b2Vec2(aabb.left + aabb.width / 2, aabb.top + aabb.height / 2),
				collisionRect.rotation
		);
		worldBody->CreateFixture(&fixDef);
	}
}

World::World() : terrain(this), collisionMap(this)
{
	transform.scale(Constants::tileSizef, Constants::tileSizef);
}

void World::loadFromFile(const std::string &filename, const std::string &tileset)
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
