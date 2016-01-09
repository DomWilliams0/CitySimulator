#ifndef CITYSIM_BODYDATA_HPP
#define CITYSIM_BODYDATA_HPP

#include "entity.hpp"
#include "world.hpp"

enum BodyDataType
{
	BODYDATA_ENTITY,
	BODYDATA_BLOCK
};

struct EntityIdentifier
{
	EntityID id;
	EntityType type;

	EntityIdentifier() : type(ENTITY_UNKNOWN), id(INVALID_ENTITY)
	{
	}

	EntityIdentifier(EntityID id, EntityType type) : type(type), id(id)
	{
	}
};

struct BlockInteraction
{
	BlockInteraction()
	{
	}

	// todo
};

struct BodyData
{
	BodyDataType type;

	union
	{
		EntityIdentifier entityID;
		BlockInteraction blockInteraction;
	};

	BodyData()
	{
	}
};

BodyData * createBodyDataForBlock(const BlockType &block);


#endif
