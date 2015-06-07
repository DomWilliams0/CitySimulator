#pragma once
#include <map>
#include <SFML/System/Vector2.hpp>
#include <bitset>
#include <vector>
#include <iostream>
#include "logger.hpp"
#include <boost/lexical_cast.hpp>

namespace TMX
{
	enum PropertyType
	{
		TYPE,
		VISIBLE,
		ERROR
	};

	PropertyType propertyTypeFromString(const std::string &s);


	class PropertyOwner
	{
	public:
		PropertyOwner()
		{
			map = new PropMap();
		}

		virtual ~PropertyOwner()
		{
			delete map;
		}

		inline void addProperty(PropertyType type, std::string value)
		{
			map->insert(std::make_pair(type, value));
		}

		inline std::string getProperty(PropertyType type)
		{
			return map->at(type);
		}


	private:
		typedef std::map<PropertyType, std::string> PropMap;
		PropMap *map;
	};

	typedef uint32_t rot;

	enum Rotation
	{
		HORIZONTAL = rot(1 << 31),
		VERTICAL = rot(1 << 30),
		DIAGONAL = rot(1 << 29)
	};

	struct Tile
	{
		Tile()
		{
		}

		explicit Tile(const std::string &id)
		{
			rotGid = gid = boost::lexical_cast<rot>(id);
			if (gid != 0)
				gid -= 1;

			rotation.set(0, (gid & HORIZONTAL) != 0);
			rotation.set(1, (gid & VERTICAL) != 0);
			rotation.set(2, (gid & DIAGONAL) != 0);

			gid &= ~(HORIZONTAL | VERTICAL | DIAGONAL);
		}

		inline bool isRotated()
		{
			return rotation.any();
		}

		unsigned gid, rotGid;
		std::bitset<3> rotation;
	};

	struct Object
	{
		unsigned gid;
		sf::Vector2i position;

		int rotation;
		std::bitset<3> flip;
	};

	template <class T>
	struct Layer : PropertyOwner
	{
		std::string name;
		T *items;
		bool visible;
	};

	typedef Layer<Tile> TileLayer;
	typedef Layer<Object> ObjectGroup;


	struct TileMap : PropertyOwner
	{
		~TileMap()
		{
			for (auto layer : layers)
				delete layer;
			for (auto object : objects)
				delete object;
		}


		int width, height;
		std::vector<TileLayer*> layers;
		std::vector<ObjectGroup*> objects;

		static TileMap* load(const std::string filename);
	};
}
