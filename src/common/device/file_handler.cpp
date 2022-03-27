#include "file_handler.h"

std::unique_ptr<FileHandler> FileHandler::sReference;

inline std::unique_ptr<FileHandler>& FileHandler::GetReference()
{
	return sReference;
}

