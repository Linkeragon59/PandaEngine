#pragma once

#include "Node.h"

#include <vector>
#include <map>

namespace Neat {

class Specie;

class Genome
{
public:
	Genome(size_t anInputCount, size_t anOutputCount);
	
	Genome(const char* aFilePath);
	void SaveToFile(const char* aFilePath) const;

	Genome(const Genome* aParent1, const Genome* aParent2);
	void Mutate();
	bool Check() const; // Asserts that the network is not malformed

	const std::map<std::uint64_t, Link>& GetLinks() const { return myLinks; }

	size_t GetNodesCount() const { return myNodes.size(); }
	size_t GetGenesCount() const { return myLinks.size(); }

	bool Evaluate(const std::vector<double>& someInputs, std::vector<double>& someOutputs);
	void SetFitness(double aFitness) { myFitness = aFitness; }
	double GetFitness() const { return myFitness; }
	void AdjustFitness(double anAdjustedFitness) { myAdjustedFitness = anAdjustedFitness; }
	double GetAdjustedFitness() const { return myAdjustedFitness; }

	void SetSpecie(Specie* aSpecie) { mySpecie = aSpecie; }
	Specie* GetSpecie() const { return mySpecie; }

private:
	size_t GetHiddenNodesCount() const { return myNodes.size() - 1 - myInputCount - myOutputCount; } // -1 for Bias
	void LinkNodes(size_t aSrcNodeIdx, size_t aDstNodeIdx, double aWeight, bool anEnable);
	void LinkNodes(std::uint64_t anInnovationId, size_t aSrcNodeIdx, size_t aDstNodeIdx, double aWeight, bool anEnable);

	bool CollectNodeDependencies(size_t aNodeIdx, std::set<size_t>& someOutNodes, size_t aRecursion = 0) const;
	void MoveNode(size_t anOldNodeIdx, size_t aNewNodeIdx);

	void MutateLinkWeights();
	void MutateAddLink();
	void MutateAddNode();

	std::vector<Node> myNodes;	// Sorted by execution order
	std::map<std::uint64_t, Link> myLinks;
	size_t myInputCount = 0;	
	size_t myOutputCount = 0;

	double myFitness = 0.0;
	double myAdjustedFitness = 0.0;

	Specie* mySpecie = nullptr;
};

}
