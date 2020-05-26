#pragma once

#include "Core/Entity.h"
#include "Core/Scene.h"
#include "Mesh.h"

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
		SceneLoader();
		~SceneLoader();

		SceneNode Load( const std::string& filename );
		std::shared_ptr<Mesh> LoadMesh( const std::string& filename );
	private:
		bool ReadFile( const std::string& filename );
		const std::string& GetDirectory() const { return m_szDirectory; }

		void PreProcess();
		void ProcessNode( aiNode* pAiNode, SceneNode& node );
		void BuildSkeleton( aiNode* pAiNode, int parent_index );
		std::shared_ptr<Mesh> ProcessMesh( aiMesh* pAiMesh );
		Future<std::shared_ptr<Mesh>> ProcessMeshAsync( aiMesh* pAiMesh );
		std::shared_ptr<Mesh> GetLoadedMesh( const aiMesh* pTarget );

		std::shared_ptr<Mesh> ExtractMesh( aiNode* pAiNode );

		TextureStorageType DetermineTextureStorageType( aiMaterial* pMat, aiTextureType type, unsigned int index );
		std::shared_ptr<Texture> LoadMaterialTexture( aiMaterial* pMaterial, aiTextureType type, unsigned int index = 0 );
		float LoadMaterialFloat( aiMaterial* pMaterial, const char* key, unsigned int type, unsigned int index );
		Vec3 LoadMaterialColour3( aiMaterial* pMaterial, const char* key, unsigned int type, unsigned int index );
		Vec4 LoadMaterialColour4( aiMaterial* pMaterial, const char* key, unsigned int type, unsigned int index );
		std::string LoadMaterialString( aiMaterial* pMaterial, const char* key, unsigned int type, unsigned int index );
	
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