#pragma once

#include <random>

namespace Neat {

class EvolutionParams
{
public:
	static void SetRandomSeed(unsigned int aSeed) { ourRandomGenerator = std::default_random_engine(aSeed); }
	static std::default_random_engine& GetRandomGenerator() { return ourRandomGenerator; }

	static std::uint64_t GetInnovationNumber();
	static std::uint64_t GetInnovationNumber(std::uint64_t aForceNext);

	static inline double ourEdgeWeightMutationProba = 0.8;
	static inline double ourEdgeWeightTotalMutationProba = 0.1;
	static inline double ourEdgeWeightPartialMutationScale = 0.05;

	static inline double ourNewEdgeProba = 0.05;
	static inline double ourNewNodeProba = 0.03;

	static inline double ourSingleParentReproductionProba = 0.25;
	static inline double ourDisableEdgeOnCrossOverProba = 0.75;
	static inline double ourAmountGenomesToKeep = 0.3;

	static inline double ourSpecieThreshold = 1.0;
	static inline double ourMatchingGeneCoeff = 2.0;
	static inline double ourNonMatchingGeneCoeff = 1.0;
	static inline int ourExtinctionAfterNoImprovement = 10;

private:
	static std::default_random_engine ourRandomGenerator;
	static std::atomic_uint64_t ourNextInnovationId;
};

}
