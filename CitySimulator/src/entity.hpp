#pragma once
#include "constants.hpp"

typedef std::unordered_map<std::string, ConfigKeyValue> EntityTags;

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

