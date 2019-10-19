#include "PCH.h"
#include "PhysicsComponent.h"

namespace Bat
{
	PhysicsComponent::PhysicsComponent( PhysicsObjectType type, const PhysicsMaterial& material )
		:
		m_Type( type ),
		m_Material( material )
	{}

	PhysicsComponent& PhysicsComponent::AddSphereShape( float radius )
	{
		m_pObject->AddSphereShape( radius, m_Material );
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddCapsuleShape( float radius, float half_height )
	{
		m_pObject->AddCapsuleShape( radius, half_height, m_Material );
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddBoxShape( float length_x, float length_y, float length_z )
	{
		m_pObject->AddBoxShape( length_x, length_y, length_z, m_Material );
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddConvexShape( const Vec3* convex_verts, size_t convex_verts_count )
	{
		m_pObject->AddConvexShape( convex_verts, convex_verts_count, m_Material );
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddMeshShape()
	{
		m_AddMeshShape = AddMeshType::SHAPE;
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddSphereTrigger( float radius )
	{
		m_pObject->AddSphereTrigger( radius );
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddCapsuleTrigger( float radius, float half_height )
	{
		m_pObject->AddCapsuleTrigger( radius, half_height );
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddBoxTrigger( float length_x, float length_y, float length_z )
	{
		m_pObject->AddBoxTrigger( length_x, length_y, length_z );
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddConvexTrigger( const Vec3* convex_verts, size_t convex_verts_count )
	{
		m_pObject->AddConvexTrigger( convex_verts, convex_verts_count );
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddMeshTrigger()
	{
		m_AddMeshShape = AddMeshType::TRIGGER;
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddPlaneShape()
	{
		GetStaticObject()->AddPlaneShape( m_Material );
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddPlaneTrigger()
	{
		GetStaticObject()->AddPlaneTrigger();
		return *this;
	}
	PhysicsComponent& PhysicsComponent::SetKinematic( bool kinematic )
	{
		GetDynamicObject()->SetKinematic( kinematic );
		return *this;
	}
	bool PhysicsComponent::IsKinematic() const
	{
		return GetDynamicObject()->IsKinematic();
	}
	PhysicsComponent& PhysicsComponent::SetGravityDisabled( bool grav_disabled )
	{
		GetDynamicObject()->SetGravityDisabled( grav_disabled );
		return *this;
	}
	bool PhysicsComponent::IsGravityDisabled() const
	{
		return GetDynamicObject()->IsGravityDisabled();
	}
	PhysicsComponent& PhysicsComponent::SetCentreOfMass( const Vec3& pos )
	{
		GetDynamicObject()->SetCentreOfMass( pos );
		return *this;
	}
	Vec3 PhysicsComponent::GetCentreOfMass() const
	{
		return GetDynamicObject()->GetCentreOfMass();
	}
	PhysicsComponent& PhysicsComponent::SetMass( float mass )
	{
		GetDynamicObject()->SetMass( mass );
		return *this;
	}
	float PhysicsComponent::GetMass() const
	{
		return GetDynamicObject()->GetMass();
	}
	float PhysicsComponent::GetInvMass() const
	{
		return GetDynamicObject()->GetInvMass();
	}
	PhysicsComponent& PhysicsComponent::SetLinearDamping( float lin_damping )
	{
		GetDynamicObject()->SetLinearDamping( lin_damping );
		return *this;
	}
	float PhysicsComponent::GetLinearDamping() const
	{
		return GetDynamicObject()->GetLinearDamping();
	}
	PhysicsComponent& PhysicsComponent::SetAngularDamping( float ang_damping )
	{
		GetDynamicObject()->SetAngularDamping( ang_damping );
		return *this;
	}
	float PhysicsComponent::GetAngularDamping() const
	{
		return GetDynamicObject()->GetAngularDamping();
	}
	Vec3 PhysicsComponent::GetLinearVelocity() const
	{
		return GetDynamicObject()->GetLinearVelocity();
	}
	PhysicsComponent& PhysicsComponent::SetLinearVelocity( const Vec3& lin_vel )
	{
		GetDynamicObject()->SetLinearVelocity( lin_vel );
		return *this;
	}
	Vec3 PhysicsComponent::GetAngularVelocity() const
	{
		return GetDynamicObject()->GetAngularVelocity();
	}
	PhysicsComponent& PhysicsComponent::SetAngularVelocity( const Vec3& ang_vel )
	{
		GetDynamicObject()->SetAngularVelocity( ang_vel );
		return *this;
	}
	float PhysicsComponent::GetMaxLinearVelocity() const
	{
		return GetDynamicObject()->GetMaxLinearVelocity();
	}
	PhysicsComponent& PhysicsComponent::SetMaxLinearVelocity( float max_lin_vel )
	{
		GetDynamicObject()->SetMaxLinearVelocity( max_lin_vel );
		return *this;
	}
	float PhysicsComponent::GetMaxAngularVelocity() const
	{
		return GetDynamicObject()->GetMaxAngularVelocity();
	}
	PhysicsComponent& PhysicsComponent::SetMaxAngularVelocity( float max_ang_vel )
	{
		GetDynamicObject()->SetMaxAngularVelocity( max_ang_vel );
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddForce( const Vec3& force )
	{
		GetDynamicObject()->AddForce( force );
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddLinearImpulse( const Vec3& lin_impulse )
	{
		GetDynamicObject()->AddLinearImpulse( lin_impulse );
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddTorque( const Vec3& torque )
	{
		GetDynamicObject()->AddTorque( torque );
		return *this;
	}
	PhysicsComponent& PhysicsComponent::AddAngularImpulse( const Vec3& ang_impulse )
	{
		GetDynamicObject()->AddAngularImpulse( ang_impulse );
		return *this;
	}
	void PhysicsComponent::RemoveShape( size_t index )
	{
		m_pObject->RemoveShape( index );
	}
	size_t PhysicsComponent::GetNumShapes() const
	{
		return m_pObject->GetNumShapes();
	}
	void PhysicsComponent::GetBounds( Vec3* mins, Vec3* maxs ) const
	{
		m_pObject->GetBounds( mins, maxs );
	}
	IStaticObject* PhysicsComponent::GetStaticObject()
	{
		ASSERT( GetType() == PhysicsObjectType::STATIC, "Using dynamic physics object like static one" );
		return static_cast<IStaticObject*>(m_pObject.get());
	}
	const IStaticObject* PhysicsComponent::GetStaticObject() const
	{
		ASSERT( GetType() == PhysicsObjectType::STATIC, "Using dynamic physics object like static one" );
		return static_cast<IStaticObject*>(m_pObject.get());
	}
	IDynamicObject* PhysicsComponent::GetDynamicObject()
	{
		ASSERT( GetType() == PhysicsObjectType::DYNAMIC, "Using static physics object like dynamic one" );
		return static_cast<IDynamicObject*>(m_pObject.get());
	}
	const IDynamicObject* PhysicsComponent::GetDynamicObject() const
	{
		ASSERT( GetType() == PhysicsObjectType::DYNAMIC, "Using static physics object like dynamic one" );
		return static_cast<IDynamicObject*>(m_pObject.get());
	}
}
