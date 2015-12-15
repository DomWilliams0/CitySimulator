/*
* Modification of https://github.com/joelverhagen/2D-Bin-Packing
*/

#pragma once
#include <boost/optional.hpp>
#include <SFML/Graphics.hpp>

struct PackingTreeNode
{
	sf::IntRect binRectangle;
	PackingTreeNode *leftChild;
	PackingTreeNode *rightChild;
	bool filled;

	explicit PackingTreeNode(sf::IntRect binRect) : binRectangle(binRect)
	{
		leftChild = nullptr;
		rightChild = nullptr;
		filled = false;
	}

	~PackingTreeNode()
	{
		delete leftChild;
		delete rightChild;
	}

	boost::optional<sf::IntRect> insert(const sf::IntRect &inputRectangle)
	{
		if (leftChild != nullptr || rightChild != nullptr)
		{
			auto leftRectangle = leftChild->insert(inputRectangle);
			if (!leftRectangle)
				return rightChild->insert(inputRectangle);

			return leftRectangle;
		}
		else
		{
			if (filled || inputRectangle.width > binRectangle.width || inputRectangle.height > binRectangle.height)
				return boost::optional<sf::IntRect>();

			if (inputRectangle.width == binRectangle.width && inputRectangle.height == binRectangle.height)
			{
				filled = true;
				return binRectangle;
			}

			int widthDifference = binRectangle.width - inputRectangle.width;
			int heightDifference = binRectangle.height - inputRectangle.height;

			sf::IntRect leftRectangle(binRectangle);
			sf::IntRect rightRectangle(binRectangle);

			if (widthDifference > heightDifference)
			{
				leftRectangle.width = inputRectangle.width;
				rightRectangle.left += inputRectangle.width;
				rightRectangle.width -= inputRectangle.width;
			}
			else
			{
				leftRectangle.height = inputRectangle.height;
				rightRectangle.top += inputRectangle.height;
				rightRectangle.height -= inputRectangle.height;
			}

			leftChild = new PackingTreeNode(leftRectangle);
			rightChild = new PackingTreeNode(rightRectangle);

			return leftChild->insert(inputRectangle);
		}
	}
};
