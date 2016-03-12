#include "building.hpp"
#include "world.hpp"
#include "utils.hpp"
#include "service/logging_service.hpp"

Building::Building(World &world, const sf::IntRect &tileBounds, int id, std::string buildingWorldName)
		: outsideWorld(&world), buildingID(id), insideWorldName(buildingWorldName)
{
	for (int x = tileBounds.left; x <= tileBounds.left + tileBounds.width; ++x)
	{
		for (int y = tileBounds.top; y <= tileBounds.top + tileBounds.height; ++y)
		{
			sf::Vector2i tile(x, y);
			BlockType b = world.getBlockAt(tile, LAYER_OVERTERRAIN);

			// windows
			if (b == BLOCK_BUILDING_WINDOW_OFF || b == BLOCK_BUILDING_WINDOW_ON)
			{
				windows[tile] = b == BLOCK_BUILDING_WINDOW_ON;
				setWindowLight(tile, Utils::random(0.f, 1.f) < 0.5f);
			}

			// todo doors
		}
	}

	Logger::logDebuggier(format("Found %1% building windows", _str(windows.size())));
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
