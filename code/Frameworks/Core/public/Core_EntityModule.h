#pragma once

#include "Core_Module.h"

#include <set>
#include <map>

namespace Core
{
	typedef uint EntityId;

	class ComponentContainerBase
	{
	public:
		// anElementSize is the size of one element in bytes
		// aChunkSize is the number of elements in one contiguous chunk of data
		ComponentContainerBase(uint anElementSize, uint aChunkSize)
			: myElementSize(anElementSize)
			, myChunkSize(aChunkSize)
		{}

		virtual ~ComponentContainerBase()
		{
			for (char* chunk : myChunks)
				delete[] chunk;
		}

		virtual void OnEntityCreated(EntityId anId) = 0;
		virtual void OnEntityDestroyed(EntityId anId) = 0;

	protected:
		inline uint GetCapacity() const { return myChunkSize * (uint)myChunks.size(); }
		inline uint GetSize() const { return mySize; }

		void Reserve(uint anElementCount)
		{
			while (GetCapacity() < anElementCount)
				myChunks.push_back(new char[myElementSize * myChunkSize]);
		}

		void Resize(uint anElementCount)
		{
			if (mySize >= anElementCount)
				return;
			Reserve(anElementCount);
			mySize = anElementCount;
		}

		inline void* Get(uint anElementIndex)
		{
			return myChunks[anElementIndex / myChunkSize] + (anElementIndex % myChunkSize) * myElementSize;
		}

		inline const void* Get(uint anElementIndex) const
		{
			return myChunks[anElementIndex / myChunkSize] + (anElementIndex % myChunkSize) * myElementSize;
		}

		uint myElementSize = 0;
		uint myChunkSize = 0;
		uint mySize = 0;
		std::vector<char*> myChunks;
	};

	template<typename Type, uint ChunkSize = 128>
	class ComponentContainer : public ComponentContainerBase
	{
	public:
		ComponentContainer() : ComponentContainerBase(sizeof(Type), ChunkSize) {}

		~ComponentContainer() override
		{
			for (const auto it : myEntityIdToIndexMap)
				reinterpret_cast<Type*>(Get(it.second))->~Type();
		}

		inline bool HasComponent(EntityId anId)
		{
			return myEntityIdToIndexMap.find(anId) != myEntityIdToIndexMap.end();
		}

		inline Type* GetComponent(EntityId anId)
		{
			const auto it = myEntityIdToIndexMap.find(anId);
			if (it == myEntityIdToIndexMap.end())
				return nullptr;
			return reinterpret_cast<Type*>(Get(it->second));
		}

		inline const Type* GetComponent(EntityId anId) const
		{
			const auto it = myEntityIdToIndexMap.find(anId);
			if (it == myEntityIdToIndexMap.end())
				return nullptr;
			return reinterpret_cast<Type*>(Get(it->second));
		}

		template<typename ... Args>
		Type* AddComponent(EntityId anId, Args&&... SomeArgs)
		{
			if (Type* component = GetComponent(anId))
				return component;

			uint index = UINT_MAX;
			if (myFreeIndices.size() == 0)
			{
				index = (uint)myEntityIdToIndexMap.size();
				Resize(index + 1);
			}
			else
			{
				index = *myFreeIndices.begin();
				myFreeIndices.erase(index);
			}

			void* ptr = Get(index);
			new(ptr) Type(std::forward<Args>(SomeArgs)...);
			myEntityIdToIndexMap[anId] = index;
			return reinterpret_cast<Type*>(ptr);
		}

		void RemoveComponent(EntityId anId)
		{
			auto it = myEntityIdToIndexMap.find(anId);
			if (it != myEntityIdToIndexMap.end())
			{
				reinterpret_cast<Type*>(Get(it->second))->~Type();
				myFreeIndices.insert(it->second);
				myEntityIdToIndexMap.erase(it);
			}
		}

		void OnEntityCreated(EntityId /*anId*/) override {}
		void OnEntityDestroyed(EntityId anId) override
		{
			RemoveComponent(anId);
		}

		struct Iterator
		{
			using SubIterator = std::map<EntityId, uint>::iterator;

			Iterator(ComponentContainer& aContainer, SubIterator anIterator)
				: myContainer(aContainer)
				, myIterator(anIterator)
			{}

			EntityId GetEntityId() const { return myIterator->first; }
			Type* GetComponent() const { return myContainer.GetAt(myIterator->second); }
			Type* operator*() const { return myContainer.GetAt(myIterator->second); }
			Iterator& operator++() { myIterator++; return *this; }

			bool operator==(const Iterator& anOther) const { return myIterator == anOther.myIterator; }
			bool operator!=(const Iterator& anOther) const { return myIterator != anOther.myIterator; }

		private:
			ComponentContainer& myContainer;
			SubIterator myIterator;
		};

		inline Iterator begin() { return Iterator(*this, myEntityIdToIndexMap.begin()); }
		inline Iterator end() { return Iterator(*this, myEntityIdToIndexMap.end()); }

	protected:
		friend Iterator;
		inline Type* GetAt(uint anIndex) { return reinterpret_cast<Type*>(Get(anIndex)); }

	private:
		std::map<EntityId, uint> myEntityIdToIndexMap;
		std::set<uint> myFreeIndices;
	};

	class EntityModule : public Module
	{
	DECLARE_CORE_MODULE(EntityModule, "Entity")

	public:
		EntityId Create();
		void Destroy(EntityId anId);
		bool Exists(EntityId anId) const { return anId < myNextEntityId&& myFreeEntityIds.find(anId) == myFreeEntityIds.end(); }

		template<typename Type>
		inline bool HasComponent(EntityId anId)
		{
			return static_cast<ComponentContainer<Type>*>(myComponentContainers[GetComponentId<Type>()])->HasComponent(anId);
		}

		template<typename Type>
		inline Type* GetComponent(EntityId anId)
		{
			return static_cast<ComponentContainer<Type>*>(myComponentContainers[GetComponentId<Type>()])->GetComponent(anId);
		}

		template<typename Type>
		inline const Type* GetComponent(EntityId anId) const
		{
			return static_cast<const ComponentContainer<Type>*>(myComponentContainers[GetComponentId<Type>()])->GetComponent(anId);
		}

		template<typename Type, typename ... Args>
		inline Type* AddComponent(EntityId anId, Args&&... SomeArgs)
		{
			return static_cast<ComponentContainer<Type>*>(myComponentContainers[GetComponentId<Type>()])->AddComponent(anId, std::forward<Args>(SomeArgs)...);
		}

		template<typename Type>
		inline void RemoveComponent(EntityId anId)
		{
			return static_cast<ComponentContainer<Type>*>(myComponentContainers[GetComponentId<Type>()])->RemoveComponent(anId);
		}

		template<typename Type>
		inline ComponentContainer<Type>* GetComponentContainer()
		{
			return static_cast<ComponentContainer<Type>*>(myComponentContainers[GetComponentId<Type>()]);
		}

	protected:
		void OnRegister() override;
		void OnUnregister() override;

	private:
		template<typename Type>
		inline uint GetComponentId()
		{
			static uint id = myComponentIdCounter++;
			if ((uint)myComponentContainers.size() == id)
				myComponentContainers.push_back(new ComponentContainer<Type>());
			return id;
		}

		EntityId myNextEntityId = 0;
		std::set<EntityId> myFreeEntityIds;

		uint myComponentIdCounter = 0;
		std::vector<ComponentContainerBase*> myComponentContainers;
	};
}
