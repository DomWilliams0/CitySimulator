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

		struct UnloadedDoor
		{
			sf::Vector2i tile;

			// only one needed
			std::string worldName;
			int worldID;
		};


		int lastWorldID;
		std::vector<int> flippedTileGIDs;
		WorldTreeNode &treeRoot;

		WorldLoader(WorldTreeNode &treeRoot);

		World *loadWorlds(const std::string &mainWorldName, Tileset &tileset);

		World *loadMainWorld(const std::string &name, Tileset &tileset, TMX::TileMap &tmx);

		std::string getBuildingFilePath(const std::string &name);

		void findBuildingsAndDoors(TMX::TileMap tmx,
								   std::vector<UnloadedBuilding> &buildingsToLoad,
								   std::vector<UnloadedDoor> &doorsToLoad);

		UnloadedBuilding *findBuildingOwner(const UnloadedDoor &door,
											std::vector<UnloadedBuilding> &buildings);
	};

};

#endif
