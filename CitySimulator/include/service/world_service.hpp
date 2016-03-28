#ifndef CITYSIMULATOR_WORLD_SERVICE_HPP
#define CITYSIMULATOR_WORLD_SERVICE_HPP

#include "base_service.hpp"
#include "world.hpp"
#include "building.hpp"
#include "bodydata.hpp"

class WorldService : public BaseService
{
public:
	WorldService(const std::string &mainWorldPath, const std::string &tilesetPath);

	virtual void onEnable() override;

	virtual void onDisable() override;

	World &getWorld();

private:
	typedef TreeNode<World> WorldTreeNode;

	Tileset tileset;
	std::string mainWorldName;

	World *mainWorld;
	WorldTreeNode worldTree;

	struct WorldLoader
	{
		struct UnloadedBuilding
		{
			int insideWorldID;
			std::string insideWorldName;
			sf::IntRect bounds;
		};

		enum DoorTag
		{
			DOORTAG_WORLD_ID,
			DOORTAG_WORLD_SHARE,
			DOORTAG_WORLD_NAME,
			DOORTAG_UNKNOWN
		};

		struct UnloadedDoor
		{
			sf::Vector2i tile;
			int doorID;

			DoorTag doorTag;
			std::string worldName;
			std::string worldShare;
			int worldID;
		};

		struct LoadedWorld
		{
			World *world;
			TMX::TileMap tmx;

			bool failed() const {
				return world == nullptr;
			}
		};

		int lastWorldID;
		std::vector<int> flippedTileGIDs;
		WorldTreeNode &treeRoot;

		std::vector<UnloadedBuilding> buildingsToLoad;
		std::vector<UnloadedDoor> doorsToLoad;

		WorldLoader(WorldTreeNode &treeRoot);

		World *loadWorlds(const std::string &mainWorldName, Tileset &tileset);

		void recurseOnDoors();

		void *loadWorld(const std::string &name, LoadedWorld &out, bool isBuilding);

		int generateBuildingID();

		/**
		 * @param name The world name, sans file extension
		 * @param isBuilding True if the world is a building, otherwise false
		 * @return The true file path of the world
		 */
		std::string getWorldFilePath(const std::string &name, bool isBuilding);

		void findBuildingsAndDoors(TMX::TileMap tmx);

		WorldService::WorldLoader::UnloadedBuilding *findBuildingOwner(
				const WorldService::WorldLoader::UnloadedDoor &door);
	};

};

#endif
