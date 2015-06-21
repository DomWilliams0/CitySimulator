#include <iostream>
#include <yaml-cpp/yaml.h>
#include <boost/filesystem.hpp>
#include "config.hpp"
#include "logger.hpp"

void Config::loadNode(const YAML::Node &node, const std::string &prefix)
{
	// null
	if (!node.IsDefined())
		return;

	// map
	if (node.IsMap())
	{
		for (auto it = node.begin(); it != node.end(); ++it)
		{
			std::string name(it->first.as<std::string>());
			loadNode(it->second, prefix + it->first.as<std::string>() + "-");
		}
	}

	// single value
	else if (node.IsScalar())
	{
		auto key(prefix.substr(0, prefix.size() - 1));
		auto value(node.as<std::string>());

		std::stringstream ss(value);
		int testInt;
		if (ss >> testInt)
			configMap.insert({ key, testInt });
		else if (ss.str() == "true" || ss.str() == "false")
			configMap.insert({ key, ss.str() != "false"});
		else
			configMap.insert({ key, value });
	}

	// invalid
	else
		Logger::logWarning(str(boost::format("Ignored config key: %1%") % prefix));
}

void Config::ensureConfigExists()
{
	// create default config
	if (!boost::filesystem::exists(configPath))
	{
		Logger::logInfo("Config not found, loading default");
		createDefaultConfig();
	}
	else
		Logger::logInfo("Found config");
}

void beginMap(YAML::Emitter &emitter, const std::string &sectionName)
{
	using namespace YAML;

	emitter << BeginMap << Key << sectionName;
	emitter << Value << BeginMap;
}

void Config::createDefaultConfig()
{
	using namespace YAML;
	Emitter out;
	out << BeginMap << "config";

	// display
	beginMap(out, "display");
	out << Key << "borderless-fullscreen" << Value << 0;
	out << Key << "resolution-width" << Value << 1080;
	out << Key << "resolution-height" << Value << 768;
	out << EndMap;

	// end config
	out << EndMap;

	// write to config file
	std::ofstream config;
	config.open(configPath.string());
	config << out.c_str();
	config.close();
}

void Config::loadConfig()
{
	getInstance().ensureConfigExists();

	YAML::Node root = YAML::LoadFile(getInstance().configPath.string());

	getInstance().loadNode(root["config"], "");
	getInstance().loadNode(root["debug"], "debug-");
}

