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
	double m_angle; //360 degrees for a spherical explosion. Spherical symmetry with respect to z axis.
	vec3f m_gravity; //gravitational acceleration for curved projectiles
	double m_initialSpeed; //how fast the particles are projected
	double m_frameTime; //time per frame
	int m_numFrame; //number of frames
	int m_numParticles; //number of particles projected
	double m_initialLife;
	//Render parameters
public:
	ParticleSource(Scene *scene, Material *mat, TransformNode *transform)
		: MaterialSceneObject(scene, mat), m_initialSpeed(1.0), m_initialLife(10.0), m_numFrame(5), m_frameTime(0.1), m_numParticles(10)
	{
		this->transform = transform;
		m_gravity = vec3f(0, 0, 0);
	}
	~ParticleSource();
	//draw the system based on given parameters
	void render();
	void setGravity(double x, double y, double z) {
		m_gravity[0] = x; m_gravity[1] = y; m_gravity[2] = z;
	}
	void setAngle(double angle) {
		m_angle = angle;
	}
	void setInitialSpeed(double initialSpeed) {
		m_initialSpeed = initialSpeed;
	}
	void setFrameTime(double frameTime) {
		m_frameTime = frameTime;
	}
	void setNumFrame(int numFrame) {
		m_numFrame = numFrame;
	}
	void setNumParticles(int numParticles) {
		m_numParticles = numParticles;
	}
	void setInitialLife(double initialLife) {
		m_initialLife = initialLife;
	}

};

#endif //PARTICLE_SYS_H