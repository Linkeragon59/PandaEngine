#pragma once

#include <atomic>

class SharedResource
{
public:
	// If use outside of a SharedPtr, myRefCount should stay 0
	SharedResource() {}
	SharedResource(const SharedResource& /*anOther*/) {}
	virtual ~SharedResource() {}

	void AddRef() { myRefCount++; }
	virtual void Release() { if (myRefCount.fetch_sub(1) == 1) delete this; }
protected:
	std::atomic<int> myRefCount = 0;
};

template<typename T>
class SharedPtr
{
public:
	SharedPtr() {}
	SharedPtr(T* aPtr) : myPtr(aPtr) { if (myPtr) myPtr->AddRef(); }
	SharedPtr(const SharedPtr& aPtr) : myPtr(aPtr.myPtr) { if (myPtr) myPtr->AddRef(); }
	SharedPtr(SharedPtr&& aPtr) : myPtr(aPtr.myPtr) { aPtr.myPtr = nullptr; }

	~SharedPtr() { if (myPtr) myPtr->Release(); }

	SharedPtr& operator=(T* aPtr)
	{
		T* oldPtr = myPtr;
		myPtr = aPtr;
		if (myPtr) myPtr->AddRef();
		if (oldPtr) oldPtr->Release();
		return *this;
	}
	SharedPtr& operator=(const SharedPtr& aPtr)
	{
		T* oldPtr = myPtr;
		myPtr = aPtr.myPtr;
		if (myPtr) myPtr->AddRef();
		if (oldPtr) oldPtr->Release();
		return *this;
	}
	SharedPtr& operator=(SharedPtr&& aPtr)
	{
		T* oldPtr = myPtr;
		myPtr = aPtr.myPtr;
		aPtr.myPtr = nullptr;
		if (oldPtr) oldPtr->Release();
		return *this;
	}

	operator T* () const { return myPtr; }
	T* operator->() const { return myPtr; }
	T& operator*() const { return *myPtr; }

	bool operator!() const { return !myPtr; }
	bool operator==(const SharedPtr& aPtr) const { return myPtr == aPtr->myPtr; }
	bool operator!=(const SharedPtr& aPtr) const { return myPtr != aPtr->myPtr; }

private:
	T* myPtr = nullptr;
};
