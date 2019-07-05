#include "PCH.h"

#include "Physics.h"
#include "Common.h"
#include "Event.h"
#include "PhysicsEvents.h"

#include <PhysX/PxPhysicsAPI.h>
#include <PhysX/extensions/PxDefaultAllocator.h>
#include <PhysX/common/windows/PxWindowsDelayLoadHook.h>
#include <PhysX/cooking/PxCooking.h>

#include "JobSystem.h"

using namespace physx;

namespace Bat
{
	static PxFoundation* g_pPxFoundation;
	static PxPhysics*    g_pPxPhysics;
	static PxCooking*    g_pPxCooking;
	static PxScene*      g_pPxScene = nullptr;
	static PxMaterial*   g_pPxDefaultMaterial = nullptr;

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

	class BatPxSimulationEventCallback : public PxSimulationEventCallback
	{
	public:
		virtual void onConstraintBreak( PxConstraintInfo* constraints, PxU32 count ) {}
		virtual void onWake( PxActor** actors, PxU32 count )
		{
			for( PxU32 i = 0; i < count; i++ )
			{
				EventDispatcher::DispatchGlobalEvent<PhysicsObjectWakeEvent>( reinterpret_cast<IPhysicsObject*>(actors[i]->userData) );
			}
		}
		virtual void onSleep( PxActor** actors, PxU32 count )
		{
			for( PxU32 i = 0; i < count; i++ )
			{
				EventDispatcher::DispatchGlobalEvent<PhysicsObjectSleepEvent>( reinterpret_cast<IPhysicsObject*>(actors[i]->userData) );
			}
		}
		virtual void onContact( const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs )
		{
			auto a = reinterpret_cast<IPhysicsObject*>(pairHeader.actors[0]->userData);
			auto b = reinterpret_cast<IPhysicsObject*>(pairHeader.actors[1]->userData);
			if( pairs->events & PxPairFlag::eNOTIFY_TOUCH_FOUND )
			{
				EventDispatcher::DispatchGlobalEvent<PhysicsObjectStartTouchEvent>( a, b );
				EventDispatcher::DispatchGlobalEvent<PhysicsObjectTouchEvent>( a, b );
			}
			else if( pairs->events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS )
			{
				EventDispatcher::DispatchGlobalEvent<PhysicsObjectTouchEvent>( a, b );
			}
			else if( pairs->events & PxPairFlag::eNOTIFY_TOUCH_LOST )
			{
				EventDispatcher::DispatchGlobalEvent<PhysicsObjectEndTouchEvent>( a, b );
			}
		}
		virtual void onTrigger( PxTriggerPair* pairs, PxU32 count )
		{
			for( PxU32 i = 0; i < count; i++ )
			{
				auto trigger = reinterpret_cast<IPhysicsObject*>(pairs[i].triggerActor->userData);
				auto other = reinterpret_cast<IPhysicsObject*>(pairs[i].otherActor->userData);
				if( pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND )
				{
					EventDispatcher::DispatchGlobalEvent<PhysicsTriggerStartTouchEvent>( trigger, other );
				}
				else if( pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST )
				{
					EventDispatcher::DispatchGlobalEvent<PhysicsTriggerEndTouchEvent>( trigger, other );
				}
			}
		}
		virtual void onAdvance( const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count ) {};
	};
	static BatPxSimulationEventCallback g_PxSimulationCallback;

	static PxVec3 Bat2PxVec( const Vec3& vec )
	{
		return { vec.x, vec.y, vec.z };
	}

	static Vec3 Px2BatVec( const PxVec3& vec )
	{
		return { vec.x, vec.y, vec.z };
	}
	
	static PxQuat Bat2PxAng( const Vec3& ang )
	{
		Vec4 q = Math::EulerToQuaternion( ang );
		return { q.x, q.y, q.z, q.w };
	}
	
	static Vec3 Px2BatAng( const PxQuat& q )
	{
		Vec3 ang = Math::QuaternionToEuler( { q.x, q.y, q.z, q.w } );
		return ang;
	}

	static PxMaterial* GetPxMaterial( const PhysicsMaterial& material )
	{
		if( &material == &Physics::DEFAULT_MATERIAL )
		{
			g_pPxDefaultMaterial->acquireReference();
			return g_pPxDefaultMaterial;
		}

		return g_pPxPhysics->createMaterial( material.static_friction, material.dynamic_friction, material.restitution );
	}

