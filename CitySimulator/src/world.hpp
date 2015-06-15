#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include "maploader.hpp"
#include "utils.hpp"

class Object;
class BaseWorld;

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


enum LayerType
{
	UNDERTERRAIN,
	TERRAIN,
	OVERTERRAIN,
	OBJECTS,
	COLLISIONS,
	LT_ERROR
};

LayerType layerTypeFromString(const std::string &s);

inline bool isTileLayer(LayerType &layerType)
{
	return layerType == UNDERTERRAIN || layerType == TERRAIN || layerType == OVERTERRAIN;
}

class Tileset
{
public:
	explicit Tileset(const std::string &filename);

	void textureQuad(sf::Vertex *quad, const BlockType &blockType, int rotationAngle, int flipGID);

	inline sf::Texture* getTexture()
	{
		if (!converted)
			throw std::exception("Tileset has not yet been converted to a texture");

		return &texture;
	}

	inline sf::Image* getImage() const
	{
		if (converted)
			throw std::exception("Tileset has already been converted to a texture");
		return image;
	}

	inline sf::Vector2u getSize() const
	{
		return size;
	}

	void convertToTexture(const std::vector<int> &flippedGIDs);
	sf::IntRect getTileRect(unsigned blockType);
	void createTileImage(sf::Image *image, unsigned blockType);
protected:
	friend class BaseWorld;

private:
	sf::Image *image;
	sf::Texture texture;
	sf::Vector2f *points;
	sf::Vector2u size;

	std::unordered_map<int, int> flippedBlockTypes;
	bool converted;

	inline void addPoint(int x, int y)
	{
		points[getIndex(x, y)] = sf::Vector2f(x * Constants::tileSizef,
		                                      y * Constants::tileSizef);
	}

	void generatePoints();

	inline int getIndex(int x, int y) const
	{
		return x + (size.x + 1) * y;
	}
};

class BaseWorld : public sf::Drawable
{
public:
	explicit BaseWorld(const sf::Vector2i &size);
	~BaseWorld();
	void setBlockType(const sf::Vector2i &pos, BlockType blockType, LayerType layer = TERRAIN, int rotationAngle = 0, int flipGID = 0);
	void addObject(const sf::Vector2f &pos, BlockType blockType, LayerType layer = OBJECTS, float rotationAngle = 0, int flipGID = 0);

	virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
	static BaseWorld* loadWorld(const std::string &filename);


	inline Tileset* getTileset() const
	{
		return tileset;
	}

	inline sf::Vector2i getPixelSize() const
	{
		return pixelSize;
	}

	inline sf::Vector2i getTileSize() const
	{
		return tileSize;
	}

private:
	sf::Vector2i tileSize;
	sf::Vector2i pixelSize;
	Tileset *tileset;

	sf::VertexArray vertices;
	sf::Transform transform;

	std::vector<BlockType> blockTypes;

	void discoverLayers(std::vector<TMX::Layer*> layers, LayerType *layerTypes);
	void discoverFlippedTiles(const std::vector<TMX::Layer*> &layers, std::vector<int> &flippedGIDs);
	void addTiles(std::vector<TMX::Layer*> layers, LayerType *types);
	int BaseWorld::getBlockIndex(const sf::Vector2i &pos, LayerType layerType);

	void rotateObject(sf::Vertex *quad, float degrees, const sf::Vector2f &pos);
	void BaseWorld::positionVertices(sf::Vertex *quad, const sf::Vector2f &pos, int delta);

protected:
	std::unordered_map<LayerType, int> layerDepths;

	inline void resizeVertexArray()
	{
		const int size = tileSize.x * tileSize.y * layerDepths.size() * 4;
		vertices.resize(size);
		blockTypes.resize(size);
	}

	inline void registerLayer(LayerType layerType, int depth)
	{
		layerDepths.insert(std::make_pair(layerType, depth));
	}

	friend struct TMX::TileMap;
};

//class Object : public sf::Sprite
//{
//public:
//	Object(BaseWorld *world_, BlockType blockType) : sf::Sprite(*world_->getTileset()->getTexture(),
//	                                                            world_->getTileset()->getTileRect(blockType)), world(world_)
//	{
//		auto r = world_->getTileset()->getTileRect(blockType);
//		setScale(Constants::tileScale, Constants::tileScale);
//		auto bounds(getLocalBounds());
//		//		setOrigin(bounds.width / 2, bounds.height / 2);
//	}
//
//
//private:
//	BaseWorld *world;
//};

