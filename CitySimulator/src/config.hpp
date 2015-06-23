#pragma once
#include <boost/variant.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include "utils.hpp"
#include "constants.hpp"

#define FAIL_GET(key) FAIL("Invalid config key: %1%", key)


class ConfigurationFile
{
public:
	virtual ~ConfigurationFile()
	{
	}

	void load(const std::string &fileName,
		const std::map<std::string, std::string> &rootNodes = {},
		const std::initializer_list<std::string> &toRemove = {}, bool loadAll = false);


	template <class T>
	T getValue(const std::string &key)
	{
		ConfigValue cv = getConfigValue(key);

		try
		{
			return boost::get<T>(cv);
		}
		catch (boost::bad_get&)
		{
			FAIL_GET(key);
		}
	}


protected:
	boost::filesystem::path configPath;
	ConfigMap configMap;

	// path: variable name
	std::vector<std::pair<std::string, std::string>> variablesToProcess;

	void loadNode(const YAML::Node &node, const std::string &prefix);

	ConfigValue parseString(const std::string &s, const std::string &path);
	virtual void loadScalar(const YAML::Node &node, const std::string &prefix);
	virtual void loadMap(const YAML::Node &node, const std::string &prefix);
	virtual void loadSequence(const YAML::Node &node, const std::string &prefix);
	virtual void loadOther(const YAML::Node &node, const std::string &prefix);

	virtual void substituteVariables();

	ConfigValue getConfigValue(const std::string &key)
	{
		auto value(configMap.find(key));
		boost::optional<ConfigValue> ret;

		if (value != configMap.end())
			ret = value->second;

		if (ret)
			return ret.get();

		FAIL_GET(key);
	}

	inline bool ConfigurationFile::exists()
	{
		return boost::filesystem::exists(configPath);
	}

};

class Config : public ConfigurationFile
{
public:

	static void loadConfig();

	template <class T>
	static T get(const std::string &key)
	{
		return getInstance().getValue<T>(key);
	}

	static Config& getInstance()
	{
		static Config instance;
		return instance;
	}


private:

	void ensureConfigExists();
	void createDefaultConfig();
};

class EntityConfig : public ConfigurationFile
{
public:
	EntityTags &getTags()
	{
		return tags;
	}

protected:
	void loadSequence(const YAML::Node& node, const std::string& prefix) override;
	void substituteVariables() override;

private:
	// name: list of <key, value>
	EntityTags tags;
};

#undef FAIL_GET