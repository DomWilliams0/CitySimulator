#include <unordered_set>
#include "world.hpp"
#include "service/logging_service.hpp"
#include "service/render_service.hpp"

Tileset::Tileset(const std::string &path) : path(path), converted(false)
{
}


void Tileset::load()
{
	// load image
	image = new sf::Image;
	if (!image->loadFromFile(path))
		error("Could not load tileset '%1%'", path);

	size = image->getSize();
	size.x /= Constants::tilesetResolution;
	size.y /= Constants::tilesetResolution;

	generatePoints();
}

void Tileset::textureQuad(sf::Vertex *quad, const BlockType &blockType, int rotationAngle, int flipGID)
{
	int blockID;
	auto flipResult = flippedBlockTypes.find(flipGID);

	// not flipped
	if (flipGID == 0 || flipResult == flippedBlockTypes.end())
		blockID = blockType;

	// flipped
	else
		blockID = flipResult->second;

	int row = blockID % size.x;
	int col = blockID / size.x;

	// rotating
	int offset = 0;
	if (rotationAngle == -90)
		offset = 1;
	else if (rotationAngle == 90)
		offset = 3;

	quad[(0 + offset) % 4].texCoords = points[getIndex(row, col)];
	quad[(1 + offset) % 4].texCoords = points[getIndex(row + 1, col)];
	quad[(2 + offset) % 4].texCoords = points[getIndex(row + 1, col + 1)];
	quad[(3 + offset) % 4].texCoords = points[getIndex(row, col + 1)];
}

sf::Texture *Tileset::getTexture() const
{
	if (!converted)
		throw std::runtime_error("Tileset has not yet been converted to a texture");

	return const_cast<sf::Texture *>(&texture);
}

sf::Image *Tileset::getImage() const
{
	if (converted)
		throw std::runtime_error("Tileset has already been converted to a texture");
	return image;
}

sf::Vector2u Tileset::getSize() const
{
	return size;
}

void Tileset::convertToTexture(const std::unordered_set<int> &flippedGIDs)
{
	// resize image
	int totalBlockTypes = BLOCK_UNKNOWN + flippedGIDs.size();
	int rowsRequired = totalBlockTypes / size.x;

	if (totalBlockTypes % size.x != 0)
		rowsRequired += 1;

	// transfer to new image
	sf::Image newImage;
	newImage.create(size.x * Constants::tilesetResolution, rowsRequired * Constants::tilesetResolution);
	newImage.copy(*image, 0, 0);

	// update size
	size.y = rowsRequired;
	generatePoints();

	// render flipped blocktypes
	int currentBlockType(BLOCK_UNKNOWN);
	for (int flippedGID : flippedGIDs)
	{
		std::bitset<3> flips;
		int blockType(TMX::stripFlip(flippedGID, flips));

		// create image
		sf::Image flippedImage;
		flippedImage.create(16, 16);
		createTileImage(&flippedImage, blockType);

		// flip
		if (flips[0])
			flippedImage.flipHorizontally();
		if (flips[1])
			flippedImage.flipVertically();

		// copy to tileset
		sf::IntRect rect = getTileRect(currentBlockType);
		newImage.copy(flippedImage, rect.left, rect.top);

		// remember this new blocktype
		flippedBlockTypes.insert(std::make_pair(flippedGID, currentBlockType));

		++currentBlockType;
	}

	// write to texture
	if (!texture.loadFromImage(newImage))
		throw std::runtime_error("Could not render tileset");
	texture.setSmooth(false);

	converted = true;
	delete image;
}

sf::IntRect Tileset::getTileRect(unsigned blockType)
{
	int tileX = blockType % size.x;
	int tileY = blockType / size.x;
	return sf::IntRect(tileX * Constants::tilesetResolution,
	                   tileY * Constants::tilesetResolution,
	                   Constants::tilesetResolution,
	                   Constants::tilesetResolution);
}

void Tileset::createTileImage(sf::Image *image, unsigned blockType)
{
	if (converted)
		throw std::runtime_error("Tileset has already been converted to a texture");

	image->create(Constants::tilesetResolution, Constants::tilesetResolution);
	image->copy(*this->image, 0, 0, getTileRect(blockType));
}

void Tileset::addPoint(int x, int y)
{
	points[getIndex(x, y)] = sf::Vector2f(x * Constants::tilesetResolution,
	                                      y * Constants::tilesetResolution);
}

void Tileset::generatePoints()
{
	points.resize((size.x + 1) * (size.y + 1));
	for (size_t y = 0; y <= size.y; y++)
		for (size_t x = 0; x <= size.x; x++)
			addPoint(x, y);
}

int Tileset::getIndex(int x, int y) const
{
	return x + (size.x + 1) * y;
}

RenderService::RenderService(sf::RenderWindow *renderWindow) : window(renderWindow)
{
}

void RenderService::onEnable()
{
}

sf::RenderWindow *RenderService::getWindow()
{
	return window;
}

void RenderService::render(const World &world)
{
	limitView(world);

	window->setView(*view);
	window->draw(world);
}

sf::Vector2f RenderService::mapScreenToWorld(const sf::Vector2i &screenPos)
{
	return window->mapPixelToCoords(screenPos, *view);
}

void RenderService::setView(sf::View &view)
{
	this->view = &view;
}

void RenderService::limitView(const World &world)
{
	const sf::Vector2i &worldSize = world.getPixelSize();
	bool update = false;

	sf::Vector2f centre = view->getCenter();
	sf::Vector2f size = view->getSize();

	// horizontal
	if (size.x > worldSize.x)
	{
		update = true;
		centre.x = worldSize.x / 2;
	}
	else
	{

		float leftBound = centre.x - size.x / 2.f;
		if (leftBound < 0)
		{
			update = true;
			centre.x -= leftBound;
		}
		else
		{
			float rightBound = centre.x + size.x / 2.f;
			if (rightBound > worldSize.x)
			{
				update = true;
				centre.x -= (rightBound - worldSize.x);
			}
		}
	}

	// vertical
	if (size.y > worldSize.y)
	{
		update = true;
		centre.y = worldSize.y / 2;
	}
	else
	{
		float topBound = centre.y - size.y / 2.f;
		if (topBound < 0)
		{
			update = true;
			centre.y -= topBound;
		}
		else
		{
			float bottomBound = centre.y + size.y / 2.f;
			if (bottomBound > worldSize.y)
			{
				update = true;
				centre.y -= (bottomBound - worldSize.y);
			}
		}
	}

	if (update)
		view->setCenter(centre);
}

