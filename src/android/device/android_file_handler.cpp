#include "android_file_handler.h"
#include "../../common/logger.h"

AAssetManager* AndroidFileHandler::sAssetManager;

void AndroidFileHandler::CreateReference(AAssetManager* assetManager)
{
    if (sReference == nullptr)
    {
        sReference = std::make_unique<AndroidFileHandler>();
        sAssetManager = assetManager;
    }
}

bool AndroidFileHandler::ReadFile(const std::string& path, std::string& data)
{
    AAsset* file = AAssetManager_open(sAssetManager, path.c_str(), AASSET_MODE_BUFFER);

    if(file == nullptr)
    {
        LOGE("FILE_HANDLER", "Error reading file: %s", path.c_str());
        return false;
    }

    // Get the file length
    size_t fileLength = AAsset_getLength(file);

    // Allocate memory to read your file
    char* fileContent = new char[fileLength + 1];

    // Read your file
    AAsset_read(file, fileContent, fileLength);
    // For safety you can add a 0 terminating character at the end of your file ...
    fileContent[fileLength] = '\0';

    data = fileContent;
    delete[] fileContent;

    return true;
}