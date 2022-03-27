#include "windows_file_handler.h"

#include "../../common/logger.h"

#include <fstream>
#include <iostream>
#include <sstream>

void WindowsFileHandler::CreateReference()
{
	if (sReference == nullptr)
	{
		sReference = std::make_unique<WindowsFileHandler>();
	}
}

bool WindowsFileHandler::ReadFile(const std::string& path, std::string& data)
{
    std::ifstream file;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        if (!path.empty())
        {
            file.open(path);
            std::stringstream stream;
            stream << file.rdbuf();
            file.close();
            data = stream.str();
            return true;
        }
    }
    catch (const std::ifstream::failure& e)
    {
        LOGE("WIN_FILE", "Error reading file: %s\nError:", path.c_str(), e.what());
    }
    return false;
}