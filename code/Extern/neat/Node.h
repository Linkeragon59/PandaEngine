#pragma once

#include "Link.h"

#include <string>
#include <vector>
#include <set>
#include <map>

namespace Neat {

class Node
{
public:
	enum class Type
	{
		Bias,
		Input,
		Hidden,
		Output,
	};

	Node(Type aType)
		: myType(aType)
	{}

	Type GetType() const { return myType; }
	const std::set<std::uint64_t>& GetInputLinks() const { return myInputLinks; }
	void AddInputLink(std::uint64_t anLinkId) { myInputLinks.insert(anLinkId); }
	void SetInputValue(double aValue) { myInputValue = aValue; }
	double GetOutputValue() const { return myOutputValue; }

	void Evaluate(const std::vector<Node>& someNodes, const std::map<std::uint64_t, Link>& someLinks);

private:
	Type myType = Type::Input;
	std::set<std::uint64_t> myInputLinks;
	double myInputValue = 0.0;
	double myOutputValue = 0.0;
};

}
