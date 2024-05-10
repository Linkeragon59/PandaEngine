#pragma once

#include "Core_EntityModule.h"

namespace Core
{
	class Entity
	{
	public:
		Entity() : myId(UINT_MAX) {}
		Entity(EntityId anId) : myId(anId) {}
		operator EntityId() const { return myId; }

		static Entity Create()
		{
			Entity newEntity = EntityModule::GetInstance()->Create();
			return newEntity;
		}

		inline void Destroy()
		{
			EntityModule::GetInstance()->Destroy(myId);
		}

		template<typename T>
		inline bool HasComponent()
		{
			return EntityModule::GetInstance()->HasComponent<T>(myId);
		}

		template<typename T>
		inline T* GetComponent()
		{
			return EntityModule::GetInstance()->GetComponent<T>(myId);
		}

		template<typename T>
		inline const T* GetComponent() const
		{
			return EntityModule::GetInstance()->GetComponent<T>(myId);
		}

		template<typename T, typename... Args>
		inline T* AddComponent(Args&&... someArgs)
		{
			return EntityModule::GetInstance()->AddComponent<T>(myId, std::forward<Args>(someArgs)...);
		}

		template<typename T>
		inline void RemoveComponent()
		{
			EntityModule::GetInstance()->RemoveComponent<T>(myId);
		}

	private:
		EntityId myId;
	};
}
