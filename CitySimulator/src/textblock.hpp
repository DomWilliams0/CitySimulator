#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <sstream>
#include "game.hpp"
#include "utils.hpp"
#include "constants.hpp"

class TextBlock
{
public:
	TextBlock()
	{
	}

	TextBlock(BaseGame *game, const std::string &message, int startY)
	{
		setText(game, message, startY);
	}

	void setText(BaseGame *game, const std::string &message, int startY, int charSize = 50)
	{
		std::stringstream stream(message);
		std::string s;
		float lastHeight(0);
		while (getline(stream, s))
		{
			if (!s.empty())
			{
				sf::Text t(s, Constants::mainFont, charSize);
				centre(t, startY);
				text.push_back(t);
				lastHeight = t.getLocalBounds().height * 1.5;
			}

			if (lastHeight)
				startY += lastHeight;
		}
	}

	void render(sf::RenderWindow &window)
	{
		for (auto &t : text)
			window.draw(t);
	}

private:
	std::vector<sf::Text> text;

	inline void centre(sf::Text &text, int y = -1)
	{
		text.setPosition(Constants::windowSize.x / 2 - text.getLocalBounds().width / 2, y < 0 ? text.getPosition().y : y);
	}
};
