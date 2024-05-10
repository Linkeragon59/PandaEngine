#include "Render_EntityRenderComponent.h"

#include "Render_Gui.h"
#include "Render_Model.h"
#include "Render_glTFModel.h"

namespace Render
{
	EntityGuiComponent::EntityGuiComponent(GLFWwindow* aWindow, bool aMenuBar)
		: myWindow(aWindow)
	{
		myGui = new Gui(myWindow, aMenuBar);
	}

	EntityGuiComponent::~EntityGuiComponent()
	{
		SafeDelete(myGui);
	}

	void EntityGuiComponent::Update()
	{
		myGui->Update(myCallback);
	}

	ImFont* EntityGuiComponent::GetFont(FontType aFontType) const
	{
		return myGui->GetFont(aFontType);
	}

	EntityModelComponent::~EntityModelComponent()
	{
		Unload();
	}

	void EntityModelComponent::Unload()
	{
		SafeDelete(myModel);
	}

	void EntityModelComponent::Update(const glm::mat4& aMatrix)
	{
		myModel->Update(aMatrix);
	}

	void EntitySimpleGeometryModelComponent::Load()
	{
		myModel = new SimpleGeometryModel(myVertices, myIndices, myTextureFilename);
	}

	void EntitySimpleGeometryModelComponent::FillWithPreset(Preset aPreset)
	{
		switch (aPreset)
		{
		case EntitySimpleGeometryModelComponent::Preset::VectorBaseWidget:
			FillVectorBaseWidget();
			break;
		case EntitySimpleGeometryModelComponent::Preset::Square:
			FillSquare();
			break;
		case EntitySimpleGeometryModelComponent::Preset::Cube:
			FillCube();
			break;
		case EntitySimpleGeometryModelComponent::Preset::Disc:
			FillDisc();
			break;
		case EntitySimpleGeometryModelComponent::Preset::Sphere:
			FillSphere();
			break;
		case EntitySimpleGeometryModelComponent::Preset::Panda:
			FillPanda();
			break;
		default:
			Assert(false, "Unsupported preset");
			break;
		}
	}

