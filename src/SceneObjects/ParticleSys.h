#ifndef PARTICLE_SYS_H
#define PARTICLE_SYS_H

#include <list>
#include <vector>
#include "../vecmath/vecmath.h"
#include "../scene/ray.h"
#include "../scene/material.h"
#include "../scene/scene.h"

struct Particle
{
public:
	vec3f position;
	vec3f velocity;
	double life;
};

class ParticleSource : public MaterialSceneObject
{
	friend class Particle;
	typedef list<Particle*> Particles; //use list to remove particles more easily
	Particles particles;

	//parameters for the particle source
	//System states
	//vec3f origin;
	//vec3f orientation; // where the z axis is pointing at.
	double angle; //360 degrees for a spherical explosion. Spherical symmetry with respect to z axis.
	vec3f gravity; //gravitational acceleration for curved projectiles
	double initialSpeed; //how fast the particles are projected
	double frameTime; //time per frame
	int numFrame; //number of frames
	int numParticles; //number of particles projected
	double initialLife;
	//Render parameters
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