#include "PCH.h"
#include "Scene.h"

#include "Model.h"
#include "Light.h"

namespace Bat
{
	BasicSceneNode::BasicSceneNode( const DirectX::XMMATRIX& transform, ISceneNode* pParent )
		:
		m_pParentNode( pParent ),
		m_matTransform( transform )
	{}

	ISceneNode* BasicSceneNode::GetParentNode()
	{
		return m_pParentNode;
	}

	std::vector<ISceneNode*> BasicSceneNode::GetChildNodes()
	{
		std::vector<ISceneNode*> res( m_pChildNodes.size() );
		std::transform( m_pChildNodes.begin(), m_pChildNodes.end(), res.begin(), []( std::unique_ptr<ISceneNode>& pNode )
		{
			return pNode.get();
		} );
		return res;
	}

	void BasicSceneNode::AddChildNode( std::unique_ptr<ISceneNode> pNode )
	{
		m_pChildNodes.emplace_back( std::move( pNode ) );
	}

	bool BasicSceneNode::RemoveChildNode( ISceneNode* pTarget )
	{
		for( size_t i = 0; i < m_pChildNodes.size(); i++ )
		{
			if( m_pChildNodes[i].get() == pTarget )
			{
				m_pChildNodes.erase( m_pChildNodes.begin() + i );
				return true;
			}
		}

		return false;
	}

	size_t BasicSceneNode::GetModelCount()
	{
		return m_pModels.size();
	}

	Model * BasicSceneNode::GetModel( size_t index )
	{
		return m_pModels[index].get();
	}

	Model* BasicSceneNode::AddModel( const Model& model )
	{
		m_pModels.emplace_back( std::make_unique<Model>( model ) );
		return m_pModels.back().get();
	}

	void BasicSceneNode::RemoveModel( Model* pModel )
	{
		m_pModels.erase( std::remove_if( m_pModels.begin(), m_pModels.end(),
			[pModel]( const std::unique_ptr<Model>& l )
			{
				return l.get() == pModel;
			}
		) );
	}

	size_t BasicSceneNode::GetLightCount()
	{
		return m_pLights.size();
	}

	Light* BasicSceneNode::GetLight( size_t index )
	{
		return m_pLights[index].get();
	}

	Light* BasicSceneNode::AddLight( const Light& light )
	{
		m_pLights.emplace_back( std::make_unique<Light>( light ) );
		return m_pLights.back().get();
	}

	void BasicSceneNode::RemoveLight( Light* pLight )
	{
		m_pLights.erase( std::remove_if( m_pLights.begin(), m_pLights.end(),
			[pLight]( const std::unique_ptr<Light>& l )
			{
				return l.get() == pLight;
			}
		) );
	}

	DirectX::XMMATRIX BasicSceneNode::GetTransform() const
	{
		return m_matTransform;
	}

	void BasicSceneNode::SetTransform( const DirectX::XMMATRIX& transform )
	{
		m_matTransform = transform;
	}

	void BasicSceneNode::AcceptVisitor( const DirectX::XMMATRIX& transform, ISceneVisitor& visitor )
	{
		auto new_transform = m_matTransform * transform;
		visitor.Visit( new_transform, *this );
		for( auto& child : m_pChildNodes )
		{
			child->AcceptVisitor( new_transform, visitor );
		}
	}

	ISceneNode& SceneGraph::GetRootNode()
	{
		return m_RootNode;
	}

	void SceneGraph::AcceptVisitor( ISceneVisitor& visitor )
	{
		m_RootNode.AcceptVisitor( m_RootNode.GetTransform(), visitor );
	}
}