	void EntitySimpleGeometryModelComponent::FillVectorBaseWidget()
	{
		const float arrowLength = 0.9f;
		const float spikeLength = 0.1f;
		const float arrowDiameter = 0.01f;
		const uint arrowPolyPrecision = 8;

		const uint verticesPerArrow = 3 * arrowPolyPrecision + 1;
		const uint trianglesPerArrow = 5 * arrowPolyPrecision;

		// 3 arrows
		myVertices.resize(3 * verticesPerArrow);
		myIndices.resize(9 * trianglesPerArrow);

		// Vertices

		// Arrows tubes
		for (uint i = 0; i < arrowPolyPrecision; ++i)
		{
			const float sin = arrowDiameter * std::sin(i * 2.0f * 3.1416f / arrowPolyPrecision);
			const float cos = arrowDiameter * std::cos(i * 2.0f * 3.1416f / arrowPolyPrecision);

			const float doubleSin = 2.0f * sin;
			const float doubleCos = 2.0f * cos;

			// x
			myVertices[i].myPosition = glm::vec3(0.0f, sin, cos);
			myVertices[i].myNormal = glm::vec3(0.0f);
			myVertices[i].myUV = glm::vec2(0.0f);
			myVertices[i].myColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			myVertices[i + arrowPolyPrecision].myPosition = glm::vec3(arrowLength, sin, cos);
			myVertices[i + arrowPolyPrecision].myNormal = glm::vec3(0.0f);
			myVertices[i + arrowPolyPrecision].myUV = glm::vec2(0.0f);
			myVertices[i + arrowPolyPrecision].myColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			myVertices[i + 2 * arrowPolyPrecision].myPosition = glm::vec3(arrowLength, doubleSin, doubleCos);
			myVertices[i + 2 * arrowPolyPrecision].myNormal = glm::vec3(0.0f);
			myVertices[i + 2 * arrowPolyPrecision].myUV = glm::vec2(0.0f);
			myVertices[i + 2 * arrowPolyPrecision].myColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

			// y
			myVertices[i + verticesPerArrow].myPosition = glm::vec3(cos, 0.0f, sin);
			myVertices[i + verticesPerArrow].myNormal = glm::vec3(0.0f);
			myVertices[i + verticesPerArrow].myUV = glm::vec2(0.0f);
			myVertices[i + verticesPerArrow].myColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			myVertices[i + verticesPerArrow + arrowPolyPrecision].myPosition = glm::vec3(cos, arrowLength, sin);
			myVertices[i + verticesPerArrow + arrowPolyPrecision].myNormal = glm::vec3(0.0f);
			myVertices[i + verticesPerArrow + arrowPolyPrecision].myUV = glm::vec2(0.0f);
			myVertices[i + verticesPerArrow + arrowPolyPrecision].myColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			myVertices[i + verticesPerArrow + 2 * arrowPolyPrecision].myPosition = glm::vec3(doubleCos, arrowLength, doubleSin);
			myVertices[i + verticesPerArrow + 2 * arrowPolyPrecision].myNormal = glm::vec3(0.0f);
			myVertices[i + verticesPerArrow + 2 * arrowPolyPrecision].myUV = glm::vec2(0.0f);
			myVertices[i + verticesPerArrow + 2 * arrowPolyPrecision].myColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

			// z
			myVertices[i + 2 * verticesPerArrow].myPosition = glm::vec3(sin, cos, 0.0f);
			myVertices[i + 2 * verticesPerArrow].myNormal = glm::vec3(0.0f);
			myVertices[i + 2 * verticesPerArrow].myUV = glm::vec2(0.0f);
			myVertices[i + 2 * verticesPerArrow].myColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			myVertices[i + 2 * verticesPerArrow + arrowPolyPrecision].myPosition = glm::vec3(sin, cos, arrowLength);
			myVertices[i + 2 * verticesPerArrow + arrowPolyPrecision].myNormal = glm::vec3(0.0f);
			myVertices[i + 2 * verticesPerArrow + arrowPolyPrecision].myUV = glm::vec2(0.0f);
			myVertices[i + 2 * verticesPerArrow + arrowPolyPrecision].myColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			myVertices[i + 2 * verticesPerArrow + 2 * arrowPolyPrecision].myPosition = glm::vec3(doubleSin, doubleCos, arrowLength);
			myVertices[i + 2 * verticesPerArrow + 2 * arrowPolyPrecision].myNormal = glm::vec3(0.0f);
			myVertices[i + 2 * verticesPerArrow + 2 * arrowPolyPrecision].myUV = glm::vec2(0.0f);
			myVertices[i + 2 * verticesPerArrow + 2 * arrowPolyPrecision].myColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		}

		// Arrows spikes
		// x
		myVertices[3 * arrowPolyPrecision].myPosition = glm::vec3(arrowLength + spikeLength, 0.0f, 0.0f);
		myVertices[3 * arrowPolyPrecision].myNormal = glm::vec3(0.0f);
		myVertices[3 * arrowPolyPrecision].myUV = glm::vec2(0.0f);
		myVertices[3 * arrowPolyPrecision].myColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		// y
		myVertices[verticesPerArrow + 3 * arrowPolyPrecision].myPosition = glm::vec3(0.0f, arrowLength + spikeLength, 0.0f);
		myVertices[verticesPerArrow + 3 * arrowPolyPrecision].myNormal = glm::vec3(0.0f);
		myVertices[verticesPerArrow + 3 * arrowPolyPrecision].myUV = glm::vec2(0.0f);
		myVertices[verticesPerArrow + 3 * arrowPolyPrecision].myColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		// z
		myVertices[2 * verticesPerArrow + 3 * arrowPolyPrecision].myPosition = glm::vec3(0.0f, 0.0f, arrowLength + spikeLength);
		myVertices[2 * verticesPerArrow + 3 * arrowPolyPrecision].myNormal = glm::vec3(0.0f);
		myVertices[2 * verticesPerArrow + 3 * arrowPolyPrecision].myUV = glm::vec2(0.0f);
		myVertices[2 * verticesPerArrow + 3 * arrowPolyPrecision].myColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

		// Indices

		for (uint i = 0; i < arrowPolyPrecision; ++i)
		{
			// Arrows tubes
			// x
			myIndices[3 * i] = i;
			myIndices[3 * i + 1] = (i + 1) % arrowPolyPrecision;
			myIndices[3 * i + 2] = i + arrowPolyPrecision;

			myIndices[3 * (i + arrowPolyPrecision)] = i + arrowPolyPrecision;
			myIndices[3 * (i + arrowPolyPrecision) + 1] = (i + 1) % arrowPolyPrecision;
			myIndices[3 * (i + arrowPolyPrecision) + 2] = (i + 1) % arrowPolyPrecision + arrowPolyPrecision;

			myIndices[3 * (i + 2 * arrowPolyPrecision)] = i + arrowPolyPrecision;
			myIndices[3 * (i + 2 * arrowPolyPrecision) + 1] = (i + 1) % arrowPolyPrecision + arrowPolyPrecision;
			myIndices[3 * (i + 2 * arrowPolyPrecision) + 2] = (i + 1) % arrowPolyPrecision + 2 * arrowPolyPrecision;

			myIndices[3 * (i + 3 * arrowPolyPrecision)] = i + arrowPolyPrecision;
			myIndices[3 * (i + 3 * arrowPolyPrecision) + 1] = (i + 1) % arrowPolyPrecision + 2 * arrowPolyPrecision;
			myIndices[3 * (i + 3 * arrowPolyPrecision) + 2] = i + 2 * arrowPolyPrecision;

			// Arrows spikes
			// x
			myIndices[3 * (i + 4 * arrowPolyPrecision)] = i + 2 * arrowPolyPrecision;
			myIndices[3 * (i + 4 * arrowPolyPrecision) + 1] = (i + 1) % arrowPolyPrecision + 2 * arrowPolyPrecision;
			myIndices[3 * (i + 4 * arrowPolyPrecision) + 2] = 3 * arrowPolyPrecision;
		}

		for (uint i = 0; i < 3 * trianglesPerArrow; ++i)
		{
			// y
			myIndices[i + 3 * trianglesPerArrow] = myIndices[i] + verticesPerArrow;
			// z
			myIndices[i + 6 * trianglesPerArrow] = myIndices[i] + 2 * verticesPerArrow;
		}
	}

