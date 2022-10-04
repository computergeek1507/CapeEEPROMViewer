#ifndef CAPE_INFO_H
#define CAPE_INFO_H

#include <string>

struct cape_info
{
	std::string name;
	std::string version;
	std::string serialNumber;
	std::string folder;

	std::string AsString() const
	{
		return name + " v" + version;
	}
};

#endif // CAPE_INFO_H