#include <iostream>
#include <yaml-cpp/yaml.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <cctype>
#include <regex>
#include "config.hpp"
#include "logger.hpp"
#include "utils.hpp"


void ConfigurationFile::load(const std::string &fileName,
	const std::map<std::string, std::string> &rootNodes,
	const std::initializer_list<std::string> &toRemove, bool loadAll)
{
	if (configPath.empty())
		configPath = boost::filesystem::absolute(Utils::searchForFile(fileName));

	// doesn't exist
	if (!exists())
		throw Utils::filenotfound_exception(str(boost::format("Config file not found: %s") % fileName));

	YAML::Node root = YAML::LoadFile(configPath.string());

	if (rootNodes.empty())
		loadAll = true;

	// load all from root
	if (loadAll)
		loadNode(root, "");

	// load only specified nodes
	else
	{
		for (auto &pair : root)
		{
			std::string key(pair.first.as<std::string>());

			// must be in the map of keys given
			auto found(rootNodes.find(key));
			if (found == rootNodes.end())
				continue;

			std::string prefix = found->second;
			if (!prefix.empty())
				prefix += "-";

			loadNode(pair.second, prefix);
		}

	}

	substituteVariables();

	// remove vars tagged for deletion
	if (toRemove.size() > 0)
		for (auto &tag : toRemove)
			configMap.erase(tag);

}

void ConfigurationFile::loadNode(const YAML::Node &node, const std::string &prefix)
{
	switch (node.Type())
	{
	case YAML::NodeType::Map:
		loadMap(node, prefix);
		break;
	case YAML::NodeType::Scalar:
		loadScalar(node, prefix);
		break;
	case YAML::NodeType::Sequence:
		loadSequence(node, prefix);
		break;
	default:
		loadOther(node, prefix);
		break;
	}
}

ConfigValue ConfigurationFile::parseString(const std::string &s, const std::string &path)
{
	static std::regex noCharRegex(".*[a-zA-Z ].*");
	static std::regex varRegex("^=([a-zA-Z-_ ]+)=$");

	// int
	if (!std::regex_match(s, noCharRegex))
	{
		std::stringstream ss(s);
		int testInt;

		if (ss >> testInt)
			return boost::lexical_cast<int>(s);
	}

	// boolean
	std::string lower(s);
	std::transform(lower.begin(), lower.end(), lower.begin(), std::tolower);

	if (lower == "true" || lower == "false")
		return lower == "true";

	// variable
	std::smatch match;
	if (std::regex_search(s, match, varRegex))
	{
		std::string var = match[1];
		variablesToProcess.push_back({ path, var });
	}

	// string
	return s;
}

void ConfigurationFile::loadScalar(const YAML::Node &node, const std::string &prefix)
{
	auto key(prefix.substr(0, prefix.size() - 1));
	auto value(node.as<std::string>());

	configMap.insert({ key, parseString(value, key) });
}

void ConfigurationFile::loadMap(const YAML::Node &node, const std::string &prefix)
{
	for (auto &childPair : node)
		loadNode(childPair.second, prefix + childPair.first.as<std::string>() + "-");
}

void ConfigurationFile::loadSequence(const YAML::Node &node, const std::string &prefix)
{
	loadOther(node, prefix);
}

void ConfigurationFile::loadOther(const YAML::Node &node, const std::string &prefix)
{
	if (node.IsDefined())
		Logger::logWarning(str(boost::format("Ignored config key: %1%") % prefix));
}

void ConfigurationFile::substituteVariables()
{
	for (auto &var : variablesToProcess)
	{
		ConfigValue sub = getConfigValue(var.second);
		configMap[var.first] = sub;
	}

	variablesToProcess.clear();
}

void Config::loadConfig()
{
	getInstance().configPath = boost::filesystem::absolute("res/config/config.yml");
	getInstance().ensureConfigExists();

	YAML::Node root = YAML::LoadFile(getInstance().configPath.string());

	getInstance().load(getInstance().configPath.filename().string(), { { "debug", "debug-" } }, {}, true);
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

void Config::createDefaultConfig()
{
	using namespace YAML;
	Emitter out;

	// display
	out << BeginMap << Key << "display";
	out << Value << BeginMap;
	out << Key << "borderless-fullscreen" << Value << false;

	// resolution
	out << Key << "resolution";
	out << Value << BeginMap;
	out << Key << "width" << Value << 1080;
	out << Key << "height" << Value << 768;
	out << EndMap;

	// end display
	out << EndMap;

	// debug
	out << Key << "debug";
	out << Value << BeginMap;
	out << Key << "world-name" << Value << "small.tmx";
	out << EndMap;

	// end debug
	out << EndMap;

	// end config
	out << EndMap;


	// write to config file
	std::ofstream config;
	config.open(configPath.string());
	config << out.c_str();
	config.close();
}

void EntityConfig::loadSequence(const YAML::Node &node, const std::string &prefix)
{
	for (auto &entity : node)
	{
		std::string name = entity["name"].as<std::string>();
		std::vector<std::pair<std::string, ConfigValue>> entityTags;

		for (auto &pair : entity)
		{

			std::string key = pair.first.as<std::string>();
			if (key == "name")
				continue;

			ConfigValue value = parseString(pair.second.as<std::string>(), str(boost::format("%1%%2%-%3%") % prefix % name % key));

			entityTags.push_back({ key, value });
		}

		tags.insert({ name, entityTags });
	}
}

void EntityConfig::substituteVariables()
{
	static std::regex dashSplit("-");

	for (auto &var : variablesToProcess)
	{
		// get value
		ConfigValue value(getConfigValue(var.second));

		// split path by -
		std::vector<std::string> parts(std::sregex_token_iterator(var.first.begin(), var.first.end(), dashSplit, -1), std::sregex_token_iterator());

		// 0...: path
		// last-1: name
		// last: tag

		std::string tag(parts[parts.size() - 1]);
		std::string name(parts[parts.size() - 2]);

		std::vector<std::string> pathParts(parts.begin(), parts.end() - 2);
		std::string fullPath(boost::algorithm::join(pathParts, "-"));

		// get entity tags by name
		auto entityTags(tags.find(name));
		if (entityTags == tags.end())
			FAIL("Unknown entity while substituting variable: %1%", var.second);

		// find tag
		bool found(false);
		for (auto &tagPair : entityTags->second)
		{
			if (tagPair.first == tag)
			{
				tagPair.second = value;
				found = true;
				break;
			}
		}

		// not found
		if (!found)
			FAIL("Unknown tag while substituting variable: %1%", fullPath);
	}
}

