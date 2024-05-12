#include "Node.h"

#include <cmath>

namespace Neat {

void Node::Evaluate(const std::vector<Node>& someNodes, const std::map<std::uint64_t, Edge>& someEdges)
{
	switch (myType)
	{
	case Type::Bias:
	case Type::Input:
		myOutputValue = myInputValue;
		break;
	default:
		myInputValue = 0.0;
		for (std::uint64_t edgeId : myInputEdges)
		{
			auto it = someEdges.find(edgeId);
			if (it == someEdges.end())
				continue;

			if (it->second.IsEnabled())
				myInputValue += someNodes[it->second.GetSrcNodeIdx()].GetOutputValue() * it->second.GetWeight();
		}
		myOutputValue = std::tanh(myInputValue);
		break;
	}
}

}
