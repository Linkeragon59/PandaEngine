#include "Genome.h"

#include "EvolutionParams.h"
#include "Specie.h"

#include <fstream>
#include <sstream>
#include <string>
#include <cassert>

namespace Neat {

namespace
{
	const char* ws = " \t\n\r\f\v";

	// trim from end of string (right)
	inline std::string& rtrim(std::string& s, const char* t = ws)
	{
		s.erase(s.find_last_not_of(t) + 1);
		return s;
	}

	// trim from beginning of string (left)
	inline std::string& ltrim(std::string& s, const char* t = ws)
	{
		s.erase(0, s.find_first_not_of(t));
		return s;
	}

	// trim from both ends of string (right then left)
	inline std::string& trim(std::string& s, const char* t = ws)
	{
		return ltrim(rtrim(s, t), t);
	}
}

Genome::Genome(size_t anInputCount, size_t anOutputCount)
	: myInputCount(anInputCount)
	, myOutputCount(anOutputCount)
{
	std::uniform_real_distribution<> rand(-8.0, 8.0);

	myNodes.reserve(1 + myInputCount + myOutputCount); // +1 for Bias
	myNodes.push_back(Node::Type::Bias);

	for (size_t i = 0; i < myInputCount; ++i)
		myNodes.push_back(Node::Type::Input);

	for (size_t i = 0; i < myOutputCount; ++i)
	{
		myNodes.push_back(Node::Type::Output);

		size_t nodeIdx = 1 + myInputCount + i;
		LinkNodes(0, nodeIdx, rand(EvolutionParams::GetRandomGenerator()), true);
		for (size_t j = 0; j < myInputCount; ++j)
			LinkNodes(1 + j, nodeIdx, rand(EvolutionParams::GetRandomGenerator()), true);
	}
}

Genome::Genome(const char* aFilePath)
{
	std::ifstream file(aFilePath);
	if (file.is_open())
	{
		std::vector<std::string> lines;
		std::string line;
		while (std::getline(file, line, ';'))
		{
			trim(line);
			if (!line.empty())
				lines.push_back(line);
		}
		file.close();

		if (lines.size() < 2)
			return;

		// The two first tokens are the input and output nodes counts
		myInputCount = std::stoi(lines[0]);
		size_t hiddenCount = std::stoi(lines[1]);
		myOutputCount = std::stoi(lines[2]);

		myNodes.reserve(1 + myInputCount + hiddenCount + myOutputCount); // +1 for Bias
		myNodes.push_back(Node::Type::Bias);

		for (size_t i = 0; i < myInputCount; ++i)
			myNodes.push_back(Node::Type::Input);

		for (size_t i = 0; i < hiddenCount; ++i)
			myNodes.push_back(Node::Type::Hidden);

		for (size_t i = 0; i < myOutputCount; ++i)
			myNodes.push_back(Node::Type::Output);

		for (size_t i = 3; i < lines.size(); ++i)
		{
			std::vector<std::string> tokens;
			std::string token;
			std::istringstream tokensStream(lines[i]);
			while (std::getline(tokensStream, token, ' '))
			{
				trim(token);
				if (!token.empty())
					tokens.push_back(token);
			}

			if (tokens.size() != 5)
				continue;

			std::uint64_t innovationId = std::stoull(tokens[0]);
			LinkNodesWithInnovationId(std::stoull(tokens[1]), std::stoull(tokens[2]), std::stod(tokens[3]), std::stoi(tokens[4]), innovationId);
			EvolutionParams::SetNextInnovationNumber(innovationId + 1);
		}
	}
}

void Genome::SaveToFile(const char* aFilePath) const
{
	std::ofstream file(aFilePath);
	if (!file.is_open())
		return;

	file << myInputCount << ";" << GetHiddenNodesCount() << ";" << myOutputCount << ";" << std::endl;

	for (auto it = myLinks.begin(); it != myLinks.end(); ++it)
	{
		const Link& link = it->second;
		file << it->first << " " << link.GetSrcNodeIdx() << " " << link.GetDstNodeIdx() << " "
			<< link.GetWeight() << " " << link.IsEnabled() << ";" << std::endl;
	}
}

Genome::Genome(const Genome* aParent1, const Genome* aParent2)
{
	const Genome* primaryParent = aParent1->myFitness >= aParent2->myFitness ? aParent1 : aParent2;
	const Genome* secondaryParent = aParent1->myFitness >= aParent2->myFitness ? aParent2 : aParent1;
	
	myInputCount = primaryParent->myInputCount;
	myOutputCount = primaryParent->myOutputCount;
	
	for (const Node& node : primaryParent->myNodes)
		myNodes.push_back(Node(node.GetType()));

	for (auto it = primaryParent->myLinks.begin(); it != primaryParent->myLinks.end(); ++it)
	{
		std::uint64_t innovationId = it->first;
		const Link& primaryParentLink = it->second;
		
		auto it2 = secondaryParent->myLinks.find(innovationId);
		if (it2 != secondaryParent->myLinks.end())
		{
			// Common gene, choose weight randomly
			std::uniform_int_distribution<> rand(0, 1);
			double weight = (rand(EvolutionParams::GetRandomGenerator()) == 0) ? primaryParentLink.GetWeight() : it2->second.GetWeight();

			LinkNodesWithInnovationId(primaryParentLink.GetSrcNodeIdx(), primaryParentLink.GetDstNodeIdx(), weight, primaryParentLink.IsEnabled(), innovationId);
		}
		else
		{
			// Disjoint or Excess gene, ignore secondary parent
			LinkNodesWithInnovationId(primaryParentLink.GetSrcNodeIdx(), primaryParentLink.GetDstNodeIdx(), primaryParentLink.GetWeight(), primaryParentLink.IsEnabled(), innovationId);
		}
	}
}

void Genome::Mutate()
{
	std::uniform_real_distribution<> rand(0.0, 1.0);

	if (rand(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourLinkWeightMutationProba)
		MutateLinkWeights();

	if (rand(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourNewLinkProba)
		MutateAddLink();

	if (rand(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourNewNodeProba)
		MutateAddNode();

	//VerifyLinks();
}

bool Genome::Evaluate(const std::vector<double>& someInputs, std::vector<double>& someOutputs)
{
	if (someInputs.size() != myInputCount)
		return false;

	someOutputs.resize(myOutputCount);
	if (myOutputCount == 0)
		return true;

	myNodes[0].SetInputValue(1.0);
	for (size_t i = 0; i < myInputCount; ++i)
		myNodes[i + 1].SetInputValue(someInputs[i]);

	for (Node& node : myNodes)
		node.Evaluate(myNodes, myLinks);

	for (size_t i = 0; i < myOutputCount; ++i)
		someOutputs[i] = myNodes[i + myNodes.size() - myOutputCount].GetOutputValue();

	return true;
}

void Genome::LinkNodes(size_t aSrcNodeIdx, size_t aDstNodeIdx, double aWeight, bool anEnable)
{
	for (auto it = myLinks.begin(); it != myLinks.end(); ++it)
	{
		Link& link = it->second;
		if (link.GetSrcNodeIdx() == aSrcNodeIdx && link.GetDstNodeIdx() == aDstNodeIdx)
		{
			link.SetWeight(aWeight);
			link.SetEnabled(anEnable);
			return;
		}
	}
	std::uint64_t innovationId = EvolutionParams::GetInnovationNumber();
	myLinks.insert({innovationId, Link(aSrcNodeIdx, aDstNodeIdx, aWeight, anEnable)});
	myNodes[aDstNodeIdx].AddInputLink(innovationId);
}

void Genome::LinkNodesWithInnovationId(size_t aSrcNodeIdx, size_t aDstNodeIdx, double aWeight, bool anEnable, std::uint64_t anInnovationId)
{
	myLinks.insert({anInnovationId,Link(aSrcNodeIdx, aDstNodeIdx, aWeight, anEnable)});
	myNodes[aDstNodeIdx].AddInputLink(anInnovationId);
}

bool Genome::DoNodesHaveDependencies(size_t aSrcNodeIdx, size_t aDstNodeIdx, std::set<size_t>& someCheckedNodes) const
{
	for (std::uint64_t linkId : myNodes[aDstNodeIdx].GetInputLinks())
	{
		auto it = myLinks.find(linkId);
		if (it == myLinks.end())
			continue;

		size_t srcIdx = it->second.GetSrcNodeIdx();
		if (srcIdx < aSrcNodeIdx)
			continue;

		if (someCheckedNodes.contains(srcIdx))
			continue;

		if (srcIdx == aSrcNodeIdx)
			return true;

		if (DoNodesHaveDependencies(aSrcNodeIdx, srcIdx, someCheckedNodes))
			return true;

		someCheckedNodes.insert(srcIdx);
	}
	return false;
}

void Genome::MutateLinkWeights()
{
	for (auto it = myLinks.begin(); it != myLinks.end(); ++it)
	{
		Link& link = it->second;
		std::uniform_real_distribution<> rand(0.0, 1.0);
		if (rand(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourLinkWeightTotalMutationProba)
		{
			std::uniform_real_distribution<> rand2(-8.0, 8.0);
			link.SetWeight(rand2(EvolutionParams::GetRandomGenerator()));
		}
		else
		{
			std::normal_distribution<> rand2(0.0, EvolutionParams::ourLinkWeightPartialMutationScale);
			link.SetWeight(std::clamp(link.GetWeight() + rand2(EvolutionParams::GetRandomGenerator()), -8.0, 8.0));
		}
	}
}

void Genome::MutateAddLink()
{
	auto getRandomConnectableSrcNodeIdx = [this](size_t aDstNodeIdx) {
		Node& dstNode = myNodes[aDstNodeIdx];

		std::set<size_t> availableSrcNodeIdx;
		for (size_t i = 0; i < aDstNodeIdx; ++i)
			availableSrcNodeIdx.insert(i);
		for (size_t i = aDstNodeIdx + 1, e = myNodes.size() - myOutputCount; i < e; ++i)
		{
			std::set<size_t> checkedNodes;
			if (!DoNodesHaveDependencies(aDstNodeIdx, i, checkedNodes))
				availableSrcNodeIdx.insert(i);
		}

		if (dstNode.GetInputLinks().size() >= availableSrcNodeIdx.size())
			return aDstNodeIdx;

		for (uint64_t linkId : dstNode.GetInputLinks())
		{
			auto it = myLinks.find(linkId);
			if (it != myLinks.end() && it->second.IsEnabled())
				availableSrcNodeIdx.erase(it->second.GetSrcNodeIdx());
		}

		std::uniform_int_distribution<> rand(0, (int)availableSrcNodeIdx.size() - 1);
		auto randIt = availableSrcNodeIdx.begin();
		std::advance(randIt, rand(EvolutionParams::GetRandomGenerator()));
		return *randIt;
	};

	std::uniform_int_distribution<> rand(1 + (int)myInputCount, (int)myNodes.size() - 1);
	size_t dstNodeIdx = rand(EvolutionParams::GetRandomGenerator());
	size_t srcNodeIdx = getRandomConnectableSrcNodeIdx(dstNodeIdx);
	if (srcNodeIdx == dstNodeIdx)
		return;

	if (srcNodeIdx > dstNodeIdx)
	{
		// Need to push the dst node later in the execution order, just after the src node
		myNodes.insert(myNodes.begin() + srcNodeIdx + 1, std::move(myNodes[dstNodeIdx]));
		myNodes.erase(myNodes.begin() + dstNodeIdx);

		// Node indices changed, so all links have to be updated
		for (auto it = myLinks.begin(); it != myLinks.end(); ++it)
			it->second.UpdateAfterNodePush(dstNodeIdx, srcNodeIdx);

		dstNodeIdx = srcNodeIdx;
		srcNodeIdx--;
	}

	std::uniform_real_distribution<> rand2(-8.0, 8.0);
	LinkNodes(srcNodeIdx, dstNodeIdx, rand2(EvolutionParams::GetRandomGenerator()), true);
}

void Genome::MutateAddNode()
{
	auto getRandomSplittableLink = [this]() -> Link& {
		int splittableLinksCount = 0;
		for (auto it = myLinks.begin(); it != myLinks.end(); ++it)
		{
			if (it->second.IsSplittable())
				splittableLinksCount++;
		}

		std::uniform_int_distribution<> rand(0, splittableLinksCount - 1);

		int selectedLinkIdx = rand(EvolutionParams::GetRandomGenerator());
		int visitedSplittableLinks = -1;
		for (auto it = myLinks.begin(); it != myLinks.end(); ++it)
		{
			if (it->second.IsSplittable())
				visitedSplittableLinks++;

			if (visitedSplittableLinks == selectedLinkIdx)
				return it->second;
		}

		// Never happens
		return myLinks.begin()->second;
	};
	Link& linkToSplit = getRandomSplittableLink();

	// Disable the link, but keep it to potential be re-enabled later or participate to cross-overs
	linkToSplit.SetEnabled(false);

	// Insert a new hidden node while keeping the noed vector sorted by execution order
	size_t newNodeIdx = 0;
	if (myNodes[linkToSplit.GetDstNodeIdx()].GetType() == Node::Type::Output)
	{
		// Place the new hidden node just before the outputs
		newNodeIdx = myNodes.size() - myOutputCount;
		myNodes.insert(myNodes.end() - myOutputCount, Node(Node::Type::Hidden));
	}
	else
	{
		// Place the new hidden node just before the dest node
		newNodeIdx = linkToSplit.GetDstNodeIdx();
		myNodes.insert(myNodes.begin() + linkToSplit.GetDstNodeIdx(), Node(Node::Type::Hidden));
	}

	// Node indices changed, so all links have to be updated
	for (auto it = myLinks.begin(); it != myLinks.end(); ++it)
		it->second.UpdateAfterNodeAdd(newNodeIdx);

	LinkNodes(0, newNodeIdx, 0.0, true);
	LinkNodes(linkToSplit.GetSrcNodeIdx(), newNodeIdx, 1.0, true);
	LinkNodes(newNodeIdx, linkToSplit.GetDstNodeIdx(), linkToSplit.GetWeight(), true);
}

//void Genome::VerifyLinks() const
//{
//	for (size_t idx = 0; idx < myNodes.size(); ++idx)
//	{
//		for (std::uint64_t linkId : myNodes[idx].GetInputLinks())
//		{
//			auto it = myLinks.find(linkId);
//			assert(it != myLinks.end());
//			assert(it->second.GetDstNodeIdx() == idx);
//		}
//	}
//}

}
