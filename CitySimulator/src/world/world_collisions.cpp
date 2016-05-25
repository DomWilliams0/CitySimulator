#include "world.hpp"
#include "service/render_service.hpp"
#include "service/locator.hpp"

CollisionMap::CollisionMap(World *container) 
: BaseWorld(container), world({0.f, 0.f})
	{
		world.SetAllowSleeping(true);
		world.SetContactListener(&globalContactListener);
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
			// the only collidable tile layer
			BlockType bt = container->getTerrain()->getBlockType({x, y}, LAYER_TERRAIN);
			BlockInteractivity interactivity = getInteractivity(bt);

			if (interactivity != INTERACTIVTY_NONE)
			{
				sf::Vector2f pos(Utils::toPixel(sf::Vector2f(x, y)));
				rects.emplace_back(sf::FloatRect(pos, size), 0.f, bt);
			}
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

bool CollisionMap::compareRectsHorizontally(const CollisionRect &acr, const CollisionRect &bcr)
{
	const sf::FloatRect &a = acr.rect;
	const sf::FloatRect &b = bcr.rect;

	if (a.top < b.top) return true;
	if (b.top < a.top) return false;

	if (a.left < b.left) return true;
	if (b.left < a.left) return false;

	return false;
}

bool CollisionMap::compareRectsVertically(const CollisionRect &acr, const CollisionRect &bcr)
{
	const sf::FloatRect &a = acr.rect;
	const sf::FloatRect &b = bcr.rect;

	if (a.left < b.left) return true;
	if (b.left < a.left) return false;

	if (a.top < b.top) return true;
	if (b.top < a.top) return false;

	return false;
}

void CollisionMap::moveRects(std::vector<CollisionRect> &src, std::vector<CollisionRect> &dst,
                             bool (*pred)(const CollisionRect &))
{
	// TODO: use STL collection wizardry
	auto it = src.begin();
	while (it != src.end())
	{
		if (pred(*it))
		{
			dst.push_back(*it);
			it = src.erase(it);
		}
		else
			++it;
	}

}

void CollisionMap::mergeAdjacentTiles(std::vector<CollisionRect> &rects)
{
	std::vector<CollisionRect> rectangles;

	// move all non-interactive rects to rectangles vector
	moveRects(rects, rectangles,
	          [](const CollisionRect &r)
	          { return !isInteractable(r.blockType) && r.rotation == 0.f; });

	// join individual rects
	sort(rectangles.begin(), rectangles.end(), compareRectsHorizontally);
	mergeHelper(rectangles, distanceChecker);

	// join rows together
	sort(rectangles.begin(), rectangles.end(), compareRectsVertically);
	mergeHelper(rectangles, dimensionChecker);

	// add back to returning list
	for (auto &mergedRect : rectangles)
		rects.push_back(mergedRect);

	// TODO: join adjacent identical interactive rects in the least hacky way possible
	rectangles.clear();
	moveRects(rects, rectangles,
	          [](const CollisionRect &r)
	          { return isInteractable(r.blockType); });

	sort(rectangles.begin(), rectangles.end(), compareRectsHorizontally);
	mergeHelper(rectangles, interactivityChecker);
	sort(rectangles.begin(), rectangles.end(), compareRectsVertically);
	mergeHelper(rectangles, dimensionChecker);

	for (auto &mergedRect : rectangles)
		rects.push_back(mergedRect);
}

bool CollisionMap::distanceChecker(const CollisionRect *lastCRect, const CollisionRect *currentCRect)
{
	const sf::FloatRect &rect = currentCRect->rect;
	const sf::FloatRect &lastRect = lastCRect->rect;
	return powf(rect.left - lastRect.left, 2.f) + powf(rect.top - lastRect.top, 2.f) >
	       Constants::tileSizef * Constants::tileSizef;
}

bool CollisionMap::dimensionChecker(const CollisionRect *lastCRect, const CollisionRect *currentCRect)
{
	const sf::FloatRect &rect = currentCRect->rect;
	const sf::FloatRect &lastRect = lastCRect->rect;

	// adjacent and same dimensions
	return !(lastRect.left <= rect.left + rect.width &&
	         rect.left <= lastRect.left + lastRect.width &&
	         lastRect.top <= rect.top + rect.height &&
	         rect.top <= lastRect.top + lastRect.height &&
	         lastRect.width == rect.width && lastRect.height == rect.height);


}

bool CollisionMap::interactivityChecker(const CollisionRect *last, const CollisionRect *current)
{
	return (last->blockType != current->blockType) || distanceChecker(last, current);
}

void CollisionMap::mergeHelper(std::vector<CollisionRect> &rects,
                               bool (*nextRowFunc)(const CollisionRect *last, const CollisionRect *current))
{
	std::vector<CollisionRect> rectsCopy(rects.begin(), rects.end());
	rects.clear();

	CollisionRect *current = nullptr;
	CollisionRect *lastRect = nullptr;

	static CollisionRect placeholder(sf::FloatRect(-100.f, -100.f, 0.f, 0.f), 0.f);
	rectsCopy.push_back(placeholder); // to ensure the last rect is included

	for (size_t i = 0; i < rectsCopy.size(); ++i)
	{
		CollisionRect *rect = &rectsCopy[i];

		// no current rect expanding
		if (current == nullptr)
		{
			current = lastRect = rect;
			continue;
		}

		// done with current
		if ((nextRowFunc)(lastRect, rect))
		{
			rects.push_back(*current);

			// move on
			current = lastRect = rect;
			continue;
		}

		// stretch current
		const sf::FloatRect &mergeRect = rect->rect;
		sf::FloatRect &currentRect = current->rect;

		currentRect.left     = std::min(currentRect.left, mergeRect.left);
		currentRect.top      = std::min(currentRect.top, mergeRect.top);
		currentRect.width    = std::max(currentRect.left + currentRect.width,
		                                mergeRect.left + mergeRect.width) - currentRect.left;
		currentRect.height   = std::max(currentRect.top + currentRect.height,
		                                mergeRect.top + mergeRect.height) - currentRect.top;

		lastRect = rect;
	}
}

CollisionMap::~CollisionMap()
{
	if (worldBody != nullptr)
		world.DestroyBody(worldBody);
}

void CollisionMap::load()
{
	std::vector<CollisionRect> rects;

	// gather all collidable tiles
	findCollidableTiles(rects);

	// merge adjacents
	mergeAdjacentTiles(rects);

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
	auto worldSize = container->getPixelSize();
	rects.emplace_back(sf::FloatRect(-borderThickness - padding, 0, borderThickness, worldSize.y), 0.f);
	rects.emplace_back(sf::FloatRect(0, -borderThickness - padding, worldSize.x, borderThickness), 0.f);
	rects.emplace_back(sf::FloatRect(worldSize.x + padding, 0, borderThickness, worldSize.y), 0.f);
	rects.emplace_back(sf::FloatRect(0, worldSize.y + padding, worldSize.x, borderThickness), 0.f);

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
	// outside only
	if (container->isOutside())
	{
		// building doors
		if (blockType == BLOCK_SLIDING_DOOR)
		{
			BodyData *data = new BodyData; // todo cache
			data->type = BODYDATA_BLOCK;
			data->blockData.blockDataType = BLOCKDATA_DOOR;
			data->blockData.location.set(container->getID(), tilePos);

			boost::optional<std::pair<BuildingID, DoorID>> buildingAndDoor;
			container->getBuildingConnectionMap()->getBuildingByOutsideDoorTile(tilePos, buildingAndDoor);

			if (!buildingAndDoor)
			{
				Logger::logWarning(
						format("Cannot add block data for door at (%1%, %2%), because there is no building there",
						       _str(tilePos.x), _str(tilePos.y)));
				return nullptr;
			}

			/* DoorBlockData *doorData = &data->blockData.door; */

			Logger::logDebuggiest(format("Added door block data to door %1% of building %2% in world %3%",
			                             _str(buildingAndDoor->second),
			                             _str(buildingAndDoor->first),
			                             _str(container->getID())));

			return data;
		}
	}

	// inside
	else
	{
		// entrance
		if (blockType == BLOCK_ENTRANCE_MAT)
		{
			BodyData *data = new BodyData; // todo cache
			data->type = BODYDATA_BLOCK;
			data->blockData.blockDataType = BLOCKDATA_DOOR;
			data->blockData.location.set(container->getID(), tilePos);

			Door *door = container->getDomesticConnectionMap()->getDoorByTile(tilePos);

			if (door == nullptr)
			{
				Logger::logWarning(
						format("Cannot add block data for door at (%1%, %2%), because there is no door there",
						       _str(tilePos.x), _str(tilePos.y)));
				return nullptr;
			}

			/* DoorBlockData *doorData = &data->blockData.door; */

			Location &loc = data->blockData.location;
			Logger::logDebuggiest(format("Added door block data at (%1%, %2%) in world %3%",
			                             _str(loc.x),
			                             _str(loc.y),
			                             _str(loc.world)));

			return data;
		}
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
		BlockData &blockData = block->blockData;

		// door
		if (block->blockData.blockDataType == BLOCKDATA_DOOR)
		{
			WorldService *ws = Locator::locate<WorldService>();

			Location target;
			if (!ws->getConnectionDestination(block->blockData.location, target))
			{
				Logger::logError(format("Door at (%1%, %2%) in world %3% has no target location",
										_str(blockData.location.x),
										_str(blockData.location.y),
										_str(blockData.location.world)));
				return;
			}

			Event event;
			event.type = EVENT_HUMAN_SWITCH_WORLD;
			event.entityID = entity->entityID.id;
			event.humanSwitchWorld.newWorld = target.world;

			/* event.humanSwitchWorld.spawnDirection = DIRECTION_NORTH; */ // todo store in Door
			event.humanSwitchWorld.spawnX = target.x;
			event.humanSwitchWorld.spawnY = target.y;

			Logger::logDebug(format("Door interaction at (%1%, %2%) in world %3%",
			                        _str(blockData.location.x),
			                        _str(blockData.location.y),
			                        _str(blockData.location.world)));

			Locator::locate<EventService>()->callEvent(event);
		}


	}

}
