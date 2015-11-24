#include <regex>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "config.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "constants.hpp"


void ConfigurationFile::load()
{
	// doesn't exist
	if (configPath.empty() || !exists(configPath))
		throw Utils::filenotfound_exception(FORMAT("Config file not found: %1%", (configPath.empty() ? "none given" : configPath.string())));

	read_json(configPath.string(), propertyTree);
}

ValueStruct& ConfigurationFile::getValueStruct(const std::string &path, ValueType type)
{
	auto found(configMap.find(path));

	// invalid path
	if (found == configMap.end())
	FAIL_GET(path);

	ValueStruct &value = found->second;

	// invalid type
	if (value.type != type)
	FAIL2("Wrong valuetype given for %1% (%2%)", path, value.type);

	return value;
}

void splitString(const std::string &s, const char *delimiter, std::vector<std::string> &ret)
{
	boost::char_separator<char> sep(delimiter);
	boost::tokenizer<boost::char_separator<char>> tokens(s, sep);
	for (auto &t : tokens)
		ret.push_back(t);
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


int ConfigurationFile::stringToInt(const std::string &s)
{
	static std::regex noCharRegex(".*[a-zA-Z ].*");
	if (!regex_match(s, noCharRegex))
	{
		std::stringstream ss(s);
		int testInt;

		if (ss >> testInt)
			return boost::lexical_cast<int>(s);
	}

	FAIL("Could not convert '%1%' to int", s);
}

bool ConfigurationFile::stringToBool(const std::string &s)
{
	return boost::to_lower_copy(s) == "true";
}

void ConfigurationFile::parseConfig(std::map<std::string, std::string> &config)
{
	static std::regex listRegex("^(.+)-(#{1,2})(\\d+)-?(.+)?");
	static std::smatch match;

	struct ListData
	{
		int index;

		int nonSimpleCurrentCount;
		std::vector<int> nonSimpleCounts;

		std::string path;
		std::list<std::string> list;
		std::string next;

		bool simple;

		ListData()
		{
			reset();
		}

		void reset()
		{
			index = 0;
			list.clear();
			simple = false;
			nonSimpleCurrentCount = 0;
			nonSimpleCounts.clear();
			next.clear();
			path.clear();
		}

		void storeCount(bool plusOne = true)
		{
			nonSimpleCounts.push_back(plusOne ? nonSimpleCurrentCount : nonSimpleCurrentCount - 1);
			nonSimpleCurrentCount = 1;
		}
	};

	struct ListData currentList;
	bool endList(false);

	int configIndex(0);
	const int origSize(config.size());
	for (auto it(config.begin()); it != config.end();)
	{
		auto pair = *it;
		endList = configIndex++ == origSize - 1;

		// lists
		if (std::regex_search(pair.first, match, listRegex))
		{
			it = config.erase(it);

			std::string listPath(match[1]);
			bool simple = match[2] == "#";
			int index(std::stoi(match[3]));

			int currentIndex(currentList.index);

			// index decremented: new list
			if (index < currentIndex)
			{
				endList = true;
				currentList.next = pair.second;

				if (!currentList.simple)
					currentList.storeCount();
			}

			// index incremented: same list
			else if (index >= currentIndex)
			{
				// path=value;...
				if (!simple)
				{
					currentList.list.push_back(match[4].str() + "=" + pair.second + ";");

					++currentList.nonSimpleCurrentCount;

					// next element
					if (index != currentIndex)
					{
						// remember count of non simple lists
						currentList.storeCount(false);
					}
				}
				else
					currentList.list.push_back(pair.second);
			}

			if (currentList.next.empty())
			{
				currentList.index = index;
				currentList.simple = simple;

				if (currentList.path.empty())
					currentList.path = listPath;
			}
		}

		// not a list
		else
		{
			++it;

			// store scalar value
			ValueStruct val;
			val.type = SCALAR;
			val.value = pair.second;
			configMap.insert({pair.first, val});

			// ending current list
			if (!currentList.list.empty())
				endList = true;
		}

		// list ended
		if (endList)
		{
			ValueStruct val;
			std::stringstream output;

			// create flat list
			if (currentList.simple)
			{
				val.type = FLAT_LIST;

				for (auto &x : currentList.list)
					output << x << "|";
			}
			else
			{
				val.type = MAP_LIST;

				std::stringstream current;
				currentList.storeCount();

				// append one more to wrap around the last element
				currentList.list.push_back("");

				// division counter
				auto division(currentList.nonSimpleCounts.begin());

				int i(0);
				for (auto &s : currentList.list)
				{
					// division reached
					if (i == *division)
					{
						if (division != currentList.nonSimpleCounts.end())
							++division;

						i = -1;

						std::string element(current.str());
						output << element.substr(0, element.size() - 1) << "|";
						current.str("");
					}

					// append to current
					else
						current << s;

					++i;
				}
			}

			// store
			std::string full(output.str());
			val.value = full.substr(0, full.size() - 1);
			configMap.insert({currentList.path, val});

			// reset and possibly add first item of next list
			std::string next = currentList.next;
			currentList.reset();

			if (!next.empty())
			{
				currentList.list.push_back(next);
				if (!currentList.simple)
					++currentList.nonSimpleCurrentCount;
			}
		}
	}
}

void ConfigurationFile::loadNode(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config)
{
	switch (node.Type())
	{
	case YAML::NodeType::Map:
		loadMap(node, prefix, config);
		break;
	case YAML::NodeType::Scalar:
		loadScalar(node, prefix, config);
		break;
	case YAML::NodeType::Sequence:
		loadSequence(node, prefix, config);
		break;
	default:
		loadOther(node, prefix, config);
		break;
	}
}

void ConfigurationFile::loadScalar(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config)
{
	static std::regex reservedChars(".*[|;].*");

	auto realPrefix(prefix.substr(0, prefix.size() - 1));
	auto value(node.as<std::string>());

	// reserved symbols
	if (std::regex_match(value, reservedChars) || std::regex_match(realPrefix, reservedChars))
	FAIL2("Reserved characters used in path %1%: %2%", realPrefix, value);

	config.insert({realPrefix, value});
}

void ConfigurationFile::loadMap(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config)
{
	for (auto &pair : node)
		loadNode(pair.second, prefix + pair.first.as<std::string>() + "-", config);
}

void ConfigurationFile::loadSequence(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config)
{
	int i(0);
	std::string listChar = node.begin()->IsScalar() ? "#" : "##";

	for (auto &value : node)
		loadNode(value, prefix + listChar + std::to_string(i++) + "-", config);
}

void ConfigurationFile::loadOther(const YAML::Node &node, const std::string &prefix, std::map<std::string, std::string> &config)
{
	FAIL("Unhandled config node: %1%", prefix);
}

void Config::loadConfig()
{
	getInstance().config.configPath = boost::filesystem::absolute("res/config.json");
	getInstance().ensureConfigExists();
	getInstance().config.load();
}


void Config::ensureConfigExists()
{
	// create default config
	if (!exists(getInstance().config.configPath))
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
	out << Key << "limit-fps" << Value << false;
	out << EndMap;

	// end debug
	out << EndMap;

	// end config
	out << EndMap;


	// write to config file
	std::ofstream configStream;
	configStream.open(config.configPath.string());
	configStream << out.c_str();
	configStream.close();
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
