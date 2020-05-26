#pragma once

#include "Physics.h"
#include "Core/Entity.h"
#include "Core/CoreEntityComponents.h"

namespace Bat
{
	class PhysicsComponent
	{
	public:
		BAT_COMPONENT( PHYSICS );

		PhysicsComponent( PhysicsObjectType type, const PhysicsMaterial& material = Physics::DEFAULT_MATERIAL );

		PhysicsComponent& AddSphereShape( float radius );
		PhysicsComponent& AddCapsuleShape( float radius, float half_height );
		PhysicsComponent& AddBoxShape( float length_x, float length_y, float length_z );
		PhysicsComponent& AddConvexShape( const Vec3* convex_verts, size_t convex_verts_count );
		PhysicsComponent& AddMeshShape();
		PhysicsComponent& AddSphereTrigger( float radius );
		PhysicsComponent& AddCapsuleTrigger( float radius, float half_height );
		PhysicsComponent& AddBoxTrigger( float length_x, float length_y, float length_z );
		PhysicsComponent& AddConvexTrigger( const Vec3* convex_verts, size_t convex_verts_count );
		PhysicsComponent& AddMeshTrigger();

		void RemoveShape( size_t index );
		size_t GetNumShapes() const;

		void GetBounds( Vec3* mins, Vec3* maxs ) const;

		PhysicsObjectType GetType() const { return m_Type; }

		// === Static objects only begin ===
		PhysicsComponent& AddPlaneShape();
		PhysicsComponent& AddPlaneTrigger();
		// === Static objects only end ===

		// === Dynamic objects only begin ===
		PhysicsComponent& SetKinematic( bool kinematic );
		bool IsKinematic() const;

		PhysicsComponent& SetGravityDisabled( bool grav_disabled );
		bool IsGravityDisabled() const;

		PhysicsComponent& SetCentreOfMass( const Vec3& pos );
		Vec3 GetCentreOfMass() const;

		PhysicsComponent& SetMass( float mass );
		float GetMass() const;
		float GetInvMass() const;

		PhysicsComponent& SetLinearDamping( float lin_damping );
		float GetLinearDamping() const;
		PhysicsComponent& SetAngularDamping( float ang_damping );
		float GetAngularDamping() const;

		Vec3 GetLinearVelocity() const;
		PhysicsComponent& SetLinearVelocity( const Vec3& lin_vel );
		Vec3 GetAngularVelocity() const;
		PhysicsComponent& SetAngularVelocity( const Vec3& ang_vel );

		float GetMaxLinearVelocity() const;
		PhysicsComponent& SetMaxLinearVelocity( float max_lin_vel );
		float GetMaxAngularVelocity() const;
		PhysicsComponent& SetMaxAngularVelocity( float max_ang_vel );

		PhysicsComponent& AddForce( const Vec3& force );
		PhysicsComponent& AddLinearImpulse( const Vec3& lin_impulse );
		PhysicsComponent& AddTorque( const Vec3& torque );
		PhysicsComponent& AddAngularImpulse( const Vec3& ang_impulse );
		// === Dynamic objects only end ===
	private:
		IStaticObject* GetStaticObject();
		const IStaticObject* GetStaticObject() const;
		IDynamicObject* GetDynamicObject();
		const IDynamicObject* GetDynamicObject() const;
	private:
		friend class PhysicsSystem;

		enum class AddMeshType
		{
			NONE,
			SHAPE,
			TRIGGER
		};
		AddMeshType m_AddMeshShape = AddMeshType::NONE;
		PhysicsObjectType m_Type;
		PhysicsMaterial m_Material;
		std::unique_ptr<IPhysicsObject> m_pObject;
	};
}