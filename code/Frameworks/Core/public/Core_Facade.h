#pragma once

#include "Core_CommandLine.h"

struct GLFWwindow;

namespace Core
{
	class ModuleManager;

	class Facade
	{
	public:
		static void Create(int argc, char* argv[]);
		static void Destroy();
		static Facade* GetInstance() { return ourInstance; }
		static CommandLine* GetCommandLine() { return &ourInstance->myCommandLine; }

		void Run(GLFWwindow* aWindow);
		void Quit() { myShouldQuit = true; }

		ModuleManager* GetModuleManager() const { return myModuleManager; }
		GLFWwindow* GetMainWindow() const { return myMainWindow; }

	private:
		static Facade* ourInstance;
		Facade();
		~Facade();

		void Initialize();
		void Finalize();
		bool Update();

		CommandLine myCommandLine;
		ModuleManager* myModuleManager = nullptr;
		GLFWwindow* myMainWindow = nullptr;
		bool myShouldQuit = false;
	};
}
