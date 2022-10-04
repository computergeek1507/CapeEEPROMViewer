#include "cape_utils.h"
#include <iostream>
#include <filesystem>
#include <array>
#include <QProcess>
namespace cape_utils
{
    std::string exec(const std::string& cmd, const std::string& dir) {
        QProcess gzip;
        gzip.setWorkingDirectory(dir.c_str());
        gzip.start(cmd.c_str());
        if (!gzip.waitForStarted())
            return "";

        gzip.closeWriteChannel();

        if (!gzip.waitForFinished())
            return "";

        QByteArray result = gzip.readAll();
        return QString(result).toStdString();
    }

    std::string trim(std::string str) {
        // remove trailing white space
        while (!str.empty() && (std::isspace(str.back()) || str.back() == 0))
            str.pop_back();

        // return residue after leading white space
        std::size_t pos = 0;
        while (pos < str.size() && std::isspace(str[pos]))
            ++pos;
        return str.substr(pos);
    }
    std::string read_string(FILE* file, int len) {
        std::string buf;
        buf.reserve(len + 1);
        buf.resize(len);
        fread(&buf[0], 1, len, file);
        return trim(buf);
    }

    void put_file_contents(const std::string& path, const uint8_t* data, int len) {
        FILE* f = fopen(path.c_str(), "w+b");
        fwrite(data, 1, len, f);
        fclose(f);
    }

    cape_info parseEEPROM(std::string const& EEPROM) {
        cape_info info;
        try {
            std::filesystem::path eeprompath(EEPROM);
            std::string eepromdir = eeprompath.parent_path().string();
            eepromdir += "/";
            eepromdir += eeprompath.stem().string();
            eepromdir += "/";
            std::filesystem::remove_all(eepromdir);
            std::filesystem::create_directories(eepromdir);
            uint8_t* buffer = new uint8_t[32768]; //32K is the largest eeprom we support, more than enough
            FILE* file = fopen(EEPROM.c_str(), "rb");
            if (file == nullptr) {
                return info;
            }
            int l = fread(buffer, 1, 6, file);

            if (buffer[0] == 'F' && buffer[1] == 'P' && buffer[2] == 'P' && buffer[3] == '0' && buffer[4] == '2') {
                info.name = read_string(file, 26);   // cape name + nulls
                info.version = read_string(file, 10);  // cape version + nulls
                info.serialNumber = read_string(file, 16); // cape serial# + nulls

                std::string flenStr = read_string(file, 6); //length of the section
                int flen = std::stoi(flenStr);
                while (flen) {
                    int flag = std::stoi(read_string(file, 2));
                    std::string path{ eepromdir };
                    if (flag < 50) {
                        path += read_string(file, 64);
                    }
                    switch (flag) {
                    case 0:
                    case 1:
                    case 2:
                    case 3: {
                        int l = fread(buffer, 1, flen, file);

                        char* s1 = strdup(path.c_str());
                        std::filesystem::path p(s1);
                        std::string dir = p.parent_path().filename().string();
                        std::filesystem::create_directories(eepromdir + dir);
                        info.folder = eepromdir + dir;
                        put_file_contents(path, buffer, flen);
                        free(s1);
                        if (flag == 1) {
                            std::string cmd = "7z x " + path;
                            exec(cmd, eepromdir + dir);
                        }
                        else if (flag == 2) {
                            std::string cmd = "tar -xzvf " + path;
                            exec(cmd, eepromdir + dir);
                        }
                        else if (flag == 3) {
                            std::string cmd = "tar -xjvf " + path;
                            exec(cmd, eepromdir + dir);
                        }
                        break;
                    }
                    case 96: {
                        info.serialNumber = read_string(file, 16);
                        read_string(file, 42);
                        break;
                    }
                    case 97: {
                        read_string(file, 12);
                        read_string(file, flen - 12);
                        break;
                    }
                    case 98: {
                        read_string(file, 2);
                        break;
                    }
                    case 99: {
                        read_string(file, 6);
                        fread(buffer, 1, flen - 6, file);
                        break;
                    }
                    default:
                        fseek(file, flen, SEEK_CUR);
                    }
                    flenStr = read_string(file, 6); //length of the section
                    flen = std::stoi(flenStr);
                }
            }
            fclose(file);
            delete[] buffer;
        }
        catch (...)
        {
        }
        return info;
    }
}