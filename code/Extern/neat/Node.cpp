#include "Node.h"

#include <cmath>

namespace Neat {

namespace {
	double fsigmoid(double anInput)
	{
		static const double slope = 4.924273;
		static const double constant = 0.0; // 2.4621365;
		return 1.0 / (1.0 + std::exp(-(anInput * slope + constant)));
	}
}

void Node::Evaluate(const std::vector<Node>& someNodes, const std::map<std::uint64_t, Link>& someLinks)
{
	switch (myType)
	{
	case Type::Bias:
	case Type::Input:
		myOutputValue = myInputValue;
		break;
	default:
		myInputValue = 0.0;
		for (std::uint64_t linkId : myInputLinks)
		{
			auto it = someLinks.find(linkId);
			if (it == someLinks.end())
				continue;

			if (it->second.IsEnabled())
				myInputValue += someNodes[it->second.GetSrcNodeIdx()].GetOutputValue() * it->second.GetWeight();
		}
		myOutputValue = fsigmoid(myInputValue);
		break;
	}
}

}
