#include <regex>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include "config.hpp"
#include "utils.hpp"
#include "service/logging_service.hpp"

bool ConfigurationFile::load()
{
	// doesn't exist
	if (appConfigPath.empty() || !exists(appConfigPath))
		throw Utils::filenotfound_exception(format("Config file not found: '%1%'", appConfigPath.string()));

	lastModification = boost::filesystem::last_write_time(appConfigPath);

	try
	{
		read_json(appConfigPath.string(), propertyTree);
		return true;

	} catch (boost::exception &e)
	{
		Logger::logWarning(format("Could not load app config '%1%': %2%", appConfigPath.string(),
		                          boost::current_exception_diagnostic_information()));
		return false;
	}

}

void ConfigurationFile::loadOnTop()
{
	if (!boost::filesystem::exists(userConfigPath))
	{
		Logger::logDebug(format("User config not found (%1%)", userConfigPath.string()));
		return;
	}
	Logger::logDebug(format("Found user config at %1%", getUserConfigPath()));

	// load user config
	ConfigurationFile loaded(userConfigPath.string());

	if (loaded.load())
	{
		// put all values
		Logger::pushIndent();
		recurseAndOverwrite(loaded.propertyTree, "");
		Logger::popIndent();
	}

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

int ConfigurationFile::getInt(const std::string &path)
{
	return get<int>(path);
}

int ConfigurationFile::getInt(const std::string &path, int defaultValue)
{
	return get(path, defaultValue);
}

float ConfigurationFile::getFloat(const std::string &path)
{
	return get<float>(path);
}
float ConfigurationFile::getFloat(const std::string &path, float defaultValue)
{
	return get(path, defaultValue);
}

bool ConfigurationFile::getBool(const std::string &path)
{
	return get<bool>(path);
}
bool ConfigurationFile::getBool(const std::string &path, bool defaultValue)
{
	return get(path, defaultValue);
}

std::string ConfigurationFile::getString(const std::string &path, const std::string &defaultValue)
{
	return get(path, defaultValue);
}
std::string ConfigurationFile::getString(const std::string &path)
{
	return get<std::string>(path);
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
		: config(Utils::joinPaths(directory, appConfigPath), Utils::joinPaths(directory, userConfigPath)),
		  rootDirectory(directory)
{
	if (appConfigPath.empty())
		config.setAppConfigPath("");
	if (userConfigPath.empty())
		config.setUserConfigPath("");
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

int ConfigService::getInt(const std::string &path, int defaultValue)
{
	return config.getInt(path, defaultValue);
}

float ConfigService::getFloat(const std::string &path)
{
	return config.getFloat(path);
}
float ConfigService::getFloat(const std::string &path, float defaultValue)
{
	return config.getFloat(path, defaultValue);
}

bool ConfigService::getBool(const std::string &path)
{
	return config.getBool(path);
}
bool ConfigService::getBool(const std::string &path, bool defaultValue)
{
	return config.getBool(path, defaultValue);
}

std::string ConfigService::getString(const std::string &path)
{
	return config.getString(path);
}
std::string ConfigService::getString(const std::string &path, const std::string &defaultValue)
{
	return config.getString(path, defaultValue);
}

std::string ConfigService::getResource(const std::string &path)
{
	std::string value(getString("resources." + path));

	auto firstSep = path.find_first_of('.');

	// no section
	if (firstSep == std::string::npos)
		return (rootDirectory / value).string();

	// find section root
	std::string rootSection(path.substr(0, firstSep));
	std::string root = getString("resources." + rootSection + ".root", "");

	// not looking for root
	if (boost::algorithm::ends_with(path, ".root"))
		value = "";

	return (rootDirectory / root / value).string();
}

int Config::getInt(const std::string &path)
{
	return Locator::locate<ConfigService>()->getInt(path);
}
int Config::getInt(const std::string &path, int defaultValue)
{
	return Locator::locate<ConfigService>()->getInt(path, defaultValue);
}
float Config::getFloat(const std::string &path)
{
	return Locator::locate<ConfigService>()->getFloat(path);
}
float Config::getFloat(const std::string &path, float defaultValue)
{
	return Locator::locate<ConfigService>()->getFloat(path, defaultValue);
}
bool Config::getBool(const std::string &path)
{
	return Locator::locate<ConfigService>()->getBool(path);
}
bool Config::getBool(const std::string &path, bool defaultValue)
{
	return Locator::locate<ConfigService>()->getBool(path, defaultValue);
}
std::string Config::getString(const std::string &path)
{
	return Locator::locate<ConfigService>()->getString(path);
}
std::string Config::getString(const std::string &path, const std::string &defaultValue)
{
	return Locator::locate<ConfigService>()->getString(path, defaultValue);
}
std::string Config::getResource(const std::string &path)
{
	return Locator::locate<ConfigService>()->getResource(path);
}
