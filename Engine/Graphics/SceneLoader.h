#pragma once

#include "PCH.h"
#include "Entity.h"
#include "Mesh.h"
#include "ResourceManager.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Bat
{
	class Mesh;
	class Material;
	class AnimationComponent;
	struct AnimationClip;

	class SceneLoader
	{
	private:
		enum class TextureStorageType
		{
			None,
			IndexCompressed,
			IndexNonCompressed,
			EmbeddedCompressed,
			EmbeddedNonCompressed,
			Disk
		};
	public:
		SceneNode Load( const std::string& filename );
	private:
		bool ReadFile( const std::string& filename );
		const std::string& GetDirectory() const { return m_szDirectory; }

		void PreProcess();
		void ProcessNode( aiNode* pAiNode, SceneNode& node );
		void BuildSkeleton( aiNode* pAiNode, int parent_index );
		Resource<Mesh> ProcessMesh( aiMesh* pAiMesh );
		Resource<Mesh> GetLoadedMesh( const aiMesh* pTarget );

		TextureStorageType DetermineTextureStorageType( aiMaterial* pMat, aiTextureType type );
		void LoadMaterialTexture( Material& mat, aiMaterial* pMaterial, aiTextureType type, TextureStorageType storetype );
		void LoadMaterialTextureType( Material& mat, aiMaterial* pMaterial, aiTextureType type );
	
		void AddAiBone( aiBone* pAiBone );
		aiBone* GetAiBoneByName( const std::string& name );
		int AddSkeletonNode( aiNode* pAiNode, int parent_index );
		int AddBone( aiNode* pAiNode, aiBone* pAiBone, int parent_index );
		int FindNodeByName( const std::string& name ) const;
		Entity GetSceneNode( int node_index );
		int FindBoneByName( const std::string& name ) const;
		size_t GetNumBones() const;
		
		void LoadAnimations( AnimationComponent* animation_out );
	private:
		Assimp::Importer m_Importer;

		std::string m_szFilename;
		std::string m_szDirectory;
		const aiScene* m_pAiScene;

		struct LoadedMesh
		{
			aiMesh* pAssimpMesh;
			Resource<Mesh> pBatMesh;
		};
		std::vector<LoadedMesh> m_LoadedMeshes;

		std::vector<AnimationClip> m_Animations;

		std::vector<BoneData> m_Bones;
		SkeletonPose m_OriginalSkeleton;
		std::unordered_map<std::string, aiBone*> m_mapBoneNameToAiBone;
		std::unordered_map<std::string, int> m_mapNodeNameToIndex;
		std::unordered_map<std::string, int> m_mapBoneNameToIndex;
		std::vector<Entity> m_SceneNodes;
	};
}