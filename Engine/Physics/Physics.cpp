#include "PCH.h"

#include "Physics.h"
#include "Common.h"

#include <PhysX/PxPhysicsAPI.h>
#include <PhysX/extensions/PxDefaultAllocator.h>
#include <PhysX/common/windows/PxWindowsDelayLoadHook.h>

#include "JobSystem.h"

using namespace physx;

namespace Bat
{
	static PxFoundation* g_pPxFoundation;
	static PxPhysics*    g_pPxPhysics;
	static PxScene*      g_pPxScene = nullptr;

	static bool g_bFixedTimestep = false;
	static float g_flFixedTimestep = 0.0f;

	class BatPxErrorCallback : public PxErrorCallback
	{
	public:
		virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file,
			int line) override
		{
			// Fatal errors, debug break to make sure we see this!!!
			if( code & (PxErrorCode::eABORT | PxErrorCode::eINVALID_OPERATION |
				PxErrorCode::eINVALID_PARAMETER | PxErrorCode::eOUT_OF_MEMORY) )
			{
				BAT_ABORT( "[Fatal PhysX Error %s:%i] %s", file, line, message );
			}
			// Error, but not fatal
			else if( code & PxErrorCode::eINTERNAL_ERROR )
			{
				BAT_ERROR( "[PhysX %s:%i] %s", file, line, message );
			}
			// Performance warning
			else if( code & PxErrorCode::ePERF_WARNING )
			{
				BAT_WARN( "[PhysX %s:%i] %s", file, line, message );
			}
			// Debug messages
			else if( code & PxErrorCode::eDEBUG_WARNING )
			{
				BAT_WARN( "[PhysX %s:%i] %s", file, line, message );
			}
			else if( code & PxErrorCode::eDEBUG_INFO )
			{
				BAT_LOG( "[PhysX %s:%i] %s", file, line, message );
			}
		}
	};

	static PxVec3 Bat2PxVec( const Vec3& vec )
	{
		return { vec.x, vec.y, vec.x };
	}

	static Vec3 Px2BatVec( const PxVec3& vec )
	{
		return { vec.x, vec.y, vec.x };
	}

	static DirectX::XMMATRIX Transform2Mat( const PxTransform& transform )
	{
		PxMat44 transform_mat( transform );
		return *reinterpret_cast<DirectX::XMMATRIX*>(&transform_mat.column0.x);
	}

	static PxTransform Mat2Transform( const DirectX::XMMATRIX& mat )
	{
		PxMat44 transform_mat = *reinterpret_cast<const PxMat44*>(&mat.r[0].m128_f32[0]);
		PxTransform transform( transform_mat );
		return transform;
	}

	class PxStaticObject : public IStaticObject
	{
	public:
		PxStaticObject( PxRigidStatic* pStaticActor )
			:
			m_pStaticActor( pStaticActor )
		{
			m_pStaticActor->userData = this;
		}

		~PxStaticObject()
		{
			m_pStaticActor->release();
		}

		virtual void AddSphereShape( float radius, const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = g_pPxPhysics->createMaterial( material.static_friction, material.dynamic_friction, material.restitution );
			PxRigidActorExt::createExclusiveShape( *m_pStaticActor, PxSphereGeometry( radius ), *px_material );
			px_material->release();
		}
		virtual void AddCapsuleShape( float radius, float half_height, const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = g_pPxPhysics->createMaterial( material.static_friction, material.dynamic_friction, material.restitution );
			PxRigidActorExt::createExclusiveShape( *m_pStaticActor, PxCapsuleGeometry( radius, half_height ), *px_material );
			px_material->release();
		}
		virtual void AddBoxShape( float length_x, float length_y, float length_z, const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = g_pPxPhysics->createMaterial( material.static_friction, material.dynamic_friction, material.restitution );
			PxRigidActorExt::createExclusiveShape( *m_pStaticActor, PxBoxGeometry( length_x / 2, length_y / 2, length_z / 2 ), *px_material );
			px_material->release();
		}
		virtual void AddPlaneShape( const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = g_pPxPhysics->createMaterial( material.static_friction, material.dynamic_friction, material.restitution );
			PxRigidActorExt::createExclusiveShape( *m_pStaticActor, PxPlaneGeometry(), *px_material );
			px_material->release();
		}

		virtual size_t GetNumShapes() const  override { return (size_t)m_pStaticActor->getNbShapes(); }
		virtual void RemoveShape( size_t index ) override
		{
			PxShape* shape;
			m_pStaticActor->getShapes( &shape, 1, (PxU32)index );
			m_pStaticActor->detachShape( *shape );
			shape->release();
		}

		virtual void GetBounds( Vec3* mins, Vec3* maxs ) const override
		{
			PxBounds3 bounds = m_pStaticActor->getWorldBounds();
			*mins = Px2BatVec( bounds.minimum );
			*maxs = Px2BatVec( bounds.maximum );
		}

		virtual DirectX::XMMATRIX GetTransform() const override
		{
			PxTransform transform = m_pStaticActor->getGlobalPose();
			return Transform2Mat( transform );
		}

		virtual void SetTransform( const DirectX::XMMATRIX& transform ) const override
		{
			PxTransform pose = Mat2Transform( transform );
			m_pStaticActor->setGlobalPose( pose );
		}
	private:
		PxRigidStatic* m_pStaticActor;
	};

	class PxDynamicObject : public IDynamicObject
	{
	public:
		PxDynamicObject( PxRigidDynamic* pDynamicActor )
			:
			m_pDynamicActor( pDynamicActor )
		{
			m_pDynamicActor->userData = this;
		}

		~PxDynamicObject()
		{
			m_pDynamicActor->release();
		}

		virtual void AddSphereShape( float radius, const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = g_pPxPhysics->createMaterial( material.static_friction, material.dynamic_friction, material.restitution );
			PxRigidActorExt::createExclusiveShape( *m_pDynamicActor, PxSphereGeometry( radius ), *px_material );
			px_material->release();
		}
		virtual void AddCapsuleShape( float radius, float half_height, const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = g_pPxPhysics->createMaterial( material.static_friction, material.dynamic_friction, material.restitution );
			PxRigidActorExt::createExclusiveShape( *m_pDynamicActor, PxCapsuleGeometry( radius, half_height ), *px_material );
			px_material->release();
		}
		virtual void AddBoxShape( float length_x, float length_y, float length_z, const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = g_pPxPhysics->createMaterial( material.static_friction, material.dynamic_friction, material.restitution );
			PxRigidActorExt::createExclusiveShape( *m_pDynamicActor, PxBoxGeometry( length_x / 2, length_y / 2, length_z / 2 ), *px_material );
			px_material->release();
		}

		virtual size_t GetNumShapes() const  override { return (size_t)m_pDynamicActor->getNbShapes(); }
		virtual void RemoveShape( size_t index ) override
		{
			PxShape* shape;
			m_pDynamicActor->getShapes( &shape, 1, (PxU32)index );
			m_pDynamicActor->detachShape( *shape );
			shape->release();
		}

		virtual void GetBounds( Vec3 * mins, Vec3 * maxs ) const override
		{
			PxBounds3 bounds = m_pDynamicActor->getWorldBounds();
			*mins = Px2BatVec( bounds.minimum );
			*maxs = Px2BatVec( bounds.maximum );
		}

		virtual DirectX::XMMATRIX GetTransform() const override
		{
			PxTransform transform = m_pDynamicActor->getGlobalPose();
			return Transform2Mat( transform );
		}

		virtual void SetTransform( const DirectX::XMMATRIX& transform ) const override
		{
			PxTransform pose = Mat2Transform( transform );
			m_pDynamicActor->setGlobalPose( pose );
		}

		virtual void SetKinematic( bool kinematic )  override { m_pDynamicActor->setRigidBodyFlag( PxRigidBodyFlag::eKINEMATIC, kinematic ); }
		virtual bool IsKinematic() const  override { return m_pDynamicActor->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC; }
		virtual void MoveTo( const DirectX::XMMATRIX& world_transform ) override
		{
			m_pDynamicActor->setKinematicTarget( Mat2Transform( world_transform ) );
		}

		virtual void SetGravityDisabled( bool grav_disabled )  override { m_pDynamicActor->setActorFlag( PxActorFlag::eDISABLE_GRAVITY, grav_disabled ); }
		virtual bool IsGravityDisabled() const  override { return m_pDynamicActor->getActorFlags() & PxActorFlag::eDISABLE_GRAVITY; }

		virtual void SetCentreOfMass( const DirectX::XMMATRIX& local_transform ) override
		{
			m_pDynamicActor->setCMassLocalPose( Mat2Transform( local_transform ) );
		}
		virtual DirectX::XMMATRIX GetCentreOfMass() const override
		{
			PxTransform transform = m_pDynamicActor->getCMassLocalPose();
			return Transform2Mat( transform );
		}

		virtual void SetMass( float mass )  override { m_pDynamicActor->setMass( mass ); }
		virtual float GetMass() const  override { return m_pDynamicActor->getMass(); }
		virtual float GetInvMass() const  override { return m_pDynamicActor->getInvMass(); }

		virtual void SetLinearDamping( float lin_damping )  override { m_pDynamicActor->setLinearDamping( lin_damping ); }
		virtual float GetLinearDamping() const  override { return m_pDynamicActor->getLinearDamping(); }
		virtual void SetAngularDamping( float ang_damping )  override { m_pDynamicActor->setAngularDamping( ang_damping ); }
		virtual float GetAngularDamping() const  override { return m_pDynamicActor->getAngularDamping(); }

		virtual Vec3 GetLinearVelocity() const  override { return Px2BatVec( m_pDynamicActor->getLinearVelocity() ); }
		virtual void SetLinearVelocity( const Vec3& lin_vel )  override { m_pDynamicActor->setLinearVelocity( Bat2PxVec( lin_vel ) ); }
		virtual Vec3 GetAngularVelocity() const  override { return Px2BatVec( m_pDynamicActor->getAngularVelocity() ); }
		virtual void SetAngularVelocity( const Vec3& ang_vel )  override { m_pDynamicActor->setLinearVelocity( Bat2PxVec( ang_vel ) ); }

		virtual float GetMaxLinearVelocity() const  override { return m_pDynamicActor->getMaxLinearVelocity(); }
		virtual void SetMaxLinearVelocity( float max_lin_vel )  override { m_pDynamicActor->setMaxLinearVelocity( max_lin_vel ); }
		virtual float GetMaxAngularVelocity() const  override { return m_pDynamicActor->getMaxAngularVelocity(); }
		virtual void SetMaxAngularVelocity( float max_ang_vel )  override { m_pDynamicActor->setMaxAngularVelocity( max_ang_vel ); }

		virtual void AddForce( const Vec3& force ) override
		{
			m_pDynamicActor->addForce( Bat2PxVec( force ) );
		}
		virtual void AddLinearImpulse( const Vec3& lin_impulse ) override
		{
			m_pDynamicActor->addForce( Bat2PxVec( lin_impulse ), PxForceMode::eIMPULSE );
		}
		virtual void AddTorque( const Vec3& torque ) override
		{
			m_pDynamicActor->addTorque( Bat2PxVec( torque ) );
		}
		virtual void AddAngularImpulse( const Vec3& ang_impulse ) override
		{
			m_pDynamicActor->addTorque( Bat2PxVec( ang_impulse ), PxForceMode::eIMPULSE );
		}
	private:
		PxRigidDynamic* m_pDynamicActor;
	};

	void Physics::Initialize()
	{
		static BatPxErrorCallback px_error_callback;
		static PxDefaultAllocator px_allocator_callback;

		PxTolerancesScale tolerances_scale;

		g_pPxFoundation = PxCreateFoundation( PX_PHYSICS_VERSION, px_allocator_callback, px_error_callback );
		if( !g_pPxFoundation )
		{
			BAT_ABORT( "Failed to initialize PhysX foundation!" );
		}

		const bool record_mem_allocs = false;
		g_pPxPhysics = PxCreatePhysics( PX_PHYSICS_VERSION, *g_pPxFoundation, tolerances_scale, record_mem_allocs );
		if( !g_pPxPhysics )
		{
			BAT_ABORT( "Failed to initialize PhysX physics object!" );
		}

		PxInitExtensions( *g_pPxPhysics, nullptr );

		PxSceneDesc scene_desc( tolerances_scale );
		scene_desc.gravity = { 0.0f, -9.8f, 0.0f };
		scene_desc.cpuDispatcher = PxDefaultCpuDispatcherCreate( 4 );
		scene_desc.filterShader = PxDefaultSimulationFilterShader;

		g_pPxScene = g_pPxPhysics->createScene( scene_desc );
	}

	void Physics::Shutdown()
	{
		g_pPxScene->release();
		PxCloseExtensions();
		g_pPxPhysics->release();
		g_pPxFoundation->release();
	}

	void Physics::SetGravity( const Vec3& gravity )
	{
		g_pPxScene->setGravity( Bat2PxVec( gravity ) );
	}

	void Physics::EnableFixedTimestep( float deltatime )
	{
		g_flFixedTimestep = deltatime;
		g_bFixedTimestep = true;
	}

	void Physics::DisableFixedTimestep()
	{
		g_bFixedTimestep = false;
	}

	void Physics::Simulate( float deltatime )
	{
		static float accumulator = 0.0f;

		if( g_bFixedTimestep )
		{
			accumulator += deltatime;
			while( accumulator >= g_flFixedTimestep )
			{
				g_pPxScene->simulate( g_flFixedTimestep );
				g_pPxScene->fetchResults( true ); // TODO: Investigate better ways to do this without blocking
				accumulator -= g_flFixedTimestep;
			}
		}
		else
		{
			g_pPxScene->simulate( deltatime );
			g_pPxScene->fetchResults( true );
		}
	}

	IStaticObject* Physics::CreateStaticObject( const DirectX::XMMATRIX& transform )
	{
		PxRigidStatic* static_actor = g_pPxPhysics->createRigidStatic( Mat2Transform( transform ) );
		g_pPxScene->addActor( *static_actor );

		return new PxStaticObject( static_actor );
	}

	IDynamicObject* Physics::CreateDynamicObject( const DirectX::XMMATRIX& transform )
	{
		PxRigidDynamic* dynamic_actor = g_pPxPhysics->createRigidDynamic( Mat2Transform( transform ) );
		g_pPxScene->addActor( *dynamic_actor );

		return new PxDynamicObject( dynamic_actor );
	}
}