	class PxStaticObject : public IStaticObject
	{
	public:
		PxStaticObject( PxRigidStatic* pStaticActor, void* pUserData )
			:
			m_pStaticActor( pStaticActor ),
			m_pUserData( pUserData )
		{
			m_pStaticActor->userData = this;
		}

		~PxStaticObject()
		{
			m_pStaticActor->release();
		}

		virtual void AddSphereShape( float radius, const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = GetPxMaterial( material );
			PxRigidActorExt::createExclusiveShape( *m_pStaticActor, PxSphereGeometry( radius ), *px_material );
			px_material->release();
		}
		virtual void AddCapsuleShape( float radius, float half_height, const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = GetPxMaterial( material );
			PxRigidActorExt::createExclusiveShape( *m_pStaticActor, PxCapsuleGeometry( radius, half_height ), *px_material );
			px_material->release();
		}
		virtual void AddBoxShape( float length_x, float length_y, float length_z, const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = GetPxMaterial( material );
			PxRigidActorExt::createExclusiveShape( *m_pStaticActor, PxBoxGeometry( length_x / 2, length_y / 2, length_z / 2 ), *px_material );
			px_material->release();
		}
		virtual void AddSphereTrigger( float radius ) override
		{
			PxMaterial* px_material = GetPxMaterial( Physics::DEFAULT_MATERIAL );
			PxRigidActorExt::createExclusiveShape( *m_pStaticActor, PxSphereGeometry( radius ), *px_material, PxShapeFlag::eTRIGGER_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE );
			px_material->release();
		}
		virtual void AddCapsuleTrigger( float radius, float half_height ) override
		{
			PxMaterial* px_material = GetPxMaterial( Physics::DEFAULT_MATERIAL );
			PxRigidActorExt::createExclusiveShape( *m_pStaticActor, PxCapsuleGeometry( radius, half_height ), *px_material, PxShapeFlag::eTRIGGER_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE );
			px_material->release();
		}
		virtual void AddBoxTrigger( float length_x, float length_y, float length_z ) override
		{
			PxMaterial* px_material = GetPxMaterial( Physics::DEFAULT_MATERIAL );
			PxRigidActorExt::createExclusiveShape( *m_pStaticActor, PxBoxGeometry( length_x / 2, length_y / 2, length_z / 2 ), *px_material, PxShapeFlag::eTRIGGER_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE );
			px_material->release();
		}
		virtual void AddPlaneShape( const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = GetPxMaterial( Physics::DEFAULT_MATERIAL );
			PxRigidActorExt::createExclusiveShape( *m_pStaticActor, PxPlaneGeometry(), *px_material );
			px_material->release();
		}
		virtual void AddPlaneTrigger() override
		{
			PxMaterial* px_material = GetPxMaterial( Physics::DEFAULT_MATERIAL );
			PxRigidActorExt::createExclusiveShape( *m_pStaticActor, PxPlaneGeometry(), *px_material, PxShapeFlag::eTRIGGER_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE );
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

		virtual Vec3 GetPosition() const
		{
			return Px2BatVec( m_pStaticActor->getGlobalPose().p );
		}

		virtual void SetPosition( const Vec3& pos )
		{
			PxTransform transform = m_pStaticActor->getGlobalPose();
			transform.p = Bat2PxVec( pos );
			m_pStaticActor->setGlobalPose( transform );
		}

		virtual Vec3 GetRotation() const
		{
			return Px2BatAng( m_pStaticActor->getGlobalPose().q );
		}

		virtual void SetRotation( const Vec3& ang )
		{
			PxTransform transform = m_pStaticActor->getGlobalPose();
			transform.q = Bat2PxAng( ang );
			m_pStaticActor->setGlobalPose( transform );
		}
	private:
		PxRigidStatic* m_pStaticActor;
		void* m_pUserData;
	};

	class PxDynamicObject : public IDynamicObject
	{
	public:
		PxDynamicObject( PxRigidDynamic* pDynamicActor, void* pUserData )
			:
			m_pDynamicActor( pDynamicActor ),
			m_pUserData( pUserData )
		{
			m_pDynamicActor->userData = this;
		}

		~PxDynamicObject()
		{
			m_pDynamicActor->release();
		}

