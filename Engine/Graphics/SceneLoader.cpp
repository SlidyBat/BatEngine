#include "PCH.h"
#include "SceneLoader.h"

#include "CoreEntityComponents.h"
#include "AnimationComponent.h"
#include "AnimationClip.h"
#include "Mesh.h"
#include "Model.h"
#include "Colour.h"
#include "ResourceManager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>
#include "Material.h"
#include "FrameTimer.h"

namespace Bat
{
	static Mat4 AiToBatMatrix( const aiMatrix4x4& aimat )
	{
		return Mat4( &aimat.a1 );
	}

	Resource<Mesh> SceneLoader::GetLoadedMesh( const aiMesh* pTarget )
	{
		auto it = std::find_if( m_LoadedMeshes.begin(), m_LoadedMeshes.end(), [pTarget]( const LoadedMesh& loaded )
		{
			return loaded.pAssimpMesh == pTarget;
		} );

		if( it == m_LoadedMeshes.end() )
		{
			return nullptr;
		}

		return it->pBatMesh;
	}

	std::shared_ptr<Mesh> SceneLoader::ExtractMesh( aiNode* pAiNode )
	{
		if( pAiNode->mNumMeshes > 0 )
		{
			aiMesh* pMesh = m_pAiScene->mMeshes[pAiNode->mMeshes[0]];
			return ProcessMesh( pMesh );
		}

		for( unsigned int i = 0; i < pAiNode->mNumChildren; i++ )
		{
			std::shared_ptr<Mesh> mesh = ExtractMesh( pAiNode->mChildren[i] );
			if( mesh )
			{
				return mesh;
			}
		}

		return nullptr;
	}

	SceneLoader::TextureStorageType SceneLoader::DetermineTextureStorageType( aiMaterial* pMat, aiTextureType type, unsigned int index = 0 )
	{
		if( !pMat->GetTextureCount( type ) )
		{
			return TextureStorageType::None;
		}

		aiString textype;
		pMat->GetTexture( type, index, &textype );
		if( textype.C_Str()[0] == '*' )
		{
			if( m_pAiScene->mTextures[0]->mHeight == 0 )
			{
				return TextureStorageType::IndexCompressed;
			}
			else
			{
				return TextureStorageType::IndexNonCompressed;
			}
		}
		else if( auto pTex = m_pAiScene->GetEmbeddedTexture( textype.C_Str() ) )
		{
			if( pTex->mHeight == 0 )
			{
				return TextureStorageType::EmbeddedCompressed;
			}
			else
			{
				return TextureStorageType::EmbeddedNonCompressed;
			}
		}
		else if( std::string( textype.C_Str() ).find( '.' ) != std::string::npos )
		{
			return TextureStorageType::Disk;
		}

		return TextureStorageType::None; // default to disk and let it fail
	}

	static int GetTextureIndex( aiString* pStr )
	{
		if( !pStr->length || pStr->C_Str()[0] != '*' )
		{
			ASSERT( false, "Invalid texture index" );
			return -1;
		}
		return std::stoi( &pStr->C_Str()[1] );
	}

	Resource<Texture> SceneLoader::LoadMaterialTexture( aiMaterial* pMaterial, aiTextureType type, unsigned int index )
	{
		TextureStorageType storetype = DetermineTextureStorageType( pMaterial, type, index );
		if( storetype == TextureStorageType::None )
		{
			return nullptr;
		}

		Resource<Texture> pTexture = nullptr;

		aiString str;
		pMaterial->GetTexture( type, index, &str );

		if( storetype == TextureStorageType::IndexCompressed )
		{
			int idx = GetTextureIndex( &str );
			pTexture = std::make_shared<Texture>( reinterpret_cast<char*>(m_pAiScene->mTextures[idx]->pcData),
				m_pAiScene->mTextures[idx]->mWidth );
		}
		else if( storetype == TextureStorageType::IndexNonCompressed )
		{
			int idx = GetTextureIndex( &str );
			pTexture = std::make_shared<Texture>( reinterpret_cast<char*>(m_pAiScene->mTextures[idx]->pcData),
				m_pAiScene->mTextures[idx]->mWidth * m_pAiScene->mTextures[idx]->mHeight );
		}
		else if( storetype == TextureStorageType::EmbeddedCompressed )
		{
			auto pAiTex = m_pAiScene->GetEmbeddedTexture( str.C_Str() );
			pTexture = std::make_shared<Texture>( reinterpret_cast<char*>(pAiTex->pcData), pAiTex->mWidth );
		}
		else if( storetype == TextureStorageType::EmbeddedNonCompressed )
		{
			auto pAiTex = m_pAiScene->GetEmbeddedTexture( str.C_Str() );
			pTexture = std::make_shared<Texture>( reinterpret_cast<char*>(pAiTex->pcData), pAiTex->mWidth * pAiTex->mHeight );
		}
		else
		{
			const std::string filename = GetDirectory() + '/' + str.C_Str();
			pTexture = ResourceManager::GetTexture( filename );
		}

		return std::move( pTexture );
	}

