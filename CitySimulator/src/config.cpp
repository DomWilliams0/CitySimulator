#include <regex>
#include <boost/property_tree/json_parser.hpp>
#include "constants.hpp"
#include "config.hpp"
#include "services.hpp"

void ConfigurationFile::load()
{
	// doesn't exist
	if (configPath.empty() || !exists(configPath))
		throw Utils::filenotfound_exception(
				format("Config file not found: %1%", (configPath.empty() ? "none given" : configPath.string())));

	lastModification = boost::filesystem::last_write_time(configPath);
	read_json(configPath.string(), propertyTree);
}

void ConfigurationFile::loadOnTop()
{
	if (!boost::filesystem::exists(overwriteConfigPath))
	{
		Logger::logDebug(format("Overriding config not found (%1%)", overwriteConfigPath.string()));
		return;
	}

	// load overriding config
	ConfigurationFile loaded(overwriteConfigPath.string());
	loaded.load();

	Logger::logDebug(format("Found overriding config at %1%", getOverwriteConfigPath()));

	// put all values
	Logger::pushIndent();
	recurseAndOverwrite(loaded.propertyTree, "");
	Logger::popIndent();

	// update path to overwriting
	overwriteConfigPath = loaded.configPath;
	lastModification = boost::filesystem::last_write_time(overwriteConfigPath);
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

	if (!exists(overwriteConfigPath))
	{
		// Logger::logWarning(format("Could not reload config as it doesn't exist (%1%)", overwriteConfigPath.string()));
		return;
	}

	std::time_t time = last_write_time(overwriteConfigPath);

	if (time > lastModification)
	{
		lastModification = time;

		Logger::logDebug(format("Reloading overwriting config from '%1%'", overwriteConfigPath.string()));
		Logger::pushIndent();
		loadOnTop();
		Logger::popIndent();
	}
}

void ConfigurationFile::setConfigPath(const std::string &path)
{
	configPath = boost::filesystem::path(path);
}
void ConfigurationFile::setOverridingConfigPath(const std::string &path)
{
	overwriteConfigPath = boost::filesystem::path(path);
}

std::string ConfigurationFile::getConfigPath() const
{
	return configPath.string();
}

std::string ConfigurationFile::getOverwriteConfigPath() const
{
	return overwriteConfigPath.string();
}

ConfigService::ConfigService(const std::string &path, const std::string &overridingPath) :
		config(path, overridingPath)
{
}

void ConfigService::onEnable()
{
	// load reference
	ensureConfigExists();
	config.load();

	// overriding config
	config.loadOnTop();

	// reload?
	config.setReloadFromFile(getBool("debug.reload-config"));
}


void ConfigService::ensureConfigExists()
{
	// create default config
	if (!boost::filesystem::exists(config.getConfigPath()))
	{
		Logger::logInfo("Config not found!");
		error("Could not find reference config at %1%", config.getConfigPath());
	}

	Logger::logInfo(format("Found reference config at %1%", config.getConfigPath()));
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
	return getString("resources." + path);
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
