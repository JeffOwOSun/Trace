#ifndef PARTICLE_SYS_H
#define PARTICLE_SYS_H

#include <list>
#include <vector>
#include "../vecmath/vecmath.h"
#include "../scene/ray.h"
#include "../scene/material.h"
#include "../scene/scene.h"

class Particle
{
public:
	vec3f position;
	vec3f velocity;
	double life;
};

class ParticleSource : public MaterialSceneObject
{
	friend class Particle;
	typedef vector<Particle*> Particles;
	Particles particles;

	//parameters for the particle source
	//System states
	double angle; //360 degrees for a spherical explosion
	vec3f gravity; //gravitational acceleration for curved projectiles
	double initialSpeed; //how fast the particles are projected
	double frameTime; //time per frame
	int numFrame; //number of frames
	int numParticles; //number of particles projected
	double initialLife;
	//Render parameters
	//color?
	//
public:
	ParticleSource(Scene *scene, Material *mat, TransformNode *transform)
		: MaterialSceneObject(scene, mat) //DO WE NEED MAT HERE?
	{
		this->transform = transform;
	}
	~ParticleSource();
	//draw the system based on given parameters
	void render();
};

#endif //PARTICLE_SYS_H