	float SceneLoader::LoadMaterialFloat( aiMaterial* pMaterial, const char* key, unsigned int type, unsigned int index )
	{
		ai_real value;
		pMaterial->Get( key, type, index, value );
		return value;
	}

	Vec3 SceneLoader::LoadMaterialColour3( aiMaterial* pMaterial, const char* key, unsigned int type, unsigned int index )
	{
		aiColor3D colour;
		pMaterial->Get( key, type, index, colour );
		return Vec3{ colour.r, colour.g, colour.b };
	}

	Vec4 SceneLoader::LoadMaterialColour4( aiMaterial* pMaterial, const char* key, unsigned int type, unsigned int index )
	{
		aiColor4D colour;
		pMaterial->Get( key, type, index, colour );
		return Vec4{ colour.r, colour.g, colour.b, colour.a };
	}

	std::string SceneLoader::LoadMaterialString( aiMaterial* pMaterial, const char* key, unsigned int type, unsigned int index )
	{
		aiString value;
		pMaterial->Get( key, type, index, value );
		return value.C_Str();
	}

	void SceneLoader::AddAiBone( aiBone* pAiBone )
	{
		m_mapBoneNameToAiBone[pAiBone->mName.C_Str()] = pAiBone;
	}

	aiBone* SceneLoader::GetAiBoneByName( const std::string& name )
	{
		auto it = m_mapBoneNameToAiBone.find( name );
		if( it == m_mapBoneNameToAiBone.end() )
		{
			return nullptr;
		}

		return it->second;
	}

	int SceneLoader::AddSkeletonNode( aiNode* pAiNode, int parent_index )
	{
		int index = (int)m_OriginalSkeleton.bones.size();
		m_mapNodeNameToIndex[pAiNode->mName.C_Str()] = index;
		
		BoneNode node;
		node.transform = BoneTransform::FromMatrix( AiToBatMatrix( pAiNode->mTransformation ) );
		node.parent_index = parent_index;

		m_OriginalSkeleton.bones.push_back( node );

		return index;
	}

	int SceneLoader::AddBone( aiNode* pAiNode, aiBone* pAiBone, int parent_index )
	{
		int node_index = AddSkeletonNode( pAiNode, parent_index );

		BoneData bone;
		bone.index = node_index;
		bone.name = pAiBone->mName.C_Str();
		bone.inverse_bind_transform = AiToBatMatrix( pAiBone->mOffsetMatrix );

		m_mapBoneNameToIndex[bone.name] = (int)m_Bones.size();

		m_Bones.push_back( bone );

		return node_index;
	}

	int SceneLoader::FindNodeByName( const std::string& name ) const
	{
		auto it = m_mapNodeNameToIndex.find( name );
		if( it == m_mapNodeNameToIndex.end() )
		{
			return -1;
		}

		return it->second;
	}

	Entity SceneLoader::GetSceneNode( int node_index )
	{
		return m_SceneNodes[node_index];
	}

	int SceneLoader::FindBoneByName( const std::string& name ) const
	{
		auto it = m_mapBoneNameToIndex.find( name );
		if( it == m_mapBoneNameToIndex.end() )
		{
			return -1;
		}

		return it->second;
	}

	size_t SceneLoader::GetNumBones() const
	{
		return m_Bones.size();
	}