	void EntitySimpleGeometryModelComponent::FillSquare()
	{
		myVertices.resize(4);

		myVertices[0].myPosition = glm::vec3(-0.5f, 0.5f, 0.0f);
		myVertices[0].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		myVertices[0].myColor = glm::vec4(1.0f);
		myVertices[0].myUV = glm::vec2(0.0f, 0.0f);
		myVertices[1].myPosition = glm::vec3(-0.5f, -0.5f, 0.0f);
		myVertices[1].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		myVertices[1].myColor = glm::vec4(1.0f);
		myVertices[1].myUV = glm::vec2(0.0f, 1.0f);
		myVertices[2].myPosition = glm::vec3(0.5f, -0.5f, 0.0f);
		myVertices[2].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		myVertices[2].myColor = glm::vec4(1.0f);
		myVertices[2].myUV = glm::vec2(1.0f, 1.0f);
		myVertices[3].myPosition = glm::vec3(0.5f, 0.5f, 0.0f);
		myVertices[3].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		myVertices[3].myColor = glm::vec4(1.0f);
		myVertices[3].myUV = glm::vec2(1.0f, 0.0f);

		myIndices.resize(6);

		myIndices[0] = 0;
		myIndices[1] = 1;
		myIndices[2] = 2;
		myIndices[3] = 0;
		myIndices[4] = 2;
		myIndices[5] = 3;
	}

