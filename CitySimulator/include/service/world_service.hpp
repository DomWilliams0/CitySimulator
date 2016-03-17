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
		unsigned int lastWorldID;
		std::vector<int> flippedTileGIDs;
		WorldTreeNode &treeRoot;

		WorldLoader(WorldTreeNode &treeRoot);

		World *loadWorlds(const std::string &mainWorldName, Tileset &tileset);

		World *loadMainWorld(const std::string &name, Tileset &tileset);

		std::string getBuildingFilePath(const std::string &name);

		void findBuildings(World *world);
	};

};

#endif
