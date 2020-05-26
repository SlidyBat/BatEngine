#pragma once

#include "Core/ResourceManager.h"
#include "Core/Entity.h"
#include "Colour.h"

namespace Bat
{
	static constexpr int MAX_PARTICLES = 1000;

	struct Particle
	{
		Vec3 velocity;
		float rot_velocity;
		Vec3 position;
		float age;
		Vec4 colour;
	};

	struct ParticleEmitterComponent
	{
		BAT_COMPONENT( PARTICLE_EMITTER );

		ParticleEmitterComponent( Resource<Texture> texture )
			:
			texture( std::move( texture ) )
		{}

		std::vector<Particle> particles;
		int num_particles = 0;
		Resource<Texture> texture;
		Gradient gradient;

		float particles_per_sec = 60.0f;
		float lifetime = 1.0f;
		float start_scale = 1.0f;
		float end_scale = 1.0f;
		float start_alpha = 1.0f;
		float end_alpha = 1.0f;
		float motion_blur = 0.0f;
		Vec3 force = { 0.0f, -9.8f, 0.0f };
		float force_multiplier = 0.0f;

		Vec3 rand_velocity_range = { 0.5f, 0.5f, 0.5f }; // Range of values to randomly add to velocity
		float rand_rot_velocity_range = 5.0f;
		Vec3 normal = { 0.0f, 1.0f, 0.0f }; // The direction to emit particles to (velocity is relative to size of this)
		float timer = 0.0f;
	};

	class ParticleSystem
	{
	public:
		void Update( EntityManager& world, float dt );
	};
}