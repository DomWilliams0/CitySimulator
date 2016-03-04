#ifndef CITYSIMULATOR_WORLD_SERVICE_HPP
#define CITYSIMULATOR_WORLD_SERVICE_HPP

#include "base_service.hpp"
#include "world.hpp"
#include "building.hpp"

class WorldService : public BaseService
{
public:
	WorldService(const std::string &worldPath, const std::string &tilesetPath);

	virtual void onEnable() override;

	virtual void onDisable() override;

	inline World &getWorld()
	{
		return world;
	}

private:
	World world;
	std::vector<Building> buildings;

	std::string worldPath, tilesetPath;

	void discoverBuildings();

	sf::IntRect discoverBuildingHeight(const sf::Vector2i &start, const sf::Vector2i &end);
};

#endif
