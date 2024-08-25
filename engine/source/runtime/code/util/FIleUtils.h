#pragma once

#include <fstream>
#include <vector>
#include "runtime/code/base/macro.h"

namespace DynastyEngine
{
    class FileUtils 
    {
    public:
        static bool exists(const std::string &path) 
        {
            std::ifstream file(path);
            return file.good();
        }

        static std::vector<char> readFile(const std::string& filename) 
        {
            LOG_INFO("filename: {}", filename);

            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            if (!file.is_open()) 
            {
                throw std::runtime_error("failed to open file!");
            }

            size_t fileSize = (size_t) file.tellg();
            std::vector<char> buffer(fileSize);

            file.seekg(0);
            file.read(buffer.data(), fileSize);

            file.close();
            return buffer;
        }

        static std::vector<uint8_t> readBytes(const std::string &path) 
        {
            std::vector<uint8_t> ret;
            std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
            if (!file.is_open()) 
            {
                std::cout << "failed to open file: %s" << path.c_str() << std::endl;
                return ret;
            }

            size_t size = file.tellg();
            if (size <= 0) 
            {
                std::cout << "failed to read file, invalid size: %d" << size << std::endl;
                return ret;
            }

            ret.resize(size);

            file.seekg(0, std::ios::beg);
            file.read(reinterpret_cast<char *>(ret.data()), (std::streamsize) size);

            return ret;
        }

        static std::string readText(const std::string &path) 
        {
            auto data = readBytes(path);
            if (data.empty()) 
            {
                return "";
            }

            return {(char *) data.data(), data.size()};
        }

        static bool writeBytes(const std::string &path, const char *data, size_t length) 
        {
            std::ofstream file(path, std::ios::out | std::ios::binary);
            if (!file.is_open()) 
            {
                // LOGE("failed to open file: %s", path.c_str());
                return false;
            }

            file.write(data, length);
            return true;
        }

        static bool writeText(const std::string &path, const std::string &str) 
        {
            std::ofstream file(path, std::ios::out);
            if (!file.is_open()) 
            {
                // LOGE("failed to open file: %s", path.c_str());
                return false;
            }

            file.write(str.c_str(), str.length());
            file.close();

            return true;
        }
    };
}