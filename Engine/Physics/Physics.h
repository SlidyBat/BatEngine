#pragma once

namespace Bat
{
	class IPhysicsObject;
	class IStaticObject;
	class IDynamicObject;

	struct PhysicsMaterial
	{
		float static_friction;
		float dynamic_friction;
		float restitution;
	};

	struct RayCastResult
	{
		bool hit; // Whether or not there was a hit. If false, other data in the result is invalid.
		Vec3 position;
		Vec3 normal;
		IPhysicsObject* object;
	};

	enum RayCastFilterFlags
	{
		HIT_STATICS = (1 << 0),
		HIT_DYNAMICS = (1 << 1),
	};

	class Physics
	{
	public:
		static void Initialize();
		static void Shutdown();

		// Sets the gravity of the world. Default is { 0.0, -9.8, 0.0 }.
		static void SetGravity( const Vec3& gravity );
		// Enables fixed timestep with the value provided.
		// Whenever `Simulate` is called it will accumulate the values passed in until
		// it reaches the set fixed timestep value.
		static void EnableFixedTimestep( float deltatime );
		static void DisableFixedTimestep();

		static void Simulate( float deltatime );

		// Creates a new static object (body with infinite mass/inertia) with the given world position/rotation
		// NOTE: must be freed using `delete`
		static IStaticObject* CreateStaticObject( const Vec3& pos, const Vec3& ang );
		// Creates a new dynamic object (body with mass, inertia, velocity) with the given world position/rotation
		// NOTE: must be freed using `delete`
		static IDynamicObject* CreateDynamicObject( const Vec3& pos, const Vec3& ang );

		// See RayCastFilterFlags for possible filter flags
		static RayCastResult RayCast( const Vec3& origin, const Vec3& unit_direction, float max_distance, int filter = (HIT_STATICS|HIT_DYNAMICS) );
	public:
		static constexpr PhysicsMaterial DEFAULT_MATERIAL = { 0.5f, 0.5f, 0.5f };
	};

	class IPhysicsObject
	{
	public:
		virtual void AddSphereShape( float radius, const PhysicsMaterial& material = Physics::DEFAULT_MATERIAL ) = 0;
		virtual void AddCapsuleShape( float radius, float half_height, const PhysicsMaterial& material = Physics::DEFAULT_MATERIAL ) = 0;
		virtual void AddBoxShape( float length_x, float length_y, float length_z, const PhysicsMaterial& material = Physics::DEFAULT_MATERIAL ) = 0;
		// TODO: Convex shapes (PX cooking library)
		
		virtual size_t GetNumShapes() const = 0;
		virtual void RemoveShape( size_t index ) = 0;

		// Gets AABB mins/maxs
		virtual void GetBounds( Vec3* mins, Vec3* maxs ) const = 0;

		// Get world space position
		virtual Vec3 GetPosition() const = 0;
		// Set world space position
		virtual void SetPosition( const Vec3& pos ) = 0;
		// Get world space rotation (euler angle. x=pitch, y=yaw, z=roll)
		virtual Vec3 GetRotation() const = 0;
		// Set world space rotation (euler angle. x=pitch, y=yaw, z=roll)
		virtual void SetRotation( const Vec3& ang ) = 0;
	};

	// Body with implicit infinite mass/inertia
	class IStaticObject : public IPhysicsObject
	{
	public:
		virtual void AddPlaneShape( const PhysicsMaterial& material = Physics::DEFAULT_MATERIAL ) = 0;
		// Uh, nothing else really (these objects just sit there and do nothing)
	};

	// Body with mass, inertia, velocity
	class IDynamicObject : public IPhysicsObject
	{
	public:
		// Kinematic actors don't follow the laws of motions directly, they must instead be updated explicitly using `MoveTo`
		// A kinematic actor can influence any other dynamic body, but the opposite is not true
		virtual void SetKinematic( bool kinematic ) = 0;
		virtual bool IsKinematic() const = 0;
		// Moves a kinematic actor to the desired position
		virtual void MoveTo( const Vec3& pos, const Vec3& ang ) = 0;

		virtual void SetGravityDisabled( bool grav_disabled ) = 0;
		virtual bool IsGravityDisabled() const = 0;

		// Sets the centre of mass of the body (in local object space)
		virtual void SetCentreOfMass( const Vec3& pos ) = 0;
		// Sets the centre of mass of the body (in local object space)
		virtual Vec3 GetCentreOfMass() const = 0;

		// Sets mass of the body. Default is 1.
		// Mass of 0 is considered infinite.
		virtual void SetMass( float mass ) = 0;
		virtual float GetMass() const = 0;
		// Gets inverse of mass
		virtual float GetInvMass() const = 0;

		// Sets linear damping (value that linear velocity is multiplied by). Default is 0.
		// Value of 0 signifies no damping.
		virtual void SetLinearDamping( float lin_damping ) = 0;
		// Gets linear damping (value that linear velocity is multiplied by). Default is 0.
		// Value of 0 signifies no damping.
		virtual float GetLinearDamping() const = 0;
		// Sets angular damping (value that angular velocity is multiplied by). Default is 0.
		// Value of 0 signifies no damping.
		virtual void SetAngularDamping( float ang_damping ) = 0;
		// Gets angular damping (value that angular velocity is multiplied by). Default is 0.
		// Value of 0 signifies no damping.
		virtual float GetAngularDamping() const = 0;

		virtual Vec3 GetLinearVelocity() const = 0;
		virtual void SetLinearVelocity( const Vec3& lin_vel ) = 0;
		virtual Vec3 GetAngularVelocity() const = 0;
		virtual void SetAngularVelocity( const Vec3& ang_vel ) = 0;

		virtual float GetMaxLinearVelocity() const = 0;
		virtual void SetMaxLinearVelocity( float max_lin_vel ) = 0;
		virtual float GetMaxAngularVelocity() const = 0;
		virtual void SetMaxAngularVelocity( float max_ang_vel ) = 0;

		// Applies force to the body's centre of mass (therefore causing no torque/rotation)
		virtual void AddForce( const Vec3& force ) = 0;
		// Applies impulse to body's centre of mass
		virtual void AddLinearImpulse( const Vec3& lin_impulse ) = 0;
		// Applies torque to an object
		virtual void AddTorque( const Vec3& torque ) = 0;
		// Applies angular impulse to object
		virtual void AddAngularImpulse( const Vec3& ang_impulse ) = 0;
	};
}