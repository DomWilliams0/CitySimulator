#ifndef CITYSIMULATOR_BUILDING_HPP
#define CITYSIMULATOR_BUILDING_HPP

#include <SFML/Graphics/Rect.hpp>
#include <map>
#include "utils.hpp"

class World;
class Building;

typedef int BuildingID;
typedef int DoorID;
typedef int WindowID;

struct Window
{
	sf::Vector2i location;
	bool status;
};


/**
 * A building entrance/exit, either inside or outside
 */
struct Door
{
	Location location;
	// todo locked status?
};

/**
 * A building, with doors and windows
 */
class Building
{
public:
	Building(const sf::IntRect &tileBounds, BuildingID id,
			WorldID outsideWorld, WorldID insideWorld);

	/**
	 * Finds all windows in the bounds
	 */
	void discoverWindows();

	/**
	 * Adds a door to the building at the given location
	 * This must be in either the outside or inside world
	 */
	void addDoor(const Location &location, DoorID id);

	void setWindowLight(WindowID window, bool isNowLit);

	bool isWindowLightOn(WindowID window) const;

	Door *getDoor(DoorID id);

	WindowID getWindowCount() const;

	DoorID getDoorCount() const;

	BuildingID getID() const;

	std::string getInsideWorldName() const;

	World *getOutsideWorld() const;

	World *getInsideWorld() const;

private:
	BuildingID id;
	World *insideWorld;
	World *outsideWorld;
	std::string insideWorldName;

	std::map<WindowID, Window> windows;
	std::map<DoorID, Door> doors;

	sf::IntRect bounds;
};


#endif
