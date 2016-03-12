#include <boost/property_tree/xml_parser.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include "maploader.hpp"
#include "utils.hpp"
#include "service/logging_service.hpp"

TMX::PropertyType TMX::propertyTypeFromString(const std::string &s)
{
	if (s == "type")
		return PROPERTY_TYPE;
	if (s == "visible")
		return PROPERTY_VISIBLE;
	if (s == "buildingWorld")
		return PROPERTY_BUILDING_WORLD;
	if (s == "buildingID")
		return PROPERTY_BUILDING_ID;
	if (s == "door")
		return PROPERTY_BUILDING_DOOR;

	Logger::logWarning("Unknown PropertyType: " + s);
	return PROPERTY_UNKNOWN;
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

			TMX::PropertyType type = TMX::propertyTypeFromString(name);
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


int TMX::stripFlip(const int &gid, std::bitset<3> &flips)
{
	flips.set(0, (gid & HORIZONTAL) != 0);
	flips.set(1, (gid & VERTICAL) != 0);
	flips.set(2, (gid & DIAGONAL) != 0);

	return gid & ~(HORIZONTAL | VERTICAL | DIAGONAL);
}

TMX::Tile::Tile(TileType type, const std::string &id) : tileType(type)
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
		layer->items.reserve(map->width * map->height);

		// tile layerDepths
		if (pair.first == "layer")
		{
			// add all tiles
			typedef boost::char_separator<char> sep;
			typedef boost::tokenizer<sep> tk;

			std::string data = pair.second.get<std::string>("data");
			tk tokens(data, sep(",\n\r"));

			for (auto it = tokens.begin(); it != tokens.end(); ++it)
				layer->items.push_back(new Tile(*it));

			layer->items.resize(map->width * map->height);
		}

			// object layers
		else
		{
			for (auto &o : pair.second)
			{
				if (o.first == "object")
				{

					boost::optional<std::string> gid = o.second.get_optional<std::string>("<xmlattr>.gid");

					// has a tile gid
					if (gid)
					{
						Object *obj = new Object(*gid);

						obj->position.x = o.second.get<float>("<xmlattr>.x");
						obj->position.y = o.second.get<float>("<xmlattr>.y");
						obj->rotationAnglef = o.second.get<float>("<xmlattr>.rotation", 0.0);

						layer->items.push_back(obj);
					}

						// property object
					else
					{
						PropertyObject *propObj = new PropertyObject;
						for (auto &prop : o.second.get_child("properties"))
						{
							std::string key(prop.second.get<std::string>("<xmlattr>.name"));
							std::string value(prop.second.get<std::string>("<xmlattr>.value"));

							PropertyType propType = propertyTypeFromString(key);
							if (propType == PROPERTY_UNKNOWN)
							{
								Logger::logWarning(format("Found unknown property '%1%' in layer '%2%', skipping",
								                          layer->name, key));
								continue;

							}

							propObj->addProperty(propType, value);
							layer->items.push_back(propObj);
						}

					}
				}
			}
		}

		map->layers.push_back(layer);
	}


	return map;
}
void TMX::Tile::processRotation(std::bitset<3> rotation)
{
	rotationAngle = 0;
	flipGID = gid;

	bool h = rotation[0];
	bool v = rotation[1];
	bool d = rotation[2];

	if (h)
		flipGID |= HORIZONTAL;
	if (v)
		flipGID |= VERTICAL;
	if (d)
	{
		if (h && v)
		{
			rotationAngle = 90;
			flipGID ^= VERTICAL;
		}

		else if (h)
		{
			rotationAngle = -90;
			flipGID ^= VERTICAL;
		}

		else if (v)
		{
			rotationAngle = 90;
			flipGID ^= HORIZONTAL;
		}

		else
		{
			rotationAngle = -90;
			flipGID ^= HORIZONTAL;
		}
	}
}
TMX::Layer::~Layer()
{
	for (Tile *tile : items)
		delete tile;
}
TMX::TileMap::~TileMap()
{
	for (auto &layer : layers)
		delete layer;
}