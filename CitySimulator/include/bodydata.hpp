#ifndef CITYSIMULATOR_BODYDATA_HPP
#define CITYSIMULATOR_BODYDATA_HPP

#include "world.hpp"
#include "ecs.hpp"

enum BodyDataType
{
	BODYDATA_ENTITY,
	BODYDATA_BLOCK
};

enum BlockDataType
{
	BLOCKDATA_DOOR
};

struct DoorBlockData
{
	int doorID;
	int buildingID;
};

struct BlockData
{
	BlockDataType blockDataType;
	DoorBlockData door;
};

struct BodyData
{
	BodyDataType type;

	union
	{
		EntityIdentifier entityID;
		BlockData blockData;
	};

	BodyData()
	{
	}

};

#endif
