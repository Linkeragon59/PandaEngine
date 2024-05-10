#pragma once

namespace FileHelpers
{
	bool ReadAsBuffer(const std::string& aFilePath, std::vector<char>& anOutBuffer);
	bool ReadAsString(const std::string& aFilePath, std::string& anOutString);
}
