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

	void Physics::Initialize()
	{
		static BatPxErrorCallback px_error_callback;
		static PxDefaultAllocator px_allocator_callback;

		g_pPxFoundation = PxCreateFoundation( PX_PHYSICS_VERSION, px_allocator_callback, px_error_callback );
		if( !g_pPxFoundation )
		{
			BAT_ABORT( "Failed to initialize PhysX foundation!" );
		}

		const bool record_mem_allocs = false;
		g_pPxPhysics = PxCreatePhysics( PX_PHYSICS_VERSION, *g_pPxFoundation, PxTolerancesScale(), record_mem_allocs );
		if( !g_pPxPhysics )
		{
			BAT_ABORT( "Failed to initialize PhysX physics object!" );
		}
	}

	void Physics::Shutdown()
	{
		g_pPxPhysics->release();
		g_pPxFoundation->release();
	}
}