	void SceneLoader::LoadAnimations( AnimationComponent* animation_out )
	{
		m_Animations.reserve( m_pAiScene->mNumAnimations );

		for( unsigned int anim_index = 0; anim_index < m_pAiScene->mNumAnimations; anim_index++ )
		{
			aiAnimation* pAiAnimation = m_pAiScene->mAnimations[anim_index];

			float ticks_per_second = (float)pAiAnimation->mTicksPerSecond;
			if( m_szFilename.find( ".gltf" ) != std::string::npos || m_szFilename.find( ".glb" ) != std::string::npos )
			{
				ticks_per_second = 1000.0f;
			}
			if( ticks_per_second == 0.0f )
			{
				ticks_per_second = 1.0f;
			}

			AnimationClip animation;
			animation.name = pAiAnimation->mName.C_Str();
			animation.channels.resize( pAiAnimation->mNumChannels );
			animation.duration = (float)pAiAnimation->mDuration / ticks_per_second;

			for( unsigned int channel_index = 0; channel_index < pAiAnimation->mNumChannels; channel_index++ )
			{
				aiNodeAnim* pAiNodeAnim = pAiAnimation->mChannels[channel_index];
				int node_index = FindNodeByName( pAiNodeAnim->mNodeName.C_Str() );
				if( node_index == -1 )
				{
					BAT_WARN( "Missing animated node: %s", pAiNodeAnim->mNodeName.C_Str() );
					continue;
				}

				AnimationChannel& channel = animation.channels[channel_index];
				channel.node_index = node_index;

				channel.position_keyframes.reserve( pAiNodeAnim->mNumPositionKeys );
				for( unsigned int keyframe_index = 0; keyframe_index < pAiNodeAnim->mNumPositionKeys; keyframe_index++ )
				{
					const aiVectorKey& ai_key = pAiNodeAnim->mPositionKeys[keyframe_index];

					PosKeyFrame keyframe;
					keyframe.timestamp = (float)ai_key.mTime / ticks_per_second;
					keyframe.value.x = ai_key.mValue.x;
					keyframe.value.y = ai_key.mValue.y;
					keyframe.value.z = ai_key.mValue.z;
					
					channel.position_keyframes.push_back( keyframe );
				}

				channel.rotation_keyframes.reserve( pAiNodeAnim->mNumRotationKeys );
				for( unsigned int keyframe_index = 0; keyframe_index < pAiNodeAnim->mNumRotationKeys; keyframe_index++ )
				{
					const aiQuatKey& ai_key = pAiNodeAnim->mRotationKeys[keyframe_index];

					RotKeyFrame keyframe;
					keyframe.timestamp = (float)ai_key.mTime / ticks_per_second;
					keyframe.value.x = ai_key.mValue.x;
					keyframe.value.y = ai_key.mValue.y;
					keyframe.value.z = ai_key.mValue.z;
					keyframe.value.w = ai_key.mValue.w;

					channel.rotation_keyframes.push_back( keyframe );
				}
			}

			m_Animations.push_back( animation );
		}

		for( int i = 0; i < (int)m_OriginalSkeleton.bones.size(); i++ )
		{
			m_OriginalSkeleton.bones[i].entity = GetSceneNode( i );
		}

		animation_out->bind_pose = std::move( m_OriginalSkeleton );
		animation_out->bones = std::move( m_Bones );
		animation_out->clips = std::move( m_Animations );
		animation_out->current_pose = animation_out->bind_pose;
	}

	static void AddBoneWeight( std::vector<Veu4>* ids, std::vector<Vec4>* weights, unsigned int vertex_id, unsigned int bone_id, float weight )
	{
		Veu4& curr_id = (*ids)[vertex_id];
		Vec4& curr_weight = (*weights)[vertex_id];

		if( curr_weight.x == 0.0f )
		{
			curr_id.x = bone_id;
			curr_weight.x = weight;
		}
		else if( curr_weight.y == 0.0f )
		{
			curr_id.y = bone_id;
			curr_weight.y = weight;
		}
		else if( curr_weight.z == 0.0f )
		{
			curr_id.z = bone_id;
			curr_weight.z = weight;
		}
		else if( curr_weight.w == 0.0f )
		{
			curr_id.w = bone_id;
			curr_weight.w = weight;
		}
		else
		{
			ASSERT( false, "Single vertex has more than 4 contributing bone weights" );
		}
	}

