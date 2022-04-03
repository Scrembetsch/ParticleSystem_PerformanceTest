#include "../../common/device/file_handler.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "../../common/logger.h"

AAssetManager* sAssetManager;
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
    sAssetManager = static_cast<AAssetManager*>(dataManager);
}

bool FileHandler::ReadFile(const std::string& path, char** data, size_t& size)
{
    AAsset* file = AAssetManager_open(sAssetManager, path.c_str(), AASSET_MODE_BUFFER);

    if(file == nullptr)
    {
        LOGE("FILE_HANDLER", "Error reading file: %s", path.c_str());
        return false;
    }

    // Get the file length
    size = AAsset_getLength(file);

    delete[] *data;
    // Allocate memory to read your file
    *data = new char[size + 1];

    // Read your file
    AAsset_read(file, *data, size);
    // For safety you can add a 0 terminating character at the end of your file ...
    (*data)[size] = '\0';
    return true;
}

bool FileHandler::ReadFile(const std::string& path, std::string& data)
{
    char* fileData = nullptr;
    size_t size = 0;
    if(!ReadFile(path, &fileData, size))
    {
        return false;
    }

    data = reinterpret_cast<char*>(fileData);
    delete[] fileData;

    return true;
}