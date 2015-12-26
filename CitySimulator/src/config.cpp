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
		throw Utils::filenotfound_exception(
				format("Config file not found: %1%", (configPath.empty() ? "none given" : configPath.string())));

	read_json(configPath.string(), propertyTree);
}

void ConfigurationFile::loadOnTop(const std::string &path)
{
	// load overriding config
	ConfigurationFile loaded(path);
	loaded.load();

	// put all values
	recurseAndOverwrite(loaded.propertyTree, "");
}

void ConfigurationFile::recurseAndOverwrite(boost::property_tree::ptree &tree, std::string prefix)
{
	static std::string sep = ".";

	auto it = tree.begin();

	// leaf
	if (it == tree.end())
	{
		auto name = prefix.substr(0, prefix.length() - 1);
		auto value = tree.get_value<std::string>();

		// overwrite value
		propertyTree.put(name, value);
		Logger::logDebug(format("Overwriting config value '%1%' with '%2%'", name, value));

		return;
	}

	// recurse
	while (it != tree.end())
	{
		recurseAndOverwrite(it->second, std::string(prefix).append(it->first).append(sep));
		it++;
	}
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
	// load reference
	getInstance().config.configPath = boost::filesystem::canonical(Constants::referenceConfigPath);
	getInstance().ensureConfigExists();
	getInstance().config.load();

	// overriding config
	getInstance().config.loadOnTop(Constants::configPath);
}


void Config::ensureConfigExists()
{
	// create default config
	if (!exists(getInstance().config.configPath))
	{
		Logger::logInfo("Config not found!");
		error("Could not find reference config at %1%", getInstance().config.configPath.string());
	}

	Logger::logInfo("Found reference config");
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

std::string Config::getResource(const std::string &path)
{
	return getString("resources." + path);
}