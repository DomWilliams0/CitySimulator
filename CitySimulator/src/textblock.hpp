#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "game.hpp"
#include "utils.hpp"
#include "constants.hpp"

class TextBlock
{
public:
	TextBlock()
	{
	}

	TextBlock(Game *game, const std::string &message, int startY)
	{
		setText(game, message, startY);
	}

	void setText(Game *game, const std::string &message, int startY, int charSize = 50)
	{
		std::stringstream stream(message);
		std::string s;
		float lastHeight(0);
		while (getline(stream, s))
		{
			if (!s.empty())
			{
				sf::Text t(s, Constants::mainFont, charSize);
				Utils::centre(t, game, startY);
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
};
