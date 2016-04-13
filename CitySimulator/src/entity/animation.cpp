#include <SFML/Graphics.hpp>
#include <regex>
#include "PackingTreeNode.h"
#include "animation.hpp"
#include "service/animation_service.hpp"
#include "service/config_service.hpp"
#include "service/logging_service.hpp"

void AnimationService::onEnable()
{
	preProcessImageData = new std::map<sf::Image *, std::pair<ConfigKeyValue, EntityType>>;
	processed = false;
}

Animation *AnimationService::getAnimation(EntityType entityType, const std::string &name)
{
	auto anims = animations.find(entityType);
	if (anims != animations.end())
	{
		auto anim = anims->second.find(name);
		if (anim != anims->second.end())
			return &anim->second;
	}

	error("Animation '%1%' not found", name);
	return nullptr; // shh compiler is ok
}

std::string AnimationService::getRandomAnimationName(EntityType entityType)
{
	auto anims = animations.find(entityType);
	if (anims == animations.end())
	{
		error("No animations found for entity type %1%", _str(entityType));
		return "";
	}

	auto it = anims->second.cbegin();
	std::advance(it, Utils::random(static_cast<size_t>(0), anims->second.size()));
	return it->first;
}

void AnimationService::loadSprite(ConfigKeyValue &entityTags, EntityType entityType)
{
	checkProcessed(false);

	std::string fileName(entityTags["sprite"]);

	sf::Image *image = new sf::Image;
	if (!image->loadFromFile(Utils::searchForFile(fileName, Config::getResource("entities.sprites"))))
		error("Could not load sprite %1%", fileName);

	preProcessImageData->insert({image, {entityTags, entityType}});
	Logger::logDebuggier(format("Loaded sprite %1%", entityTags["name"]));
}

void AnimationService::loadGUI()
{
	// todo load all animations from a gui.json
	sf::Image *image = new sf::Image;
	if (!image->loadFromFile(Config::getResource("gui.controller-arrow")))
		error("Could not load controller arrow image! Serves you right for being so hacky");

	// lord forgive me
	ConfigKeyValue tags;
	tags["name"] = "Controller Arrow";
	tags["anim-count"] = "1";
	tags["anim-length"] = "1";
	tags["anim-dimensions-all"] = "16x8";

	preProcessImageData->insert({image, {tags, ENTITY_UNKNOWN}});
	Logger::logDebuggier("Loaded controller arrow");
}


void AnimationService::checkProcessed(bool shouldBe)
{
	if (shouldBe != processed)
		error("Spritesheet has %1% been processed", (shouldBe ? "not yet" : "already"));

}

sf::Vector2i AnimationService::stringToVector(const std::string &s)
{
	static std::regex reg("^(\\d+)x(\\d+)$");

	std::smatch match;

	// invalid
	if (!regex_search(s, match, reg))
		error("Could not convert string to vector: %1%", s);

	int x(std::stoi(match[1].str().c_str()));
	int y(std::stoi(match[2].str().c_str()));

	return sf::Vector2i(x, y);
}