	void EntitySimpleGeometryModelComponent::FillCube()
	{
		// TODO: I'm sure this can be written more efficiently, I just wanted to get some intuition by writing it explicitly

		// each vertex must be defined 3 times with a different normal and uv for the 3 faces it belongs to
		myVertices.resize(24);

		// face up
		myVertices[0].myPosition = glm::vec3(-0.5f, 0.5f, -0.5f);
		myVertices[0].myNormal = glm::vec3(0.0f, 1.0f, 0.0f);
		myVertices[0].myColor = glm::vec4(1.0f);
		myVertices[0].myUV = glm::vec2(0.0f, 0.0f);
		myVertices[1].myPosition = glm::vec3(-0.5f, 0.5f, 0.5f);
		myVertices[1].myNormal = glm::vec3(0.0f, 1.0f, 0.0f);
		myVertices[1].myColor = glm::vec4(1.0f);
		myVertices[1].myUV = glm::vec2(0.0f, 1.0f);
		myVertices[2].myPosition = glm::vec3(0.5f, 0.5f, 0.5f);
		myVertices[2].myNormal = glm::vec3(0.0f, 1.0f, 0.0f);
		myVertices[2].myColor = glm::vec4(1.0f);
		myVertices[2].myUV = glm::vec2(1.0f, 1.0f);
		myVertices[3].myPosition = glm::vec3(0.5f, 0.5f, -0.5f);
		myVertices[3].myNormal = glm::vec3(0.0f, 1.0f, 0.0f);
		myVertices[3].myColor = glm::vec4(1.0f);
		myVertices[3].myUV = glm::vec2(1.0f, 0.0f);

		// face left
		myVertices[4].myPosition = glm::vec3(-0.5f, 0.5f, -0.5f);
		myVertices[4].myNormal = glm::vec3(-1.0f, 0.0f, 0.0f);
		myVertices[4].myColor = glm::vec4(1.0f);
		myVertices[4].myUV = glm::vec2(0.0f, 0.0f);
		myVertices[5].myPosition = glm::vec3(-0.5f, -0.5f, -0.5f);
		myVertices[5].myNormal = glm::vec3(-1.0f, 0.0f, 0.0f);
		myVertices[5].myColor = glm::vec4(1.0f);
		myVertices[5].myUV = glm::vec2(0.0f, 1.0f);
		myVertices[6].myPosition = glm::vec3(-0.5f, -0.5f, 0.5f);
		myVertices[6].myNormal = glm::vec3(-1.0f, 0.0f, 0.0f);
		myVertices[6].myColor = glm::vec4(1.0f);
		myVertices[6].myUV = glm::vec2(1.0f, 1.0f);
		myVertices[7].myPosition = glm::vec3(-0.5f, 0.5f, 0.5f);
		myVertices[7].myNormal = glm::vec3(-1.0f, 0.0f, 0.0f);
		myVertices[7].myColor = glm::vec4(1.0f);
		myVertices[7].myUV = glm::vec2(1.0f, 0.0f);

		// face front
		myVertices[8].myPosition = glm::vec3(-0.5f, 0.5f, 0.5f);
		myVertices[8].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		myVertices[8].myColor = glm::vec4(1.0f);
		myVertices[8].myUV = glm::vec2(0.0f, 0.0f);
		myVertices[9].myPosition = glm::vec3(-0.5f, -0.5f, 0.5f);
		myVertices[9].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		myVertices[9].myColor = glm::vec4(1.0f);
		myVertices[9].myUV = glm::vec2(0.0f, 1.0f);
		myVertices[10].myPosition = glm::vec3(0.5f, -0.5f, 0.5f);
		myVertices[10].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		myVertices[10].myColor = glm::vec4(1.0f);
		myVertices[10].myUV = glm::vec2(1.0f, 1.0f);
		myVertices[11].myPosition = glm::vec3(0.5f, 0.5f, 0.5f);
		myVertices[11].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		myVertices[11].myColor = glm::vec4(1.0f);
		myVertices[11].myUV = glm::vec2(1.0f, 0.0f);

		// face right
		myVertices[12].myPosition = glm::vec3(0.5f, 0.5f, 0.5f);
		myVertices[12].myNormal = glm::vec3(1.0f, 0.0f, 0.0f);
		myVertices[12].myColor = glm::vec4(1.0f);
		myVertices[12].myUV = glm::vec2(0.0f, 0.0f);
		myVertices[13].myPosition = glm::vec3(0.5f, -0.5f, 0.5f);
		myVertices[13].myNormal = glm::vec3(1.0f, 0.0f, 0.0f);
		myVertices[13].myColor = glm::vec4(1.0f);
		myVertices[13].myUV = glm::vec2(0.0f, 1.0f);
		myVertices[14].myPosition = glm::vec3(0.5f, -0.5f, -0.5f);
		myVertices[14].myNormal = glm::vec3(1.0f, 0.0f, 0.0f);
		myVertices[14].myColor = glm::vec4(1.0f);
		myVertices[14].myUV = glm::vec2(1.0f, 1.0f);
		myVertices[15].myPosition = glm::vec3(0.5f, 0.5f, -0.5f);
		myVertices[15].myNormal = glm::vec3(1.0f, 0.0f, 0.0f);
		myVertices[15].myColor = glm::vec4(1.0f);
		myVertices[15].myUV = glm::vec2(1.0f, 0.0f);

		// face down
		myVertices[16].myPosition = glm::vec3(-0.5f, -0.5f, 0.5f);
		myVertices[16].myNormal = glm::vec3(0.0f, -1.0f, 0.0f);
		myVertices[16].myColor = glm::vec4(1.0f);
		myVertices[16].myUV = glm::vec2(0.0f, 0.0f);
		myVertices[17].myPosition = glm::vec3(-0.5f, -0.5f, -0.5f);
		myVertices[17].myNormal = glm::vec3(0.0f, -1.0f, 0.0f);
		myVertices[17].myColor = glm::vec4(1.0f);
		myVertices[17].myUV = glm::vec2(0.0f, 1.0f);
		myVertices[18].myPosition = glm::vec3(0.5f, -0.5f, -0.5f);
		myVertices[18].myNormal = glm::vec3(0.0f, -1.0f, 0.0f);
		myVertices[18].myColor = glm::vec4(1.0f);
		myVertices[18].myUV = glm::vec2(1.0f, 1.0f);
		myVertices[19].myPosition = glm::vec3(0.5f, -0.5f, 0.5f);
		myVertices[19].myNormal = glm::vec3(0.0f, -1.0f, 0.0f);
		myVertices[19].myColor = glm::vec4(1.0f);
		myVertices[19].myUV = glm::vec2(1.0f, 0.0f);

		// face back
		myVertices[20].myPosition = glm::vec3(-0.5f, -0.5f, -0.5f);
		myVertices[20].myNormal = glm::vec3(0.0f, 0.0f, -1.0f);
		myVertices[20].myColor = glm::vec4(1.0f);
		myVertices[20].myUV = glm::vec2(0.0f, 0.0f);
		myVertices[21].myPosition = glm::vec3(-0.5f, 0.5f, -0.5f);
		myVertices[21].myNormal = glm::vec3(0.0f, 0.0f, -1.0f);
		myVertices[21].myColor = glm::vec4(1.0f);
		myVertices[21].myUV = glm::vec2(0.0f, 1.0f);
		myVertices[22].myPosition = glm::vec3(0.5f, 0.5f, -0.5f);
		myVertices[22].myNormal = glm::vec3(0.0f, 0.0f, -1.0f);
		myVertices[22].myColor = glm::vec4(1.0f);
		myVertices[22].myUV = glm::vec2(1.0f, 1.0f);
		myVertices[23].myPosition = glm::vec3(0.5f, -0.5f, -0.5f);
		myVertices[23].myNormal = glm::vec3(0.0f, 0.0f, -1.0f);
		myVertices[23].myColor = glm::vec4(1.0f);
		myVertices[23].myUV = glm::vec2(1.0f, 0.0f);

		myIndices.resize(36);

		for (uint i = 0; i < 6; ++i)
		{
			myIndices[i * 6 + 0] = i * 4 + 0;
			myIndices[i * 6 + 1] = i * 4 + 1;
			myIndices[i * 6 + 2] = i * 4 + 2;
			myIndices[i * 6 + 3] = i * 4 + 0;
			myIndices[i * 6 + 4] = i * 4 + 2;
			myIndices[i * 6 + 5] = i * 4 + 3;
		}
	}

