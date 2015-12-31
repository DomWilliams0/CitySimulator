#include <regex>
#include <boost/property_tree/json_parser.hpp>
#include "constants.hpp"
#include "config.hpp"
#include "services.hpp"

void ConfigurationFile::load()
{
	// doesn't exist
	if (appConfigPath.empty() || !exists(appConfigPath))
		throw Utils::filenotfound_exception(format("Config file not found: '%1%'", appConfigPath.string()));

	lastModification = boost::filesystem::last_write_time(appConfigPath);
	read_json(appConfigPath.string(), propertyTree);
}

void ConfigurationFile::loadOnTop()
{
	if (!boost::filesystem::exists(userConfigPath))
	{
		Logger::logDebug(format("User config not found (%1%)", userConfigPath.string()));
		return;
	}

	// load user config
	ConfigurationFile loaded(userConfigPath.string());
	loaded.load();

	Logger::logDebug(format("Found user config at %1%", getUserConfigPath()));

	// put all values
	Logger::pushIndent();
	recurseAndOverwrite(loaded.propertyTree, "");
	Logger::popIndent();

	// update path to overwriting
	userConfigPath = loaded.appConfigPath;
	lastModification = boost::filesystem::last_write_time(userConfigPath);
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
		Logger::logDebuggiest(format("Overwriting config value '%1%' with '%2%'", name, value));

		return;
	}

	// recurse
	while (it != tree.end())
	{
		recurseAndOverwrite(it->second, std::string(prefix).append(it->first).append(sep));
		it++;
	}
}

void ConfigurationFile::getIntRef(const std::string &path, int &i, int defaultValue)
{
	i = getInt(path, defaultValue);
}

int ConfigurationFile::getInt(const std::string &path, int defaultValue)
{
	return get(path, defaultValue);
}

void ConfigurationFile::getFloatRef(const std::string &path, float &f, float defaultValue)
{
	f = getFloat(path, defaultValue);
}

float ConfigurationFile::getFloat(const std::string &path, float defaultValue)
{
	return get(path, defaultValue);
}

void ConfigurationFile::getBoolRef(const std::string &path, bool &b, bool defaultValue)
{
	b = getBool(path, defaultValue);
}

bool ConfigurationFile::getBool(const std::string &path, bool defaultValue)
{
	return get(path, defaultValue);
}

void ConfigurationFile::getStringRef(const std::string &path, std::string &s, const std::string &defaultValue)
{
	s = getString(path, defaultValue);
}

std::string ConfigurationFile::getString(const std::string &path, const std::string &defaultValue)
{
	return get(path, defaultValue);
}

void ConfigurationFile::setReloadFromFile(bool reload)
{
	reloadFromFile = reload;
}

void ConfigurationFile::reload()
{
	using namespace boost::filesystem;

	if (!exists(userConfigPath))
		return;

	std::time_t time = last_write_time(userConfigPath);

	if (time > lastModification)
	{
		lastModification = time;

		Logger::logDebug(format("Reloading overwriting config from '%1%'", userConfigPath.string()));
		Logger::pushIndent();
		loadOnTop();
		Logger::popIndent();
	}
}

void ConfigurationFile::setAppConfigPath(const std::string &path)
{
	appConfigPath = path;
}
void ConfigurationFile::setUserConfigPath(const std::string &path)
{
	userConfigPath = path;
}

std::string ConfigurationFile::getAppConfigPath() const
{
	return appConfigPath.string();
}

std::string ConfigurationFile::getUserConfigPath() const
{
	return userConfigPath.string();
}

ConfigService::ConfigService(const std::string &directory,
                             const std::string &appConfigPath, const std::string &userConfigPath)
		: config((boost::filesystem::path(directory) / appConfigPath).string(),
		         (boost::filesystem::path(directory) / userConfigPath).string()), rootDirectory(directory)
{
}

void ConfigService::onEnable()
{
	// load reference
	ensureConfigExists();
	config.load();

	// load user config on top
	config.loadOnTop();

	// reload?
	config.setReloadFromFile(getBool("debug.reload-config"));
}


void ConfigService::ensureConfigExists()
{
	// create default config
	if (!boost::filesystem::exists(config.getAppConfigPath()))
	{
		Logger::logInfo("Config not found!");
		error("Could not find reference config at %1%", config.getAppConfigPath());
	}

	Logger::logInfo(format("Found reference config at %1%", config.getAppConfigPath()));
}

int ConfigService::getInt(const std::string &path)
{
	return config.getInt(path);
}

float ConfigService::getFloat(const std::string &path)
{
	return config.getFloat(path);
}

bool ConfigService::getBool(const std::string &path)
{
	return config.getBool(path);
}

std::string ConfigService::getString(const std::string &path)
{
	return config.getString(path);
}

std::string ConfigService::getResource(const std::string &path)
{
	return (rootDirectory / getString("resources." + path)).string();
}

int Config::getInt(const std::string &path)
{
	return Locator::locate<ConfigService>()->getInt(path);
}

float Config::getFloat(const std::string &path)
{
	return Locator::locate<ConfigService>()->getFloat(path);
}

bool Config::getBool(const std::string &path)
{
	return Locator::locate<ConfigService>()->getBool(path);
}

std::string Config::getString(const std::string &path)
{
	return Locator::locate<ConfigService>()->getString(path);
}

std::string Config::getResource(const std::string &path)
{
	return Locator::locate<ConfigService>()->getResource(path);
}
