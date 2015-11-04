#include "ParticleSys.h"
#include "Cylinder.h"
#include <random>

void ParticleSource::render()
{
	double time = 0;
	for (int frame = 0; frame < numFrame; ++frame) {
		time = frame * frameTime;
		std::default_random_engine generator1, generator2;
		std::uniform_real_distribution<double> position_distribution(-1.0, 1.0);
		std::uniform_real_distribution<double> velocity_distribution(-1.0, 1.0);
		//initialize the particles
		for (int i = 0; i < numParticles; ++i) {
			Particle *myParticle = new Particle;
			myParticle->life = initialLife;
			vec3f position;
			for (int i = 0; i < 3; ++i) {
				position[i] = position_distribution(generator1);
			}
			myParticle->position = position;
			vec3f velocity;
			for (int i = 0; i < 3; ++i) {
				velocity[i] = velocity_distribution(generator2);
			}
			myParticle->velocity = velocity.normalize() * initialSpeed;

			particles.push_back(myParticle);
		}
		//calculate the particles
		for (Particles::iterator iter = particles.begin(); iter != particles.end(); ++iter) {
			Particle* particle = *iter;
			particle->life -= frameTime;

			//kill the ones that are dead
			if (particle->life < 0) {
				delete particle;
				particles.erase(iter);
				continue;
			}

			//evolve
			particle->position += particle->velocity * frameTime;
			particle->velocity += gravity * frameTime;
		}
	}

	//Now that the system is complete, I need to render the particles into primitives for ray tracing
	/**
	//transformations of the particle source
	//Set an upDir
	vec3f upDir;
	if (abs(orientation[2]) > RAY_EPSILON) {
		upDir = vec3f(1, 1, -(orientation[0] + orientation[1]) / orientation[2]).normalize();
	}
	else {
		//Given Orientation is in xy plane
		upDir = vec3f(0, 0, 1);
	}
	//basic transformation of the particle system
	TransformRoot transform;
	//translate to the origin
	transform.createChild(mat4f::translate(origin));
	//rotation of the axes;
	vec4f row1(orientation.cross(upDir)); row1[3] = 0.0;
	vec4f row2(upDir); row2[3] = 0.0;
	vec4f row3(orientation); row3[3] = 0.0;
	vec4f row4; row4[3] = 1.0;
	transform.createChild(mat4f(row1, row2, row3, row4).transpose());
	*/
	for (Particles::iterator iter = particles.begin(); iter != particles.end(); ++iter) {
		Particle* particle = *iter;

		//set the material according to life============================================
		Material* myMat;
		myMat->ke = vec3f(1.0, 0.0, 0.0); //pure red, for now

		//the object====================================================================
		SceneObject* obj = new Cylinder(scene, myMat, true);

		//set the transformation matrix of the obj======================================
		TransformNode *transform = this->transform;

		//translate the cylinder to the position of the particle
		transform = transform->createChild(mat4f::translate(particle->position));

		//rotate the cylinder to the direction of velocity
		vec3f upDir;
		if (abs(particle->velocity[2]) > RAY_EPSILON) {
			upDir = vec3f(1, 1, -(particle->velocity[0] + particle->velocity[1]) / particle->velocity[2]).normalize();
		}
		else {
			//Given velocity is in xy plane
			upDir = vec3f(0, 0, 1);
		}
		vec4f row1(particle->velocity.cross(upDir)); row1[3] = 0.0;
		vec4f row2(upDir); row2[3] = 0.0;
		vec4f row3(particle->velocity); row3[3] = 0.0;
		vec4f row4; row4[3] = 1.0;
		transform = transform-> createChild(mat4f(row1, row2, row3, row4).transpose());

		//scale the cylinder to be as long as the velocity times frameTime
		transform = transform->createChild(mat4f::scale(vec3f(0.1, 0.1, frameTime * particle->velocity.length())));

		//move down z axis by 0.5
		transform = transform->createChild(mat4f::translate(vec3f(0.0, 0.0, -0.5)));
		
		obj->setTransform(transform);
		scene->add(obj);
	}
}