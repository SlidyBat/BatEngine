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
			case aiTextureType_NORMALS:
			case aiTextureType_HEIGHT:
				return nullptr; // we don't create default bumpmaps
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
		MeshParameters params;
		std::vector<int> indices;
		Material* pMeshMaterial = new Material();
		
		if( pMesh->mMaterialIndex >= 0 )
		{
			aiMaterial* pMat = pScene->mMaterials[pMesh->mMaterialIndex];
		}

		for( UINT i = 0; i < pMesh->mNumVertices; i++ )
		{
			Vec4 position;
			position.x = pMesh->mVertices[i].x;
			position.y = pMesh->mVertices[i].y;
			position.z = pMesh->mVertices[i].z;
			position.w = 1.0f;
			params.position.emplace_back( position );

			Vec4 normal;
			normal.x = pMesh->mNormals[i].x;
			normal.y = pMesh->mNormals[i].y;
			normal.z = pMesh->mNormals[i].z;
			normal.w = 0.0f;
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
				Vec4 tangent;
				tangent.x = pMesh->mTangents[i].x;
				tangent.x = pMesh->mTangents[i].x;
				tangent.x = pMesh->mTangents[i].x;
				tangent.w = 0.0f;
				params.tangent.emplace_back( tangent );

				Vec4 bitangent;
				bitangent.x = pMesh->mTangents[i].x;
				bitangent.y = pMesh->mTangents[i].y;
				bitangent.z = pMesh->mTangents[i].z;
				tangent.w = 0.0f;
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
			Texture* pTexture = nullptr;

			pTexture = LoadMaterialTexture( pMaterial, aiTextureType_DIFFUSE, pScene, dir );
			pMeshMaterial->SetDiffuseTexture( pTexture );
			pTexture = LoadMaterialTexture( pMaterial, aiTextureType_SPECULAR, pScene, dir );
			pMeshMaterial->SetSpecularTexture( pTexture );
			pTexture = LoadMaterialTexture( pMaterial, aiTextureType_EMISSIVE, pScene, dir );
			if( !pTexture )
			{
				pTexture = LoadMaterialTexture( pMaterial, aiTextureType_AMBIENT, pScene, dir );
			}
			pMeshMaterial->SetEmissiveTexture( pTexture );
			pTexture = LoadMaterialTexture( pMaterial, aiTextureType_NORMALS, pScene, dir );
			if( !pTexture )
			{
				pTexture = LoadMaterialTexture( pMaterial, aiTextureType_HEIGHT, pScene, dir );
			}
			pMeshMaterial->SetBumpMapTexture( pTexture );

			float shininess = 0.0f;
			pMaterial->Get( AI_MATKEY_SHININESS, shininess );
			if( shininess == 0.0f )
			{
				shininess = 32.0f;
			}
			pMeshMaterial->SetShininess( shininess );
		}

		return Mesh( params, indices, pMeshMaterial );
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
		const aiScene* pScene = importer.ReadFile( filename, aiProcess_ConvertToLeftHanded | aiProcess_Triangulate | aiProcess_CalcTangentSpace );

		if( pScene == nullptr )
		{
			ASSERT( false, "Failed to load model" );
			return {};
		}

		ProcessNode( pScene->mRootNode, pScene, meshes, directory );

		return meshes;
	}
}
