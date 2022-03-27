#pragma once
#include <string>
#include <memory>

class FileHandler
{
public:
	virtual ~FileHandler() = default;

	static std::unique_ptr<FileHandler>& GetReference();

	virtual bool ReadFile(const std::string& path, std::string& data) = 0;

protected:
	FileHandler() = default;
	static std::unique_ptr<FileHandler> sReference;
};
