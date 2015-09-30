#include <boost/format.hpp>
#include <SFML/Graphics.hpp>
#include <numeric>
#include <regex>
#include "PackingTreeNode.h"
#include "animation.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "config.hpp"

void SpriteSheet::loadSprite(ConfigKeyValue &entityTags)
{
	checkProcessed(false);

	std::string fileName(entityTags["sprite"]);

	sf::Image *image = new sf::Image;
	if (!image->loadFromFile(Utils::searchForFile(fileName, "res/entities")))
	{
		FAIL("Could not load sprite %1%", fileName);
	}

	preProcessSpriteImages.insert({ image, entityTags });
	Logger::logDebug(FORMAT("Loaded sprite %1%", entityTags["name"]));
}

void SpriteSheet::processAllSprites()
{
	checkProcessed(false);

	// no images
	if (preProcessSpriteImages.empty())
		return;

	// position 
	sf::Vector2i finalImageSize;
	std::map<sf::Image*, sf::IntRect> imageRects;
	positionImages(finalImageSize, imageRects);

	// create image
	sf::Image finalImage;
	finalImage.create(finalImageSize.x, finalImageSize.y, sf::Color::Transparent);

	// copy individual images to spritesheet
	for (auto &pair : imageRects)
	{
		finalImage.copy(*pair.first, pair.second.left, pair.second.top);
		delete pair.first;
	}

	// convert to texture
	if (!texture.loadFromImage(finalImage))
		throw std::runtime_error("Could not convert spritesheets to a texture");

	// create animations
	for (auto &rectPair : imageRects)
	{
		Animation anim(&texture);

		sf::Image *image(rectPair.first);
		sf::IntRect &rect(rectPair.second);
		ConfigKeyValue entityTags(preProcessSpriteImages[image]);

		int animCount, animLength;

		try
		{
			animCount = ConfigurationFile::stringToInt(entityTags.at("anim-count"));
			animLength = ConfigurationFile::stringToInt(entityTags.at("anim-length"));
		}
		catch (std::out_of_range &)
		{
			FAIL("Could not get animation info from %1% tags", entityTags["name"]);
		}

		// all dimensions the same
		if (entityTags.find("anim-dimensions-all") != entityTags.end())
		{
			sf::Vector2i dimensions(stringToVector(entityTags["anim-dimensions-all"]));

			sf::Vector2i pos(rect.left, rect.top);

			for (int seq = 0; seq < animCount; ++seq)
			{
				anim.addRow(pos, dimensions, animLength);
				pos.y += dimensions.y;
			}
		}

		// use given dimensions for each row
		else
		{
			// TODO
		}

		animations.insert({ entityTags["name"], anim });
	}

	preProcessSpriteImages.clear();
	processed = true;
}

void SpriteSheet::positionImages(sf::Vector2i &imageSize, std::map<sf::Image*, sf::IntRect> &imagePositions)
{
	// calculate initial size of bin
	sf::Vector2u totalSize = std::accumulate(preProcessSpriteImages.begin(), preProcessSpriteImages.end(), sf::Vector2u(),
		[](sf::Vector2u &acc, const std::pair<sf::Image*, ConfigKeyValue> &pair)
	{
		return acc + pair.first->getSize();
	});

	sf::IntRect binRect(0, 0, totalSize.x, totalSize.y);
	PackingTreeNode *node = new PackingTreeNode(binRect);

	// sort by area (insert into a vector first)
	std::vector<std::pair<sf::Image*, ConfigKeyValue>> asVector(preProcessSpriteImages.begin(), preProcessSpriteImages.end());

	std::sort(asVector.begin(), asVector.end(),
		[](const std::pair<sf::Image*, ConfigKeyValue> &left,
		const std::pair<sf::Image*, ConfigKeyValue> &right)
	{
		sf::Vector2i leftSize(left.first->getSize());
		sf::Vector2i rightSize(right.first->getSize());

		return (leftSize.x * leftSize.y) < (rightSize.x * rightSize.y);
	});

	// insert one at a time
	for (auto &pair : asVector)
	{
		sf::Vector2i size = static_cast<sf::Vector2i>(pair.first->getSize());
		sf::IntRect rect(0, 0, size.x, size.y);

		auto positionedRect = node->insert(rect);

		if (!positionedRect)
		{
			FAIL2("Could not pack spritesheet of size %1%, %2%", size.x, size.y);
		}

		imagePositions.insert({ pair.first, *positionedRect });
	}

	// find min bounding box
	int minX(0);
	int minY(0);

	int x, y;
	for (auto &pair : imagePositions)
	{
		sf::IntRect &rect(pair.second);
		x = rect.left + rect.width;
		if (x > minX)
			minX = x;

		y = rect.top + rect.height;
		if (y > minY)
			minY = y;
	}

	imageSize.x = minX;
	imageSize.y = minY;
}