	void EntitySimpleGeometryModelComponent::FillDisc()
	{
		const uint polyPrecision = 60;

		myVertices.resize(polyPrecision + 1);
		myIndices.resize(3 * polyPrecision);

		for (uint i = 0; i < polyPrecision; ++i)
		{
			const float cos = std::cos(i * 2.0f * 3.1416f / polyPrecision);
			const float sin = std::sin(i * 2.0f * 3.1416f / polyPrecision);
			myVertices[i].myPosition = glm::vec3(cos, sin, 0.0f);
			myVertices[i].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
			myVertices[i].myColor = glm::vec4(1.0f);
			myVertices[i].myUV = 0.5f * glm::vec2(1.0f + cos, 1.0f - sin);

			myIndices[3 * i + 0] = i;
			myIndices[3 * i + 1] = (i + 1) % polyPrecision;
			myIndices[3 * i + 2] = polyPrecision;
		}
		myVertices[polyPrecision].myPosition = glm::vec3(0.0f);
		myVertices[polyPrecision].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		myVertices[polyPrecision].myColor = glm::vec4(1.0f);
		myVertices[polyPrecision].myUV = glm::vec2(0.5f, 0.5f);
	}

	void EntitySimpleGeometryModelComponent::FillSphere()
	{
		const uint longitudePolyPrecision = 50;
		const uint latitudePolyPrecision = 50;

		uint verticesCount = (longitudePolyPrecision + 1) * (latitudePolyPrecision + 1);
		myVertices.resize(verticesCount);
		myIndices.resize(6 * longitudePolyPrecision * latitudePolyPrecision);

		for (uint i = 0; i < latitudePolyPrecision + 1; ++i)
		{
			for (uint j = 0; j < longitudePolyPrecision + 1; ++j)
			{
				float longitude01 = (float)j / longitudePolyPrecision;
				float latitude01 = (float)i / latitudePolyPrecision;
				float cosLongitude = std::cos(2.0f * 3.1416f * longitude01);
				float sinLongitude = std::sin(2.0f * 3.1416f * longitude01);
				float cosLatitude = std::cos(3.1416f * (latitude01 - 0.5f));
				float sinLatitude = std::sin(3.1416f * (latitude01 - 0.5f));

				uint vertex = i * (longitudePolyPrecision + 1) + j;
				myVertices[vertex].myPosition = glm::vec3(cosLongitude * cosLatitude, sinLatitude, sinLongitude * cosLatitude);
				myVertices[vertex].myUV = glm::vec2(1 - longitude01, 1 - latitude01);

				if (i < latitudePolyPrecision && j < longitudePolyPrecision)
				{
					uint index = 6 * (i * longitudePolyPrecision + j);
					myIndices[index + 0] = i * (longitudePolyPrecision + 1) + (j + 1) % (longitudePolyPrecision + 1);
					myIndices[index + 1] = i * (longitudePolyPrecision + 1) + j;
					myIndices[index + 2] = (i + 1) * (longitudePolyPrecision + 1) + (j + 1) % (longitudePolyPrecision + 1);

					myIndices[index + 3] = i * (longitudePolyPrecision + 1) + j;
					myIndices[index + 4] = (i + 1) * (longitudePolyPrecision + 1) + j;
					myIndices[index + 5] = (i + 1) * (longitudePolyPrecision + 1) + (j + 1) % (longitudePolyPrecision + 1);
				}
			}
		}

		// Fill vertices
		for (uint i = 0; i < verticesCount; ++i)
		{
			myVertices[i].myNormal = myVertices[i].myPosition;
			myVertices[i].myColor = glm::vec4(1.0f);
		}
	}

