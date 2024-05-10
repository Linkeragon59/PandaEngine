#include "EvolutionParams.h"

namespace Neat {

std::default_random_engine EvolutionParams::ourRandomGenerator(0);
std::atomic_uint64_t EvolutionParams::ourNextInnovationId = 0;

std::uint64_t EvolutionParams::GetInnovationNumber()
{
	return ourNextInnovationId.fetch_add(1);
}

std::uint64_t EvolutionParams::GetInnovationNumber(std::uint64_t aForceNext)
{
	ourNextInnovationId.store(aForceNext + 1);
	return aForceNext;
}

}
