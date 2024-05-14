#pragma once

#include <random>

namespace Neat {

class EvolutionParams
{
public:
	static void SetRandomSeed(unsigned int aSeed) { ourRandomGenerator = std::default_random_engine(aSeed); }
	static std::default_random_engine& GetRandomGenerator() { return ourRandomGenerator; }

	static void SetNextInnovationNumber(std::uint64_t aNextId);
	static std::uint64_t GetInnovationNumber();

	static inline double ourLinkWeightMutationProba = 0.8;
	static inline double ourLinkWeightTotalMutationProba = 0.1;
	static inline double ourLinkWeightPartialMutationPower = 2.5;
	static inline double ourLinkWeightBound = 8.0;

	static inline double ourNewLinkProba = 0.05;
	static inline double ourNewNodeProba = 0.03;

	static inline double ourSingleParentReproductionProba = 0.25;
	static inline double ourAmountGenomesToKeep = 0.3;

	static inline double ourSpecieThreshold = 3.0;
	static inline double ourMatchingGeneCoeff = 0.4;
	static inline double ourNonMatchingGeneCoeff = 1.0;

	static inline int ourPopulationStagnantThreshold = 20;
	static inline int ourSpecieStagnantThreshold = 15;
	static inline int ourSpecieNewThreshold = 10;
	static inline int ourSpecieOldThreshold = 20;
	static inline double ourSpecieStagnantPenality = 0.01;
	static inline double ourSpecieNewBonus = 1.0;

private:
	static std::default_random_engine ourRandomGenerator;
	static std::atomic_uint64_t ourNextInnovationId;
};

}
