#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include "maploader.hpp"

class World;

enum BlockType
{
	BLANK = 0,
	GRASS = 1,
	DIRT = 2,
	ROAD = 3,
	PAVEMENT = 4,
	SAND = 5,
	WATER = 6,
	COBBLESTONE = 7,
	TREE = 8,
	FENCE = 9,
	SLIDING_DOOR = 10,
	BUILDING_WALL = 11,
	BUILDING_WINDOW_ON = 12,
	BUILDING_WINDOW_OFF = 13,
	BUILDING_ROOF = 14,
	BUILDING_EDGE = 15,
	BUILDING_ROOF_CORNER = 16,
	WOODEN_FLOOR = 17,
	ENTRANCE_MAT = 18,
	RUG = 19,
	RUG_CORNER = 20,
	RUG_EDGE = 21,

	LAST
};

bool isCollidable(BlockType blockType);


enum LayerType
{
	LAYER_UNDERTERRAIN,
	LAYER_TERRAIN,
	LAYER_OVERTERRAIN,
	LAYER_OBJECTS,
	LAYER_COLLISIONS,
	LAYER_COUNT
};

LayerType layerTypeFromString(const std::string &s);

bool isTileLayer(LayerType &layerType);

class Tileset
{
public:
	explicit Tileset(const std::string &filename);

	~Tileset();

	void textureQuad(sf::Vertex *quad, const BlockType &blockType, int rotationAngle, int flipGID);

	sf::Texture* getTexture();

	sf::Image* getImage() const;

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

class WorldTerrain : public BaseWorld
{
public:

	explicit WorldTerrain(World *container);
	~WorldTerrain();

	void setBlockType(const sf::Vector2i &pos, BlockType blockType, LayerType layer = LAYER_TERRAIN, int rotationAngle = 0, int flipGID = 0);
	void addObject(const sf::Vector2f &pos, BlockType blockType, LayerType layer = LAYER_OBJECTS, float rotationAngle = 0, int flipGID = 0);

	Tileset* getTileset() const;

private:
	Tileset *tileset;
	sf::VertexArray vertices;

	std::vector<BlockType> blockTypes;

	int discoverLayers(std::vector<TMX::Layer*> &layers, std::vector<LayerType> &layerTypes);
	void discoverFlippedTiles(const std::vector<TMX::Layer*> &layers, std::vector<int> &flippedGIDs);
	void addTiles(const std::vector<TMX::Layer*> &layers, const std::vector<LayerType> &types);
	int getBlockIndex(const sf::Vector2i &pos, LayerType layerType);

	void rotateObject(sf::Vertex *quad, float degrees, const sf::Vector2f &pos);
	void positionVertices(sf::Vertex *quad, const sf::Vector2f &pos, int delta);

protected:
	std::map<LayerType, int> layerDepths;

	void resize(const int &layerCount);
	void registerLayer(LayerType layerType, int depth);

	void render(sf::RenderTarget &target, sf::RenderStates &states) const;

	void load(const TMX::TileMap *tileMap);

	friend struct TMX::TileMap;
	friend class World;
};

class CollisionMap : public BaseWorld
{
public:
	explicit CollisionMap(World *container) : BaseWorld(container)
	{
	}

	void getSurroundingTiles(const sf::Vector2i &tilePos, std::vector<sf::Rect<float>> &ret);
	std::multimap<std::pair<int, int>, sf::FloatRect> cellGrid;

protected:
	void load();
	void renderDebugTiles(sf::RenderTarget &target) const;

	friend class World;

private:
	std::vector<sf::FloatRect> debugRenderTiles;

	void findCollidableTiles(std::vector<sf::FloatRect> &rects);
	void mergeAdjacentTiles(std::vector<sf::Rect<float>> &rects);
	void mergeHelper(std::vector<sf::FloatRect> &rects, bool moveOnIfFar);
};

class World : public sf::Drawable
{
public:

	World();

	void loadFromFile(const std::string &filename);

	void resize(sf::Vector2i size);

	WorldTerrain& getTerrain();

	sf::Vector2i getPixelSize() const;

	sf::Vector2i getTileSize() const;

	sf::Transform getTransform() const;

	BlockType getBlockAt(const sf::Vector2i &tile, LayerType layer = LAYER_TERRAIN);
	void getSurroundingTiles(const sf::Vector2i &tilePos, std::vector<sf::Rect<float>> &ret);
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
