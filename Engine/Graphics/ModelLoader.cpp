#include "ModelLoader.h"

#include "BatAssert.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Material.h"

namespace Bat
{
	enum class TextureStorageType
	{
		None,
		EmbeddedCompressed,
		EmbeddedNonCompressed,
		Disk
	};

	TextureStorageType DetermineTextureStorageType( const aiScene* pScene, aiMaterial* pMat, aiTextureType type )
	{
		aiString textypec;
		pMat->GetTexture( type, 0, &textypec );
		std::string textype = textypec.C_Str();
		if( textype == "*0" || textype == "*1" || textype == "*2" || textype == "*3" || textype == "*4" || textype == "*5" )
		{
			if( pScene->mTextures[0]->mHeight == 0 )
			{
				return TextureStorageType::EmbeddedCompressed;
			}
			else
			{
				return TextureStorageType::EmbeddedNonCompressed;
			}
		}
		if( textype.find( '.' ) != std::string::npos )
		{
			return TextureStorageType::Disk;
		}

		return TextureStorageType::None; // default to disk and let it fail
	}

	static int GetTextureIndex( aiString* pStr )
	{
		if( !pStr->length )
		{
			return 0; // -1?
		}
		return pStr->C_Str()[0] - '0';
	}

	static Texture* GetTextureFromModel( const aiScene* pScene, int idx )
	{
		const int size = pScene->mTextures[idx]->mWidth;

		return new Texture( reinterpret_cast<uint8_t*>(pScene->mTextures[idx]->pcData), size );
	}

	static Texture* LoadMaterialTexture( aiMaterial* pMaterial, aiTextureType type, const aiScene* pScene, const std::string& dir )
	{
		auto storetype = DetermineTextureStorageType( pScene, pMaterial, type );
		if( storetype == TextureStorageType::None )
		{
			aiColor3D aiColour( 0.0f, 0.0f, 0.0f );
			switch( type )
			{
			case aiTextureType_AMBIENT:
			{
				pMaterial->Get( AI_MATKEY_COLOR_AMBIENT, aiColour );
				break;
			}
			case aiTextureType_DIFFUSE:
				pMaterial->Get( AI_MATKEY_COLOR_DIFFUSE, aiColour );
				// no diffuse is uggo, just use light diffuse
				if( aiColour.IsBlack() )
				{
					aiColour.r = 1.0f;
					aiColour.g = 1.0f;
					aiColour.b = 1.0f;
				}
				break;
			case aiTextureType_SPECULAR:
				pMaterial->Get( AI_MATKEY_COLOR_SPECULAR, aiColour );
				// no specular is uggo, just use light specular
				if( aiColour.IsBlack() )
				{
					aiColour.r = 1.0f;
					aiColour.g = 1.0f;
					aiColour.b = 1.0f;
				}
				break;
			case aiTextureType_EMISSIVE:
				pMaterial->Get( AI_MATKEY_COLOR_EMISSIVE, aiColour );
				break;
			default:
				ASSERT( false, "Unknown texture type" );
			}
			Colour colour;
			colour.SetA( 255 );
			colour.SetR( (unsigned char)(aiColour.r * 255) );
			colour.SetG( (unsigned char)(aiColour.g * 255) );
			colour.SetB( (unsigned char)(aiColour.b * 255) );

			return new Texture( &colour, 1, 1 );
		}

		for( UINT i = 0; i < pMaterial->GetTextureCount( type ) && i < 1; i++ )
		{
			aiString str;
			pMaterial->GetTexture( type, i, &str );

			if( storetype == TextureStorageType::EmbeddedCompressed )
			{
				int idx = GetTextureIndex( &str );
				return GetTextureFromModel( pScene, idx );
			}
			else
			{
				std::string filename = dir + '/' + str.C_Str();
				std::wstring filenamews( filename.begin(), filename.end() );
				return new Texture( filenamews );
			}
		}

		return nullptr;
	}

	static Mesh ProcessMesh( aiMesh* pMesh, const aiScene* pScene, const std::string& dir )
	{
		std::vector<Vertex> vertices;
		std::vector<int> indices;
		Material* pMeshMaterial = new Material();
		
		if( pMesh->mMaterialIndex >= 0 )
		{
			aiMaterial* pMat = pScene->mMaterials[pMesh->mMaterialIndex];
		}

		for( UINT i = 0; i < pMesh->mNumVertices; i++ )
		{
			Vertex vertex;
			vertex.position.x = pMesh->mVertices[i].x;
			vertex.position.y = pMesh->mVertices[i].y;
			vertex.position.z = pMesh->mVertices[i].z;
			vertex.normal.x = pMesh->mNormals[i].x;
			vertex.normal.y = pMesh->mNormals[i].y;
			vertex.normal.z = pMesh->mNormals[i].z;

			if( pMesh->mTextureCoords[0] )
			{
				vertex.texcoord.x = pMesh->mTextureCoords[0][i].x;
				vertex.texcoord.y = pMesh->mTextureCoords[0][i].y;
			}
			else
			{
				vertex.texcoord = { 0.0f, 0.0f };
			}

			vertices.emplace_back( vertex );
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
			Texture* pTexture = nullptr;

			pTexture = LoadMaterialTexture( pMaterial, aiTextureType_AMBIENT, pScene, dir );
			pMeshMaterial->SetAmbientTexture( pTexture );
			pTexture = LoadMaterialTexture( pMaterial, aiTextureType_DIFFUSE, pScene, dir );
			pMeshMaterial->SetDiffuseTexture( pTexture );
			pTexture = LoadMaterialTexture( pMaterial, aiTextureType_SPECULAR, pScene, dir );
			pMeshMaterial->SetSpecularTexture( pTexture );
			pTexture = LoadMaterialTexture( pMaterial, aiTextureType_EMISSIVE, pScene, dir );
			pMeshMaterial->SetEmissiveTexture( pTexture );

			float shininess = 0.0f;
			pMaterial->Get( AI_MATKEY_SHININESS, shininess );
			if( shininess == 0.0f )
			{
				shininess = 32.0f;
			}
			pMeshMaterial->SetShininess( shininess );
		}

		return Mesh( vertices, indices, pMeshMaterial );
	}

	static void ProcessNode( aiNode* pNode, const aiScene* pScene, std::vector<Mesh>& meshes, const std::string& dir )
	{
		for( UINT i = 0; i < pNode->mNumMeshes; i++ )
		{
			aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
			meshes.emplace_back( ProcessMesh( pMesh, pScene, dir ) );
		}

		for( UINT i = 0; i < pNode->mNumChildren; i++ )
		{
			ProcessNode( pNode->mChildren[i], pScene, meshes, dir );
		}
	}

	std::vector<Mesh> ModelLoader::LoadModel( const std::string& filename )
	{
		std::vector<Mesh> meshes;
		std::string directory = filename.substr( 0, filename.find_last_of( '/' ) );

		Assimp::Importer importer;
		const aiScene* pScene = importer.ReadFile( filename, aiProcess_ConvertToLeftHanded | aiProcess_Triangulate );

		if( pScene == nullptr )
		{
			ASSERT( false, "Failed to load model" );
			return {};
		}

		ProcessNode( pScene->mRootNode, pScene, meshes, directory );

		return meshes;
	}
}
