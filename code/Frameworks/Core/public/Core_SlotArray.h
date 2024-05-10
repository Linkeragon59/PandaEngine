#pragma once

template<typename T>
struct SlotArray
{
	uint Add(const T& anEntry);
	inline void Remove(uint anEntryId);
	uint GetUsedCount() const;

	std::vector<T> myEntries;
};

template<typename T>
uint SlotArray<T>::Add(const T& anEntry)
{
	for (uint i = 0; i < myEntries.size(); ++i)
	{
		if (!myEntries[i].IsSet())
		{
			myEntries[i] = anEntry;
			return i;
		}
	}

	myEntries.push_back(anEntry);
	return (uint)myEntries.size() - 1;
}

template<typename T>
void SlotArray<T>::Remove(uint anEntryId)
{
	Assert(anEntryId < myEntries.size());
	myEntries[anEntryId].Clear();
}

template<typename T>
uint SlotArray<T>::GetUsedCount() const
{
	uint count = 0;
	for (uint i = 0; i < myEntries.size(); ++i)
	{
		if (myEntries[i].IsSet())
		{
			count++;
		}
	}
	return count;
}
