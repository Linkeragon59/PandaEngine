#include "Core_CommandLine.h"

#include <iostream>

namespace Core
{
	void CommandLine::Parse(int argc, char* argv[])
	{
		myArgs.clear();
		int i = 1; // Skip the first arg as it is the exe path
		while (i < argc) 
		{
			std::string argument = argv[i++];
			if (!argument.starts_with("-"))
				continue;

			while (argument.starts_with("-"))
				argument = argument.substr(1);

			myArgs[argument] = "";

			if (i >= argc)
				break;

			std::string value = argv[i];
			if (value.starts_with("-"))
				continue;

			myArgs[argument] = value;
			i++;
		}
	}

	bool CommandLine::IsSet(const char* aName) const
	{
		return myArgs.find(aName) != myArgs.end();
	}

	void CommandLine::Set(const char* aName, const char* aValue)
	{
		myArgs[aName] = aValue;
	}

	std::string CommandLine::GetValue(const char* aName) const
	{
		auto it = myArgs.find(aName);
		if (it == myArgs.end())
			return "";
		return it->second;
	}

	int CommandLine::GetValueAsInt(const char* aName) const
	{
		return std::stoi(GetValue(aName));
	}
}