		virtual void AddSphereShape( float radius, const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = GetPxMaterial( material );
			PxRigidActorExt::createExclusiveShape( *m_pDynamicActor, PxSphereGeometry( radius ), *px_material );
			px_material->release();
		}
		virtual void AddCapsuleShape( float radius, float half_height, const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = GetPxMaterial( material );
			PxRigidActorExt::createExclusiveShape( *m_pDynamicActor, PxCapsuleGeometry( radius, half_height ), *px_material );
			px_material->release();
		}
		virtual void AddBoxShape( float length_x, float length_y, float length_z, const PhysicsMaterial& material ) override
		{
			PxMaterial* px_material = GetPxMaterial( material );
			PxRigidActorExt::createExclusiveShape( *m_pDynamicActor, PxBoxGeometry( length_x / 2, length_y / 2, length_z / 2 ), *px_material );
			px_material->release();
		}
		virtual void AddSphereTrigger( float radius ) override
		{
			PxMaterial* px_material = GetPxMaterial( Physics::DEFAULT_MATERIAL );
			PxRigidActorExt::createExclusiveShape( *m_pDynamicActor, PxSphereGeometry( radius ), *px_material, PxShapeFlag::eTRIGGER_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE );
			px_material->release();
		}
		virtual void AddCapsuleTrigger( float radius, float half_height ) override
		{
			PxMaterial* px_material = GetPxMaterial( Physics::DEFAULT_MATERIAL );
			PxRigidActorExt::createExclusiveShape( *m_pDynamicActor, PxCapsuleGeometry( radius, half_height ), *px_material, PxShapeFlag::eTRIGGER_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE );
			px_material->release();
		}
		virtual void AddBoxTrigger( float length_x, float length_y, float length_z ) override
		{
			PxMaterial* px_material = GetPxMaterial( Physics::DEFAULT_MATERIAL );
			PxRigidActorExt::createExclusiveShape( *m_pDynamicActor, PxBoxGeometry( length_x / 2, length_y / 2, length_z / 2 ), *px_material, PxShapeFlag::eTRIGGER_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE );
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

		virtual Vec3 GetPosition() const
		{
			return Px2BatVec( m_pDynamicActor->getGlobalPose().p );
		}

		virtual void SetPosition( const Vec3& pos )
		{
			PxTransform transform = m_pDynamicActor->getGlobalPose();
			transform.p = Bat2PxVec( pos );
			m_pDynamicActor->setGlobalPose( transform );
		}

		virtual Vec3 GetRotation() const
		{
			return Px2BatAng( m_pDynamicActor->getGlobalPose().q );
		}

		virtual void SetRotation( const Vec3& ang )
		{
			PxTransform transform = m_pDynamicActor->getGlobalPose();
			transform.q = Bat2PxAng( ang );
			m_pDynamicActor->setGlobalPose( transform );
		}

		virtual void SetKinematic( bool kinematic )  override { m_pDynamicActor->setRigidBodyFlag( PxRigidBodyFlag::eKINEMATIC, kinematic ); }
		virtual bool IsKinematic() const  override { return m_pDynamicActor->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC; }
		virtual void MoveTo( const Vec3& pos, const Vec3& ang ) override
		{
			m_pDynamicActor->setKinematicTarget( PxTransform( Bat2PxVec( pos ), Bat2PxAng( ang ) ) );
		}

		virtual void SetGravityDisabled( bool grav_disabled )  override { m_pDynamicActor->setActorFlag( PxActorFlag::eDISABLE_GRAVITY, grav_disabled ); }
		virtual bool IsGravityDisabled() const  override { return m_pDynamicActor->getActorFlags() & PxActorFlag::eDISABLE_GRAVITY; }

		virtual void SetCentreOfMass( const Vec3& pos ) override
		{
			m_pDynamicActor->setCMassLocalPose( PxTransform( Bat2PxVec( pos ) ) );
		}
		virtual Vec3 GetCentreOfMass() const override
		{
			PxTransform transform = m_pDynamicActor->getCMassLocalPose();
			return Px2BatVec( transform.p );
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
		void* m_pUserData;
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

		g_pPxScene->setSimulationEventCallback( &g_PxSimulationCallback );

		auto& m = DEFAULT_MATERIAL;
		g_pPxDefaultMaterial = g_pPxPhysics->createMaterial( m.static_friction, m.dynamic_friction, m.restitution );
	}

	void Physics::Shutdown()
	{
		g_pPxDefaultMaterial->release();
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

	IStaticObject* Physics::CreateStaticObject( const Vec3& pos, const Vec3& ang, void* userdata )
	{
		PxRigidStatic* static_actor = g_pPxPhysics->createRigidStatic( PxTransform( Bat2PxVec( pos ), Bat2PxAng( ang ) ) );
		g_pPxScene->addActor( *static_actor );

		return new PxStaticObject( static_actor, userdata );
	}

	IDynamicObject* Physics::CreateDynamicObject( const Vec3& pos, const Vec3& ang, void* userdata )
	{
		PxRigidDynamic* dynamic_actor = g_pPxPhysics->createRigidDynamic( PxTransform( Bat2PxVec( pos ), Bat2PxAng( ang ) ) );
		g_pPxScene->addActor( *dynamic_actor );

		return new PxDynamicObject( dynamic_actor, userdata );
	}

	RayCastResult Physics::RayCast( const Vec3& origin, const Vec3& unit_direction, float max_distance, int filter )
	{
		PxQueryFilterData filter_data((PxQueryFlag::Enum)0);
		if( filter & HIT_STATICS )
		{
			filter_data.flags |= PxQueryFlag::eSTATIC;
		}
		if( filter & HIT_DYNAMICS )
		{
			filter_data.flags |= PxQueryFlag::eDYNAMIC;
		}

		PxRaycastBuffer hit;
		bool success = g_pPxScene->raycast( Bat2PxVec( origin ), Bat2PxVec( unit_direction ), max_distance, hit, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL, filter_data );

		RayCastResult result;
		if( !success )
		{
			result.hit = false;
			result.position = { 0.0f, 0.0f, 0.0f };
			result.normal = { 0.0f, 0.0f, 0.0f };
			result.object = nullptr;
		}
		else
		{
			result.hit = true;
			result.position = Px2BatVec( hit.block.position );
			result.normal = Px2BatVec( hit.block.normal );
			result.object = reinterpret_cast<IPhysicsObject*>(hit.block.actor->userData);
		}

		return result;
	}

	static SweepResult SweepGeneric( const PxGeometry& geometry, const Vec3& origin, const Vec3& rotation, const Vec3& unit_direction, float max_distance, int filter )
	{
		PxQueryFilterData filter_data( (PxQueryFlag::Enum)0 );
		if( filter & HIT_STATICS )
		{
			filter_data.flags |= PxQueryFlag::eSTATIC;
		}
		if( filter & HIT_DYNAMICS )
		{
			filter_data.flags |= PxQueryFlag::eDYNAMIC;
		}

		PxSweepBuffer hit;
		PxTransform transform( Bat2PxVec( origin ), Bat2PxAng( rotation ) );
		bool success = g_pPxScene->sweep( geometry, transform, Bat2PxVec( unit_direction ), max_distance, hit, PxHitFlag::ePOSITION | PxHitFlag::eNORMAL, filter_data );

		SweepResult result;
		if( !success )
		{
			result.hit = false;
			result.position = { 0.0f, 0.0f, 0.0f };
			result.distance = 0.0f;
			result.object = nullptr;
		}
		else
		{
			result.hit = true;
			result.position = Px2BatVec( hit.block.position );
			result.distance = hit.block.distance;
			result.object = reinterpret_cast<IPhysicsObject*>(hit.block.actor);
		}

		return result;
	}

	SweepResult Physics::SweepSphere( float radius, const Vec3& origin, const Vec3& unit_direction, float max_distance, int filter )
	{
		PxSphereGeometry sphere( radius );
		return SweepGeneric( sphere, origin, unit_direction, { 0.0f, 0.0f, 0.0f }, max_distance, filter );
	}

	SweepResult Physics::SweepCapsule( float radius, float half_height, const Vec3& rotation, const Vec3& origin, const Vec3& unit_direction, float max_distance, int filter )
	{
		PxCapsuleGeometry capsule( radius, half_height );
		return SweepGeneric( capsule, origin, rotation, unit_direction, max_distance, filter );
	}

	SweepResult Physics::SweepBox( float length_x, float length_y, float length_z, const Vec3& rotation, const Vec3& origin, const Vec3& unit_direction, float max_distance, int filter )
	{
		PxBoxGeometry box( length_x / 2, length_y / 2, length_z / 2 );
		return SweepGeneric( box, origin, rotation, unit_direction, max_distance, filter );
	}
}
