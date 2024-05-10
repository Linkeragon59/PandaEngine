#pragma once

#include <unordered_map>

namespace Core
{
	class CommandLine
	{
	public:
		void Parse(int argc, char* argv[]);
		bool IsSet(const char* aName) const;
		void Set(const char* aName, const char* aValue = nullptr);
		std::string GetValue(const char* aName) const;
		int GetValueAsInt(const char* aName) const;
		
	private:
		bool ParseInternal(int argc, char* argv[]);
		std::unordered_map<std::string, std::string> myArgs;
	};
}
