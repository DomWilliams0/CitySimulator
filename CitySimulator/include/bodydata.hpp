#ifndef CITYSIMULATOR_BODYDATA_HPP
#define CITYSIMULATOR_BODYDATA_HPP

#include "world.hpp"
#include "ecs.hpp"

struct Door;
class Building;

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
	DoorID door;
	BuildingID building;
};

struct BlockData
{
	BlockDataType blockDataType;
	Location location;

	union
	{
		DoorBlockData door;
	};
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
