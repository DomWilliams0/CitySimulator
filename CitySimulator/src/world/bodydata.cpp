#include "bodydata.hpp"
#include "service/logging_service.hpp"

void interactWithSlidingDoor(EntityIdentifier entity)
{
	Logger::logDebuggier("An entity interacted with a sliding door!");
}


BodyData *createBodyDataForBlock(const BlockType &block)
{
	// interactive doors
	if (block == BLOCK_SLIDING_DOOR)
	{
		BodyData *data = new BodyData;
		data->type = BODYDATA_BLOCK;
		data->blockInteraction.callback = &interactWithSlidingDoor;

		Logger::logDebuggier("Creating body data for a sliding door");
		return data;
	}

	return nullptr;
}
