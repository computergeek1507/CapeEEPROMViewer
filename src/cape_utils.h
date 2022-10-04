#ifndef CAPE_UTILS_H
#define CAPE_UTILS_H

#include <QString>
#include "cape_info.h"

namespace cape_utils
{
	QString CleanQuotes(QString item);
	std::string exec(const std::string& cmd, const std::string& dir);
	std::string trim(std::string str);
    std::string read_string(FILE* file, int len);
	void put_file_contents(const std::string& path, const uint8_t* data, int len);
	cape_info parseEEPROM(std::string const& EEPROM);
};

#endif // CAPE_UTILS_H