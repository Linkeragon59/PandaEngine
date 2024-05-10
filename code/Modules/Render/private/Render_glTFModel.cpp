#include "Render_glTFModel.h"

namespace Render
{
	void glTFMesh::Load(const tinygltf::Model& aModel, uint aMeshIndex, std::vector<Vertex>& someOutVertices, std::vector<uint>& someOutIndices)
	{
		const tinygltf::Mesh& gltfMesh = aModel.meshes[aMeshIndex];

		myName = gltfMesh.name;

		for (size_t i = 0; i < gltfMesh.primitives.size(); ++i)
		{
			const tinygltf::Primitive& gltfPrimitive = gltfMesh.primitives[i];
			if (gltfPrimitive.indices < 0)
				continue;

			uint vertexStart = static_cast<uint>(someOutVertices.size());
			uint vertexCount = 0;
			uint indexStart = static_cast<uint>(someOutIndices.size());
			uint indexCount = 0;

			// Parse vertices
			{
				const float* bufferPositions = nullptr;
				const float* bufferColors = nullptr;
				const float* bufferTexCoord = nullptr;
				const float* bufferNormals = nullptr;
				const uint16_t* bufferJointIndices = nullptr;
				const float* bufferJointWeights = nullptr;

				uint numColorComponents = 0;

				// Position attribute is required
				Assert(gltfPrimitive.attributes.find("POSITION") != gltfPrimitive.attributes.end());
				if (gltfPrimitive.attributes.find("POSITION") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView& view = aModel.bufferViews[accessor.bufferView];
					bufferPositions = reinterpret_cast<const float*>(&(aModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					vertexCount = static_cast<uint>(accessor.count);
				}

				if (gltfPrimitive.attributes.find("COLOR_0") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.attributes.find("COLOR_0")->second];
					const tinygltf::BufferView& view = aModel.bufferViews[accessor.bufferView];
					bufferColors = reinterpret_cast<const float*>(&(aModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					numColorComponents = accessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;
				}

				if (gltfPrimitive.attributes.find("TEXCOORD_0") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& view = aModel.bufferViews[accessor.bufferView];
					bufferTexCoord = reinterpret_cast<const float*>(&(aModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				if (gltfPrimitive.attributes.find("NORMAL") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.attributes.find("NORMAL")->second];
					const tinygltf::BufferView& view = aModel.bufferViews[accessor.bufferView];
					bufferNormals = reinterpret_cast<const float*>(&(aModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				if (gltfPrimitive.attributes.find("JOINTS_0") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.attributes.find("JOINTS_0")->second];
					const tinygltf::BufferView& view = aModel.bufferViews[accessor.bufferView];
					bufferJointIndices = reinterpret_cast<const uint16_t*>(&(aModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				if (gltfPrimitive.attributes.find("WEIGHTS_0") != gltfPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.attributes.find("WEIGHTS_0")->second];
					const tinygltf::BufferView& view = aModel.bufferViews[accessor.bufferView];
					bufferJointWeights = reinterpret_cast<const float*>(&(aModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				const bool hasSkin = (bufferJointIndices && bufferJointWeights);

				for (size_t v = 0; v < vertexCount; ++v)
				{
					Vertex vert{};
					vert.myPosition = glm::make_vec3(&bufferPositions[v * 3]);
					vert.myNormal = bufferNormals ? glm::normalize(glm::make_vec3(&bufferNormals[v * 3])) : glm::vec3(0.0f);
					vert.myUV = bufferTexCoord ? glm::make_vec2(&bufferTexCoord[v * 2]) : glm::vec2(0.0f);
					vert.myColor = glm::vec4(1.0f);
					if (bufferColors)
					{
						if (numColorComponents == 3)
							vert.myColor = glm::vec4(glm::make_vec3(&bufferColors[v * 3]), 1.0f);
						else if (numColorComponents == 4)
							vert.myColor = glm::make_vec4(&bufferColors[v * 4]);
					}
					vert.myJoint = hasSkin ? glm::vec4(glm::make_vec4(&bufferJointIndices[v * 4])) : glm::vec4(0.0f);
					vert.myWeight = hasSkin ? glm::make_vec4(&bufferJointWeights[v * 4]) : glm::vec4(1.0f);
					//vert.myTangent
					someOutVertices.push_back(vert);
				}
			}

			// Parse indices
			{
				const tinygltf::Accessor& accessor = aModel.accessors[gltfPrimitive.indices];
				const tinygltf::BufferView& bufferView = aModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = aModel.buffers[bufferView.buffer];
				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				indexCount = static_cast<uint>(accessor.count);

				switch (accessor.componentType)
				{
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
				{
					const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
					for (size_t index = 0; index < accessor.count; ++index)
						someOutIndices.push_back(buf[index] + vertexStart);
				}
				break;
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
				{
					const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
					for (size_t index = 0; index < accessor.count; ++index)
						someOutIndices.push_back(buf[index] + vertexStart);
				}
				break;
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
				{
					const uint* buf = static_cast<const uint*>(dataPtr);
					for (size_t index = 0; index < accessor.count; ++index)
						someOutIndices.push_back(buf[index] + vertexStart);
				}
				break;
				default:
					Assert(false, "Index component type not supported");
				}
			}

			myPrimitives.push_back(glTFPrimitive());
			glTFPrimitive& primitive = myPrimitives.back();
			primitive.myFirstVertex = vertexStart;
			primitive.myVertexCount = vertexCount;
			primitive.myFirstIndex = indexStart;
			primitive.myIndexCount = indexCount;
			primitive.myMaterial = gltfPrimitive.material;
		}
	}

	void glTFImage::Load(const tinygltf::Model& aModel, uint anImageIndex, VkQueue aTransferQueue)
	{
		const tinygltf::Image& gltfImage = aModel.images[anImageIndex];

		// TODO: check anImage.component, if equal to 3 and the Vulkan device doesn't support RGB only, modify the buffer
		Assert(gltfImage.component == 4);
		const unsigned char* buffer = &gltfImage.image[0];
		VkDeviceSize bufferSize = gltfImage.image.size();

		uint width = gltfImage.width;
		uint height = gltfImage.height;

		Load(width, height, buffer, bufferSize, aTransferQueue);
	}

	void glTFImage::LoadEmpty(VkQueue aTransferQueue)
	{
		uint width = 1;
		uint height = 1;
		VkDeviceSize bufferSize = width * height * 4;
		unsigned char* buffer = new unsigned char[bufferSize];
		memset(buffer, 0xff, bufferSize);

		Load(width, height, buffer, bufferSize, aTransferQueue);

		delete[] buffer;
	}

	void glTFImage::Load(uint aWidth, uint aHeight, const unsigned char* aBuffer, VkDeviceSize aBufferSize, VkQueue aTransferQueue)
	{
		// TODO : generate MipMaps
		//uint mipsLevels = static_cast<uint>(floor(log2(std::max(width, height))) + 1.0);

		Buffer stagingBuffer;
		stagingBuffer.Create(aBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.Map();
		memcpy(stagingBuffer.myMappedData, aBuffer, aBufferSize);
		stagingBuffer.Unmap();

		myImage = new Image(aWidth, aHeight,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// TODO: Could merge all the commands together instead of doing several commands
		myImage->TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, aTransferQueue);
		VkCommandBuffer commandBuffer = Helpers::BeginOneTimeCommand();
		{
			VkBufferImageCopy copyRegion{};
			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = { aWidth, aHeight, 1 };
			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.myBuffer, myImage->myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
		}
		Helpers::EndOneTimeCommand(commandBuffer, aTransferQueue);
		myImage->TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, aTransferQueue);

		stagingBuffer.Destroy();

		myImage->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myImage->CreateImageSampler();
		myImage->SetupDescriptor();
	}

	void glTFMaterial::Load(const tinygltf::Model& aModel, uint aMaterialIndex)
	{
		const tinygltf::Material& gltfMaterial = aModel.materials[aMaterialIndex];

		// TODO: values and additionalValues are deprecated, change to use the correct accessors
		if (gltfMaterial.values.find("baseColorTexture") != gltfMaterial.values.end())
		{
			myBaseColorTexture = gltfMaterial.values.at("baseColorTexture").TextureIndex();
		}
		if (gltfMaterial.values.find("baseColorFactor") != gltfMaterial.values.end())
		{
			myBaseColorFactor = glm::make_vec4(gltfMaterial.values.at("baseColorFactor").ColorFactor().data());
		}

		if (gltfMaterial.values.find("metallicRoughnessTexture") != gltfMaterial.values.end())
		{
			myMetallicRoughnessTexture = gltfMaterial.values.at("metallicRoughnessTexture").TextureIndex();
		}
		if (gltfMaterial.values.find("metallicFactor") != gltfMaterial.values.end())
		{
			myMetallicFactor = static_cast<float>(gltfMaterial.values.at("metallicFactor").Factor());
		}
		if (gltfMaterial.values.find("roughnessFactor") != gltfMaterial.values.end())
		{
			myRoughnessFactor = static_cast<float>(gltfMaterial.values.at("roughnessFactor").Factor());
		}

		if (gltfMaterial.additionalValues.find("normalTexture") != gltfMaterial.additionalValues.end())
		{
			myNormalTexture = gltfMaterial.additionalValues.at("normalTexture").TextureIndex();
		}
		if (gltfMaterial.additionalValues.find("emissiveTexture") != gltfMaterial.additionalValues.end())
		{
			myEmissiveTexture = gltfMaterial.additionalValues.at("emissiveTexture").TextureIndex();
		}
		if (gltfMaterial.additionalValues.find("occlusionTexture") != gltfMaterial.additionalValues.end())
		{
			myOcclusionTexture = gltfMaterial.additionalValues.at("occlusionTexture").TextureIndex();
		}

		if (gltfMaterial.additionalValues.find("alphaMode") != gltfMaterial.additionalValues.end())
		{
			tinygltf::Parameter param = gltfMaterial.additionalValues.at("alphaMode");
			if (param.string_value == "BLEND")
				myAlphaMode = AlphaMode::ALPHAMODE_BLEND;
			else if (param.string_value == "MASK")
				myAlphaMode = AlphaMode::ALPHAMODE_MASK;
		}
		if (gltfMaterial.additionalValues.find("alphaCutoff") != gltfMaterial.additionalValues.end())
		{
			myAlphaCutoff = static_cast<float>(gltfMaterial.additionalValues.at("alphaCutoff").Factor());
		}

		Load();
	}

	void glTFMaterial::LoadEmpty()
	{
		Load();
	}

	void glTFMaterial::Load()
	{
		// Store material info in a shader storage buffer object (SSBO)
		VkDeviceSize ssboSize = sizeof(glm::vec4);
		mySSBO = new Buffer(ssboSize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		mySSBO->SetupDescriptor();
		mySSBO->Map();
		memcpy(mySSBO->myMappedData, &myBaseColorFactor, ssboSize);
		mySSBO->Unmap();
	}

	void glTFAnimation::Load(glTFModel* aContainer, const tinygltf::Model& aModel, uint anAnimationIndex)
	{
		const tinygltf::Animation& gltfAnimation = aModel.animations[anAnimationIndex];

		myName = gltfAnimation.name;

		// Samplers
		mySamplers.resize(gltfAnimation.samplers.size());
		for (uint i = 0; i < (uint)gltfAnimation.samplers.size(); ++i)
		{
			const tinygltf::AnimationSampler& gltfSampler = gltfAnimation.samplers[i];

			// Read sampler input time values
			{
				const tinygltf::Accessor& accessor = aModel.accessors[gltfSampler.input];
				const tinygltf::BufferView& bufferView = aModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = aModel.buffers[bufferView.buffer];
				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				Assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				const float* buf = static_cast<const float*>(dataPtr);
				for (size_t index = 0; index < accessor.count; index++)
				{
					mySamplers[i].myInputTimes.push_back(buf[index]);
				}

				for (float inputTime : mySamplers[i].myInputTimes)
				{
					if (inputTime < myStartTime)
						myStartTime = inputTime;
					if (inputTime > myEndTime)
						myEndTime = inputTime;
				}
			}

			// Read sampler output T/R/S values 
			{
				const tinygltf::Accessor& accessor = aModel.accessors[gltfSampler.output];
				const tinygltf::BufferView& bufferView = aModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = aModel.buffers[bufferView.buffer];
				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				Assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				switch (accessor.type)
				{
				case TINYGLTF_TYPE_VEC3:
				{
					const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
					{
						mySamplers[i].myOutputValues.push_back(glm::vec4(buf[index], 0.0f));
					}
				}
				break;
				case TINYGLTF_TYPE_VEC4:
				{
					const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
					{
						mySamplers[i].myOutputValues.push_back(buf[index]);
					}
				}
				break;
				default:
					Assert(false, "AnimationSampler output format not supported yet");
					break;
				}
			}
		}

		// Channels
		myChannels.resize(gltfAnimation.channels.size());
		for (uint i = 0; i < (uint)gltfAnimation.channels.size(); ++i)
		{
			const tinygltf::AnimationChannel& gltfChannel = gltfAnimation.channels[i];

			if (gltfChannel.target_path == "rotation")
				myChannels[i].myPath = glTFAnimationChannel::Path::ROTATION;
			else if (gltfChannel.target_path == "translation")
				myChannels[i].myPath = glTFAnimationChannel::Path::TRANSLATION;
			else if (gltfChannel.target_path == "scale")
				myChannels[i].myPath = glTFAnimationChannel::Path::SCALE;
			else
				Assert(false, "AnimationChannel not supported yet");

			myChannels[i].mySamplerIndex = gltfChannel.sampler;
			myChannels[i].myNode = aContainer->GetNodeByIndex(gltfChannel.target_node);
		}

		myCurrentTime = myStartTime;
	}

	void glTFAnimation::Update(float aDeltaTime)
	{
		myCurrentTime += aDeltaTime;
		if (myCurrentTime > myEndTime)
			myCurrentTime -= myEndTime;

		for (glTFAnimationChannel& channel : myChannels)
		{
			glTFAnimationSampler& sampler = mySamplers[channel.mySamplerIndex];
			for (size_t i = 0; i < sampler.myInputTimes.size() - 1; i++)
			{
				if ((myCurrentTime >= sampler.myInputTimes[i]) && (myCurrentTime <= sampler.myInputTimes[i + 1]))
				{
					float u = (myCurrentTime - sampler.myInputTimes[i]) / (sampler.myInputTimes[i + 1] - sampler.myInputTimes[i]);
					Assert(u >= 0.0f && u <= 1.0f);

					switch (channel.myPath)
					{
					case glTFAnimationChannel::Path::TRANSLATION:
						channel.myNode->myTranslation = glm::vec3(glm::mix(sampler.myOutputValues[i], sampler.myOutputValues[i + 1], u));
						break;
					case glTFAnimationChannel::Path::ROTATION:
						glm::quat q1;
						q1.x = sampler.myOutputValues[i].x;
						q1.y = sampler.myOutputValues[i].y;
						q1.z = sampler.myOutputValues[i].z;
						q1.w = sampler.myOutputValues[i].w;
						glm::quat q2;
						q2.x = sampler.myOutputValues[i + 1].x;
						q2.y = sampler.myOutputValues[i + 1].y;
						q2.z = sampler.myOutputValues[i + 1].z;
						q2.w = sampler.myOutputValues[i + 1].w;
						channel.myNode->myRotation = glm::normalize(glm::slerp(q1, q2, u));
						break;
					case glTFAnimationChannel::Path::SCALE:
						channel.myNode->myScale = glm::vec3(glm::mix(sampler.myOutputValues[i], sampler.myOutputValues[i + 1], u));
						break;
					default:
						break;
					}
				}
			}
		}
	}

	void glTFSkin::Load(glTFModel* aContainer, const tinygltf::Model& aModel, uint aSkinIndex)
	{
		const tinygltf::Skin& gltfSkin = aModel.skins[aSkinIndex];

		myName = gltfSkin.name;

		mySkeletonRoot = aContainer->GetNodeByIndex(gltfSkin.skeleton);
		for (int joint : gltfSkin.joints)
		{
			glTFNode* jointNode = aContainer->GetNodeByIndex(joint);
			if (jointNode)
				myJoints.push_back(jointNode);
		}

		if (gltfSkin.inverseBindMatrices > -1)
		{
			const tinygltf::Accessor& accessor = aModel.accessors[gltfSkin.inverseBindMatrices];
			const tinygltf::BufferView& bufferView = aModel.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = aModel.buffers[bufferView.buffer];

			myInverseBindMatrices.resize(accessor.count);
			VkDeviceSize ssboSize = accessor.count * sizeof(glm::mat4);

			memcpy(myInverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], ssboSize);

			// Store inverse bind matrices for this skin in a shader storage buffer object (SSBO)
			mySSBO = new Buffer(ssboSize,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			mySSBO->SetupDescriptor();
			mySSBO->Map();
			memcpy(mySSBO->myMappedData, myInverseBindMatrices.data(), ssboSize);
		}

		Assert(myJoints.size() == myInverseBindMatrices.size());
	}

	void glTFSkin::LoadEmpty()
	{
		VkDeviceSize ssboSize = 1 * sizeof(glm::mat4);
		glm::mat4 identity(1.0f);
		mySSBO = new Buffer(ssboSize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		mySSBO->SetupDescriptor();
		mySSBO->Map();
		memcpy(mySSBO->myMappedData, &identity, ssboSize);
	}

	glTFNode::~glTFNode()
	{
		for (glTFNode* child : myChildren)
			delete child;
	}

	void glTFNode::Load(const tinygltf::Model& aModel, uint aNodeIndex, std::vector<glTFMesh::Vertex>& someOutVertices, std::vector<uint>& someOutIndices)
	{
		const tinygltf::Node& gltfNode = aModel.nodes[aNodeIndex];
		myIndex = aNodeIndex;
		myName = gltfNode.name;

		mySkinIndex = gltfNode.skin;

		if (gltfNode.translation.size() == 3)
			myTranslation = glm::make_vec3(gltfNode.translation.data());
		if (gltfNode.rotation.size() == 4)
			myRotation = glm::make_quat(gltfNode.rotation.data());
		if (gltfNode.scale.size() == 3)
			myScale = glm::make_vec3(gltfNode.scale.data());
		if (gltfNode.matrix.size() == 16)
			myMatrix = glm::make_mat4x4(gltfNode.matrix.data());

		// Load children
		myChildren.resize(gltfNode.children.size());
		for (uint i = 0; i < (uint)gltfNode.children.size(); ++i)
		{
			myChildren[i] = new glTFNode();
			myChildren[i]->myParent = this;
			myChildren[i]->Load(aModel, gltfNode.children[i], someOutVertices, someOutIndices);
		}

		// Parse node mesh
		if (gltfNode.mesh > -1)
			myMesh.Load(aModel, gltfNode.mesh, someOutVertices, someOutIndices);

		myUBO = new Buffer(
			sizeof(ShaderHelpers::ModelMatrixData),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myUBO->SetupDescriptor();
		myUBO->Map();
	}

	void glTFNode::UpdateUBO(const glm::mat4& aMatrix)
	{
		glm::mat4 matrix = aMatrix * GetMatrix();
		memcpy(myUBO->myMappedData, &matrix, sizeof(glm::mat4));

		for (glTFNode* child : myChildren)
			child->UpdateUBO(aMatrix);
	}

	void glTFNode::UpdateJoints(const glTFModel* aContainer)
	{
		if (mySkinIndex > -1)
		{
			const glTFSkin* skin = aContainer->GetSkin(mySkinIndex);

			glm::mat4 inverseTransform = glm::inverse(GetMatrix());

			size_t numJoints = (uint)skin->myJoints.size();
			std::vector<glm::mat4> jointMatrices(numJoints);
			for (size_t i = 0; i < numJoints; i++)
			{
				jointMatrices[i] = skin->myJoints[i]->GetMatrix() * skin->myInverseBindMatrices[i];
				jointMatrices[i] = inverseTransform * jointMatrices[i];
			}

			memcpy(skin->mySSBO->myMappedData, jointMatrices.data(), jointMatrices.size() * sizeof(glm::mat4));
		}

		for (glTFNode* child : myChildren)
			child->UpdateJoints(aContainer);
	}

	void glTFNode::Draw(const glTFModel* aContainer, VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, const std::function<VkDescriptorSet(const ShaderHelpers::DescriptorInfo&)>& myDescriptorSetGetter)
	{
		const glTFSkin* skin = nullptr;
		const glTFImage* image = nullptr;
		const glTFMaterial* material = nullptr;

		if (mySkinIndex > -1)
		{
			skin = aContainer->GetSkin(mySkinIndex);
		}
		if (!skin)
		{
			skin = aContainer->GetEmptySkin();
		}

		for (glTFPrimitive& primitive : myMesh.myPrimitives)
		{
			if (primitive.myMaterial > -1)
			{
				material = aContainer->GetMaterial(primitive.myMaterial);
				Assert(material);

				if (material->myBaseColorTexture > -1)
				{
					const glTFTexture* texture = aContainer->GetTexture(material->myBaseColorTexture);
					Assert(texture);
					image = aContainer->GetImage(texture->myImageIndex);
					Assert(image);
				}
			}
			if (!image)
			{
				image = aContainer->GetEmptyImage();
			}
			if (!material)
			{
				material = aContainer->GetEmptyMaterial();
			}

			ShaderHelpers::ObjectDescriptorInfo info;
			info.myModelMatrixInfo = &myUBO->myDescriptor;
			info.myImageSamplerInfo = &image->myImage->myDescriptor;
			info.myMaterialInfo = &material->mySSBO->myDescriptor;
			info.myJointMatricesInfo = &skin->mySSBO->myDescriptor;

			VkDescriptorSet objectSet = myDescriptorSetGetter(info);
			vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aDescriptorSetIndex, 1, &objectSet, 0, nullptr);

			vkCmdDrawIndexed(aCommandBuffer, primitive.myIndexCount, 1, primitive.myFirstIndex, 0, 0);
		}

		for (glTFNode* child : myChildren)
			child->Draw(aContainer, aCommandBuffer, aPipelineLayout, aDescriptorSetIndex, myDescriptorSetGetter);
	}

	glm::mat4 glTFNode::GetLocalMatrix() const
	{
		return glm::translate(glm::mat4(1.0f), myTranslation) * glm::mat4(myRotation) * glm::scale(myScale) * myMatrix;
	}

	glm::mat4 glTFNode::GetMatrix() const
	{
		glm::mat4 matrix = GetLocalMatrix();
		glTFNode* parent = myParent;
		while (parent)
		{
			matrix = parent->GetLocalMatrix() * matrix;
			parent = parent->myParent;
		}
		return matrix;
	}

	glTFModel::glTFModel(const std::string& aFilename)
	{
		myDevice = RenderCore::GetInstance()->GetDevice();

		LoadFromFile(aFilename, RenderCore::GetInstance()->GetGraphicsQueue());
	}

	glTFModel::~glTFModel()
	{
		for (glTFNode* node : myNodes)
			delete node;
	}

	void glTFModel::Update(const glm::mat4& aMatrix)
	{
		if (myAnimations.size() > 0)
			myAnimations[0].Update(1.0f / 60.0f);

		for (glTFNode* node : myNodes)
			node->UpdateJoints(this);

		for (glTFNode* node : myNodes)
			node->UpdateUBO(aMatrix);
	}

	void glTFModel::Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, const std::function<VkDescriptorSet(const ShaderHelpers::DescriptorInfo&)>& myDescriptorSetGetter)
	{
		// All vertices and indices are stored in a single buffer, so we only need to bind once
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(aCommandBuffer, 0, 1, &myVertexBuffer->myBuffer, offsets);
		vkCmdBindIndexBuffer(aCommandBuffer, myIndexBuffer->myBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (glTFNode* node : myNodes)
			node->Draw(this, aCommandBuffer, aPipelineLayout, aDescriptorSetIndex, myDescriptorSetGetter);
	}

	bool glTFModel::LoadFromFile(const std::string& aFilename, VkQueue aTransferQueue)
	{
		myTransferQueue = aTransferQueue;

		tinygltf::TinyGLTF gltfContext;
		tinygltf::Model gltfModel;
		std::string error, warning;
		if (!gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, aFilename))
		{
			// TODO: Display the error message
			return false;
		}

		LoadImages(gltfModel);
		LoadTextures(gltfModel);
		LoadMaterials(gltfModel);

		std::vector<glTFMesh::Vertex> vertexBuffer;
		std::vector<uint> indexBuffer;
		LoadNodes(gltfModel, vertexBuffer, indexBuffer);

		auto countNodes = [this](glTFNode* aNode) { (void)aNode; myNodeCount++; };
		IterateNodes(countNodes);

		LoadSkins(gltfModel);
		LoadAnimations(gltfModel);

		// Calculate initial pose
		for (glTFNode* node : myNodes)
			node->UpdateJoints(this);

		// Fill initial matrices
		for (glTFNode* node : myNodes)
			node->UpdateUBO();

		size_t vertexBufferSize = vertexBuffer.size() * sizeof(glTFMesh::Vertex);
		size_t indexBufferSize = indexBuffer.size() * sizeof(uint);
		Assert((vertexBufferSize > 0) && (indexBufferSize > 0));

		Buffer vertexStagingBuffer, indexStagingBuffer;

		vertexStagingBuffer.Create(vertexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vertexStagingBuffer.Map();
		memcpy(vertexStagingBuffer.myMappedData, vertexBuffer.data(), vertexBufferSize);
		vertexStagingBuffer.Unmap();

		indexStagingBuffer.Create(indexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		indexStagingBuffer.Map();
		memcpy(indexStagingBuffer.myMappedData, indexBuffer.data(), indexBufferSize);
		indexStagingBuffer.Unmap();

		myVertexBuffer = new Buffer(vertexBufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		myIndexBuffer = new Buffer(indexBufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkCommandBuffer commandBuffer = Helpers::BeginOneTimeCommand();
		{
			VkBufferCopy copyRegion = {};

			copyRegion.size = vertexBufferSize;
			vkCmdCopyBuffer(commandBuffer, vertexStagingBuffer.myBuffer, myVertexBuffer->myBuffer, 1, &copyRegion);

			copyRegion.size = indexBufferSize;
			vkCmdCopyBuffer(commandBuffer, indexStagingBuffer.myBuffer, myIndexBuffer->myBuffer, 1, &copyRegion);
		}
		Helpers::EndOneTimeCommand(commandBuffer, myTransferQueue);

		vertexStagingBuffer.Destroy();
		indexStagingBuffer.Destroy();

		return true;
	}

	void glTFModel::LoadImages(const tinygltf::Model& aModel)
	{
		myImages.resize(aModel.images.size() + 1);
		for (uint i = 0; i < (uint)aModel.images.size(); i++)
			myImages[i].Load(aModel, i, myTransferQueue);
		myImages.back().LoadEmpty(myTransferQueue); // Add an empty image at the end
	}

	void glTFModel::LoadTextures(const tinygltf::Model& aModel)
	{
		myTextures.resize(aModel.textures.size());
		for (uint i = 0; i < (uint)aModel.textures.size(); i++)
			myTextures[i].myImageIndex = aModel.textures[i].source;
	}

	void glTFModel::LoadMaterials(tinygltf::Model& aModel)
	{
		myMaterials.resize(aModel.materials.size() + 1);
		for (uint i = 0; i < (uint)aModel.materials.size(); i++)
			myMaterials[i].Load(aModel, i);
		myMaterials.back().LoadEmpty(); // Add an empty material at the end
	}

	void glTFModel::LoadSkins(const tinygltf::Model& aModel)
	{
		mySkins.resize(aModel.skins.size() + 1);
		for (uint i = 0; i < (uint)aModel.skins.size(); i++)
			mySkins[i].Load(this, aModel, i);
		mySkins.back().LoadEmpty(); // Add an empty skin at the end
	}

	void glTFModel::LoadAnimations(const tinygltf::Model& aModel)
	{
		myAnimations.resize(aModel.animations.size());
		for (uint i = 0; i < (uint)aModel.animations.size(); i++)
			myAnimations[i].Load(this, aModel, i);
	}

	void glTFModel::LoadNodes(const tinygltf::Model& aModel, std::vector<glTFMesh::Vertex>& someOutVertices, std::vector<uint>& someOutIndices)
	{
		const tinygltf::Scene& scene = aModel.scenes[aModel.defaultScene > -1 ? aModel.defaultScene : 0];
		myNodes.resize(scene.nodes.size());
		for (size_t i = 0; i < scene.nodes.size(); i++)
		{
			myNodes[i] = new glTFNode;
			myNodes[i]->Load(aModel, scene.nodes[i], someOutVertices, someOutIndices);
		}
	}

	glTFNode* glTFModel::GetNodeByIndex(uint anIndex)
	{
		glTFNode* node = nullptr;
		auto findNode = [anIndex, &node](glTFNode* aNode) { if (aNode->myIndex == anIndex) node = aNode; };
		IterateNodes(findNode);
		return node;
	}
}
