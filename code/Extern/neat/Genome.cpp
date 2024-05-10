#include "Genome.h"

#include "EvolutionParams.h"
#include "Specie.h"

#include <fstream>
#include <sstream>
#include <string>

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
	std::uniform_real_distribution<> rand(-1.0, 1.0);

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

			LinkNodesWithInnovationId(std::stoull(tokens[1]), std::stoull(tokens[2]), std::stod(tokens[3]), std::stoi(tokens[4]), std::stoull(tokens[0]));
		}
	}
}

void Genome::SaveToFile(const char* aFilePath) const
{
	std::ofstream file(aFilePath);
	if (!file.is_open())
		return;

	file << myInputCount << ";" << GetHiddenNodesCount() << ";" << myOutputCount << ";" << std::endl;

	for (auto it = myEdges.begin(); it != myEdges.end(); ++it)
	{
		const Edge& edge = it->second;
		file << it->first << " " << edge.GetSrcNodeIdx() << " " << edge.GetDstNodeIdx() << " "
			<< edge.GetWeight() << " " << edge.IsEnabled() << ";" << std::endl;
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

	for (auto it = primaryParent->myEdges.begin(); it != primaryParent->myEdges.end(); ++it)
	{
		std::uint64_t innovationId = it->first;
		const Edge& primaryParentEdge = it->second;
		
		auto it2 = secondaryParent->myEdges.find(innovationId);
		if (it2 != secondaryParent->myEdges.end())
		{
			// Common gene, choose weight randomly
			const Edge& secondaryParentEdge = it2->second;
			
			std::uniform_int_distribution<> rand(0, 1);
			double weight = (rand(EvolutionParams::GetRandomGenerator()) == 0) ? primaryParentEdge.GetWeight() : it2->second.GetWeight();

			bool edgeEnabled = true;
			if (!primaryParentEdge.IsEnabled() || !secondaryParentEdge.IsEnabled())
			{
				std::uniform_real_distribution<> rand2(0.0, 1.0);
				if (rand2(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourDisableEdgeOnCrossOverProba)
					edgeEnabled = false;
			}

			LinkNodesWithInnovationId(primaryParentEdge.GetSrcNodeIdx(), primaryParentEdge.GetDstNodeIdx(), weight, edgeEnabled, innovationId);
		}
		else
		{
			// Disjoint or Excess gene, ignore secondary parent

			bool edgeEnabled = true;
			if (!primaryParentEdge.IsEnabled())
			{
				std::uniform_real_distribution<> rand2(0.0, 1.0);
				if (rand2(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourDisableEdgeOnCrossOverProba)
					edgeEnabled = false;
			}

			LinkNodesWithInnovationId(primaryParentEdge.GetSrcNodeIdx(), primaryParentEdge.GetDstNodeIdx(), primaryParentEdge.GetWeight(), edgeEnabled, innovationId);
		}
	}
}

void Genome::Mutate()
{
	std::uniform_real_distribution<> rand(0.0, 1.0);

	if (rand(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourEdgeWeightMutationProba)
		MutateEdgeWeights();

	if (rand(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourNewEdgeProba)
		MutateAddEdge();

	if (rand(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourNewNodeProba)
		MutateAddNode();
}

bool Genome::Evaluate(const std::vector<double>& someInputs, std::vector<double>& someOutputs)
{
	if (someInputs.size() != myInputCount)
		return false;

	someOutputs.resize(myOutputCount);
	if (myOutputCount == 0)
		return true;

	myNodes[0].SetInputValue(1);
	for (size_t i = 0; i < myInputCount; ++i)
		myNodes[i + 1].SetInputValue(someInputs[i]);

	for (Node& node : myNodes)
		node.Evaluate(myNodes, myEdges);

	for (size_t i = 0; i < myOutputCount; ++i)
		someOutputs[i] = myNodes[i + myNodes.size() - myOutputCount].GetOutputValue();

	return true;
}

void Genome::SetFitness(double aFitness)
{
	myFitness = aFitness;
	myAdjustedFitness = aFitness;
	if (mySpecie)
	{
		myAdjustedFitness /= mySpecie->GetSize();
	}
}

void Genome::LinkNodes(size_t aSrcNodeIdx, size_t aDstNodeIdx, double aWeight, bool anEnable)
{
	for (auto it = myEdges.begin(); it != myEdges.end(); ++it)
	{
		Edge& edge = it->second;
		if (edge.GetSrcNodeIdx() == aSrcNodeIdx && edge.GetDstNodeIdx() == aDstNodeIdx)
		{
			edge.SetWeight(aWeight);
			edge.SetEnabled(anEnable);
			return;
		}
	}
	std::uint64_t innovationId = EvolutionParams::GetInnovationNumber();
	myEdges.insert({innovationId, Edge(aSrcNodeIdx, aDstNodeIdx, aWeight, anEnable)});
	myNodes[aDstNodeIdx].AddInputEdge(innovationId);
}

void Genome::LinkNodesWithInnovationId(size_t aSrcNodeIdx, size_t aDstNodeIdx, double aWeight, bool anEnable, std::uint64_t anInnovationId)
{
	std::uint64_t innovationId = EvolutionParams::GetInnovationNumber(anInnovationId);
	myEdges.insert({innovationId,Edge(aSrcNodeIdx, aDstNodeIdx, aWeight, anEnable)});
	myNodes[aDstNodeIdx].AddInputEdge(innovationId);
}

void Genome::MutateEdgeWeights()
{
	for (auto it = myEdges.begin(); it != myEdges.end(); ++it)
	{
		Edge& edge = it->second;
		std::uniform_real_distribution<> rand(0.0, 1.0);
		if (rand(EvolutionParams::GetRandomGenerator()) <= EvolutionParams::ourEdgeWeightTotalMutationProba)
		{
			std::uniform_real_distribution<> rand2(-1.0, 1.0);
			edge.SetWeight(rand2(EvolutionParams::GetRandomGenerator()));
		}
		else
		{
			std::normal_distribution<> rand2(0.0, EvolutionParams::ourEdgeWeightPartialMutationScale);
			edge.SetWeight(std::clamp(edge.GetWeight() + rand2(EvolutionParams::GetRandomGenerator()), -1.0, 1.0));
		}
	}
}

void Genome::MutateAddEdge()
{
	auto getRandomConnectableSrcNodeIdx = [this](size_t aDstNodeIdx) {
		Node& dstNode = myNodes[aDstNodeIdx];
		size_t srcIdxBound = std::min(aDstNodeIdx, myNodes.size() - myOutputCount);

		if (dstNode.GetInputEdges().size() >= srcIdxBound)
			return aDstNodeIdx; // The selected dst node was already fully connected

		std::set<size_t> availableSrcNodeIdx;
		for (size_t i = 0; i < srcIdxBound; ++i)
			availableSrcNodeIdx.insert(i);
		for (uint64_t edgeId : dstNode.GetInputEdges())
		{
			auto it = myEdges.find(edgeId);
			if (it != myEdges.end() && it->second.IsEnabled())
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

	std::uniform_real_distribution<> rand2(-1.0, 1.0);
	LinkNodes(srcNodeIdx, dstNodeIdx, rand2(EvolutionParams::GetRandomGenerator()), true);
}

void Genome::MutateAddNode()
{
	auto getRandomSplittableEdge = [this]() -> Edge& {
		int splittableEdgesCount = 0;
		for (auto it = myEdges.begin(); it != myEdges.end(); ++it)
		{
			if (it->second.IsSplittable())
				splittableEdgesCount++;
		}

		std::uniform_int_distribution<> rand(0, splittableEdgesCount - 1);

		int selectedEdgeIdx = rand(EvolutionParams::GetRandomGenerator());
		int visitedSplittableEdges = -1;
		for (auto it = myEdges.begin(); it != myEdges.end(); ++it)
		{
			if (it->second.IsSplittable())
				visitedSplittableEdges++;

			if (visitedSplittableEdges == selectedEdgeIdx)
				return it->second;
		}

		// Never happens
		return myEdges.begin()->second;
	};
	Edge& edgeToSplit = getRandomSplittableEdge();

	// Disable the edge, but keep it to potential be re-enabled later or participate to cross-overs
	edgeToSplit.SetEnabled(false);

	// Insert a new hidden node while keeping the noed vector sorted by execution order
	size_t newNodeIdx = 0;
	if (myNodes[edgeToSplit.GetDstNodeIdx()].GetType() == Node::Type::Output)
	{
		// Place the new hidden node just before the outputs
		newNodeIdx = myNodes.size() - myOutputCount;
		myNodes.insert(myNodes.end() - myOutputCount, Node(Node::Type::Hidden));
	}
	else
	{
		// Place the new hidden node just before the dest node
		newNodeIdx = edgeToSplit.GetDstNodeIdx();
		myNodes.insert(myNodes.begin() + edgeToSplit.GetDstNodeIdx(), Node(Node::Type::Hidden));
	}

	// Node indices changed, so all edges have to be updated
	for (auto it = myEdges.begin(); it != myEdges.end(); ++it)
		it->second.UpdateNodeIdx(newNodeIdx);

	LinkNodes(0, newNodeIdx, 0.0, true);
	LinkNodes(edgeToSplit.GetSrcNodeIdx(), newNodeIdx, 1.0, true);
	LinkNodes(newNodeIdx, edgeToSplit.GetDstNodeIdx(), edgeToSplit.GetWeight(), true);
}

}
