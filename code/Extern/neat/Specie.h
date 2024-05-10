#pragma once

#include <vector>

namespace Neat {

class Genome;

class Specie
{
public:
	bool BelongsToSpecie(const Genome* aGenome) const;

private:
	std::vector<Genome*> myGenomes;
};

}