void SpriteSheet::checkProcessed(bool shouldBe)
{
	if (shouldBe != processed)
	{
		FAIL("Spritesheet has %1% been processed", (shouldBe ? "not yet" : "already"));
	}
}

sf::Vector2i SpriteSheet::stringToVector(const std::string &s)
{
	static std::regex reg("^(\\d+)x(\\d+)$");

	std::smatch match;

	// invalid
	if (!regex_search(s, match, reg))
	{
		FAIL("Could not convert string to vector: %1%", s);
	}

	int x(stoi(match[1]));
	int y(stoi(match[2]));

	return sf::Vector2i(x, y);
}

Animation* Animation::addRow(const sf::Vector2i &startPosition, const sf::Vector2i &spriteDimensions, int rowLength)
{
	Sequence seq;

	sf::IntRect rect(startPosition, spriteDimensions);
	for (int i = 0; i < rowLength; i++)
	{
		seq.push_back(rect);
		rect.left += spriteDimensions.x;
	}

	sequences.push_back(seq);
	return this;
}

Animator::Animator(Animation *anim, float step) : animation(anim), frameStep(step), currentFrameTime(0),
currentSequence(0), currentFrame(0), playing(true), direction(Direction::SOUTH)
{
	vertices.setPrimitiveType(sf::Quads);
	vertices.resize(4);
	updateFrame();
}

void Animator::tick(float delta)
{
	if (!playing)
		return;

	currentFrameTime += delta;

	// next frame
	if (currentFrameTime >= frameStep)
	{
		currentFrameTime = fmod(currentFrameTime, frameStep);

		if (++currentFrame >= animation->sequences.size())
			currentFrame = 0;

		updateFrame();
	}
}

void Animator::turn(int direction, bool reset)
{
	if (direction == this->direction)
		return;

	this->direction = direction;

	// convert direction to sequence
	if (direction == Direction::NORTH)
		currentSequence = 3;
	else if (direction == Direction::EAST)
		currentSequence = 2;
	else if (direction == Direction::WEST)
		currentSequence = 1;
	else if (direction == Direction::SOUTH)
		currentSequence = 0;

	if (reset)
		currentFrame = 0;

	updateFrame();
}

void Animator::setPlaying(bool playing, bool reset)
{
	this->playing = playing;

	if (reset)
	{
		currentFrame = 0;
		updateFrame();
	}
}

void Animator::togglePlaying(bool resetEachTime)
{
	setPlaying(!playing, resetEachTime);
}

void Animator::resizeVertices(float width, float height)
{
	if (width == currentSize.x && height == currentSize.y)
		return;

	vertices[0].position = sf::Vector2f(0, 0);
	vertices[1].position = sf::Vector2f(width, 0);
	vertices[2].position = sf::Vector2f(width, height);
	vertices[3].position = sf::Vector2f(0, height);

	currentSize.x = width;
	currentSize.y = height;
}

void Animator::updateFrame()
{
	Animation::Sequence sequence(animation->sequences[currentSequence]);
	sf::FloatRect rect(sequence[currentFrame]);

	resizeVertices(rect.width, rect.height);

	vertices[0].texCoords = sf::Vector2f(rect.left, rect.top);
	vertices[1].texCoords = sf::Vector2f(rect.left + rect.width, rect.top);
	vertices[2].texCoords = sf::Vector2f(rect.left + rect.width, rect.top + rect.height);
	vertices[3].texCoords = sf::Vector2f(rect.left, rect.top + rect.height);
}

void Animator::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.texture = animation->texture;
	target.draw(vertices, states);
}