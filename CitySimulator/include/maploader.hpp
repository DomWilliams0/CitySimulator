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

		/**
		 * The visibility of an object/layer
		 */
		PROPERTY_VISIBLE,

		/**
		 * A building's world name
		 */
		PROPERTY_BUILDING_WORLD,

		/**
		 * Any doors with an equal WORLD_SHARE_SPECIFIER 
		 * will share this door's world
		 */
		PROPERTY_DOOR_WORLD_SHARE_SOURCE,

		/**
		 * This door's world is specified by the single
		 * door with an equivalent WORLD_SHARE_SOURCE
		 */
		PROPERTY_DOOR_WORLD_SHARE_SPECIFIER,

		/**
		 * The ID of a preloaded door's world
		 */
		PROPERTY_DOOR_WORLD_ID,

		/**
		 * The name of a door's world
		 */
		PROPERTY_DOOR_WORLD,

		/**
		 * A door ID
		 */
		PROPERTY_DOOR_ID,

		/**
		 * An invalid property
		 */

		PROPERTY_UNKNOWN
	};

	PropertyType propertyTypeFromString(const std::string &s);


	/**
	 * A map of PropertyType -> string pairs
	 */
	class PropertyHolder
	{
	public:

		virtual ~PropertyHolder()
		{
		}

		void addProperty(PropertyType type, std::string value);

		std::string getProperty(PropertyType type) const;

		bool hasProperty(PropertyType type) const;

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

		virtual ~Tile()
		{
		}

		void setGID(const std::string &id);

		TileType getTileType() const;

		bool isFlipped() const;

		// @return Either -90, 0 or 90
		int getRotationAngle() const;

		// @return Block ID | horizontal or vertical flip flags
		int getFlipGID() const;

		unsigned int getGID() const;

		sf::Vector2f position;

	private:
		unsigned gid;
		bool flipped;
		TileType tileType;
		int rotationAngle, flipGID;

		void processRotation(std::bitset<3> rotation);
	};

	struct PropertyObject : PropertyHolder
	{
		sf::Vector2f dimensions;
	};

	struct TileWrapper
	{
		TileType type;
		Tile tile;

		// would be a union but too trivial to bother
		float objectRotation;
		PropertyObject property;
	};

	struct Layer : PropertyHolder
	{
		std::string name;
		std::vector<TileWrapper> items;
		bool visible;
	};

	struct TileMap : PropertyHolder
	{
		std::string filePath;
		sf::Vector2i size;
		std::vector<Layer> layers;

		void load(const std::string &filePath);
	};
}
#endif
