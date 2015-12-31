#ifndef CITYSIM_CONFIG_HPP
#define CITYSIM_CONFIG_HPP

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <map>

class ConfigurationFile
{
public:
	ConfigurationFile(const std::string &appConfigPath, const std::string &userConfigPath = "") :
			reloadFromFile(false)
	{
		setAppConfigPath(appConfigPath);
		setUserConfigPath(userConfigPath);
	}

	void load();
	void loadOnTop();
	void recurseAndOverwrite(boost::property_tree::ptree &tree, std::string prefix);

	void getIntRef(const std::string &path, int &i, int defaultValue = 0);
	int getInt(const std::string &path, int defaultValue = 0);
	void getFloatRef(const std::string &path, float &f, float defaultValue = 0.f);
	float getFloat(const std::string &path, float defaultValue = 0.f);
	void getBoolRef(const std::string &path, bool &b, bool defaultValue = false);
	bool getBool(const std::string &path, bool defaultValue = false);
	void getStringRef(const std::string &path, std::string &s, const std::string &defaultValue = "");
	std::string getString(const std::string &path, const std::string &defaultValue = "");

	template<class T>
	void getList(const std::string &path, std::vector<T> &l)
	{
		for (auto &item : propertyTree.get_child(path))
			l.push_back(item.second.get_value<T>());
	}

	template<class T>
	void getMapList(const std::string &path, std::vector<std::map<std::string, T>> &ml)
	{
		for (auto &item : propertyTree.get_child(path))
		{
			std::map<std::string, T> map;
			for (auto &element : item.second)
				map.insert({element.first, element.second.get_value<T>()});

			ml.push_back(map);
		}
	}

	void setReloadFromFile(bool reload);
	void reload();

	void setAppConfigPath(const std::string &path);
	void setUserConfigPath(const std::string &path);

	std::string getAppConfigPath() const;

	std::string getUserConfigPath() const;

private:
	boost::property_tree::ptree propertyTree;

	bool reloadFromFile;
	std::time_t lastModification;

	boost::filesystem::path appConfigPath;
	boost::filesystem::path userConfigPath;

	template<class T>
	T get(const std::string &path, T defaultValue)
	{
		if (reloadFromFile)
			reload();

		return propertyTree.get(path, defaultValue);
	}
};

#endif