	Resource<Mesh> SceneLoader::ProcessMesh( aiMesh* pAiMesh )
	{
		if( auto pBatMesh = GetLoadedMesh( pAiMesh ) )
		{
			return pBatMesh;
		}

		MeshParameters params;
		std::vector<unsigned int> indices;
		std::vector<BoneData> bones;
		Material material;
		
		if( pAiMesh->mMaterialIndex >= 0 )
		{
			aiMaterial* pMat = m_pAiScene->mMaterials[pAiMesh->mMaterialIndex];
		}

		// Reserve space in buffers
		if( pAiMesh->HasPositions() )             params.position.reserve( pAiMesh->mNumVertices );
		if( pAiMesh->HasNormals() )               params.normal.reserve( pAiMesh->mNumVertices );
		if( pAiMesh->HasTextureCoords( 0 ) )      params.uv.reserve( pAiMesh->mNumVertices );
		if( pAiMesh->HasTangentsAndBitangents() ) params.tangent.reserve( pAiMesh->mNumVertices );
		if( pAiMesh->HasTangentsAndBitangents() ) params.bitangent.reserve( pAiMesh->mNumVertices );
		if( pAiMesh->HasVertexColors( 0 ) )       params.colour.reserve( pAiMesh->mNumVertices );
		if( pAiMesh->HasBones() )                 params.bone_ids.resize( pAiMesh->mNumVertices, { 0, 0, 0, 0 } );
		if( pAiMesh->HasBones() )                 params.bone_weights.resize( pAiMesh->mNumVertices, { 0.0f, 0.0f, 0.0f, 0.0f } );

		for( unsigned int i = 0; i < pAiMesh->mNumVertices; i++ )
		{
			Vec3 position;
			position.x = pAiMesh->mVertices[i].x;
			position.y = pAiMesh->mVertices[i].y;
			position.z = pAiMesh->mVertices[i].z;
			params.position.emplace_back( position );

			Vec3 normal;
			normal.x = pAiMesh->mNormals[i].x;
			normal.y = pAiMesh->mNormals[i].y;
			normal.z = pAiMesh->mNormals[i].z;
			params.normal.emplace_back( normal );

			Vec2 texcoord;
			if( pAiMesh->HasTextureCoords( 0 ) )
			{
				texcoord.x = pAiMesh->mTextureCoords[0][i].x;
				texcoord.y = 1.0f - pAiMesh->mTextureCoords[0][i].y;
			}
			else
			{
				texcoord = { 0.0f, 0.0f };
			}
			params.uv.emplace_back( texcoord );

			if( pAiMesh->HasTangentsAndBitangents() )
			{
				Vec3 tangent;
				tangent.x = pAiMesh->mTangents[i].x;
				tangent.y = pAiMesh->mTangents[i].y;
				tangent.z = pAiMesh->mTangents[i].z;
				params.tangent.emplace_back( tangent );

				Vec3 bitangent;
				bitangent.x = pAiMesh->mBitangents[i].x;
				bitangent.y = pAiMesh->mBitangents[i].y;
				bitangent.z = pAiMesh->mBitangents[i].z;
				params.bitangent.emplace_back( bitangent );
			}

			if( pAiMesh->HasVertexColors( 0 ) )
			{
				Vec4 colour;
				colour.x = pAiMesh->mColors[0]->r;
				colour.y = pAiMesh->mColors[0]->g;
				colour.z = pAiMesh->mColors[0]->b;
				colour.w = pAiMesh->mColors[0]->a;
				params.colour.emplace_back( colour );
			}
		}

		for( unsigned int i = 0; i < pAiMesh->mNumFaces; i++ )
		{
			const aiFace face = pAiMesh->mFaces[i];
			ASSERT( face.mNumIndices == 3, "Face is not composed of triangles" );
			for( unsigned int j = 0; j < face.mNumIndices; j++ )
			{
				indices.push_back( face.mIndices[j] );
			}
		}

		for( unsigned int i = 0; i < pAiMesh->mNumBones; i++ )
		{
			aiBone* pBone = pAiMesh->mBones[i];
			int bone_index = FindBoneByName( pBone->mName.C_Str() );
			ASSERT( bone_index != -1, "Referencing invalid bone" );
			for( unsigned int weight_index = 0; weight_index < pBone->mNumWeights; weight_index++ )
			{
				const aiVertexWeight& pWeight = pBone->mWeights[weight_index];
				AddBoneWeight( &params.bone_ids, &params.bone_weights, pWeight.mVertexId, bone_index, pWeight.mWeight );
			}
		}

		if( pAiMesh->mMaterialIndex >= 0 )
		{
			aiMaterial* pAiMaterial = m_pAiScene->mMaterials[pAiMesh->mMaterialIndex];
			Resource<Texture> pTexture = nullptr;

			// Base colour
			{
				Vec4 colour = LoadMaterialColour4( pAiMaterial, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR );
				material.SetBaseColourFactor( colour.x, colour.y, colour.z, colour.w );

				if( pTexture = LoadMaterialTexture( pAiMaterial, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE ) )
				{
					material.SetBaseColour( std::move( pTexture ) );
				}
			}
			
			// Metallic & Roughness
			{
				float metallic = LoadMaterialFloat( pAiMaterial, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR );
				material.SetMetallicFactor( metallic );
				float roughness = LoadMaterialFloat( pAiMaterial, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR );
				material.SetRoughnessFactor( roughness );

				if( pTexture = LoadMaterialTexture( pAiMaterial, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE ) )
				{
					material.SetMetallicRoughness( std::move( pTexture ) );
				}
			}

			// Normal map
			{
				if( pTexture = LoadMaterialTexture( pAiMaterial, aiTextureType_NORMALS ) )
				{
					material.SetNormalMap( std::move( pTexture ) );
				}
			}

			// Occlusion map
			{
				if( pTexture = LoadMaterialTexture( pAiMaterial, aiTextureType_AMBIENT_OCCLUSION ) )
				{
					material.SetOcclusionMap( std::move( pTexture ) );
				}
			}

			// Emissive map
			{
				Vec3 emissive = LoadMaterialColour3( pAiMaterial, AI_MATKEY_COLOR_EMISSIVE );
				material.SetEmissiveFactor( emissive.x, emissive.y, emissive.z );

				if( pTexture = LoadMaterialTexture( pAiMaterial, aiTextureType_EMISSIVE ) )
				{
					material.SetEmissiveMap( std::move( pTexture ) );
				}
			}

			// Alpha mode/cutoff
			{
				std::string alphamode = LoadMaterialString( pAiMaterial, AI_MATKEY_GLTF_ALPHAMODE );
				if( alphamode == "OPAQUE" )     material.SetAlphaMode( AlphaMode::NONE );
				else if( alphamode == "MASK" )  material.SetAlphaMode( AlphaMode::MASK );
				else if( alphamode == "BLEND" ) material.SetAlphaMode( AlphaMode::BLEND );

				float cutoff = LoadMaterialFloat( pAiMaterial, AI_MATKEY_GLTF_ALPHACUTOFF );
				material.SetAlphaCutoff( cutoff );
			}
		}

		auto pBatMesh = std::make_shared<Mesh>( params, indices, material );
		pBatMesh->SetName( pAiMesh->mName.C_Str() );
		LoadedMesh loaded_mesh;
		loaded_mesh.pAssimpMesh = pAiMesh;
		loaded_mesh.pBatMesh = pBatMesh;
		m_LoadedMeshes.emplace_back( loaded_mesh );

		return pBatMesh;
	}

