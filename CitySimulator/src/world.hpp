#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace TMX{ struct TileMap; }

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
	RUG_EDGE = 21
};

enum LayerType
{
	UNDERTERRAIN,
	TERRAIN,
	OVERTERRAIN,
	OBJECTS,
	COLLISIONS,
	ERROR
};

LayerType layerTypeFromString(const std::string &s);


class Tileset
{
public:
	explicit Tileset(const std::string &filename);

	void textureQuad(sf::Vertex *quad, const BlockType &blockType);

	inline sf::Texture getTexture() const
	{
		return texture;
	}

private:
	sf::Texture texture;
	sf::Vector2f *points;

	sf::Vector2u size;

	inline int getIndex(int x, int y) const
	{
		return x + ((size.x + 1) * y);
	}
};

class BaseWorld : public sf::Drawable
{
public:
	explicit BaseWorld(const sf::Vector2i &size);
	~BaseWorld();
	void setBlockType(const sf::Vector2u &pos, BlockType blockType, LayerType layer = TERRAIN);

	virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
	
	static boost::shared_ptr<BaseWorld> loadWorld(const std::string &filename);

private:
	sf::Vector2i tileSize;
	sf::Vector2i pixelSize;

	Tileset *tileset;
	sf::VertexArray vertices;
	sf::Transform transform;


protected:
	std::unordered_map<LayerType, int> layers;
	std::unordered_set<unsigned int> rotations;

	inline void resizeVertexArray()
	{
		vertices.resize(tileSize.x * tileSize.y * layers.size() * 4);
	}

	inline void registerLayer(LayerType layerType, int depth)
	{
		layers.insert(std::make_pair(layerType, depth));
	}

	friend struct TMX::TileMap;

};

