#include "utils.hpp"
#include "building.hpp"
#include "service/logging_service.hpp"
#include "service/world_service.hpp"
#include "service/locator.hpp"


Building::Building(const sf::IntRect &tileBounds, BuildingID id,
		WorldID outsideWorld, WorldID insideWorld) : bounds(tileBounds), id(id)
{
	WorldService *ws = Locator::locate<WorldService>();
	this->insideWorld = ws->getWorld(insideWorld);
	this->outsideWorld = ws->getWorld(outsideWorld);

	if (this->insideWorld == nullptr || this->outsideWorld == nullptr)
	{
		WorldID badID = this->insideWorld == nullptr ? insideWorld : outsideWorld;
		error("Building world ID %1% has not been loaded", _str(badID));
	}

	insideWorldName = this->insideWorld->getName();
}

void Building::discoverWindows()
{
	WindowID id(0);
	for (int x = bounds.left; x <= bounds.left + bounds.width; ++x)
	{
		for (int y = bounds.top; y <= bounds.top + bounds.height; ++y)
		{
			sf::Vector2i tile(x, y);
			BlockType b = outsideWorld->getTerrain()->getBlockType(tile, LAYER_OVERTERRAIN);

			if (b == BLOCK_BUILDING_WINDOW_OFF || b == BLOCK_BUILDING_WINDOW_ON)
			{
				Window &window = windows.emplace(id++, Window{}).first->second;
				window.location = tile;
				window.status = b == BLOCK_BUILDING_WINDOW_ON;
			}
		}
	}

	Logger::logDebuggiest(format("Discovered %1% building windows in building %2%", 
				_str(windows.size()), _str(this->id)));
}

void Building::addDoor(const Location &location, DoorID id)
{
	/* static DoorID lastDoorID = 0; */

	if (location.world != insideWorld->getID() && 
			location.world != outsideWorld->getID())
	{
		Logger::logWarning(
				format("Tried to add a door at (%1%, %2%) to a building which isn't in its world",
					   _str(location.x), _str(location.y)));
		return;
	}

	Door &door = doors.emplace(id, Door{}).first->second;
	door.location = location;
}

void Building::setWindowLight(WindowID windowID, bool isNowLit)
{
	auto window = windows.find(windowID);
	if (window == windows.end())
	{
		Logger::logWarning(format("Window ID %d not found", _str(windowID)));
		return;
	}

	window->second.status = isNowLit;

	BlockType newBlock = isNowLit ? BLOCK_BUILDING_WINDOW_ON : BLOCK_BUILDING_WINDOW_OFF;
	outsideWorld->getTerrain()->setBlockType(window->second.location, newBlock, LAYER_OVERTERRAIN);
}

bool Building::isWindowLightOn(WindowID window) const
{
	auto it = windows.find(window);
	if (it == windows.end())
		return false;

	return it->second.status;
}

Door *Building::getDoor(DoorID id)
{
	auto door = doors.find(id);
	return door == doors.end() ? nullptr : &door->second;
}

std::size_t Building::getWindowCount() const
{
	return windows.size();
}

std::size_t Building::getDoorCount() const
{
	return doors.size();
}

BuildingID Building::getID() const
{
	return id;
}

std::string Building::getInsideWorldName() const
{
	return insideWorldName;
}

World *Building::getOutsideWorld() const
{
	return outsideWorld;
}

World *Building::getInsideWorld() const
{
	return insideWorld;
}
