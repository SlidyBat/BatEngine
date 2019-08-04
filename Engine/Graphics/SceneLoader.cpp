#include "PCH.h"
#include "SceneLoader.h"

#include "CoreEntityComponents.h"
#include "Mesh.h"
#include "Model.h"
#include "Colour.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Material.h"
#include "FrameTimer.h"

namespace Bat
{
	struct LoadedMesh
	{
		aiMesh* pAssimpMesh;
		Resource<Mesh> pBatMesh;
	};
	static std::vector<LoadedMesh> g_LoadedMeshes;
	Resource<Mesh> GetLoadedMesh( const aiMesh* pTarget )
	{
		auto it = std::find_if( g_LoadedMeshes.begin(), g_LoadedMeshes.end(), [pTarget]( const LoadedMesh& loaded )
		{
			return loaded.pAssimpMesh == pTarget;
		} );

		if( it == g_LoadedMeshes.end() )
		{
			return nullptr;
		}

		return it->pBatMesh;
	}

	enum class TextureStorageType
	{
		None,
		IndexCompressed,
		IndexNonCompressed,
		EmbeddedCompressed,
		EmbeddedNonCompressed,
		Disk
	};

	TextureStorageType DetermineTextureStorageType( const aiScene* pScene, aiMaterial* pMat, aiTextureType type )
	{
		if( !pMat->GetTextureCount( type ) )
		{
			return TextureStorageType::None;
		}

		aiString textype;
		pMat->GetTexture( type, 0, &textype );
		if( textype.C_Str()[0] == '*' )
		{
			if( pScene->mTextures[0]->mHeight == 0 )
			{
				return TextureStorageType::IndexCompressed;
			}
			else
			{
				return TextureStorageType::IndexNonCompressed;
			}
		}
		else if( auto pTex = pScene->GetEmbeddedTexture( textype.C_Str() ) )
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

	static void LoadMaterialTexture( Material& mat, aiMaterial* pMaterial, aiTextureType type, const aiScene* pScene, const std::string& dir, TextureStorageType storetype )
	{
		Resource<Texture> pTexture = nullptr;

		for( UINT i = 0; i < pMaterial->GetTextureCount( type ) && i < 1; i++ )
		{
			aiString str;
			pMaterial->GetTexture( type, i, &str );

			if( storetype == TextureStorageType::IndexCompressed )
			{
				int idx = GetTextureIndex( &str );
				pTexture = std::make_shared<Texture>( reinterpret_cast<char*>(pScene->mTextures[idx]->pcData),
					pScene->mTextures[idx]->mWidth );
			}
			else if( storetype == TextureStorageType::IndexNonCompressed )
			{
				int idx = GetTextureIndex( &str );
				pTexture = std::make_shared<Texture>( reinterpret_cast<char*>(pScene->mTextures[idx]->pcData),
					pScene->mTextures[idx]->mWidth * pScene->mTextures[idx]->mHeight );
			}
			else if( storetype == TextureStorageType::EmbeddedCompressed )
			{
				auto pAiTex = pScene->GetEmbeddedTexture( str.C_Str() );
				pTexture = std::make_shared<Texture>( reinterpret_cast<char*>(pAiTex->pcData), pAiTex->mWidth );
			}
			else if( storetype == TextureStorageType::EmbeddedNonCompressed )
			{
				auto pAiTex = pScene->GetEmbeddedTexture( str.C_Str() );
				pTexture = std::make_shared<Texture>( reinterpret_cast<char*>(pAiTex->pcData), pAiTex->mWidth * pAiTex->mHeight );
			}
			else
			{
				const std::string filename = dir + '/' + str.C_Str();
				pTexture = ResourceManager::GetTexture( filename );
			}
		}

		switch( type )
		{
			case aiTextureType_AMBIENT:
				mat.SetAmbientTexture( std::move( pTexture ) );
				break;
			case aiTextureType_DIFFUSE:
				mat.SetDiffuseTexture( std::move( pTexture ) );
				break;
			case aiTextureType_SPECULAR:
				mat.SetSpecularTexture( std::move( pTexture ) );
				break;
			case aiTextureType_EMISSIVE:
				mat.SetEmissiveTexture( std::move( pTexture ) );
				break;
			case aiTextureType_NORMALS:
			case aiTextureType_HEIGHT:
				mat.SetNormalTexture( std::move( pTexture ) );
				break;
			default:
				ASSERT( false, "Unknown texture type" );
		}
	}

	void LoadMaterialColour( Material& mat, aiMaterial* pMaterial, aiTextureType type )
	{
		aiColor3D aiColour( 0.0f, 0.0f, 0.0f );
		switch( type )
		{
			case aiTextureType_AMBIENT:
				pMaterial->Get( AI_MATKEY_COLOR_AMBIENT, aiColour );
				mat.SetAmbientColour( aiColour.r, aiColour.g, aiColour.b );
				break;
			case aiTextureType_DIFFUSE:
				pMaterial->Get( AI_MATKEY_COLOR_DIFFUSE, aiColour );
				mat.SetDiffuseColour( aiColour.r, aiColour.g, aiColour.b );
				break;
			case aiTextureType_SPECULAR:
				pMaterial->Get( AI_MATKEY_COLOR_SPECULAR, aiColour );
				if( aiColour.IsBlack() ) // Yuck!
				{
					mat.SetSpecularColour( 1.0f, 1.0f, 1.0f );
				}
				else
				{
					mat.SetSpecularColour( aiColour.r, aiColour.g, aiColour.b );
				}
				break;
			case aiTextureType_EMISSIVE:
				pMaterial->Get( AI_MATKEY_COLOR_EMISSIVE, aiColour );
				mat.SetEmissiveColour( aiColour.r, aiColour.g, aiColour.b );
				break;
			case aiTextureType_NORMALS:
			case aiTextureType_HEIGHT:
				return; // we don't create default normal colours
			default:
				ASSERT( false, "Unknown texture type" );
		}
	}

	static void LoadMaterialTextureType( Material& mat, aiMaterial* pMaterial, aiTextureType type, const aiScene* pScene, const std::string& dir )
	{
		auto storetype = DetermineTextureStorageType( pScene, pMaterial, type );

		if( storetype == TextureStorageType::None )
		{
			LoadMaterialColour( mat, pMaterial, type );
		}
		else
		{
			LoadMaterialTexture( mat, pMaterial, type, pScene, dir, storetype );
		}
	}

	static Resource<Mesh> ProcessMesh( aiMesh* pMesh, const aiScene* pScene, const std::string& dir )
	{
		if( auto pBatMesh = GetLoadedMesh( pMesh ) )
		{
			return pBatMesh;
		}

		MeshParameters params;
		std::vector<unsigned int> indices;
		Material material;
		
		if( pMesh->mMaterialIndex >= 0 )
		{
			aiMaterial* pMat = pScene->mMaterials[pMesh->mMaterialIndex];
		}

		for( UINT i = 0; i < pMesh->mNumVertices; i++ )
		{
			Vec3 position;
			position.x = pMesh->mVertices[i].x;
			position.y = pMesh->mVertices[i].y;
			position.z = pMesh->mVertices[i].z;
			params.position.emplace_back( position );

			Vec3 normal;
			normal.x = pMesh->mNormals[i].x;
			normal.y = pMesh->mNormals[i].y;
			normal.z = pMesh->mNormals[i].z;
			params.normal.emplace_back( normal );

			Vec2 texcoord;
			if( pMesh->mTextureCoords[0] )
			{
				texcoord.x = pMesh->mTextureCoords[0][i].x;
				texcoord.y = pMesh->mTextureCoords[0][i].y;
			}
			else
			{
				texcoord = { 0.0f, 0.0f };
			}
			params.uv.emplace_back( texcoord );

			if( pMesh->HasTangentsAndBitangents() )
			{
				Vec3 tangent;
				tangent.x = pMesh->mTangents[i].x;
				tangent.y = pMesh->mTangents[i].y;
				tangent.z = pMesh->mTangents[i].z;
				params.tangent.emplace_back( tangent );

				Vec3 bitangent;
				bitangent.x = pMesh->mBitangents[i].x;
				bitangent.y = pMesh->mBitangents[i].y;
				bitangent.z = pMesh->mBitangents[i].z;
				params.bitangent.emplace_back( bitangent );
			}

			if( pMesh->HasVertexColors(0) )
			{
				Vec4 colour;
				colour.x = pMesh->mColors[0]->r;
				colour.y = pMesh->mColors[0]->g;
				colour.z = pMesh->mColors[0]->b;
				colour.w = pMesh->mColors[0]->a;
				params.colour.emplace_back( colour );
			}
		}

		for( UINT i = 0; i < pMesh->mNumFaces; i++ )
		{
			const aiFace face = pMesh->mFaces[i];
			ASSERT( face.mNumIndices == 3, "Face is not composed of triangles" );
			for( UINT j = 0; j < face.mNumIndices; j++ )
			{
				indices.push_back( face.mIndices[j] );
			}
		}

		if( pMesh->mMaterialIndex >= 0 )
		{
			aiMaterial* pMaterial = pScene->mMaterials[pMesh->mMaterialIndex];
			Resource<Texture> pTexture = nullptr;

			LoadMaterialTextureType( material, pMaterial, aiTextureType_DIFFUSE, pScene, dir );
			LoadMaterialTextureType( material, pMaterial, aiTextureType_SPECULAR, pScene, dir );
			LoadMaterialTextureType( material, pMaterial, aiTextureType_EMISSIVE, pScene, dir );
			LoadMaterialTextureType( material, pMaterial, aiTextureType_AMBIENT, pScene, dir );
			LoadMaterialTextureType( material, pMaterial, aiTextureType_NORMALS, pScene, dir );
			if( !material.GetNormalTexture() )
			{
				LoadMaterialTextureType( material, pMaterial, aiTextureType_HEIGHT, pScene, dir );
			}

			float shininess = 0.0f;
			pMaterial->Get( AI_MATKEY_SHININESS, shininess );
			if( shininess <= 1.0f )
			{
				shininess = 32.0f;
			}
			material.SetShininess( shininess );
		}

		auto pBatMesh = std::make_shared<Mesh>( params, indices, material );
		pBatMesh->SetName( pMesh->mName.C_Str() );
		LoadedMesh loaded_mesh;
		loaded_mesh.pAssimpMesh = pMesh;
		loaded_mesh.pBatMesh = pBatMesh;
		g_LoadedMeshes.emplace_back( loaded_mesh );

		return pBatMesh;
	}

	static void ProcessNode( aiNode* pAssimpNode, const aiScene* pAssimpScene, SceneNode& node, const std::string& dir )
	{
		Entity e = node.Get();
		e.Add<NameComponent>( pAssimpNode->mName.C_Str() );

		if( pAssimpNode->mNumMeshes > 0 )
		{
			std::vector<Resource<Mesh>> meshes;
			meshes.reserve( pAssimpNode->mNumMeshes );
			for( UINT i = 0; i < pAssimpNode->mNumMeshes; i++ )
			{
				aiMesh* pMesh = pAssimpScene->mMeshes[pAssimpNode->mMeshes[i]];
				meshes.emplace_back( ProcessMesh( pMesh, pAssimpScene, dir ) );
			}
			e.Add<ModelComponent>( meshes );
		}

		for( UINT i = 0; i < pAssimpNode->mNumChildren; i++ )
		{
			const auto transform = DirectX::XMMatrixTranspose(
				DirectX::XMMATRIX( &pAssimpNode->mTransformation.a1 )
			);

			Entity child = world.CreateEntity();
			child.Add<TransformComponent>( transform );
			
			size_t new_node_idx = node.AddChild( child );
			ProcessNode( pAssimpNode->mChildren[i], pAssimpScene, node.GetChild( new_node_idx ), dir );
		}
	}

	SceneNode SceneLoader::LoadScene( const std::string& filename )
	{
		if( !std::ifstream( filename ) )
		{
			BAT_WARN( "Could not open model file '%s'", filename );
			ASSERT( false, "Could not open model file '%s'", filename );
			return {};
		}
		
		FrameTimer ft;

		SceneNode node;
		Entity e = world.CreateEntity();
		node.Set( e );

		std::filesystem::path filepath( filename );
		std::string directory = filepath.parent_path().string();

		Assimp::Importer importer;
		const aiScene* pAssimpScene = importer.ReadFile( filename, aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast | aiProcess_TransformUVCoords );

		if( pAssimpScene == nullptr )
		{
			ASSERT( false, "Failed to load model '%s' (%s)", filename, importer.GetErrorString() );
			BAT_WARN( "Failed to load model '%s'", filename );
			return {};
		}

		ProcessNode( pAssimpScene->mRootNode, pAssimpScene, node, directory );

		float time = ft.Mark();
		BAT_LOG( "Loaded model '%s' in %.2fs", filename, time );

		g_LoadedMeshes.clear();

		return node;
	}
}
