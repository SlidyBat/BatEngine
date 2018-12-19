#include "PCH.h"
#include "Scene.h"

#include "Model.h"
#include "PoolAllocator.h"

namespace Bat
{
	PoolAllocator<Model> g_ModelAllocator( 500 );

	BasicSceneNode::BasicSceneNode( const DirectX::XMMATRIX& transform, ISceneNode* pParent )
		:
		m_pParentNode( pParent ),
		m_matTransform( transform )
	{}

	BasicSceneNode::~BasicSceneNode()
	{
		for( auto pModel : m_pModels )
		{
			g_ModelAllocator.Free( pModel );
		}
	}

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

	std::vector<Model*> BasicSceneNode::GetModels()
	{
		return m_pModels;
	}

	Model* BasicSceneNode::AddModel( const Model& model )
	{
		Model* pModel = g_ModelAllocator.Alloc( model );
		m_pModels.emplace_back( pModel );
		return pModel;
	}

	void BasicSceneNode::RemoveModel( Model* pModel )
	{
		m_pModels.erase( std::remove( m_pModels.begin(), m_pModels.end(), pModel ) );
		g_ModelAllocator.Free( pModel );
	}

	DirectX::XMMATRIX BasicSceneNode::GetTransform() const
	{
		return m_matTransform;
	}

	void BasicSceneNode::SetTransform( const DirectX::XMMATRIX& transform )
	{
		m_matTransform = transform;
	}

	void BasicSceneNode::AcceptVisitor( ISceneVisitor& visitor )
	{
		visitor.Visit( *this );
		for( auto pModel : m_pModels )
		{
			visitor.Visit( *pModel );
		}

		for( auto& child : m_pChildNodes )
		{
			child->AcceptVisitor( visitor );
		}
	}

	ISceneNode& SceneGraph::GetRootNode()
	{
		return m_RootNode;
	}

	void SceneGraph::AcceptVisitor( ISceneVisitor& visitor )
	{
		m_RootNode.AcceptVisitor( visitor );
	}
}
