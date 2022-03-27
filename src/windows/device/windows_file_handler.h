#pragma once
#include "../../common/device/file_handler.h"

class WindowsFileHandler : public FileHandler
{
public:
	WindowsFileHandler() = default;
	~WindowsFileHandler() override = default;
	static void CreateReference();
	bool ReadFile(const std::string& path, std::string& data) override;
};