#pragma once

#include "Node.h"

#include <vector>
#include <map>

namespace Neat {

class Specie;

class Genome
{
public:
	Genome();
	Genome(size_t anInputCount, size_t anOutputCount);
	
	Genome(const char* aFilePath);
	void SaveToFile(const char* aFilePath) const;

	Genome(const Genome* aParent1, const Genome* aParent2);
	void Mutate();

	const std::map<std::uint64_t, Link>& GetLinks() const { return myLinks; }

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
	void LinkNodesWithInnovationId(size_t aSrcNodeIdx, size_t aDstNodeIdx, double aWeight, bool anEnable, std::uint64_t anInnovationId);

	bool DoNodesHaveDependencies(size_t aSrcNodeIdx, size_t aDstNodeIdx, std::set<size_t>& someCheckedNodes) const;

	void MutateLinkWeights();
	void MutateAddLink();
	void MutateAddNode();

	//void VerifyLinks() const;

	std::vector<Node> myNodes;	// Sorted by execution order
	std::map<std::uint64_t, Link> myLinks;
	size_t myInputCount = 0;	
	size_t myOutputCount = 0;

	double myFitness = 0.0;
	double myAdjustedFitness = 0.0;

	Specie* mySpecie = nullptr;
};

}