	void SceneLoader::PreProcess()
	{
		for( unsigned int mesh_index = 0; mesh_index < m_pAiScene->mNumMeshes; mesh_index++ )
		{
			aiMesh* pAiMesh = m_pAiScene->mMeshes[mesh_index];
			for( unsigned int bone_index = 0; bone_index < pAiMesh->mNumBones; bone_index++ )
			{
				aiBone* pAiBone = pAiMesh->mBones[bone_index];
				BAT_TRACE( "Bone: %s", pAiBone->mName.C_Str() );
				AddAiBone( pAiBone );
			}
		}

		BuildSkeleton( m_pAiScene->mRootNode, -1 );

		m_SceneNodes.resize( m_mapNodeNameToIndex.size() );
	}

	void SceneLoader::BuildSkeleton( aiNode* pAiNode, int parent_index )
	{
		BAT_TRACE( "Node: %s", pAiNode->mName.C_Str() );
		aiBone* pAiBone = GetAiBoneByName( pAiNode->mName.C_Str() );

		int index;
		if( !pAiBone )
		{
			index = AddSkeletonNode( pAiNode, parent_index );
		}
		else
		{
			index = AddBone( pAiNode, pAiBone, parent_index );
		}

		for( unsigned int i = 0; i < pAiNode->mNumChildren; i++ )
		{
			BuildSkeleton( pAiNode->mChildren[i], index );
		}
	}

