#include "../../common/device/file_handler.h"

#include "../../common/logger.h"

#include <fstream>
#include <iostream>
#include <sstream>

std::unique_ptr<FileHandler> FileHandler::sReference;

std::unique_ptr<FileHandler>& FileHandler::Instance()
{
    if (sReference == nullptr)
    {
        sReference = std::make_unique<FileHandler>();
    }
    return sReference;
}

void FileHandler::ManagerInit(void* dataManager)
{
}

void FileHandler::SetBasePath(const std::string& basePath)
{
    mBasePath = basePath;
}

bool FileHandler::ReadFile(const std::string& path, std::string& data)
{
    std::ifstream file;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        if (!path.empty())
        {
            file.open(mBasePath + path);
            std::stringstream stream;
            stream << file.rdbuf();
            file.close();
            data = stream.str();
            return true;
        }
    }
    catch (const std::ifstream::failure& e)
    {
        LOGE("WIN_FILE", "Error reading file: %s\nError:", (mBasePath + path).c_str(), e.what());
    }
    return false;
}