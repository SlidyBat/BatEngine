#pragma once

namespace Bat
{
	class IPhysicsGeometry;
	class IPhysicsMaterial;

	enum class PhysicsGeometryType
	{
		SPHERE,
		PLANE,
		CAPSULE,
		BOX,
		CONVEXMESH,
		TRIANGLEMESH,
		HEIGHTFIELD
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

		static IPhysicsMaterial* CreateMaterial(float static_friction, float dynamic_friction, float restitution);
		static IPhysicsGeometry* CreateBoxShape();
	};

	class IPhysicsGeometry
	{
	public:
		virtual PhysicsGeometryType GetType() const = 0;
	};

	class IPhysicsMaterial
	{
	public:
		virtual float GetDynamicFriction() const = 0;
	};
}