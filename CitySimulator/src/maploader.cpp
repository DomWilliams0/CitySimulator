#include <boost/property_tree/xml_parser.hpp>
#include "maploader.hpp"
#include "utils.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

TMX::PropertyType propertyTypeFromString(const std::string &s)
{
	if (s == "type")
		return TMX::TYPE;
	if (s == "visible")
		return TMX::VISIBLE;

	Logger::logWarning("Unknown PropertyType: " + s);
	return TMX::ERROR;
}

void addProperties(TMX::TileMap *tile_map, boost::property_tree::ptree tree)
{
	// properties
	try
	{
		auto properties = tree.get_child("properties");

		for (auto propertyPair : properties)
		{
			auto name = propertyPair.second.get<std::string>("<xmlattr>.name");
			auto value = propertyPair.second.get<std::string>("<xmlattr>.value");

			TMX::PropertyType type = propertyTypeFromString(name);
			if (type != Logger::ERROR)
				tile_map->addProperty(type, value);
		}
	}
	catch (std::exception e)
	{
		// no properties
		if (strcmp(e.what(), "No such node (properties)") != 0)
			throw e;
	}
}


TMX::TileMap* TMX::TileMap::load(const std::string filename)
{
	boost::property_tree::ptree tree;
	read_xml(Utils::searchForFile(filename), tree);

	TileMap *map = new TileMap();

	map->width = tree.get<int>("map.<xmlattr>.width", 0);
	map->height = tree.get<int>("map.<xmlattr>.height", 0);

	auto treeRoot = tree.get_child("map");
	addProperties(map, treeRoot);

	for (auto pair : treeRoot)
	{
		// tile layers
		if (pair.first == "layer")
		{
			TileLayer *layer = new TileLayer;
			layer->name = pair.second.get<std::string>("<xmlattr>.name");
			layer->visible = pair.second.get<int>("<xmlattr>.visible", 1);

			layer->items = new Tile[map->width * map->height];

			// add all tiles
			typedef boost::char_separator<char> sep;
			typedef boost::tokenizer<sep> tk;

			std::string data = pair.second.get<std::string>("data");
			tk tokens(data, sep(",\n\r"));
			int index = 0;

			for (auto i = tokens.begin(); i != tokens.end(); ++i)
			{
				Tile *t = new Tile(*i);
				layer->items[index++] = *t;
			}

			map->layers.push_back(layer);
		}

		// object groups
		else if (pair.first == "objectgroup")
		{
			ObjectGroup *group = new ObjectGroup;
			group->name = pair.second.get<std::string>("<xmlattr>.name");
			group->visible = pair.second.get<int>("<xmlattr>.visible", 1);

			int amount = pair.second.size() - 1;
			group->items = new Object[amount];

			int i(0);
			for (auto o : pair.second)
			{
				if (o.first == "object")
				{
					Object *obj = new Object;
					obj->gid = o.second.get<int>("<xmlattr>.gid");
					obj->position.x = o.second.get<int>("<xmlattr>.x");
					obj->position.y = o.second.get<int>("<xmlattr>.y");

					group->items[i++] = *obj;
				}
			}
		}
	}


	return map;
}
