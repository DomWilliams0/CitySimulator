#include <boost/property_tree/xml_parser.hpp>
#include "entity.hpp"
#include "config.hpp"
#include "animation.hpp"
#include "logger.hpp"
#include "utils.hpp"

void EntityFactory::loadEntities(EntityType entityType, const std::string &fileName)
{
	ConfigurationFile config(Utils::searchForFile(fileName));
	config.load();

	// get root list name
	std::string root = boost::filesystem::path(fileName).stem().string();

	// load tags
	std::vector<ConfigKeyValue> entityMapList;
	config.getMapList(root, entityMapList);

	EntityTags allTags;
	for (auto &entity : entityMapList)
	{
		auto nameIt(entity.find("name"));

		// no name
		if (nameIt == entity.end())
		{
			Logger::logWarning(FORMAT("No name found for entity of type %1%, skipping", entityType));
			continue;
		}

		std::string name(nameIt->second);
		allTags.insert({ name, entity});
	}

	loadedTags[entityType] = allTags;

	// external resource loading
	for (auto &entity : allTags)
	{
		for (auto &keyValue : entity.second)
		{
			// sprites
			if (keyValue.first == "sprite")
				Globals::spriteSheet->loadSprite(entity.second);
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
