#ifndef CITYSIMULATOR_WORLD_SERVICE_HPP
#define CITYSIMULATOR_WORLD_SERVICE_HPP

#include "base_service.hpp"
#include "world.hpp"
#include "building.hpp"
#include "bodydata.hpp"

typedef int WorldID;


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
			WorldID insideWorldID;
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
			WorldID worldID;
		};

		struct LoadedWorld
		{
			World *world;
			TMX::TileMap tmx;

			bool failed() const
			{
				return world == nullptr;
			}
		};

		WorldID lastWorldID;
		std::unordered_set<int> flippedTileGIDs;
		WorldTreeNode &treeRoot;

		// todo implicit map, with incremental integer keys
		std::vector<LoadedWorld> loadedWorlds;

		WorldLoader(WorldTreeNode &treeRoot);

		/**
		 * Recursively loads all worlds into the WorldTree
		 * @param tileset The tileset to use
		 * @return The main world
		 */
		World *loadWorlds(const std::string &mainWorldName, Tileset &tileset);

		/**
		 * Loads the given world
		 * @param name The world name, sans file extension
		 * @param isBuilding True if the world is a building, otherwise false
		 * @return A reference to the newly loaded world
		 */
		LoadedWorld &loadWorld(const std::string &name, bool isBuilding);

		void discoverAndLoadAllWorlds(
				std::vector<UnloadedBuilding> &buildings,
				std::vector<UnloadedDoor> &doors);

		/**
		 * @return The next building ID to use
		 */
		WorldID generateBuildingID();

		/**
		 * @param name The world name, sans file extension
		 * @param isBuilding True if the world is a building, otherwise false
		 * @return The true file path of the world
		 */
		std::string getWorldFilePath(const std::string &name, bool isBuilding);

		void findBuildingsAndDoors(TMX::TileMap &tmx,
		                           std::vector<UnloadedBuilding> &buildings,
		                           std::vector<UnloadedDoor> &doors);

		/**
		 * @return The building that physically contains the given door, null if not found
		 */
		WorldService::WorldLoader::UnloadedBuilding *findBuildingOwner(UnloadedDoor &door,
		                                                               std::vector<UnloadedBuilding> &buildings);
	};

};

#endif
