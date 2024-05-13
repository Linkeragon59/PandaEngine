#include "EvolutionParams.h"

namespace Neat {

std::default_random_engine EvolutionParams::ourRandomGenerator(0);
std::atomic_uint64_t EvolutionParams::ourNextInnovationId = 0;

void EvolutionParams::SetNextInnovationNumber(std::uint64_t aNextId)
{
	ourNextInnovationId.store(aNextId);
}

std::uint64_t EvolutionParams::GetInnovationNumber()
{
	return ourNextInnovationId.fetch_add(1);
}

}
