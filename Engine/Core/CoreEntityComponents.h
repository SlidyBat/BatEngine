#pragma once

#include "MathLib.h"
#include "Entity.h"
#include "SceneEvents.h"

namespace Bat
{
	class SceneNode;

	struct NameComponent : public Component<NameComponent>
	{
		NameComponent( std::string name )
			:
			name( std::move( name ) )
		{}

		std::string name;
	};

	enum class HierarchyCache
	{
		NONE = 0,
		POS = ( 1 << 0 ),
		ROT = ( 1 << 1 ),
		SCALE = ( 1 << 2 ),
		ALL = ( POS | ROT | SCALE )
	};
	BAT_ENUM_OPERATORS( HierarchyCache );

	class TransformComponent : public Component<TransformComponent>
	{
		friend class HierarchySystem;
	public:
		TransformComponent( SceneNode* node );

		TransformComponent& SetPosition( float x, float y, float z );
		TransformComponent& SetPosition( const Vec3& pos );
		const Vec3& GetPosition() const;

		TransformComponent& SetRotation( float pitch, float yaw, float roll );
		TransformComponent& SetRotation( const Vec3& rot );
		const Vec3& GetRotation() const;

		TransformComponent& SetScale( float uniform_scale );
		float GetScale() const;

		TransformComponent& SetLocalPosition( float x, float y, float z );
		TransformComponent& SetLocalPosition( const Vec3& pos );
		const Vec3& GetLocalPosition() const;

		TransformComponent& SetLocalRotation( float pitch, float yaw, float roll );
		TransformComponent& SetLocalRotation( const Vec3& rot );
		const Vec3& GetLocalRotation() const;

		TransformComponent& SetLocalScale( float uniform_scale );
		float GetLocalScale() const;

		void SetLocalMatrix( const Mat3x4& local_matrix );
		const Mat3x4& LocalToWorldMatrix() const;
		Mat3x4 WorldToLocalMatrix() const;
	private:
		bool IsDirty( HierarchyCache cache_type ) const;
		void MarkSelfAndChildrenDirty( HierarchyCache cache_type );
		void ClearDirty( HierarchyCache cache_type ) const;
		void CalculateCache() const;
	private:
		SceneNode* m_pNode = nullptr;
		Vec3 m_vecLocalPosition = { 0.0f, 0.0f, 0.0f };
		float m_flLocalScale = 1.0f;
		Vec3 m_vecLocalRotation = { 0.0f, 0.0f, 0.0f };
		mutable HierarchyCache m_Dirty = HierarchyCache::ALL;
		mutable Vec3 m_vecPosition = { 0.0f, 0.0f, 0.0f };
		mutable Vec3 m_vecRotation = { 0.0f, 0.0f, 0.0f };
		mutable float m_flScale = 1.0f;
		mutable Mat3x4 m_matLocalToWorld = Mat3x4::Identity();
	};

	class HierarchySystem
	{
	public:
		HierarchySystem();
		~HierarchySystem();
		HierarchySystem( const HierarchySystem& ) = delete;
		HierarchySystem& operator=( const HierarchySystem& ) = delete;
		HierarchySystem( HierarchySystem&& ) = delete;
		HierarchySystem& operator=( HierarchySystem&& ) = delete;

		void OnEvent( const SceneNodeAddedEvent& e );
	};
}