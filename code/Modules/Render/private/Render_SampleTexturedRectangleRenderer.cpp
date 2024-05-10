#include "Render_SampleTexturedRectangleRenderer.h"

#include "Render_Device.h"
#include "Render_Camera.h"
#include "Render_Model.h"
#include "Render_ShaderHelpers.h"

namespace Render
{
	void SampleTexturedRectangleRenderer::SetupInternal()
	{
		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), myRenderTarget.GetExtent().width / (float)myRenderTarget.GetExtent().height, 0.1f, 256.0f);
		proj[1][1] *= -1.0f;
		SetViewProj(view, proj);
	
		SetupSimpleModel();
	}
	
	void SampleTexturedRectangleRenderer::CleanupInternal()
	{
		SafeDelete(myModel);
		SafeDelete(myModel2);
	}
	
	void SampleTexturedRectangleRenderer::StartFrameInternal()
	{
		vkCmdBindPipeline(myCurrentSecondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myPipeline);
		myCamera->Bind(myCurrentSecondaryCommandBuffer, myPipelineLayout, 0, [this](const ShaderHelpers::DescriptorInfo& someInfo) {
			VkDescriptorSet cameraSet = GetFreeDescriptorSet(myCameraSetLayout);
			ShaderHelpers::UpdateDescriptorSet(cameraSet, ShaderHelpers::BindingType::Camera, someInfo);
			return cameraSet;
		});
	
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)myRenderTarget.GetExtent().width;
		viewport.height = (float)myRenderTarget.GetExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(myCurrentSecondaryCommandBuffer, 0, 1, &viewport);
	
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = myRenderTarget.GetExtent();
		vkCmdSetScissor(myCurrentSecondaryCommandBuffer, 0, 1, &scissor);
	}
	
	void SampleTexturedRectangleRenderer::EndFrameInternal()
	{
		auto lambda = [this](const ShaderHelpers::DescriptorInfo& someInfo) {
			VkDescriptorSet objectSet = GetFreeDescriptorSet(mySimpleObjectSetLayout);
			ShaderHelpers::UpdateDescriptorSet(objectSet, ShaderHelpers::BindingType::SimpleObject, someInfo);
			return objectSet;
		};
		myModel->Draw(myCurrentSecondaryCommandBuffer, myPipelineLayout, 1, lambda);
		myModel2->Draw(myCurrentSecondaryCommandBuffer, myPipelineLayout, 1, lambda);
	}
	
	void SampleTexturedRectangleRenderer::SetupDescriptorSets()
	{
		myCameraSetLayout = ShaderHelpers::CreateDescriptorSetLayout(ShaderHelpers::BindingType::Camera);
		mySimpleObjectSetLayout = ShaderHelpers::CreateDescriptorSetLayout(ShaderHelpers::BindingType::SimpleObject);

		uint nbCameraMax = 1;
		uint nbObjectsMax = 2;

		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = (nbCameraMax + nbObjectsMax) * myRenderTarget.GetImagesCount();
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = (nbObjectsMax) * myRenderTarget.GetImagesCount();

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = (nbCameraMax + nbObjectsMax) * myRenderTarget.GetImagesCount();

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");
	}

	void SampleTexturedRectangleRenderer::DestroyDescriptorSetLayouts()
	{
		vkDestroyDescriptorSetLayout(myDevice, myCameraSetLayout, nullptr);
		myCameraSetLayout = VK_NULL_HANDLE;
		vkDestroyDescriptorSetLayout(myDevice, mySimpleObjectSetLayout, nullptr);
		mySimpleObjectSetLayout = VK_NULL_HANDLE;
	}

	void SampleTexturedRectangleRenderer::SetupRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = myRenderTarget.GetColorFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = myRenderTarget.GetFinalLayout();
	
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = myDepthFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
	
		// TODO : Use subpass dependencies for layout transitions
	
		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
	
		VK_CHECK_RESULT(vkCreateRenderPass(myDevice, &renderPassInfo, nullptr, &myRenderPass), "Failed to create the renderpass");
	}
	
	void SampleTexturedRectangleRenderer::SetupPipelines()
	{
		VkShaderModule vertModule = ShaderHelpers::CreateShaderModule("shaders/sample_texturedRectangle_vert.spv");
		VkShaderModule fragModule = ShaderHelpers::CreateShaderModule("shaders/sample_texturedRectangle_frag.spv");
	
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertModule;
		vertShaderStageInfo.pName = "main";
	
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragModule;
		fragShaderStageInfo.pName = "main";
	
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
	
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	
		auto bindingDescription = ShaderHelpers::Vertex::GetBindingDescription();
		auto attributeDescriptions = ShaderHelpers::Vertex::GetAttributeDescriptions({
			ShaderHelpers::VertexComponent::Position,
			ShaderHelpers::VertexComponent::UV,
			ShaderHelpers::VertexComponent::Color
			});
	
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
	
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
	
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
	
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
	
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
	
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;
	
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();
	
		std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {
			myCameraSetLayout,
			mySimpleObjectSetLayout
		};
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = (uint)descriptorSetLayouts.size();
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
	
		VK_CHECK_RESULT(vkCreatePipelineLayout(myDevice, &pipelineLayoutInfo, nullptr, &myPipelineLayout), "Failed to create the pipeline layout");
	
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = myPipelineLayout;
		pipelineInfo.renderPass = myRenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(myDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myPipeline), "Failed to create the pipeline");
	
		vkDestroyShaderModule(myDevice, fragModule, nullptr);
		vkDestroyShaderModule(myDevice, vertModule, nullptr);
	}
	
	void SampleTexturedRectangleRenderer::DestroyPipelines()
	{
		vkDestroyPipeline(myDevice, myPipeline, nullptr);
		myPipeline = VK_NULL_HANDLE;
		vkDestroyPipelineLayout(myDevice, myPipelineLayout, nullptr);
		myPipelineLayout = VK_NULL_HANDLE;
	}

	void SampleTexturedRectangleRenderer::SetupSimpleModel()
	{
		std::vector<EntitySimpleGeometryModelComponent::Vertex> vertices;
		vertices.resize(4);
		vertices[0].myPosition = glm::vec3(-0.5f, 0.5f, 0.0f);
		vertices[0].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertices[0].myColor = glm::vec4(1.0f);
		vertices[0].myUV = glm::vec2(0.0f, 0.0f);
		vertices[1].myPosition = glm::vec3(-0.5f, -0.5f, 0.0f);
		vertices[1].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertices[1].myColor = glm::vec4(1.0f);
		vertices[1].myUV = glm::vec2(0.0f, 1.0f);
		vertices[2].myPosition = glm::vec3(0.5f, -0.5f, 0.0f);
		vertices[2].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertices[2].myColor = glm::vec4(1.0f);
		vertices[2].myUV = glm::vec2(1.0f, 1.0f);
		vertices[3].myPosition = glm::vec3(0.5f, 0.5f, 0.0f);
		vertices[3].myNormal = glm::vec3(0.0f, 0.0f, 1.0f);
		vertices[3].myColor = glm::vec4(1.0f);
		vertices[3].myUV = glm::vec2(1.0f, 0.0f);
	
		std::vector<uint> indices;
		indices.resize(6);
		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 0;
		indices[4] = 2;
		indices[5] = 3;
	
		std::string texture = "textures/panda.jpg";
	
		myModel = new SimpleGeometryModel(vertices, indices, texture);
	
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) * glm::toMat4(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(1.0f));
		myModel->Update(transform);
	
		myModel2 = new SimpleGeometryModel(vertices, indices, texture);
	
		transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f)) * glm::toMat4(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) * glm::scale(glm::vec3(2.0f));
		myModel2->Update(transform);
	}
}
