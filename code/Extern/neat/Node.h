#pragma once

#include "Edge.h"

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
		Input, // Sensor
		Hidden,
		Output,
	};

	Node(Type aType)
		: myType(aType)
	{}

	Type GetType() const { return myType; }
	const std::set<std::uint64_t>& GetInputEdges() const { return myInputEdges; }
	void AddInputEdge(std::uint64_t anEdgeId) { myInputEdges.insert(anEdgeId); }
	void SetInputValue(double aValue) { myInputValue = aValue; }
	double GetOutputValue() const { return myOutputValue; }

	void Evaluate(const std::vector<Node>& someNodes, const std::map<std::uint64_t, Edge>& someEdges);

private:
	Type myType = Type::Input;
	std::set<std::uint64_t> myInputEdges;
	double myInputValue = 0.0;
	double myOutputValue = 0.0;
};

}
