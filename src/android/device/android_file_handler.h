#pragma once
#include "../../common/device/file_handler.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

class AndroidFileHandler : public FileHandler
{
public:
    AndroidFileHandler() = default;
    static void CreateReference(AAssetManager* assetManager);
    bool ReadFile(const std::string& path, std::string& data) override;
protected:
    static AAssetManager* sAssetManager;
};

