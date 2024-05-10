#include "Core_File.h"

#include <fstream>
#include <iostream>

namespace FileHelpers
{
	bool ReadAsBuffer(const std::string& aFilePath, std::vector<char>& anOutBuffer)
	{
		std::ifstream file(aFilePath, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			std::cout << "Failed to read the file %s" << aFilePath.c_str() << std::endl;
			return false;
		}
		std::streamsize fileSize = file.tellg();
		anOutBuffer.resize(fileSize);
		file.seekg(0);
		file.read(anOutBuffer.data(), fileSize);
		file.close();
		return true;
	}

	bool ReadAsString(const std::string& aFilePath, std::string& anOutString)
	{
		std::ifstream file(aFilePath, std::ios::ate);
		if (!file.is_open())
		{
			std::cout << "Failed to read the file %s" << aFilePath.c_str() << std::endl;
			return false;
		}
		std::streamsize fileSize = file.tellg();
		anOutString.resize(fileSize);
		file.seekg(0);
		file.read(anOutString.data(), fileSize);
		file.close();
		return true;
	}
}
