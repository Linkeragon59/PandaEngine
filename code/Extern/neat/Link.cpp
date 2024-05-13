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

void Link::UpdateAfterNodePush(size_t anOldNodeIdx, size_t aNewNodeIdx)
{
	UpdateIdxAfterNodePush(mySrcNodeIdx, anOldNodeIdx, aNewNodeIdx);
	UpdateIdxAfterNodePush(myDstNodeIdx, anOldNodeIdx, aNewNodeIdx);
}

void Link::UpdateIdxAfterNodeAdd(size_t& aInOutNodeIdx, size_t aNewNodeIdx)
{
	if (aInOutNodeIdx >= aNewNodeIdx)
		aInOutNodeIdx++;
}

void Link::UpdateIdxAfterNodePush(size_t& aInOutNodeIdx, size_t anOldNodeIdx, size_t aNewNodeIdx)
{
	if (aInOutNodeIdx > anOldNodeIdx && aInOutNodeIdx <= aNewNodeIdx)
		aInOutNodeIdx--;
	else if (aInOutNodeIdx == anOldNodeIdx)
		aInOutNodeIdx = aNewNodeIdx;
}

}
