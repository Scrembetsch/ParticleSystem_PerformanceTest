#pragma once
#include <string>
#include <memory>

class FileHandler
{
public:
	static void ManagerInit(void* dataManager);
	static std::unique_ptr<FileHandler>& Instance();
	void SetBasePath(const std::string& basePath);
	bool ReadFile(const std::string& path, std::string& data);
	bool ReadFile(const std::string& path, char** data, size_t& size);

protected:
	static std::unique_ptr<FileHandler> sReference;
	std::string mBasePath;
};
