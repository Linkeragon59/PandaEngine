#pragma once

#include <atomic>
#include <string>

namespace Neat {

class Node;

class Link
{
public:
	Link(size_t anSrcNodeIdx, size_t anDstNodeIdx, double aWeight, bool anEnable)
		: mySrcNodeIdx(anSrcNodeIdx)
		, myDstNodeIdx(anDstNodeIdx)
		, myWeight(aWeight)
		, myEnabled(anEnable)
	{}

	size_t GetSrcNodeIdx() const { return mySrcNodeIdx; }
	size_t GetDstNodeIdx() const { return myDstNodeIdx; }
	double GetWeight() const { return myWeight; }
	bool IsEnabled() const { return myEnabled; }

	bool IsSplittable() const;

	void UpdateAfterNodeAdd(size_t aNewNodeIdx);
	void UpdateAfterNodeMove(size_t anOldNodeIdx, size_t aNewNodeIdx);
	void SetWeight(double aWeight) { myWeight = aWeight; }
	void SetEnabled(bool aEnable) { myEnabled = aEnable; }

private:
	void UpdateIdxAfterNodeAdd(size_t& aInOutNodeIdx, size_t aNewNodeIdx);
	void UpdateIdxAfterNodeMove(size_t& aInOutNodeIdx, size_t anOldNodeIdx, size_t aNewNodeIdx);

	size_t mySrcNodeIdx;
	size_t myDstNodeIdx;
	double myWeight;
	bool myEnabled;
};

}
