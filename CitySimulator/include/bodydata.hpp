#ifndef CITYSIM_BODYDATA_HPP
#define CITYSIM_BODYDATA_HPP

#include "entity.hpp"
#include "world.hpp"

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
