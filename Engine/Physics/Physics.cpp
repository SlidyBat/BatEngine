#include "PCH.h"

#include "Physics.h"
#include "Common.h"

#include <PhysX/PxPhysicsAPI.h>
#include <PhysX/extensions/PxDefaultAllocator.h>
#include <PhysX/common/windows/PxWindowsDelayLoadHook.h>

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
				BAT_ERROR( "[Fatal PhysX Error %s:%i] %s", file, line, message );
				BAT_DEBUG_BREAK();
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

	class PxPhysicsMaterial : public IPhysicsMaterial
	{
	public:
		PxPhysicsMaterial( PxMaterial* pMaterial )
			:
			m_pMaterial( pMaterial )
		{
			pMaterial->userData = this;
		}

		virtual float GetDynamicFriction() const override
		{
			return m_pMaterial->getDynamicFriction();
		}
	private:
		PxMaterial* m_pMaterial;
	};

	static int Bat2PxGeometryType( PhysicsGeometryType type )
	{
		return (int)type;
	}

	static PhysicsGeometryType Px2BatGeometryType( int type )
	{
		return (PhysicsGeometryType)type;
	}

	static PxVec3 Bat2PxVec( const Vec3& vec )
	{
		return { vec.x, vec.y, vec.x };
	}

	static Vec3 Px2BatVec( const Vec3& vec )
	{
		return { vec.x, vec.y, vec.x };
	}

	void Physics::Initialize()
	{
		static BatPxErrorCallback px_error_callback;
		static PxDefaultAllocator px_allocator_callback;

		const PxTolerancesScale tolerances_scale;

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

		PxSceneDesc scene_desc( tolerances_scale );
		scene_desc.gravity = { 0.0f, -9.8f, 0.0f };

		//g_pPxScene = g_pPxPhysics->createScene( scene_desc );
	}

	void Physics::Shutdown()
	{
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
				accumulator -= g_flFixedTimestep;
			}
		}
		else
		{
			g_pPxScene->simulate( deltatime );
		}
	}

	IPhysicsMaterial* Physics::CreateMaterial(float static_friction, float dynamic_friction, float restitution)
	{
		return new PxPhysicsMaterial( g_pPxPhysics->createMaterial(static_friction, dynamic_friction, restitution) );
		return nullptr;
	}
}
