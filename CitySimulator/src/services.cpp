#include <SFML/Graphics.hpp>
#include <regex>
#include "PackingTreeNode.h"
#include "services.hpp"

void BaseService::onEnable()
{
}

void BaseService::onDisable()
{
}

Locator::Locator() : services(SERVICE_UNKNOWN, nullptr)
{
}

Locator::~Locator()
{
	for (auto &service : services)
		delete service;
}

RenderService::RenderService(sf::RenderWindow *renderWindow) : window(renderWindow)
{
}

sf::RenderWindow* RenderService::getWindow()
{
	return window;
}

void RenderService::renderEntities()
{
	Locator::locate<EntityService>()->renderSystems(*window);
}

void EntityService::onEnable()
{
	// load entities from file
	auto fileName(Config::getResource("entities.config"));
	ConfigurationFile config(fileName);
	config.load();

	loadEntities(config, ENTITY_HUMAN, "human");
	loadEntities(config, ENTITY_VEHICLE, "vehicle");


	// init entities
	for (size_t i = 0; i < MAX_ENTITIES; ++i)
		entities[i] = COMPONENT_NONE;

	// init systems in correct order
	systems.push_back(new InputSystem);
	systems.push_back(new PhysicsSystem);

	auto render = new RenderSystem;
	systems.push_back(render);
	renderSystem = render;
}


void EntityService::loadEntities(ConfigurationFile &config, EntityType entityType, const std::string &sectionName)
{
	std::vector<std::map<std::string, std::string>> entities;
	config.getMapList<std::string>(sectionName, entities);

	// load tags
	std::vector<ConfigKeyValue> entityMapList;
	config.getMapList(sectionName, entityMapList);

	Logger::pushIndent();

	AnimationService *as = Locator::locate<AnimationService>();
	EntityTags allTags;
	for (auto &entity : entityMapList)
	{
		auto nameIt(entity.find("name"));

		// no name
		if (nameIt == entity.end())
		{
			Logger::logWarning(format("No name found for entity of type %1%, skipping", std::to_string(entityType)));
			continue;
		}

		std::string name(nameIt->second);
		allTags.insert({name, entity});
	}

	loadedTags[entityType] = allTags;

	// external resource loading
	for (auto &entity : allTags)
	{
		// sprites
		auto sprite = entity.second.find("sprite");
		if (sprite != entity.second.end())
			as->loadSprite(entity.second, entityType);
	}

	Logger::popIndent();
}

void EntityService::onDisable()
{
	for (System *system : systems)
		delete system;
}

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
}

std::string AnimationService::getRandomAnimationName(EntityType entityType)
{
	auto anims = animations.find(entityType);
	if (anims == animations.end())
	{
		error("No animations found for entity type %1%", std::to_string(entityType));
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
	Logger::logDebug(format("Loaded sprite %1%", entityTags["name"]));
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
			error("Could not pack spritesheet of size %1%, %2%", std::to_string(size.x), std::to_string(size.y));

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
