#ifndef CITYSIM_WORLD_HPP
#define CITYSIM_WORLD_HPP

#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>
#include <unordered_map>
#include <set>
#include <boost/optional.hpp>
#include "SFMLDebugDraw.h"
#include "maploader.hpp"

class World;

enum BlockType
{
	BLOCK_BLANK = 0,
	BLOCK_GRASS = 1,
	BLOCK_DIRT = 2,
	BLOCK_ROAD = 3,
	BLOCK_PAVEMENT = 4,
	BLOCK_SAND = 5,
	BLOCK_WATER = 6,
	BLOCK_COBBLESTONE = 7,
	BLOCK_TREE = 8,
	BLOCK_FENCE = 9,
	BLOCK_SLIDING_DOOR = 10,
	BLOCK_BUILDING_WALL = 11,
	BLOCK_BUILDING_WINDOW_ON = 12,
	BLOCK_BUILDING_WINDOW_OFF = 13,
	BLOCK_BUILDING_ROOF = 14,
	BLOCK_BUILDING_EDGE = 15,
	BLOCK_BUILDING_ROOF_CORNER = 16,
	BLOCK_WOODEN_FLOOR = 17,
	BLOCK_ENTRANCE_MAT = 18,
	BLOCK_RUG = 19,
	BLOCK_RUG_CORNER = 20,
	BLOCK_RUG_EDGE = 21,

	BLOCK_UNKNOWN
};

bool isCollidable(BlockType blockType);


enum LayerType
{
	LAYER_UNDERTERRAIN,
	LAYER_TERRAIN,
	LAYER_OVERTERRAIN,
	LAYER_OBJECTS,
	LAYER_COLLISIONS,

	LAYER_UNKNOWN
};

LayerType layerTypeFromString(const std::string &s);

bool isTileLayer(LayerType &layerType);

class Tileset
{
public:
	Tileset();

	~Tileset();

	void load(const std::string &path);

	void textureQuad(sf::Vertex *quad, const BlockType &blockType, int rotationAngle, int flipGID);

	sf::Texture *getTexture() const;

	sf::Image *getImage() const;

	sf::Vector2u getSize() const;

	void convertToTexture(const std::vector<int> &flippedGIDs);

	sf::IntRect getTileRect(unsigned blockType);

	void createTileImage(sf::Image *image, unsigned blockType);

protected:
	friend class WorldTerrain;

private:
	sf::Image *image;
	sf::Texture texture;
	sf::Vector2f *points;
	sf::Vector2u size;

	std::unordered_map<int, int> flippedBlockTypes;
	bool converted;

	void addPoint(int x, int y);

	void generatePoints();

	int getIndex(int x, int y) const;
};

class BaseWorld
{
public:
	explicit BaseWorld(World *container) : container(container)
	{
	}

protected:
	World *container;
};

struct WorldObject
{
	BlockType type;
	float rotation;
	sf::Vector2f tilePos;

	WorldObject(BlockType blockType, float rotationAngle, const sf::Vector2f &position) :
			type(blockType), rotation(rotationAngle), tilePos(position)
	{
	}
};

class WorldTerrain : public BaseWorld
{
public:

	explicit WorldTerrain(World *container);

	~WorldTerrain();

	void setBlockType(const sf::Vector2i &pos, BlockType blockType, LayerType layer = LAYER_TERRAIN,
	                  int rotationAngle = 0, int flipGID = 0);

	void addObject(const sf::Vector2f &pos, BlockType blockType, float rotationAngle, int flipGID);

	std::vector<WorldObject> &getObjects();

private:
	Tileset tileset;
	sf::VertexArray vertices;

	std::vector<BlockType> blockTypes;
	std::vector<WorldObject> objects;

	int discoverLayers(std::vector<TMX::Layer *> &layers, std::vector<LayerType> &layerTypes);

	void discoverFlippedTiles(const std::vector<TMX::Layer *> &layers, std::vector<int> &flippedGIDs);

	void addTiles(const std::vector<TMX::Layer *> &layers, const std::vector<LayerType> &types);

	int getBlockIndex(const sf::Vector2i &pos, LayerType layerType);

	void rotateObject(sf::Vertex *quad, float degrees, const sf::Vector2f &pos);

	void positionVertices(sf::Vertex *quad, const sf::Vector2f &pos, int delta);

protected:
	std::map<LayerType, int> layerDepths;

	void resize(const int &layerCount);

	void registerLayer(LayerType layerType, int depth);

	void render(sf::RenderTarget &target, sf::RenderStates &states) const;

	void load(const TMX::TileMap *tileMap, const std::string &tilesetPath);

	friend struct TMX::TileMap;

	friend class World;
};

class CollisionMap : public BaseWorld
{
public:
	explicit CollisionMap(World *container) : BaseWorld(container), world(b2Vec2(0.f, 0.f)),
	                                          worldBody(nullptr)
	{
		world.SetAllowSleeping(true);
	}

	~CollisionMap();

	void getSurroundingTiles(const sf::Vector2i &tilePos, std::set<sf::FloatRect> &ret);

	bool getRectAt(const sf::Vector2i &tilePos, sf::FloatRect &ret);

protected:
	void load();

	b2World world;
	b2Body *worldBody;

	friend class World;

private:
	struct CollisionRect
	{
		sf::FloatRect rect;
		float rotation;

		CollisionRect(const sf::FloatRect &r, float rot) : rect(r), rotation(rot)
		{
		}
	};

	boost::optional<SFMLDebugDraw> b2Renderer;
	std::multimap<sf::Vector2i, sf::FloatRect> cellGrid;

	void findCollidableTiles(std::vector<CollisionRect> &rects) const;

	void mergeAdjacentTiles(std::vector<CollisionRect> &rects, std::vector<sf::FloatRect> &ret);

	void mergeHelper(std::vector<sf::FloatRect> &rects, bool moveOnIfFar);
};

class World : public sf::Drawable
{
public:
	World();

	void loadFromFile(const std::string &filename, const std::string &tileset);

	void resize(sf::Vector2i size);

	WorldTerrain &getTerrain();

	CollisionMap &getCollisionMap();

	b2World *getBox2DWorld();

	sf::Vector2i getPixelSize() const;

	sf::Vector2i getTileSize() const;

	sf::Transform getTransform() const;

	void tick(float delta);

	BlockType getBlockAt(const sf::Vector2i &tile, LayerType layer = LAYER_TERRAIN);

	void getSurroundingTiles(const sf::Vector2i &tilePos, std::set<sf::FloatRect> &ret);

private:
	WorldTerrain terrain;
	CollisionMap collisionMap;

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

protected:
	sf::Vector2i tileSize;
	sf::Vector2i pixelSize;
	sf::Transform transform;

	friend class BaseWorld;

	friend class WorldTerrain;

	friend class CollisionMap;
};

#endif