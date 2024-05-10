#pragma once

#include "Core_Entity.h"
#include "Render_Fonts.h"

#include <functional>

struct GLFWwindow;

namespace Render
{
	class Gui;

	struct EntityGuiComponent
	{
		EntityGuiComponent(GLFWwindow* aWindow, bool aMenuBar);
		~EntityGuiComponent();

		void Update();
		ImFont* GetFont(FontType aFontType) const;

		GLFWwindow* myWindow = nullptr;
		Gui* myGui = nullptr;
		std::function<void()> myCallback = nullptr;
	};

	class Model;

	struct EntityModelComponent
	{
		virtual ~EntityModelComponent();

		virtual void Load() = 0;
		void Unload();
		void Update(const glm::mat4& aMatrix);

		bool myIsTransparent = false;

		Model* myModel = nullptr;
	};

	struct EntitySimpleGeometryModelComponent : EntityModelComponent
	{
		void Load() override;

		struct Vertex
		{
			glm::vec3 myPosition;
			glm::vec3 myNormal;
			glm::vec2 myUV;
			glm::vec4 myColor;
		};

		enum class Preset
		{
			VectorBaseWidget,
			Square,
			Cube,
			Disc,
			Sphere,
			Panda,
		};
		void FillWithPreset(Preset aPreset);
		void FillVectorBaseWidget();
		void FillSquare();
		void FillCube();
		void FillDisc();
		void FillSphere();
		void FillPanda();

		std::vector<Vertex> myVertices;
		std::vector<uint> myIndices;
		// TODO : Replace with a TextureHandle so we request RenderCore to load textures and can use RenderTargets as textures
		std::string myTextureFilename;
	};

	struct EntityglTFModelComponent : EntityModelComponent
	{
		void Load() override;

		std::string myFilename;
		bool myIsAnimated = false;
	};
}
