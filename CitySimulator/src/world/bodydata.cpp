#include "bodydata.hpp"
#include "service/logging_service.hpp"

BodyData *createBodyDataForBlock(const BlockType &block)
{
	// interactive doors
	if (block == BLOCK_SLIDING_DOOR)
	{
		Logger::logDebuggier("Creating body data for a sliding door");
		return nullptr;
	}

	return nullptr;
}
