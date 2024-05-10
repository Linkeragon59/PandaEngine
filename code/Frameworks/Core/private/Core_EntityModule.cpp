#include "Core_EntityModule.h"

namespace Core
{
	EntityId EntityModule::Create()
	{
		EntityId newEntity;
		if (myFreeEntityIds.size() == 0)
		{
			newEntity = myNextEntityId++;
		}
		else
		{
			newEntity = *myFreeEntityIds.begin();
			myFreeEntityIds.erase(newEntity);
		}

		for (uint i = 0; i < (uint)myComponentContainers.size(); ++i)
			myComponentContainers[i]->OnEntityCreated(newEntity);

		return newEntity;
	}

	void EntityModule::Destroy(EntityId anId)
	{
		if (anId >= myNextEntityId || myFreeEntityIds.find(anId) != myFreeEntityIds.end())
			return;

		for (uint i = 0; i < (uint)myComponentContainers.size(); ++i)
			myComponentContainers[i]->OnEntityDestroyed(anId);

		myFreeEntityIds.insert(anId);
	}

	void EntityModule::OnRegister()
	{
	}

	void EntityModule::OnUnregister()
	{
		for (ComponentContainerBase* container : myComponentContainers)
			delete container;
		myComponentContainers.clear();
	}
}