	void EntitySimpleGeometryModelComponent::FillPanda()
	{
		myVertices = {
		{ {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f} },
		{ {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f} },
		{ {0.87f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.065f, 0.25f}, {0.0f, 1.0f, 0.0f, 1.0f} },
		{ {0.87f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.065f, 0.75f}, {0.0f, 1.0f, 1.0f, 1.0f} },
		{ {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} },
		{ {-0.87f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.935f, 0.75f}, {1.0f, 0.0f, 1.0f, 1.0f} },
		{ {-0.87f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.935f, 0.25f}, {1.0f, 0.0f, 0.0f, 1.0f} },

		{ {0.0f, 0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f} },
		{ {0.0f, -1.0f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f} },
		{ {0.87f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.065f, 0.25f}, {0.0f, 1.0f, 0.0f, 1.0f} },
		{ {0.87f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.065f, 0.75f}, {0.0f, 1.0f, 1.0f, 1.0f} },
		{ {0.0f, 1.0f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} },
		{ {-0.87f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.935f, 0.75f}, {1.0f, 0.0f, 1.0f, 1.0f} },
		{ {-0.87f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.935f, 0.25f}, {1.0f, 0.0f, 0.0f, 1.0f} }
		};

		myIndices = {
		0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 1,

		7, 8, 9, 7, 9, 10, 7, 10, 11, 7, 11, 12, 7, 12, 13, 7, 13, 8
		};

		myTextureFilename = "textures/panda.jpg";
	}

	void EntityglTFModelComponent::Load()
	{
		myModel = new glTFModel(myFilename);
	}
}