void AnimationService::positionImages(sf::Vector2i &imageSize, std::map<sf::Image *, sf::IntRect> &imagePositions)
{
// calculate initial size of bin
	sf::Vector2u totalSize = std::accumulate(preProcessImageData->begin(), preProcessImageData->end(), sf::Vector2u(),
											 [](sf::Vector2u &acc,
												const std::pair<sf::Image *, std::pair<ConfigKeyValue, EntityType>> &pair)
											 {
												 return acc + pair.first->getSize();
											 });

	sf::IntRect binRect(0, 0, totalSize.x, totalSize.y);
	PackingTreeNode node(binRect);

	// sort by area (insert into a vector first)
	std::vector<std::pair<sf::Image *, std::pair<ConfigKeyValue, EntityType>>> asVector;
	for (auto it(preProcessImageData->cbegin()); it != preProcessImageData->cend(); ++it)
		asVector.push_back(*it);

	std::sort(asVector.begin(), asVector.end(),
			  [](const std::pair<sf::Image *, std::pair<ConfigKeyValue, EntityType>> &left,
				 const std::pair<sf::Image *, std::pair<ConfigKeyValue, EntityType>> &right)
			  {
				  sf::Vector2i leftSize(left.first->getSize());
				  sf::Vector2i rightSize(right.first->getSize());

				  return (leftSize.x * leftSize.y) < (rightSize.x * rightSize.y);
			  });

	// insert one at a time
	for (auto &pair : asVector)
	{
		sf::Vector2u size = pair.first->getSize();
		sf::IntRect rect(0, 0, static_cast<int>(size.x), static_cast<int>(size.y));

		auto positionedRect = node.insert(rect);

		if (!positionedRect)
			error("Could not pack spritesheet of size %1%, %2%", _str(size.x), _str(size.y));

		imagePositions.insert({pair.first, *positionedRect});
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

void AnimationService::processQueuedSprites()
{
	// no images
	if (preProcessImageData->empty())
	{
		Logger::logWarning("No sprites were queued for loading by loadSprite");
		return;
	}

	// position
	sf::Vector2i finalImageSize;
	std::map<sf::Image *, sf::IntRect> imageRects;
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

		auto pair = preProcessImageData->at(image);
		ConfigKeyValue entityTags = pair.first;
		EntityType entityType = pair.second;

		int animCount, animLength;

		try
		{
			animCount = Utils::stringToInt(entityTags.at("anim-count"));
			animLength = Utils::stringToInt(entityTags.at("anim-length"));
		}
		catch (std::out_of_range &)
		{
			error("Could not get animation info from %1% tags", entityTags["name"]);
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
			Logger::logDebug(format("anim-dimensions-all not set for animation %1%, skipping", entityTags["name"]));
			continue;
		}

		// store in animation map under the entity type
		std::pair<std::string, Animation> animationPair = {entityTags["name"], anim};

		auto existingAnims = animations.find(entityType);
		if (existingAnims != animations.end())
			existingAnims->second.insert(animationPair);
		else
		{
			std::unordered_map<std::string, Animation> anims;
			anims.insert(animationPair);
			animations.insert({entityType, anims});
		}
	}

	preProcessImageData->clear();
	delete preProcessImageData;

	processed = true;
}

Animation *Animation::addRow(const sf::Vector2i &startPosition, const sf::Vector2i &spriteDimensions, int rowLength)
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

Animator::Animator(Animation *anim, float step, DirectionType initialDirection, bool initiallyPlaying)
{
	init(anim, step, initialDirection, initiallyPlaying);
}

void Animator::init(Animation *anim, float step, DirectionType initialDirection, bool initiallyPlaying)
{
	animation = anim;
	ticker.setMinAndMax(step);
	currentSequence = 0;
	currentFrame = 0;
	playing = initiallyPlaying;
	direction = initialDirection;

	vertices.setPrimitiveType(sf::Quads);
	vertices.resize(4);

	if (anim != nullptr)
	{
		direction = DIRECTION_UNKNOWN;
		turn(initialDirection);
	}
}

void Animator::tick(float delta)
{
	if (!playing)
		return;

	// next frame
	if (ticker.tick(delta))
	{
		if (++currentFrame >= animation->sequences.size())
			currentFrame = 0;

		updateFrame();
	}
}

void Animator::turn(DirectionType direction, bool reset)
{
	if (direction == this->direction)
		return;

	this->direction = direction;

	// convert direction to sequence
	if (direction == DIRECTION_NORTH)
		currentSequence = 3;
	else if (direction == DIRECTION_EAST)
		currentSequence = 2;
	else if (direction == DIRECTION_WEST)
		currentSequence = 1;
	else if (direction == DIRECTION_SOUTH)
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

void Animator::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	states.texture = animation->texture;
	target.draw(vertices, states);
}
