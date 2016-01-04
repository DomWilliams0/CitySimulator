#ifndef CITYSIM_MAPLOADER_HPP
#define CITYSIM_MAPLOADER_HPP

#include <map>
#include <SFML/System/Vector2.hpp>
#include <bitset>
#include <vector>

namespace TMX
{
	enum PropertyType
	{
		PROPERTY_TYPE,
		PROPERTY_VISIBLE,
		PROPERTY_INVALID
	};

	PropertyType propertyTypeFromString(const std::string &s);


	class PropertyOwner
	{
	public:

		virtual ~PropertyOwner()
		{
		}

		inline void addProperty(PropertyType type, std::string value)
		{
			map.insert(std::make_pair(type, value));
		}

		inline std::string getProperty(PropertyType type)
		{
			return map.at(type);
		}


	private:
		typedef std::map<PropertyType, std::string> PropMap;
		PropMap map;
	};

	typedef uint32_t rot;

	enum Rotation
	{
		HORIZONTAL = rot(1 << 31),
		VERTICAL = rot(1 << 30),
		DIAGONAL = rot(1 << 29)
	};

	int stripFlip(const int &gid, std::bitset<3> &flips);

	struct Tile
	{
		Tile() : gid(0)
		{
		}

		virtual ~Tile()
		{
		}

		explicit Tile(const std::string &id);

		virtual inline bool isTile()
		{
			return true;
		}

		inline bool isFlipped()
		{
			return flipped;
		}

		/// <returns>Either -90, 0 or 90</returns>
		inline int getRotationAngle()
		{
			return rotationAngle;
		}

		/// <returns>Block ID | horizontal or vertical flip flags</returns>
		inline int getFlipGID()
		{
			return flipGID;
		}

		unsigned gid;
		bool flipped;

	private:

		void processRotation(std::bitset<3> rotation)
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

		int rotationAngle, flipGID;
	};

	struct Object : Tile
	{
		explicit Object(const std::string &id)
				: Tile(id)
		{
		}

		inline bool isTile() override
		{
			return false;
		}

		sf::Vector2f position;
		float rotationAnglef;
	};

	struct Layer : PropertyOwner
	{
		~Layer()
		{
			for (Tile *tile : items)
				delete tile;
		}

		std::string name;
		std::vector<Tile *> items;
		bool visible;
	};

	struct TileMap : PropertyOwner
	{
		~TileMap()
		{
			for (auto &layer : layers)
				delete layer;
		}


		int width, height;
		std::vector<Layer *> layers;

		static TileMap *load(const std::string &filePath);
	};
}
#endif