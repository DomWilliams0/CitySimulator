#include "game.hpp"

void FPSCounter::init(float waitTime)
{
	ticker.init(waitTime);

	fpsText.setFont(Constants::mainFont);
	fpsText.setCharacterSize(20);
	fpsText.setPosition(20, 20);
	fpsText.setColor(sf::Color::Red);
}


void FPSCounter::tick(float delta, sf::RenderWindow &window)
{
	backlog.push_back(delta);

	if (ticker.tick(delta))
	{
		int fps(0);

		double total = accumulate(backlog.begin(), backlog.end(), 0.0);
		if (total != 0)
		{
			double average = total / backlog.size();
			if (average != 0)
				fps = static_cast<int>(1.0 / average);
		}

		fpsText.setString(std::to_string(fps) + " FPS");

		backlog.clear();
	}

	window.draw(fpsText);
}

