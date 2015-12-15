#include <regex>
#include <boost/property_tree/json_parser.hpp>
#include "constants.hpp"
#include "config.hpp"
#include "utils.hpp"
#include "logger.hpp"


void ConfigurationFile::load()
{
	// doesn't exist
	if (configPath.empty() || !exists(configPath))
		throw Utils::filenotfound_exception(FORMAT("Config file not found: %1%", (configPath.empty() ? "none given" : configPath.string())));

	read_json(configPath.string(), propertyTree);
}

void ConfigurationFile::getIntRef(const std::string &path, int &i)
{
	i = getInt(path);
}

int ConfigurationFile::getInt(const std::string &path)
{
	return propertyTree.get<int>(path);
}

void ConfigurationFile::getFloatRef(const std::string &path, float &f)
{
	f = getFloat(path);
}

float ConfigurationFile::getFloat(const std::string &path)
{
	return propertyTree.get<float>(path);
}

void ConfigurationFile::getBoolRef(const std::string &path, bool &b)
{
	b = getBool(path);
}

bool ConfigurationFile::getBool(const std::string &path)
{
	return propertyTree.get<bool>(path);
}

void ConfigurationFile::getStringRef(const std::string &path, std::string &s)
{
	s = getString(path);
}

std::string ConfigurationFile::getString(const std::string &path)
{
	return propertyTree.get<std::string>(path);
}


void Config::loadConfig()
{
	getInstance().config.configPath = boost::filesystem::canonical("res/config.json");
	getInstance().ensureConfigExists();
	getInstance().config.load();
}


void Config::ensureConfigExists()
{
	// create default config
	if (!exists(getInstance().config.configPath))
	{
		Logger::logInfo("Config not found!");
		throw Utils::filenotfound_exception("Could not find config (" + getInstance().config.configPath.string() + ")");
	}

	Logger::logInfo("Found config");
}

int Config::getInt(const std::string &path)
{
	return getInstance().config.getInt(path);
}

float Config::getFloat(const std::string &path)
{
	return getInstance().config.getFloat(path);
}

bool Config::getBool(const std::string &path)
{
	return getInstance().config.getBool(path);
}

std::string Config::getString(const std::string &path)
{
	return getInstance().config.getString(path);
}
