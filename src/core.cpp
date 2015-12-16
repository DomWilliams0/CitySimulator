#include <boost/filesystem.hpp>
#include <iostream>
#include "logger.hpp"
#include "config.hpp"
#include "game.hpp"

bool ensureCWD(int argc, char **argv)
{
	using namespace boost::filesystem;
	const std::string required("res");

	if (!exists(current_path() / required))
	{
		// no args given
		if (argc != 2)
		{
			std::cerr << "Root directory not found. \nUsage: " << argv[0] << " <relative path to root dir>" << std::endl;
			return false;
		}

		// try supplied relative path
		std::string relativePath = argv[1];
		path newPath = current_path() / relativePath;

		// doesn't exist
		if (!exists(newPath))
		{
			std::cerr << "Invalid path" << std::endl;
			return false;
		}

		// update path and try again
		current_path(newPath);
		return ensureCWD(-1, nullptr);
	}

	return true;
}


void loadConfig(int &windowStyle)
{
	Config::loadConfig();

	int width, height;

	// borderless fullscreen
	if (Config::getBool("display.borderless-fullscreen"))
	{
		windowStyle = sf::Style::None;

		auto screenSize(sf::VideoMode::getDesktopMode());
		width = screenSize.width;
		height = screenSize.height;
	}

	// standard window
	else
	{
		windowStyle = sf::Style::Default;
		width = Config::getInt("display.resolution.width");
		height = Config::getInt("display.resolution.height");
	}

	Constants::setWindowSize(width, height);
}


int main(int argc, char **argv)
{
	try
	{
		// ensure that the program root is in the project root
		if (!ensureCWD(argc, argv))
			return -1;

		// create logger
		createLogger(std::cout, Logger::DEBUG);

		// load window size/style
		int style;
		loadConfig(style);

		sf::RenderWindow window(sf::VideoMode(Constants::windowSize.x, Constants::windowSize.y), "Game", style);

		// create game
		Game game(window);
		game.beginGame();
		game.endGame();

		Logger::logDebug("Shutdown cleanly");
	}
	catch (std::exception &e)
	{
		Logger::logError(std::string("An error occurred: ") + e.what());
	}
	catch (...)
	{
		Logger::logError("An unknown error occured");
	}
}
