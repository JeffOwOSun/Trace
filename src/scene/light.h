#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "scene.h"
#include <random>

class Light
	: public SceneElement
{
public:
	virtual vec3f shadowAttenuation(const vec3f& P) const = 0;
	virtual double distanceAttenuation( const vec3f& P ) const = 0;
	virtual vec3f getColor( const vec3f& P ) const = 0;
	virtual vec3f getDirection( const vec3f& P ) const = 0;
	virtual double getCumulativeIndex() const { return 1.0; } //return the refractive index of this light source's environment

protected:
	Light( Scene *scene, const vec3f& col )
		: SceneElement( scene ), color( col ) {}

	vec3f 		color;
};

class DirectionalLight
	: public Light
{
public:
	DirectionalLight( Scene *scene, const vec3f& orien, const vec3f& color )
		: Light( scene, color ), orientation( orien ) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation( const vec3f& P ) const;
	virtual vec3f getColor( const vec3f& P ) const;
	virtual vec3f getDirection( const vec3f& P ) const;
	//TODO: add getPhoton for directional light

protected:
	vec3f 		orientation;
};

class PointLight
	: public Light
{
public:
	PointLight(Scene *scene, const vec3f& pos, const vec3f& color);
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation( const vec3f& P ) const;
	virtual vec3f getColor( const vec3f& P ) const;
	virtual vec3f getDirection( const vec3f& P ) const;
	void setDistanceAttenuation(const double constant, const double linear, const double quadratic);
	/**
	 * \brief Helper function, for easy implemente soft shadow
	 * \param P the point which intersect
	 * \param r the light
	 * \return the shade effect on this point
	 */
	vec3f _shadowAttenuation(const vec3f& P, const ray& r) const;
	virtual ray getPhoton(std::default_random_engine &generator) const;
	virtual double getCumulativeIndex();
protected:
	vec3f position;
	double m_const_atten_coeff, m_linear_atten_coeff, m_quadratic_atten_coeff;
private:
	std::uniform_real_distribution<double> m_photon_dir_dist1;
	std::uniform_real_distribution<double> m_photon_dir_dist2;
	double m_refractive_index;
};

class AmbientLight
	: public Light
{
public:
	AmbientLight(Scene *scene, const vec3f& pos, const vec3f& color) 
		: Light(scene, color), color(color) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation(const vec3f& P) const;
	virtual vec3f getColor(const vec3f& P) const;
	virtual vec3f getDirection(const vec3f& P) const;
private:
	vec3f color;
};

#endif // __LIGHT_H__
