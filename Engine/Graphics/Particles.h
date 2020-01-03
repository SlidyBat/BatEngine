#pragma once

#include "ResourceManager.h"
#include "Entity.h"
#include "Colour.h"

namespace Bat
{
	static constexpr int MAX_PARTICLES = 1000;

	struct Particle
	{
		Vec3 velocity;
		Vec3 position;
		float lifetime;
	};

	struct ParticleEmitterComponent : public Component<ParticleEmitterComponent>
	{
		ParticleEmitterComponent( Resource<Texture> texture )
			:
			texture( std::move( texture ) )
		{}

		std::vector<Particle> particles;
		int num_particles = 0;
		Resource<Texture> texture;

		float particles_per_sec = 60.0f;
		float lifetime = 1.0f;
		float start_scale = 1.0f;
		float end_scale = 1.0f;
		Colour start_colour = Colours::White;
		Colour end_colour = Colours::White;
		float start_alpha = 1.0f;
		float end_alpha = 1.0f;
		float gravity_multiplier = 1.0f;

		float timer = 0.0f;
	};

	class ParticleSystem
	{
	public:
		void Update( EntityManager& world, float dt );
	};
}