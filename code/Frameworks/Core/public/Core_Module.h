#pragma once

#include "Core_Facade.h"

namespace Core
{
	class ModuleManager;

	class Module
	{
	public:
		bool IsInitialized() const { return myIsInitialized; }

		enum class UpdateType
		{
			EarlyUpdate,	// Very beginning of the frame
			MainUpdate,
			LateUpdate,		// Very end of the frame
		};

	protected:
		friend class ModuleManager;

		Module() = default;
		virtual ~Module() = default;
		const bool operator==(const Module* anOther) { return strcmp(GetIdInternal(), anOther->GetIdInternal()) == 0; }

		virtual void OnRegister() {}
		virtual void OnUnregister() {}

		// Called when all dependencies are Initialized
		virtual void OnInitialize() {}
		// Called when any dependency was Finalized or before Unregistering
		virtual void OnFinalize() {}

		// Called each frame after the dependencies have been Updated
		virtual void OnUpdate(UpdateType /*aType*/) {}

		template<typename ModuleType>
		static bool RegisterModule(ModuleType*& anInstance, const std::vector<std::string>& someDependencies)
		{
			Assert(!anInstance, "Module %s is already registered", ModuleType::GetId());
			if (anInstance)
				return false;

			anInstance = new ModuleType();
			anInstance->myDependencies = someDependencies;
			if (RegisterToManager(anInstance))
				return true;

			SafeDelete(anInstance);
			return false;
		}

		template<typename ModuleType>
		static bool RegisterModule(ModuleType*& anInstance)
		{
			return RegisterModule(anInstance, {});
		}

		template<typename ModuleType>
		static bool UnregisterModule(ModuleType*& anInstance)
		{
			Assert(anInstance, "Module %s is not registered", ModuleType::GetId());
			if (!anInstance)
				return false;

			bool res = UnregisterFromManager(anInstance);
			SafeDelete(anInstance);
			return res;
		}

		virtual const char* GetIdInternal() const = 0;

	private:
		static bool RegisterToManager(Module* aModule);
		static bool UnregisterFromManager(Module* aModule);
		
		std::vector<std::string> myDependencies;
		bool myIsInitialized = false;
	};
}

#define DECLARE_CORE_MODULE(Module, Id, ...) \
public: \
	static bool			Register()														{ return RegisterModule<Module>(ourInstance, ##__VA_ARGS__); } \
	static bool			Unregister()													{ return UnregisterModule<Module>(ourInstance); } \
	static Module*		GetInstance()													{ return ourInstance; } \
	static const char*	GetId()															{ return Id; } \
	const char*	GetIdInternal() const override											{ return GetId(); } \
private: \
	static inline Module* ourInstance = nullptr;
