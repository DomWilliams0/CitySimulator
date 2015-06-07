#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "state.hpp"
#include "game.hpp"
#include "utils.hpp"

#define NO_SELECTION 1000110001

static const sf::Color nonSelectedColor(49, 189, 224);
static const sf::Color selectedColor(Utils::darken(nonSelectedColor, 40));

template <class T>
class ButtonSelection
{
public:
	struct Button
	{
		Button(const std::string &s, sf::Vector2f position, const sf::Vector2u &windowSize, const sf::Font &font) : text(s, font)
		{
			text.setCharacterSize(60); // todo: resize if too big

			// centre
			position.x -= text.getLocalBounds().width / 2;
			position.y -= text.getLocalBounds().height;

			text.setPosition(position);
			setSelected(false);

			rect = sf::Rect<int>(text.getGlobalBounds());
			float inflation(rect.height);
			rect.left -= inflation / 2;
			rect.top -= inflation / 2;
			rect.width += inflation;
			rect.height += inflation;
		}

		void setSelected(bool select)
		{
			text.setColor(select ? selectedColor : nonSelectedColor);
			this->selected = select;
		}

		sf::Text text;
		sf::Rect<int> rect;
		bool selected;
	};

	typedef typename std::vector<Button>::size_type BIndex;

	template <class T>
	ButtonSelection(Game *game_, T *instance_, sf::Vector2f centreOffset, void (T::*clickCallback)(const std::string &), std::vector<std::string> &buttonsText)
		: game(game_), instance(instance_), callback(clickCallback)
	{
		auto windowSize(game_->getWindowSize());
		sf::Vector2f startPos(windowSize.x * 0.5f, windowSize.y * 0.4f);
		startPos += centreOffset;

		for (auto &s : buttonsText)
		{
			Button b(s, sf::Vector2f(startPos), windowSize, game_->getFont());
			buttons.push_back(b);
			startPos.y += b.rect.height * 1.5f;
		}

		select<T>(0);
	}

	template <class T>
	void render(sf::RenderWindow &window)
	{
		for (auto &b : buttons)
			window.draw(b.text);
	}

	template <class T>
	void handleInput(sf::Event event)
	{
		if (event.type == sf::Event::KeyPressed)
		{
			auto key = event.key.code;

			// keyboard scroll
			int scroll(0);
			if (key == sf::Keyboard::Up || key == sf::Keyboard::W)
				scroll = -1;
			else if (key == sf::Keyboard::Down || key == sf::Keyboard::S)
				scroll = 1;

			if (scroll)
			{
				if (selection == NO_SELECTION)
					selection = 0;

				BIndex choice(selection + scroll);
				auto size(buttons.size());
				if (selection == 0 && scroll < 0) choice = size - 1;
				else if (choice >= size) choice = 0;
				select<T>(choice);
			}

			// keyboard select
			if (key == sf::Keyboard::Return)
				if (selection != NO_SELECTION)
					click<T>();
		}

		// mouse hover
		else if (event.type == sf::Event::MouseMoved)
		{
			sf::Vector2i mouse(game->getMousePosition());
			bool selection(false);
			for (BIndex i = 0; i < buttons.size(); i++)
			{
				Button b(buttons[i]);
				bool hover(b.rect.contains(mouse));
				if (hover)
				{
					select<T>(i);
					selection = true;
					break;
				}
			}

			if (!selection)
				select<T>(NO_SELECTION);
		}

		// mouse click
		else if (event.type == sf::Event::MouseButtonPressed)
		{
			if (selection != NO_SELECTION)
				click<T>();
		}
	}

	template <class T>
	void select(BIndex b)
	{
		if (selection != NO_SELECTION)
			buttons[selection].setSelected(false);

		selection = b;
		if (b != NO_SELECTION)
			buttons[b].setSelected(true);
	}

	template <class T>
	void click()
	{
		std::string button(buttons[selection].text.getString().toAnsiString());
		for (auto &c : button) c = tolower(c);

		(instance ->* callback)(button);
	}

private:
	Game *game;
	std::vector<Button> buttons;
	BIndex selection;
	void (T::*callback)(const std::string &);
	T *instance;
};
