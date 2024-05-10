#pragma once

#include <atomic>
#include <string>

namespace Neat {

class Node;

class Edge
{
public:
	Edge(size_t anSrcNodeIdx, size_t anDstNodeIdx, double aWeight, bool anEnable)
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

	void UpdateNodeIdx(size_t aNewNodeIdx);
	void SetWeight(double aWeight) { myWeight = aWeight; }
	void SetEnabled(bool aEnable) { myEnabled = aEnable; }

private:
	size_t mySrcNodeIdx;
	size_t myDstNodeIdx;
	double myWeight;
	bool myEnabled;
};

}
