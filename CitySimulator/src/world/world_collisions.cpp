#include "world.hpp"
#include "service/render_service.hpp"
#include "service/locator.hpp"

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
			bool collide = isCollidable(bt);
			bool interact = isInteractable(bt);

			if (!collide && !interact)
				continue;

			sf::Vector2f pos(Utils::toPixel(sf::Vector2f(x, y)));
			rects.emplace_back(sf::FloatRect(pos, size), 0.f, bt);
		}
	}

	// objects
	auto objects = container->getTerrain()->getObjects();
	for (auto &obj : objects)
	{
		auto pos = obj.tilePos;
		pos.y -= 1 / Constants::scale;
		pos = Math::multiply(pos, Constants::tileScale);

		rects.emplace_back(sf::FloatRect(pos, size), obj.rotation, obj.type);
	}
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

void CollisionMap::mergeAdjacentTiles(std::vector<CollisionRect> &rects, std::vector<sf::FloatRect> &ret)
{
	std::vector<sf::FloatRect> rectangles;

	auto it = rects.begin();
	while (it != rects.end())
	{
		if (!isInteractable(it->blockType) && it->rotation == 0.f)
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
	if (Config::getBool("debug.render-physics", false) && window != nullptr)
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

	for (auto &collisionRect : rects)
	{
		sf::FloatRect aabb = Utils::scaleToBox2D(collisionRect.rect);
		sf::Vector2f size(aabb.width, aabb.height);
		fixDef.userData = nullptr;

		// rotated
		if (collisionRect.rotation != 0.f)
		{
			sf::Transform transform;
			transform.rotate(collisionRect.rotation, aabb.left, aabb.top + aabb.height);
			aabb = transform.transformRect(aabb);
		}

		// attach block data
		fixDef.userData = createBodyData(collisionRect.blockType, {(int) aabb.left, (int) aabb.top});

		box.SetAsBox(
				size.x / 2, // half dimensions
				size.y / 2,
				b2Vec2(aabb.left + aabb.width / 2, aabb.top + aabb.height / 2),
				collisionRect.rotation
		);
		worldBody->CreateFixture(&fixDef);
	}
}

BodyData *CollisionMap::createBodyData(BlockType blockType, const sf::Vector2i &tilePos)
{
	// outside building doors
	if (blockType == BLOCK_SLIDING_DOOR)
	{
		BodyData *data = new BodyData; // todo cache
		data->type = BODYDATA_BLOCK;
		data->blockData.blockDataType = BLOCKDATA_DOOR;

		boost::optional<std::pair<Building *, Door *>> buildingAndDoor;
		// todo
		/* container->getBuildingMap().getBuildingByOutsideDoorTile(tilePos, buildingAndDoor); */

		if (!buildingAndDoor)
		{
			Logger::logWarning(
					format("Cannot add block data for door at (%1%, %2%), because there is no building there",
						   _str(tilePos.x), _str(tilePos.y)));
			return nullptr;
		}

		DoorBlockData *doorData = &data->blockData.door;
		doorData->building = buildingAndDoor->first;
		doorData->door = buildingAndDoor->second;

		return data;
	}

	return nullptr;
}

void CollisionMap::GlobalContactListener::BeginContact(b2Contact *contact)
{
	b2Fixture *a = contact->GetFixtureA();
	b2Fixture *b = contact->GetFixtureB();

	BodyData *aData = (BodyData *) a->GetUserData();
	BodyData *bData = (BodyData *) b->GetUserData();

	if (aData == nullptr || bData == nullptr)
		return;

	// entity with block
	if (aData->type != bData->type)
	{
		BodyData *entity = aData->type == BODYDATA_ENTITY ? aData : bData;
		BodyData *block = entity == aData ? bData : aData;

		// door
		if (block->blockData.blockDataType == BLOCKDATA_DOOR)
		{
			// todo
			/* DoorBlockData *door = &block->blockData.door; */
			/* Door *targetDoor = door->building->getConnectedDoor(door->door); */
			/* if (targetDoor == nullptr) */
			/* { */
			/* 	Logger::logError(format("Could not find connected door for door %1% in building %2%", */
			/* 							_str(door->door->id), _str(door->building->getID()))); */
			/* 	return; */
			/* } */

			/* Event event; */
			/* event.type = EVENT_HUMAN_JOIN_WORLD; */
			/* event.entityID = entity->entityID.id; */
			/* event.joinWorld.newWorldID = 1010101; // todo world's need IDs! */

			/* event.joinWorld.spawnDirection = DIRECTION_NORTH; // todo store in Door */
			/* event.joinWorld.spawnX = targetDoor->localTilePos.x; */
			/* event.joinWorld.spawnY = targetDoor->localTilePos.y; */

			/* Logger::logDebug(format("Door interaction with building %1%", _str(door->building->getID()))); */

			/* // todo complete the two above todos before actually calling the event */
			/* // Locator::locate<EventService>()->callEvent(event); */
		}


	}

}
