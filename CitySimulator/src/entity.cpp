#include <boost/property_tree/xml_parser.hpp>
#include "entity.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "animation.hpp"

void EntityFactory::loadEntities(EntityType entityType, const std::string &fileName)
{
	EntityConfig config;
	config.load(fileName, {}, { "human-size" });

	// add to loaded tags
	EntityTags allTags = config.getTags();
	loadedTags[entityType] = allTags;

	// external resource loading
	for (auto &entity : allTags)
	{
		for (auto &keyValue : entity.second)
		{
			// sprites
			if (keyValue.first == "sprite")
				SpriteSheet::loadSprite(boost::get<std::string>(keyValue.second));
		}
	}


	// debug printing
	//for (auto &entity : allTags)
	//{
	//	auto name = entity.first;
	//	std::cout << name << std::endl;
	//	for (auto &pair : entity.second)
	//		std::cout << "  :" << pair.first << " == " << pair.second << std::endl;
	//}
}
