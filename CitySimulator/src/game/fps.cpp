#include "constants.hpp"
#include "game.hpp"

void FPSCounter::init(float waitTime)
{
	ticker.setMinAndMax(waitTime);

	fpsText.setFont(Constants::mainFont);
	fpsText.setCharacterSize(16);
	fpsText.setPosition(20, 20);
	fpsText.setColor(sf::Color::Red);
}


void FPSCounter::tick(float delta, sf::RenderWindow &window)
{
	backlog.push_back(delta * 1000); // ms

	if (ticker.tick(delta))
	{
		float fps(0);

		if (!backlog.empty())
			fps = accumulate(backlog.begin(), backlog.end(), 0.f) / backlog.size();

		const size_t bufferLength = 32;
		char buffer[bufferLength];
		snprintf(buffer, bufferLength, "%.2f mspf\n%.2f fps", fps, fps == 0 ? 0 : 1000.f / fps);
		fpsText.setString(buffer);

		backlog.clear();
	}

	window.draw(fpsText);
}

