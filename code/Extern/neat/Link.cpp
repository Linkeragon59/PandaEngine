#include "Link.h"

namespace Neat {

bool Link::IsSplittable() const
{
	return myEnabled && mySrcNodeIdx != 0;
}

void Link::UpdateAfterNodeAdd(size_t aNewNodeIdx)
{
	UpdateIdxAfterNodeAdd(mySrcNodeIdx, aNewNodeIdx);
	UpdateIdxAfterNodeAdd(myDstNodeIdx, aNewNodeIdx);
}

void Link::UpdateAfterNodeMove(size_t anOldNodeIdx, size_t aNewNodeIdx)
{
	UpdateIdxAfterNodeMove(mySrcNodeIdx, anOldNodeIdx, aNewNodeIdx);
	UpdateIdxAfterNodeMove(myDstNodeIdx, anOldNodeIdx, aNewNodeIdx);
}

void Link::UpdateIdxAfterNodeAdd(size_t& aInOutNodeIdx, size_t aNewNodeIdx)
{
	if (aInOutNodeIdx >= aNewNodeIdx)
		aInOutNodeIdx++;
}

void Link::UpdateIdxAfterNodeMove(size_t& aInOutNodeIdx, size_t anOldNodeIdx, size_t aNewNodeIdx)
{
	if (anOldNodeIdx > aNewNodeIdx)
	{
		// Node was advanced earlier in the execution list
		if (aInOutNodeIdx >= aNewNodeIdx && aInOutNodeIdx < anOldNodeIdx)
			aInOutNodeIdx++;
		else if (aInOutNodeIdx == anOldNodeIdx)
			aInOutNodeIdx = aNewNodeIdx;
	}
	else
	{
		// Node was pushed further in the execution list
		if (aInOutNodeIdx > anOldNodeIdx && aInOutNodeIdx <= aNewNodeIdx)
			aInOutNodeIdx--;
		else if (aInOutNodeIdx == anOldNodeIdx)
			aInOutNodeIdx = aNewNodeIdx;
	}
}

}