	void SceneLoader::ProcessNode( aiNode* pAiNode, SceneNode& node )
	{
		Entity e = node.Get();
		e.Add<NameComponent>( pAiNode->mName.C_Str() );

		const auto transform = AiToBatMatrix( pAiNode->mTransformation );
		e.Get<TransformComponent>()
			.SetLocalMatrix( transform );
		
		int node_index = FindNodeByName( pAiNode->mName.C_Str() );
		if( node_index != -1 )
		{
			m_SceneNodes[node_index] = e;
		}

		if( pAiNode->mNumMeshes > 0 )
		{
			std::vector<Resource<Mesh>> meshes;
			meshes.reserve( pAiNode->mNumMeshes );
			for( unsigned int i = 0; i < pAiNode->mNumMeshes; i++ )
			{
				aiMesh* pMesh = m_pAiScene->mMeshes[pAiNode->mMeshes[i]];
				meshes.emplace_back( ProcessMesh( pMesh ) );
			}
			e.Add<ModelComponent>( meshes );
		}

		for( unsigned int i = 0; i < pAiNode->mNumChildren; i++ )
		{
			Entity child = world.CreateEntity();
			SceneNode* child_node = node.AddChild( child );
			ProcessNode( pAiNode->mChildren[i], *child_node );
		}
	}

	SceneLoader::SceneLoader()
	{
	}

	SceneLoader::~SceneLoader()
	{
	}

	SceneNode SceneLoader::Load( const std::string& filename )
	{
		if( !this->ReadFile( filename ) )
		{
			return {};
		}

		FrameTimer ft;

		PreProcess();

		SceneNode root_node;
		Entity e = world.CreateEntity();
		root_node.Set( e );

		ProcessNode( m_pAiScene->mRootNode, root_node );

		if( m_pAiScene->HasAnimations() )
		{
			auto& anim = e.Add<AnimationComponent>();
			LoadAnimations( &anim );
		}

		float time = ft.Mark();
		BAT_LOG( "Loaded model '%s' in %.2fs", filename, time );

		m_LoadedMeshes.clear();

		return root_node;
	}

	std::shared_ptr<Mesh> SceneLoader::LoadMesh( const std::string& filename )
	{
		if( !this->ReadFile( filename ) )
		{
			return {};
		}

		return ExtractMesh( m_pAiScene->mRootNode );
	}

	bool SceneLoader::ReadFile( const std::string& filename )
	{
		m_szFilename = filename;
		if( !std::ifstream( filename ) )
		{
			BAT_WARN( "Could not open model file '%s'", filename );
			ASSERT( false, "Could not open model file '%s'", filename );
			return {};
		}

		std::filesystem::path filepath( m_szFilename );
		m_szDirectory = filepath.parent_path().string();

		m_pAiScene = m_Importer.ReadFile( m_szFilename,aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcessPreset_TargetRealtime_Fast | aiProcess_TransformUVCoords );

		if( !m_pAiScene )
		{
			ASSERT( false, "Failed to load model '%s' (%s)", m_szFilename, m_Importer.GetErrorString() );
			BAT_WARN( "Failed to load model '%s'", m_szFilename );
			return false;
		}

		return true;
	}
}
