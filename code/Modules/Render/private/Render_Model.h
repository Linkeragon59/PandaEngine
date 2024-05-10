#pragma once

#include "Render_ShaderHelpers.h"
#include "Render_EntityRenderComponent.h"

namespace Render
{
	class Model
	{
	public:
		virtual ~Model() {};
		virtual void Update(const glm::mat4& aMatrix) = 0;
		virtual void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, const std::function<VkDescriptorSet(const ShaderHelpers::DescriptorInfo&)>& myDescriptorSetGetter) = 0;
	};

	class SimpleGeometryModel : public Model
	{
	public:
		SimpleGeometryModel(const std::vector<EntitySimpleGeometryModelComponent::Vertex>& someVertices, const std::vector<uint>& someIndices, const std::string& aTextureFilename);

		void Update(const glm::mat4& aMatrix) override;
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, const std::function<VkDescriptorSet(const ShaderHelpers::DescriptorInfo&)>& myDescriptorSetGetter) override;

	private:
		BufferPtr myVertexBuffer;
		BufferPtr myIndexBuffer;
		uint myIndexCount = 0;

		BufferPtr myUBOObject;
		ImagePtr myTexture;
	};
}