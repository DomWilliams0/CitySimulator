#include "building.hpp"
#include "world.hpp"
#include "utils.hpp"
#include "service/logging_service.hpp"



void Building::discoverWindows()
{
	for (int x = bounds.left; x <= bounds.left + bounds.width; ++x)
	{
		for (int y = bounds.top; y <= bounds.top + bounds.height; ++y)
		{
			sf::Vector2i tile(x, y);
			BlockType b = outsideWorld->getBlockAt(tile, LAYER_OVERTERRAIN);

			// windows
			if (b == BLOCK_BUILDING_WINDOW_OFF || b == BLOCK_BUILDING_WINDOW_ON)
			{
				windows[tile] = b == BLOCK_BUILDING_WINDOW_ON;
				setWindowLight(tile, Utils::random(0.f, 1.f) < 0.5f); // debug random windows
			}
		}
	}

	Logger::logDebuggiest(format("Found %1% building windows in building %2%", _str(windows.size()), _str(buildingID)));
}

bool Building::isWindowLit(const sf::Vector2i &tile)
{
	auto it = windows.find(tile);
	if (it == windows.end())
		return false;

	return it->second;
}


void Building::setWindowLight(const sf::Vector2i &tile, bool lit)
{
	if (windows.find(tile) == windows.end())
	{
		Logger::logWarning(format("Cannot set window light as (%1%, %2%) is not a window", _str(tile.x), _str(tile.y)));
		return;
	}

	windows[tile] = lit;

	BlockType newBlock = lit ? BLOCK_BUILDING_WINDOW_ON : BLOCK_BUILDING_WINDOW_OFF;
	outsideWorld->getTerrain().setBlockType(tile, newBlock, LAYER_OVERTERRAIN);
}
void Building::addDoor(int doorID, const sf::Vector2i &doorTilePos, World *doorWorld)
{
	std::vector<Door> *doors;

	// inside
	if (doorWorld == insideWorld)
		doors = &insideDoors;
	else if (doorWorld == outsideWorld)
		doors = &outsideDoors;
	else
	{
		Logger::logWarning(
				format("Tried to add a door at (%1%, %2%) to a building which isn't in its world",
				       _str(doorTilePos.x), _str(doorTilePos.y)));
		return;
	}

	doors->emplace_back(doorID, doorWorld, doorTilePos);

}

const std::vector<Door> &Building::getOutsideDoors() const
{
	return outsideDoors;
}

const std::vector<Door> &Building::getInsideDoors() const
{
	return insideDoors;
}