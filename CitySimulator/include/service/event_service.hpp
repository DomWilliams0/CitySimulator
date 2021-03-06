#ifndef CITYSIMULATOR_EVENT_SERVICE_HPP
#define CITYSIMULATOR_EVENT_SERVICE_HPP

#include <forward_list>
#include "base_service.hpp"
#include "events.hpp"

typedef void(EventListener::*EventCallback)(Event &);

class EventService : public BaseService
{
public:
	virtual void onEnable() override;

	virtual void onDisable() override;

	void registerListener(EventListener *listener, EventType eventType);

	void unregisterListener(EventListener *listener, EventType eventType);

	void unregisterListener(EventListener *listener);

	void processQueue();

	void callEvent(const Event &event);

private:
	std::forward_list<Event> pendingEvents;
	std::unordered_map<EventType, std::forward_list<EventListener *>, std::hash<int>> listeners;
};

#endif
