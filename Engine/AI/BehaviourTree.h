#pragma once

#include <functional>
#include "Core/Entity.h"

namespace Bat
{
	enum class BehaviourResult
	{
		RUNNING,
		SUCCEEDED,
		FAILED
	};

	class BehaviourNode
	{
	public:
		virtual ~BehaviourNode() = default;

		virtual BehaviourResult Go( Entity e ) = 0;
	};

	class SequenceNode : public BehaviourNode
	{
	public:
		SequenceNode& Add( std::unique_ptr<BehaviourNode> node )
		{
			m_Nodes.push_back( std::move( node ) );
			return *this;
		}

		virtual BehaviourResult Go( Entity e ) override
		{
			auto& node = m_Nodes[m_CurrentNode];
			
			BehaviourResult result = node->Go( e );
			switch( result )
			{
			case BehaviourResult::RUNNING:
				return BehaviourResult::RUNNING;
			case BehaviourResult::SUCCEEDED:
				m_CurrentNode++;
				if( m_CurrentNode == m_Nodes.size() )
				{
					m_CurrentNode = 0;
					return BehaviourResult::SUCCEEDED;
				}
				return BehaviourResult::RUNNING;
			case BehaviourResult::FAILED:
				m_CurrentNode = 0;
				return BehaviourResult::FAILED;
			}

			ASSERT( false, "Unhandled behaviour result" );
			return BehaviourResult::RUNNING;
		}
	private:
		size_t m_CurrentNode = 0;
		std::vector<std::unique_ptr<BehaviourNode>> m_Nodes;
	};

	class SelectorNode : public BehaviourNode
	{
	public:
		SelectorNode& Add( std::unique_ptr<BehaviourNode> node )
		{
			m_Nodes.push_back( std::move( node ) );
			return *this;
		}

		virtual BehaviourResult Go( Entity e ) override
		{
			auto& node = m_Nodes[m_CurrentNode];

			BehaviourResult result = node->Go( e );
			switch( result )
			{
			case BehaviourResult::RUNNING:
				return BehaviourResult::RUNNING;
			case BehaviourResult::SUCCEEDED:
				m_CurrentNode = 0;
				return BehaviourResult::SUCCEEDED;
			case BehaviourResult::FAILED:
				m_CurrentNode++;
				if( m_CurrentNode == m_Nodes.size() )
				{
					m_CurrentNode = 0;
					return BehaviourResult::FAILED;
				}
				return BehaviourResult::RUNNING;
			}

			ASSERT( false, "Unhandled behaviour result" );
			return BehaviourResult::RUNNING;
		}
	private:
		size_t m_CurrentNode = 0;
		std::vector<std::unique_ptr<BehaviourNode>> m_Nodes;
	};

	class LoopUntilSuccessNode : public BehaviourNode
	{
	public:
		void SetChild( std::unique_ptr<BehaviourNode> node )
		{
			m_Node = std::move( node );
		}

		virtual BehaviourResult Go( Entity e ) override
		{
			BehaviourResult result = m_Node->Go( e );
			switch( result )
			{
			case BehaviourResult::RUNNING:
				return BehaviourResult::RUNNING;
			case BehaviourResult::SUCCEEDED:
				return BehaviourResult::SUCCEEDED;
			case BehaviourResult::FAILED:
				return BehaviourResult::RUNNING;
			}

			ASSERT( false, "Unhandled behaviour result" );
			return BehaviourResult::RUNNING;
		}
	private:
		std::unique_ptr<BehaviourNode> m_Node;
	};

	class InverseNode : public BehaviourNode
	{
	public:
		void SetChild( std::unique_ptr<BehaviourNode> node )
		{
			m_Node = std::move( node );
		}

		virtual BehaviourResult Go( Entity e ) override
		{
			BehaviourResult result = m_Node->Go( e );
			switch( result )
			{
			case BehaviourResult::RUNNING:
				return BehaviourResult::RUNNING;
			case BehaviourResult::SUCCEEDED:
				return BehaviourResult::FAILED;
			case BehaviourResult::FAILED:
				return BehaviourResult::SUCCEEDED;
			}

			ASSERT( false, "Unhandled behaviour result" );
			return BehaviourResult::RUNNING;
		}
	private:
		std::unique_ptr<BehaviourNode> m_Node;
	};

	using ActionCallback_t = std::function<BehaviourResult( Entity e )>;
	class ActionNode : public BehaviourNode
	{
	public:
		ActionNode( ActionCallback_t callback ) : m_Callback( callback ) {}

		virtual BehaviourResult Go( Entity e ) override { return m_Callback( e ); }
	private:
		ActionCallback_t m_Callback;
	};

	struct BehaviourTree
	{
		BAT_COMPONENT( BEHAVIOUR_TREE );
		std::unique_ptr<BehaviourNode> root_node;
	};

	class BehaviourTreeSystem
	{
	public:
		void Update( EntityManager& world );
	};
}