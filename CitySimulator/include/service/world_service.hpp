#ifndef CITYSIMULATOR_WORLD_SERVICE_HPP
#define CITYSIMULATOR_WORLD_SERVICE_HPP

#include "base_service.hpp"
#include "world.hpp"
#include "building.hpp"
#include "bodydata.hpp"

class WorldService : public BaseService
{
public:
	WorldService(const std::string &worldPath, const std::string &tilesetPath);

	virtual void onEnable() override;

	virtual void onDisable() override;

	World &getWorld();

	BodyData*getSharedBodyDataForBlockType(BlockType blockType);


private:
	// todo 1 main world, list of auxiliary worlds (or tree?)
	World world;

	std::string worldPath, tilesetPath;

	std::unordered_map<BlockType, BodyData, std::hash<int>> bodyDataCache;
};

#endif
