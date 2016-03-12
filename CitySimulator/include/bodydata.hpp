#ifndef CITYSIMULATOR_BODYDATA_HPP
#define CITYSIMULATOR_BODYDATA_HPP

#include "world.hpp"
#include "ecs.hpp"

enum BodyDataType
{
	BODYDATA_ENTITY,
	BODYDATA_BLOCK
};


struct BlockInteraction
{
	BlockInteraction()
	{
	}

	void (*callback)(EntityIdentifier);

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

BodyData *createBodyDataForBlock(const BlockType &block);

void interactWithSlidingDoor(EntityIdentifier entity);

#endif
