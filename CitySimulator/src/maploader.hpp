#pragma once
#include <map>
#include <SFML/System/Vector2.hpp>
#include <bitset>
#include <vector>
#include <boost/lexical_cast.hpp>

enum BlockType;

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

		virtual ~Tile()
		{
		}

		explicit Tile(const std::string &id)
		{
			gid = boost::lexical_cast<rot>(id);
			if (gid != 0)
				gid -= 1;

			std::bitset<3> rotation;

			rotation.set(0, (gid & HORIZONTAL) != 0);
			rotation.set(1, (gid & VERTICAL) != 0);
			rotation.set(2, (gid & DIAGONAL) != 0);

			gid &= ~(HORIZONTAL | VERTICAL | DIAGONAL);

			processRotation(rotation);
		}

		virtual inline bool isTile()
		{
			return true;
		}

		inline int getRotationAngle()
		{
			return rotationAngle;
		}

		inline int getFlipFlags()
		{
			return flipFlags;
		}

		unsigned gid;

	private:
		
		void processRotation(std::bitset<3> rotation)
		{
			rotationAngle = 0;
			flipFlags = 0;

			bool h = rotation[0];
			bool v = rotation[1];
			bool d = rotation[2];

			if (h)
				flipFlags |= HORIZONTAL;
			if (v)
				flipFlags |= HORIZONTAL;
			if (d)
			{
				if (h && v)
				{
					rotationAngle = 90;
					flipFlags ^= HORIZONTAL;
				}

				else if (h)
				{
					rotationAngle = -90;
					flipFlags ^= HORIZONTAL;
				}

				else if (v)
				{
					rotationAngle = 90;
					flipFlags ^= HORIZONTAL;
				}

				else
				{
					rotationAngle = -90;
					flipFlags ^= HORIZONTAL;
				}
			}

			rotationAngle = rotationAngle;
			flipFlags = flipFlags;
		}
		
		int rotationAngle, flipFlags;
	};

	struct Object : Tile
	{
		Object()
		{
		}

		explicit Object(const std::string &id)
			: Tile(id)
		{
		}


		inline bool isTile() override
		{
			return false;
		}

		sf::Vector2i position;
		float rotationAngle;
	};

	struct Layer : PropertyOwner
	{
		std::string name;
		std::vector<Tile*> items;
		bool visible;
	};

	struct TileMap : PropertyOwner
	{
		~TileMap()
		{
			for (auto layer : layers)
				delete layer;
		}


		int width, height;
		std::vector<Layer*> layers;

		static TileMap* load(const std::string filename);
	};
}
