#pragma once

#include "Render_Model.h"
#include "Render_Buffer.h"

namespace Render
{
	struct glTFNode;
	class glTFModel;

	struct glTFPrimitive
	{
		uint myFirstVertex = 0;
		uint myVertexCount = 0;
		uint myFirstIndex = 0;
		uint myIndexCount = 0;

		int myMaterial = -1;
	};

	struct glTFMesh
	{
		typedef ShaderHelpers::Vertex Vertex;

		void Load(const tinygltf::Model& aModel, uint aMeshIndex, std::vector<Vertex>& someOutVertices, std::vector<uint>& someOutIndices);

		std::string myName;
		std::vector<glTFPrimitive> myPrimitives;
	};

	struct glTFImage
	{
		void Load(const tinygltf::Model& aModel, uint anImageIndex, VkQueue aTransferQueue);
		void LoadEmpty(VkQueue aTransferQueue);

		void Load(uint aWidth, uint aHeight, const unsigned char* aBuffer, VkDeviceSize aBufferSize, VkQueue aTransferQueue);

		ImagePtr myImage;
	};

	struct glTFTexture
	{
		uint myImageIndex;
	};

	struct glTFMaterial
	{
		void Load(const tinygltf::Model& aModel, uint aMaterialIndex);
		void LoadEmpty();
		void Load();

		int myBaseColorTexture = -1;
		glm::vec4 myBaseColorFactor = glm::vec4(1.0f);

		int myMetallicRoughnessTexture = -1;
		float myMetallicFactor = 1.0f;
		float myRoughnessFactor = 1.0f;

		int myNormalTexture = -1;
		int myEmissiveTexture = -1;
		int myOcclusionTexture = -1;

		enum class AlphaMode
		{
			ALPHAMODE_OPAQUE,
			ALPHAMODE_MASK,
			ALPHAMODE_BLEND
		};
		AlphaMode myAlphaMode = AlphaMode::ALPHAMODE_OPAQUE;
		float myAlphaCutoff = 1.0f;

		BufferPtr mySSBO;
	};

	struct glTFAnimationChannel
	{
		enum class Path { TRANSLATION, ROTATION, SCALE };
		Path myPath;
		uint mySamplerIndex = 0;
		glTFNode* myNode;
	};

	struct glTFAnimationSampler
	{
		std::vector<float> myInputTimes;
		std::vector<glm::vec4> myOutputValues;
	};

	struct glTFAnimation
	{
		void Load(glTFModel* aContainer, const tinygltf::Model& aModel, uint anAnimationIndex);

		void Update(float aDeltaTime);

		std::string myName;

		std::vector<glTFAnimationSampler> mySamplers;
		std::vector<glTFAnimationChannel> myChannels;

		float myStartTime = std::numeric_limits<float>::max();
		float myEndTime = std::numeric_limits<float>::min();
		float myCurrentTime = 0.0f;
	};

	struct glTFSkin
	{
		void Load(glTFModel* aContainer, const tinygltf::Model& aModel, uint aSkinIndex);
		void LoadEmpty();

		std::string myName;

		glTFNode* mySkeletonRoot = nullptr;
		std::vector<glTFNode*> myJoints;
		std::vector<glm::mat4> myInverseBindMatrices;

		BufferPtr mySSBO;
	};

	struct glTFNode
	{
		~glTFNode();

		void Load(const tinygltf::Model& aModel, uint aNodeIndex, std::vector<glTFMesh::Vertex>& someOutVertices, std::vector<uint>& someOutIndices);

		void UpdateUBO(const glm::mat4& aMatrix = glm::mat4(1.0f));
		void UpdateJoints(const glTFModel* aContainer);
		void Draw(const glTFModel* aContainer, VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, const std::function<VkDescriptorSet(const ShaderHelpers::DescriptorInfo&)>& myDescriptorSetGetter);

		glm::mat4 GetLocalMatrix() const;
		glm::mat4 GetMatrix() const;

		uint myIndex = 0;
		std::string myName;

		glTFNode* myParent = nullptr;
		std::vector<glTFNode*> myChildren;

		glTFMesh myMesh;
		int mySkinIndex = -1;

		glm::vec3 myTranslation{};
		glm::quat myRotation{};
		glm::vec3 myScale{ 1.0f };
		glm::mat4 myMatrix = glm::mat4(1.0f);

		BufferPtr myUBO;
	};

	class glTFModel : public Model
	{
	public:
		glTFModel(const std::string& aFilename);
		~glTFModel();

		void Update(const glm::mat4& aMatrix) override;
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, const std::function<VkDescriptorSet(const ShaderHelpers::DescriptorInfo&)>& myDescriptorSetGetter) override;

		glTFNode* GetNodeByIndex(uint anIndex);
		const glTFImage* GetImage(uint anIndex) const { return &myImages[anIndex]; }
		const glTFImage* GetEmptyImage() const { return &myImages.back(); }
		const glTFTexture* GetTexture(uint anIndex) const { return &myTextures[anIndex]; }
		const glTFMaterial* GetMaterial(uint anIndex) const { return &myMaterials[anIndex]; }
		const glTFMaterial* GetEmptyMaterial() const { return &myMaterials.back(); }
		const glTFSkin* GetSkin(uint anIndex) const { return &mySkins[anIndex]; }
		const glTFSkin* GetEmptySkin() const { return &mySkins.back(); }

	private:
		bool LoadFromFile(const std::string& aFilename, VkQueue aTransferQueue);

		void LoadImages(const tinygltf::Model& aModel);
		void LoadTextures(const tinygltf::Model& aModel);
		void LoadMaterials(tinygltf::Model& aModel);
		void LoadSkins(const tinygltf::Model& aModel);
		void LoadAnimations(const tinygltf::Model& aModel);

		void LoadNodes(const tinygltf::Model& aModel, std::vector<glTFMesh::Vertex>& someOutVertices, std::vector<uint>& someOutIndices);

		template<typename Functor>
		void IterateNodes(Functor aFunctor, bool aParentFirst = true)
		{
			for (glTFNode* node : myNodes)
				IterateNodeChildren(aFunctor, node, aParentFirst);
		}
		template<typename Functor>
		void IterateNodeChildren(Functor aFunctor, glTFNode* aNode, bool aParentFirst)
		{
			if (aParentFirst)
				aFunctor(aNode);
			for (glTFNode* childNode : aNode->myChildren)
				IterateNodeChildren(aFunctor, childNode, aParentFirst);
			if (!aParentFirst)
				aFunctor(aNode);
		}

		VkDevice myDevice = VK_NULL_HANDLE;
		VkQueue myTransferQueue = VK_NULL_HANDLE;

		BufferPtr myVertexBuffer;
		BufferPtr myIndexBuffer;

		uint myNodeCount = 0;
		std::vector<glTFNode*> myNodes;

		std::vector<glTFImage> myImages;
		std::vector<glTFTexture> myTextures;
		std::vector<glTFMaterial> myMaterials;
		std::vector<glTFSkin> mySkins;
		std::vector<glTFAnimation> myAnimations;

		uint myActiveAnimation = 0;
	};
}
