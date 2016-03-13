#ifndef CITYSIMULATOR_WORLD_SERVICE_HPP
#define CITYSIMULATOR_WORLD_SERVICE_HPP

#include "base_service.hpp"
#include "world.hpp"
#include "building.hpp"
#include "bodydata.hpp"

class WorldService : public BaseService
{
public:
	WorldService(const std::string &mainWorldPath, const std::string &tilesetPath);

	virtual void onEnable() override;

	virtual void onDisable() override;

	World &getWorld();

private:
	// todo 1 main world, list of auxiliary worlds (or tree?)
	World mainWorld;
	std::unordered_map<int, World> worlds;
	int lastID;

	std::string mainWorldPath, tilesetPath;
};

#endif
