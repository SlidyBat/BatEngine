#include "PCH.h"
#include "CoreEntityComponents.h"

void Bat::HierarchySystem::Update( SceneNode& root_node )
{
	m_NodeStack.clear();
	m_Transforms.clear();

	m_NodeStack.push_back( &root_node );

	Mat4 transform = Mat4::Identity();
	if( root_node.Get().Has<TransformComponent>() )
	{
		transform = root_node.Get().Get<TransformComponent>().GetTransform();
	}
	m_Transforms.push_back( transform );

	while( !m_NodeStack.empty() )
	{
		SceneNode* node = m_NodeStack.back();
		m_NodeStack.pop_back();
		transform = m_Transforms.back();
		m_Transforms.pop_back();

		Entity e = node->Get();
		auto& hier = e.Ensure<HierarchyComponent>();
		hier.abs_transform = transform;

		size_t num_children = node->GetNumChildren();
		for( size_t i = 0; i < num_children; i++ )
		{
			m_NodeStack.push_back( &node->GetChild( i ) );

			Entity child = node->GetChild( i ).Get();

			if( child.Has<TransformComponent>() )
			{
				const auto& child_transform = child.Get<TransformComponent>();
				m_Transforms.push_back( child_transform.GetTransform() * transform );
			}
			else
			{
				m_Transforms.push_back( transform );
			}
		}
	}
}
