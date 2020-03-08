#include "PCH.h"
#include "Particles.h"

#include "CoreEntityComponents.h"

namespace Bat
{
	static Particle CreateParticle( const Vec3& pos, const ParticleEmitterComponent& emitter )
	{
		Vec3 rand_vel = {
			Math::GetRandomFloat( -emitter.rand_velocity_range.x, emitter.rand_velocity_range.x ),
			Math::GetRandomFloat( -emitter.rand_velocity_range.y, emitter.rand_velocity_range.y ),
			Math::GetRandomFloat( -emitter.rand_velocity_range.z, emitter.rand_velocity_range.z )
		};

		Particle new_particle;
		new_particle.position = pos;
		new_particle.velocity = emitter.normal + rand_vel;
		new_particle.age = 0.0f;
		new_particle.colour = emitter.gradient.Get( 0.0f ).AsVector();
		new_particle.rot_velocity = Math::GetRandomFloat( -emitter.rand_rot_velocity_range, emitter.rand_rot_velocity_range );
		return new_particle;
	}

	void ParticleSystem::Update( EntityManager& world, float dt )
	{
		for( Entity e : world )
		{
			if( !e.Has<ParticleEmitterComponent>() )
			{
				continue;
			}

			auto& hier = e.Get<HierarchyComponent>();

			Vec3 emitter_pos = hier.abs_transform.GetTranslation();

			auto& emitter = e.Get<ParticleEmitterComponent>();
			emitter.particles.resize( MAX_PARTICLES );

			emitter.timer += dt;
			float creation_interval = 1.0f / emitter.particles_per_sec;
			int particles_to_create = 0;
			while( emitter.timer >= creation_interval )
			{
				particles_to_create++;
				emitter.timer -= creation_interval;
			}

			for( int i = 0; i < emitter.num_particles; i++ )
			{
				Particle& p = emitter.particles[i];
				p.velocity += emitter.force * dt * emitter.force_multiplier;
				p.position += p.velocity * dt;
				p.colour = emitter.gradient.Get( p.age / emitter.lifetime ).AsVector();

				p.age += dt;
				if( p.age >= emitter.lifetime )
				{
					if( particles_to_create )
					{
						p = CreateParticle( emitter_pos, emitter );
						particles_to_create--;
					}
					else
					{
						p = emitter.particles[emitter.num_particles - 1];
						emitter.num_particles--;
						i--;
					}
				}
			}

			for( int i = 0; i < particles_to_create; i++ )
			{
				if( emitter.num_particles > MAX_PARTICLES )
				{
					return;
				}

				emitter.particles[emitter.num_particles] = CreateParticle( emitter_pos, emitter );
				emitter.num_particles++;
			}
		}
	}
}
