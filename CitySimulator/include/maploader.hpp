#ifndef CITYSIMULATOR_MAPLOADER_HPP
#define CITYSIMULATOR_MAPLOADER_HPP

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
		PROPERTY_BUILDING_WORLD,
		PROPERTY_BUILDING_ID,
		PROPERTY_BUILDING_DOOR,

		PROPERTY_UNKNOWN
	};

	PropertyType propertyTypeFromString(const std::string &s);


	class PropertyOwner
	{
	public:

		virtual ~PropertyOwner()
		{
		}

		void addProperty(PropertyType type, std::string value);

		std::string getProperty(PropertyType type);

		bool hasProperty(PropertyType type);

		void getProperty(PropertyType type, boost::optional<std::string> &out);

		boost::optional<std::string> getPropertyOptional(PropertyType type);

	private:
		std::map<PropertyType, std::string> map;
	};

	typedef uint32_t rot;

	enum Rotation
	{
		HORIZONTAL = rot(1 << 31),
		VERTICAL = rot(1 << 30),
		DIAGONAL = rot(1 << 29)
	};

	enum TileType
	{
		TILE_TILE,
		TILE_OBJECT,
		TILE_PROPERTY_SHAPE
	};


	/**
	 * Strips flip flags from gid, and returns the block type
	 * @param flips [0] = horizontal, [1] = vertical, [2] = diagonal
	 * @return Real blocktype
	 */
	int stripFlip(const int &gid, std::bitset<3> &flips);

	struct Tile
	{
		Tile() : Tile(TILE_TILE)
		{
		}

		Tile(TileType type) : gid(0), tileType(type)
		{
		}

		Tile(const std::string &id) : Tile(TILE_TILE, id)
		{
		}

		Tile(TileType type, const std::string &id);

		virtual ~Tile()
		{
		}

		inline TileType getTileType() const
		{
			return tileType;
		}

		inline bool isFlipped() const
		{
			return flipped;
		}

		// @return Either -90, 0 or 90
		inline int getRotationAngle() const
		{
			return rotationAngle;
		}

		// @return Block ID | horizontal or vertical flip flags
		inline int getFlipGID() const
		{
			return flipGID;
		}

		inline unsigned int getGID() const
		{
			return gid;
		}

		sf::Vector2f position;

	private:
		unsigned gid;
		bool flipped;
		TileType tileType;
		int rotationAngle, flipGID;

		void processRotation(std::bitset<3> rotation);
	};

	struct PropertyObject : Tile, PropertyOwner
	{

		PropertyObject() : Tile(TILE_PROPERTY_SHAPE)
		{ }

		sf::Vector2f dimensions;
	};

	struct Object : Tile
	{
		Object(const std::string &id) : Tile(TILE_OBJECT, id)
		{
		}

		float rotationAnglef;
	};

	struct Layer : PropertyOwner
	{
		~Layer();

		std::string name;
		std::vector<Tile *> items;
		bool visible;
	};

	struct TileMap : PropertyOwner
	{
		virtual ~TileMap() override;

		sf::Vector2i size;
		std::vector<Layer *> layers;

		void load(const std::string &filePath);
	};
}
#endif