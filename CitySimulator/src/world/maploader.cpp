#include <boost/property_tree/xml_parser.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include "maploader.hpp"
#include "utils.hpp"
#include "service/logging_service.hpp"

TMX::PropertyType TMX::propertyTypeFromString(const std::string &s)
{
	if (s == "visible")
		return PROPERTY_VISIBLE;
	if (s == "building-world")
		return PROPERTY_BUILDING_WORLD;
	if (s == "world-share-source")
		return PROPERTY_DOOR_WORLD_SHARE_SOURCE;
	if (s == "world-share")
		return PROPERTY_DOOR_WORLD_SHARE_SPECIFIER;
	if (s == "loaded-world-id")
		return PROPERTY_DOOR_WORLD_ID;
	if (s == "door-world")
		return PROPERTY_DOOR_WORLD;
	if (s == "door-id")
		return PROPERTY_DOOR_ID;
	if (s == "door-orientation")
		return PROPERTY_DOOR_ORIENTATION;

	Logger::logWarning("Unknown property: " + s);
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

void TMX::Tile::setGID(const std::string &id)
{
	gid = boost::lexical_cast<rot>(id);
	if (gid != 0)
		gid -= 1;

	std::bitset<3> flips;
	gid = stripFlip(gid, flips);

	flipped = flips.any();

	processRotation(flips);
}

void TMX::TileMap::load(const std::string &filePath)
{
	Logger::logDebuggier(format("Loading world from %1%", filePath));
  	this->filePath = Utils::getFileName(filePath);
	boost::property_tree::ptree tree;
	read_xml(filePath, tree);

	size.x = tree.get<int>("map.<xmlattr>.width", 0);
	size.y = tree.get<int>("map.<xmlattr>.height", 0);

	boost::property_tree::ptree treeRoot = tree.get_child("map");
	addProperties(this, treeRoot);

	for (auto &pair : treeRoot)
	{
		if (pair.first != "layer" && pair.first != "objectgroup")
			continue;

		layers.emplace_back();
		Layer &layer = layers[layers.size() - 1];
		layer.name = pair.second.get<std::string>("<xmlattr>.name");
		layer.visible = pair.second.get<int>("<xmlattr>.visible", 1) != 0;

		// tile layers
		if (pair.first == "layer")
		{
			// add all tiles
			typedef boost::char_separator<char> sep;

			std::string data = pair.second.get<std::string>("data");
			boost::tokenizer<sep> tokens(data, sep(",\n\r"));
			layer.items.resize(size.x * size.y);
			int i(0);

			for (auto it = tokens.begin(); it != tokens.end(); ++it)
			{
				TileWrapper &wrapper = layer.items[i];
				wrapper.type = TILE_TILE;
				wrapper.tile.setGID(*it);

				wrapper.tile.position.x = i % size.x;
				wrapper.tile.position.y = i / size.x;
				i += 1;
			}
		}

		else
		{
			// object layers
			for (auto &o : pair.second)
			{
				if (o.first == "object")
				{
					TileWrapper wrapper;
					wrapper.tile.position.x = o.second.get<float>("<xmlattr>.x");
					wrapper.tile.position.y = o.second.get<float>("<xmlattr>.y");
					boost::optional<std::string> gid = o.second.get_optional<std::string>("<xmlattr>.gid");

					// has a tile gid
					if (gid)
					{
						wrapper.type = TILE_OBJECT;
						wrapper.tile.setGID(*gid);

						wrapper.objectRotation = o.second.get<float>("<xmlattr>.rotation", 0.0);
					}

					else
					{
						// property object
						wrapper.type = TILE_PROPERTY_SHAPE;
						wrapper.property.dimensions.x = o.second.get<float>("<xmlattr>.width");
						wrapper.property.dimensions.y = o.second.get<float>("<xmlattr>.height");

						for (auto &prop : o.second.get_child("properties"))
						{
							std::string key(prop.second.get<std::string>("<xmlattr>.name"));
							std::string value(prop.second.get<std::string>("<xmlattr>.value"));

							PropertyType propType = propertyTypeFromString(key);
							if (propType == PROPERTY_UNKNOWN)
							{
								Logger::logWarning(format("Found unknown property '%1%' in layer '%2%', skipping",
								                          key, layer.name));
								continue;

							}
							wrapper.property.addProperty(propType, value);
						}

					}

					layer.items.push_back(wrapper);
				}
			}
		}
	}
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

void TMX::PropertyHolder::addProperty(PropertyType type, std::string value)
{
	map.insert(std::make_pair(type, value));
}

std::string TMX::PropertyHolder::getProperty(PropertyType type) const
{
	return map.at(type);
}

bool TMX::PropertyHolder::hasProperty(PropertyType type) const
{
	return map.find(type) != map.end();
}

void TMX::PropertyHolder::getProperty(TMX::PropertyType type, boost::optional<std::string> &out)
{
	if (hasProperty(type))
		out = getProperty(type);
}


boost::optional<std::string> TMX::PropertyHolder::getPropertyOptional(TMX::PropertyType type)
{
	boost::optional<std::string> ret;
	getProperty(type);
	return ret;
}


TMX::TileType TMX::Tile::getTileType() const
{
	return tileType;
}
bool TMX::Tile::isFlipped() const
{
	return flipped;
}
int TMX::Tile::getRotationAngle() const
{
	return rotationAngle;
}
int TMX::Tile::getFlipGID() const
{
	return flipGID;
}
unsigned int TMX::Tile::getGID() const
{
	return gid;
}
