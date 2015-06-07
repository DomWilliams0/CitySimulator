#include <boost/filesystem.hpp>

#include "utils.hpp"
#include <iostream>

using namespace boost;

std::string Utils::searchForFile(const std::string &filename, const std::string &directory)
{
	filesystem::recursive_directory_iterator itr(filesystem::absolute(directory));
	filesystem::recursive_directory_iterator end;

	for (; itr != end; ++itr)
	{
		auto path = itr->path();
		if (path.filename() == filename)
			return path.string();
	}

	throw std::runtime_error(std::string("File not found: ") + filename);
}
