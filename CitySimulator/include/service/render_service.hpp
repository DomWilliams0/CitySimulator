#ifndef CITYSIM_RENDER_SERVICE_HPP
#define CITYSIM_RENDER_SERVICE_HPP

#include <SFML/Graphics.hpp>
#include "base_service.hpp"

class RenderService : public BaseService
{
public:
	RenderService(sf::RenderWindow *renderWindow);

	virtual void onEnable() override;

	void render(const World &world);

	sf::RenderWindow *getWindow();

	sf::Vector2f mapScreenToWorld(const sf::Vector2i &screenPos);

	inline void setView(sf::View &view)
	{
		this->view = &view;
	}

private:
	sf::RenderWindow *window;
	sf::View *view;

	void limitView(const World &world);
};

#endif
