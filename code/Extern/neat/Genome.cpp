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
	std::uniform_real_distribution<> rand(-EvolutionParams::ourLinkWeightBound, EvolutionParams::ourLinkWeightBound);

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
			LinkNodes(innovationId, std::stoull(tokens[1]), std::stoull(tokens[2]), std::stod(tokens[3]), std::stoi(tokens[4]));
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

			LinkNodes(innovationId, primaryParentLink.GetSrcNodeIdx(), primaryParentLink.GetDstNodeIdx(), weight, primaryParentLink.IsEnabled());
		}
		else
		{
			// Disjoint or Excess gene, ignore secondary parent
			LinkNodes(innovationId, primaryParentLink.GetSrcNodeIdx(), primaryParentLink.GetDstNodeIdx(), primaryParentLink.GetWeight(), primaryParentLink.IsEnabled());
		}
	}
}

void Genome::Mutate()
{
	//Check();

	std::uniform_real_distribution<> rand(0.0, 1.0);

	if (rand(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourLinkWeightMutationProba)
		MutateLinkWeights();

	if (rand(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourNewLinkProba)
		MutateAddLink();

	if (rand(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourNewNodeProba)
		MutateAddNode();

	//Check();
}

bool Genome::Check() const
{
	for (size_t idx = 0; idx < myNodes.size(); ++idx)
	{
		// Verify that all links are valid

		for (std::uint64_t linkId : myNodes[idx].GetInputLinks())
		{
			auto it = myLinks.find(linkId);
			assert(it != myLinks.end());
			if (it == myLinks.end())
				return false;

			assert(it->second.GetDstNodeIdx() == idx);
			if (it->second.GetDstNodeIdx() != idx)
				return false;

			assert(it->second.GetSrcNodeIdx() < it->second.GetDstNodeIdx());
			if (it->second.GetSrcNodeIdx() >= it->second.GetDstNodeIdx())
				return false;
		}

		// Verify there is no recursion in the network

		std::set<size_t> dependencies;
		bool collectedDependencies = CollectNodeDependencies(idx, dependencies);
		assert(collectedDependencies);
		if (!collectedDependencies)
			return false;

		if (myNodes[idx].GetType() == Node::Type::Bias || myNodes[idx].GetType() == Node::Type::Input)
		{
			// Verify Bias and Inputs don't have dependencies

			assert(dependencies.size() == 0);
			if (dependencies.size() > 0)
				return false;
		}
		else
		{
			// Verify that all other nodes are connected to the Bias node and at least 1 Input node

			bool connectedToBias = dependencies.contains(0);
			assert(connectedToBias);
			if (!connectedToBias)
				return false;

			bool connectedToInput = false;
			for (size_t inputIdx = 0; inputIdx < myInputCount; ++inputIdx)
			{
				if (dependencies.contains(1 + inputIdx))
				{
					connectedToInput = true;
					break;
				}
			}
			assert(connectedToInput);
			if (!connectedToInput)
				return false;
		}

		// Verify that the nodes are correctly ordered (by execution)

		for (size_t dependencyIdx : dependencies)
		{
			if (dependencyIdx >= idx)
			{
				assert(false);
				return false;
			}
		}
	}
	return true;
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

void Genome::LinkNodes(std::uint64_t anInnovationId, size_t aSrcNodeIdx, size_t aDstNodeIdx, double aWeight, bool anEnable)
{
	myLinks.insert({ anInnovationId,Link(aSrcNodeIdx, aDstNodeIdx, aWeight, anEnable) });
	myNodes[aDstNodeIdx].AddInputLink(anInnovationId);
}

bool Genome::CollectNodeDependencies(size_t aNodeIdx, std::set<size_t>& someOutNodes, size_t aRecursion) const
{
	if (aRecursion > myNodes.size())
		return false;

	for (std::uint64_t linkId : myNodes[aNodeIdx].GetInputLinks())
	{
		auto it = myLinks.find(linkId);
		if (it == myLinks.end())
			continue;

		size_t srcIdx = it->second.GetSrcNodeIdx();
		if (someOutNodes.contains(srcIdx))
			continue;

		someOutNodes.insert(srcIdx);
		CollectNodeDependencies(srcIdx, someOutNodes, aRecursion + 1);
	}
	return true;
}

void Genome::MoveNode(size_t anOldNodeIdx, size_t aNewNodeIdx)
{
	assert(anOldNodeIdx != aNewNodeIdx);
	if (anOldNodeIdx == aNewNodeIdx)
		return;

	if (anOldNodeIdx > aNewNodeIdx)
	{
		// Node was advanced earlier in the execution list
		myNodes.insert(myNodes.begin() + aNewNodeIdx, std::move(myNodes[anOldNodeIdx]));
		myNodes.erase(myNodes.begin() + anOldNodeIdx + 1);
	}
	else
	{
		// Node was pushed further in the execution list
		myNodes.insert(myNodes.begin() + aNewNodeIdx + 1, std::move(myNodes[anOldNodeIdx]));
		myNodes.erase(myNodes.begin() + anOldNodeIdx);
	}

	// Node indices changed, so all links have to be updated
	for (auto it = myLinks.begin(); it != myLinks.end(); ++it)
		it->second.UpdateAfterNodeMove(anOldNodeIdx, aNewNodeIdx);
}

void Genome::MutateLinkWeights()
{
	for (auto it = myLinks.begin(); it != myLinks.end(); ++it)
	{
		Link& link = it->second;
		std::uniform_real_distribution<> rand(0.0, 1.0);
		if (rand(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourLinkWeightTotalMutationProba)
		{
			std::uniform_real_distribution<> rand2(-EvolutionParams::ourLinkWeightBound, EvolutionParams::ourLinkWeightBound);
			link.SetWeight(rand2(EvolutionParams::GetRandomGenerator()));
		}
		else
		{
			std::normal_distribution<> rand2(0.0, EvolutionParams::ourLinkWeightPartialMutationPower);
			link.SetWeight(std::clamp(link.GetWeight() + rand2(EvolutionParams::GetRandomGenerator()), -EvolutionParams::ourLinkWeightBound, EvolutionParams::ourLinkWeightBound));
		}
	}
}

void Genome::MutateAddLink()
{
	auto getRandomConnectableSrcNodeIdx = [this](size_t aDstNodeIdx) {
		std::set<size_t> availableSrcNodeIdx;

		std::set<size_t> dstDependencies;
		CollectNodeDependencies(aDstNodeIdx, dstDependencies);
		for (size_t i = 0, e = myNodes.size() - myOutputCount; i < e; ++i)
		{
			if (i == aDstNodeIdx || dstDependencies.contains(i))
				continue;

			std::set<size_t> srcDependencies;
			CollectNodeDependencies(i, srcDependencies);

			if (srcDependencies.contains(aDstNodeIdx))
				continue;

			availableSrcNodeIdx.insert(i);
		}

		if (availableSrcNodeIdx.size() == 0)
			return aDstNodeIdx; // Already fully connected

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
		std::set<size_t> srcDependencies;
		CollectNodeDependencies(srcNodeIdx, srcDependencies);

		std::vector<size_t> nodesToMove;
		nodesToMove.reserve(srcDependencies.size() + 1);
		nodesToMove.push_back(srcNodeIdx);
		for (size_t idx : srcDependencies)
			if (idx > dstNodeIdx)
				nodesToMove.push_back(idx);
		std::sort(nodesToMove.begin(), nodesToMove.end());

		// Move al these nodes before the dstNode, keeping them ordered
		for (size_t idx : nodesToMove)
		{
			MoveNode(idx, dstNodeIdx);
			dstNodeIdx++; // the dstNode has been pushed forward as a result
		}
		srcNodeIdx = dstNodeIdx - 1;
	}

	std::uniform_real_distribution<> rand2(-EvolutionParams::ourLinkWeightBound, EvolutionParams::ourLinkWeightBound);
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

}
