#pragma once
#include <string>
#include "constants.hpp"

enum EntityType
{
	HUMAN,
	VEHICLE
};

class EntityFactory
{
public:
	void loadEntities(EntityType entityType, const std::string &fileName);

private:
	std::map<EntityType, EntityTags> loadedTags;

};

