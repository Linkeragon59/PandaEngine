#include "Render_ShaderHelpers.h"

#include "Core_File.h"

namespace Render::ShaderHelpers
{
	VkVertexInputBindingDescription Vertex::GetBindingDescription(uint aBinding /*= 0*/)
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = aBinding;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	VkVertexInputAttributeDescription Vertex::GetAttributeDescription(VertexComponent aComponent, uint aLocation /*= 0*/, uint aBinding /*= 0*/)
	{
		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.location = aLocation;
		attributeDescription.binding = aBinding;
		switch (aComponent)
		{
		case VertexComponent::Position:
			attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myPosition);
			break;
		case VertexComponent::Normal:
			attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myNormal);
			break;
		case VertexComponent::UV:
			attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myUV);
			break;
		case VertexComponent::Color:
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myColor);
			break;
		case VertexComponent::Joint:
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myJoint);
			break;
		case VertexComponent::Weight:
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myWeight);
			break;
		case VertexComponent::Tangent:
			attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescription.offset = offsetof(Vertex, myTangent);
			break;
		}
		return attributeDescription;
	}

	std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions(const std::vector<VertexComponent> someComponents, uint aBinding /*= 0*/)
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.reserve(someComponents.size());
		for (uint i = 0; i < (uint)someComponents.size(); ++i)
			attributeDescriptions.push_back(GetAttributeDescription(someComponents[i], i, aBinding));
		return attributeDescriptions;
	}

	VkDescriptorSetLayout CreateDescriptorSetLayout(BindingType aBindingType)
	{
		VkDevice device = RenderCore::GetInstance()->GetDevice();
		VkDescriptorSetLayout layout = VK_NULL_HANDLE;

		switch (aBindingType)
		{
			case BindingType::Camera:
			{
				std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
				// Binding 0 : ViewProj
				bindings[0].binding = 0;
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bindings[0].descriptorCount = 1;
				bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

				VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
				descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorLayoutInfo.bindingCount = (uint)bindings.size();
				descriptorLayoutInfo.pBindings = bindings.data();

				VK_CHECK_RESULT(
					vkCreateDescriptorSetLayout(device, &descriptorLayoutInfo, nullptr, &layout),
					"Failed to create the Camera DescriptorSetLayout");

				break;
			}
			case BindingType::SimpleObject:
			{
				std::array<VkDescriptorSetLayoutBinding, 2> bindings{};
				// Binding 0 : Model
				bindings[0].binding = 0;
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bindings[0].descriptorCount = 1;
				bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				// Binding 1 : Texture Sampler
				bindings[1].binding = 1;
				bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bindings[1].descriptorCount = 1;
				bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

				VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
				descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorLayoutInfo.bindingCount = (uint)bindings.size();
				descriptorLayoutInfo.pBindings = bindings.data();

				VK_CHECK_RESULT(
					vkCreateDescriptorSetLayout(device, &descriptorLayoutInfo, nullptr, &layout),
					"Failed to create the SimpleObject DescriptorSetLayout");

				break;
			}
			case BindingType::Object:
			{
				std::array<VkDescriptorSetLayoutBinding, 4> bindings{};
				// Binding 0 : Model
				bindings[0].binding = 0;
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bindings[0].descriptorCount = 1;
				bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				// Binding 1 : Texture Sampler
				bindings[1].binding = 1;
				bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bindings[1].descriptorCount = 1;
				bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				// Binding 2 : Material
				bindings[2].binding = 2;
				bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				bindings[2].descriptorCount = 1;
				bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				// Binding 3 : Joint Matrices
				bindings[3].binding = 3;
				bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				bindings[3].descriptorCount = 1;
				bindings[3].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

				VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
				descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorLayoutInfo.bindingCount = (uint)bindings.size();
				descriptorLayoutInfo.pBindings = bindings.data();

				VK_CHECK_RESULT(
					vkCreateDescriptorSetLayout(device, &descriptorLayoutInfo, nullptr, &layout),
					"Failed to create the Object DescriptorSetLayout");

				break;
			}
			case BindingType::LightsSet:
			{
				std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
				// Binding 0 : Lights
				bindings[0].binding = 0;
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bindings[0].descriptorCount = 1;
				bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

				VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
				descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorLayoutInfo.bindingCount = (uint)bindings.size();
				descriptorLayoutInfo.pBindings = bindings.data();

				VK_CHECK_RESULT(
					vkCreateDescriptorSetLayout(device, &descriptorLayoutInfo, nullptr, &layout),
					"Failed to create the LightsSet DescriptorSetLayout");

				break;
			}
			case BindingType::Gui:
			{
				std::array<VkDescriptorSetLayoutBinding, 1> bindings{};
				// Binding 0 : Font Sampler
				bindings[0].binding = 0;
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bindings[0].descriptorCount = 1;
				bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

				VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo{};
				descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorLayoutInfo.bindingCount = (uint)bindings.size();
				descriptorLayoutInfo.pBindings = bindings.data();

				VK_CHECK_RESULT(
					vkCreateDescriptorSetLayout(device, &descriptorLayoutInfo, nullptr, &layout),
					"Failed to create the Gui DescriptorSetLayout");

				break;
			}
			default:
			{
				Assert(false, "Unsupported BindingType");
				break;
			}
		}

		return layout;
	}

	void UpdateDescriptorSet(VkDescriptorSet aDescriptorSet, BindingType aBindingType, const ShaderHelpers::DescriptorInfo& someDescriptorInfo)
	{
		VkDevice device = RenderCore::GetInstance()->GetDevice();

		switch (aBindingType)
		{
			case BindingType::Camera:
			{
				const ShaderHelpers::CameraDescriptorInfo& info = static_cast<const ShaderHelpers::CameraDescriptorInfo&>(someDescriptorInfo);

				std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
				// Binding 0 : ViewProj
				writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[0].descriptorCount = 1;
				writeDescriptorSets[0].dstSet = aDescriptorSet;
				writeDescriptorSets[0].dstBinding = 0;
				writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSets[0].pBufferInfo = info.myViewProjMatricesInfo;

				vkUpdateDescriptorSets(device, (uint)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);

				break;
			}
			case BindingType::SimpleObject:
			{
				const ShaderHelpers::ObjectDescriptorInfo& info = static_cast<const ShaderHelpers::ObjectDescriptorInfo&>(someDescriptorInfo);
			
				std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
				// Binding 0 : Model
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].dstSet = aDescriptorSet;
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].pBufferInfo = info.myModelMatrixInfo;
				// Binding 1 : Texture Sampler
				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].dstSet = aDescriptorSet;
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].pImageInfo = info.myImageSamplerInfo ? info.myImageSamplerInfo : RenderCore::GetInstance()->GetWhiteTextureDescriptorInfo();
			
				vkUpdateDescriptorSets(device, static_cast<uint>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

				break;
			}
			case BindingType::Object:
			{
				const ShaderHelpers::ObjectDescriptorInfo& info = static_cast<const ShaderHelpers::ObjectDescriptorInfo&>(someDescriptorInfo);
			
				std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
				// Binding 0 : Model
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].dstSet = aDescriptorSet;
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].pBufferInfo = info.myModelMatrixInfo;
				// Binding 1 : Texture Sampler
				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].dstSet = aDescriptorSet;
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].pImageInfo = info.myImageSamplerInfo ? info.myImageSamplerInfo : RenderCore::GetInstance()->GetWhiteTextureDescriptorInfo();
				// Binding 2 : Material
				descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[2].descriptorCount = 1;
				descriptorWrites[2].dstSet = aDescriptorSet;
				descriptorWrites[2].dstBinding = 2;
				descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorWrites[2].pBufferInfo = info.myMaterialInfo ? info.myMaterialInfo : RenderCore::GetInstance()->GetDefaultMaterialDescriptorInfo();
				// Binding 3 : Joint Matrices
				descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[3].descriptorCount = 1;
				descriptorWrites[3].dstSet = aDescriptorSet;
				descriptorWrites[3].dstBinding = 3;
				descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorWrites[3].pBufferInfo = info.myJointMatricesInfo ? info.myJointMatricesInfo : RenderCore::GetInstance()->GetDefaultJointsMatrixDescriptorInfo();
			
				vkUpdateDescriptorSets(device, static_cast<uint>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

				break;
			}
			case BindingType::LightsSet:
			{
				const ShaderHelpers::LightsSetDescriptorInfo& info = static_cast<const ShaderHelpers::LightsSetDescriptorInfo&>(someDescriptorInfo);
			
				std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
				// Binding 0 : Lights
				writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[0].descriptorCount = 1;
				writeDescriptorSets[0].dstSet = aDescriptorSet;
				writeDescriptorSets[0].dstBinding = 0;
				writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSets[0].pBufferInfo = info.myLightsInfo;
			
				vkUpdateDescriptorSets(device, static_cast<uint>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

				break;
			}
			case BindingType::Gui:
			{
				const ShaderHelpers::GuiDescriptorInfo& info = static_cast<const ShaderHelpers::GuiDescriptorInfo&>(someDescriptorInfo);
			
				std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
				// Binding 0 : Font Sampler
				writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[0].descriptorCount = 1;
				writeDescriptorSets[0].dstSet = aDescriptorSet;
				writeDescriptorSets[0].dstBinding = 0;
				writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[0].pImageInfo = info.myFontSamplerInfo;
			
				vkUpdateDescriptorSets(device, static_cast<uint>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

				break;
			}
			default:
			{
				Assert(false, "Unsupported Descriptor Type");
				break;
			}
		}
	}

	VkShaderModule CreateShaderModule(const std::string& aFilename)
	{
		VkShaderModule shaderModule;

		std::vector<char> shaderCode;
		Verify(FileHelpers::ReadAsBuffer(aFilename, shaderCode), "Couldn't read shader file: %s", aFilename.c_str());

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint*>(shaderCode.data());

		VK_CHECK_RESULT(vkCreateShaderModule(RenderCore::GetInstance()->GetDevice(), &createInfo, nullptr, &shaderModule), "Failed to create a module shader!");

		return shaderModule;
	}
}
