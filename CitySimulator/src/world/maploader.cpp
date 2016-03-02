#include <boost/property_tree/xml_parser.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include "maploader.hpp"
#include "utils.hpp"
#include "services.hpp"

TMX::PropertyType propertyTypeFromString(const std::string &s)
{
	if (s == "type")
		return TMX::PROPERTY_TYPE;
	if (s == "visible")
		return TMX::PROPERTY_VISIBLE;

	Logger::logWarning("Unknown PropertyType: " + s);
	return TMX::PROPERTY_UNKNOWN;
}

void addProperties(TMX::TileMap *tile_map, boost::property_tree::ptree &tree)
{
	// properties
	try
	{
		boost::property_tree::ptree empty;
		boost::property_tree::ptree properties = tree.get_child("properties", empty);
		if (properties.empty())
		{
			Logger::logDebug("No world properties found");
			return;
		}

		for (auto &propertyPair : properties)
		{
			auto name = propertyPair.second.get<std::string>("<xmlattr>.name");
			auto value = propertyPair.second.get<std::string>("<xmlattr>.value");

			Logger::logDebuggier(format("Found world property '%1%' => '%2%'", name, value));

			TMX::PropertyType type = propertyTypeFromString(name);
			if (type != TMX::PROPERTY_UNKNOWN)
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


/// <summary>
/// Strips flip flags from gid, and returns the blocktype
/// </summary>
/// <param name="flips">[0] = horizontal, [1] = vertical, [2] = diagonal</param>
/// <returns>Real blocktype</returns>
int TMX::stripFlip(const int &gid, std::bitset<3> &flips)
{
	flips.set(0, (gid & HORIZONTAL) != 0);
	flips.set(1, (gid & VERTICAL) != 0);
	flips.set(2, (gid & DIAGONAL) != 0);

	return gid & ~(HORIZONTAL | VERTICAL | DIAGONAL);
}

TMX::Tile::Tile(const std::string &id)
{
	gid = boost::lexical_cast<rot>(id);
	if (gid != 0)
		gid -= 1;

	std::bitset<3> flips;
	gid = stripFlip(gid, flips);

	flipped = flips.any();

	processRotation(flips);
}

TMX::TileMap *TMX::TileMap::load(const std::string &filePath)
{
	boost::property_tree::ptree tree;
	read_xml(filePath, tree);

	TileMap *map = new TileMap();

	map->width = tree.get<int>("map.<xmlattr>.width", 0);
	map->height = tree.get<int>("map.<xmlattr>.height", 0);

	boost::property_tree::ptree treeRoot = tree.get_child("map");
	addProperties(map, treeRoot);

	for (auto &pair : treeRoot)
	{
		if (pair.first != "layer" && pair.first != "objectgroup")
			continue;

		Layer *layer = new Layer;
		layer->name = pair.second.get<std::string>("<xmlattr>.name");
		layer->visible = pair.second.get<int>("<xmlattr>.visible", 1) != 0;
		layer->items.resize(map->width * map->height);
		int i(0);

		// tile layerDepths
		if (pair.first == "layer")
		{
			// add all tiles
			typedef boost::char_separator<char> sep;
			typedef boost::tokenizer<sep> tk;

			std::string data = pair.second.get<std::string>("data");
			tk tokens(data, sep(",\n\r"));

			for (auto it = tokens.begin(); it != tokens.end(); ++it)
			{
				Tile *t = new Tile(*it);
				layer->items[i++] = t;
			}
		}

			// object groups
		else
		{
			i = 0;
			for (auto &o : pair.second)
			{
				if (o.first == "object")
				{
					std::string gid = o.second.get<std::string>("<xmlattr>.gid");
					Object *obj = new Object(gid);

					obj->position.x = o.second.get<float>("<xmlattr>.x");
					obj->position.y = o.second.get<float>("<xmlattr>.y");
					obj->rotationAnglef = o.second.get<float>("<xmlattr>.rotation", 0.0);

					layer->items[i++] = obj;
				}
			}
		}

		map->layers.push_back(layer);
	}


	return map;
}
