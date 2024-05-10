#include "Specie.h"

#include "EvolutionParams.h"
#include "Genome.h"

namespace Neat {

bool Specie::BelongsToSpecie(const Genome* aGenome) const
{
	if (myGenomes.empty())
		return false;

	std::uniform_int_distribution<> rand(0, (int)myGenomes.size() - 1);
	const Genome* representativeGenome = myGenomes[rand(EvolutionParams::GetRandomGenerator())];

	size_t maxGenomeSize = std::max(representativeGenome->GetEdges().size(), aGenome->GetEdges().size());
	if (maxGenomeSize == 0)
		return false;
	
	size_t matchingGenesCount = 0;
	size_t nonMatchingGenesCount = 0;
	double averageWeightDifference = 0.0;

	for (auto it = representativeGenome->GetEdges().begin(); it != representativeGenome->GetEdges().end(); ++it)
	{
		std::uint64_t innovationId = it->first;
		const Edge& representativeEdge = it->second;

		auto it2 = aGenome->GetEdges().find(innovationId);
		if (it2 != aGenome->GetEdges().end())
		{
			// Common gene
			matchingGenesCount++;
			averageWeightDifference += std::abs(representativeEdge.GetWeight() - it2->second.GetWeight());
		}
		else
		{
			// Disjoint or Excess gene
			nonMatchingGenesCount++;
		}
	}

	if (matchingGenesCount > 0)
		averageWeightDifference /= matchingGenesCount;

	return EvolutionParams::ourMatchingGeneCoeff * averageWeightDifference + EvolutionParams::ourMatchingGeneCoeff * nonMatchingGenesCount / maxGenomeSize;
}

}
