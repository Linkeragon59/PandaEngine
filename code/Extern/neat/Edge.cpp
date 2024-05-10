#include "Edge.h"

namespace Neat {

bool Edge::IsSplittable() const
{
	return myEnabled && mySrcNodeIdx != 0;
}

void Edge::UpdateNodeIdx(size_t aNewNodeIdx)
{
	if (mySrcNodeIdx >= aNewNodeIdx)
		mySrcNodeIdx++;
	if (myDstNodeIdx >= aNewNodeIdx)
		myDstNodeIdx++;
}

}
