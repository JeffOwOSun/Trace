#include <cmath>
#include <set>
#include "light.h"

double DirectionalLight::distanceAttenuation( const vec3f& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


vec3f DirectionalLight::shadowAttenuation( const vec3f& P ) const
{
	vec3f d = getDirection(P);

	// loop to get the attenuation
	vec3f curP = P;
	isect isecP;
	vec3f ret = getColor(P);
	ray r = ray(curP, d);
	while (scene->intersect(r, isecP))
	{
		//if not transparent return black
		if (isecP.getMaterial().kt.iszero()) return vec3f(0, 0, 0);
		//use current intersection point as new light source
		curP = r.at(isecP.t);
		r = ray(curP, d);
		ret = prod(ret, isecP.getMaterial().kt);
	}
	return ret;
}

vec3f DirectionalLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f DirectionalLight::getDirection( const vec3f& P ) const
{
	return -orientation;
}

PointLight::PointLight(Scene *scene, const vec3f& pos, const vec3f& color)
	: Light(scene, color),
	position(pos),
	m_const_atten_coeff(0.0),
	m_linear_atten_coeff(0.0),
	m_quadratic_atten_coeff(0.0),
	m_photon_dir_dist1(-1.0, 1.0),
	m_photon_dir_dist2(-1.0, 1.0),
	m_refractive_index(-1.0)
{}

double PointLight::distanceAttenuation( const vec3f& P ) const
{
	double d2 = (P - position).length_squared();
	double d = sqrt(d2);
	double coeff = m_const_atten_coeff + m_linear_atten_coeff * d + m_quadratic_atten_coeff * d2;
	return 1.0 / max<double>(coeff, 1.0);
}

vec3f PointLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f PointLight::getDirection( const vec3f& P ) const
{
	return (position - P).normalize();
}


vec3f PointLight::shadowAttenuation(const vec3f& P) const
{ 
	vec3f d = getDirection(P);

	// distance from P to this light
	double distance = (position - P).length();
	// loop to get the attenuation
	vec3f curP = P;
	isect isecP;
	vec3f ret = getColor(P);
	ray r = ray(curP, d);
	while (scene->intersect(r, isecP))
	{
		//prevent going beyond this light
		if ((distance -= isecP.t) < RAY_EPSILON) return ret;
		//if not transparent return black
		if (isecP.getMaterial().kt.iszero()) return vec3f(0, 0, 0);
		//use current intersection point as new light source
		curP = r.at(isecP.t);
		r = ray(curP, d);	
		ret = prod(ret, isecP.getMaterial().kt);
	}

	return ret;
}


void PointLight::setDistanceAttenuation(const double constant,
	const double linear, const double quadratic)
{
	m_const_atten_coeff = constant;
	m_linear_atten_coeff = linear;
	m_quadratic_atten_coeff = quadratic;
}

ray PointLight::getPhoton(std::default_random_engine &generator) const
{
	double x1, x2, x0;
	do {
		x1 = m_photon_dir_dist1(generator);
		x2 = m_photon_dir_dist2(generator);
	} while ((x0 = x1 * x1 + x2 * x2) >= 1);
	double x = 2 * x1 * sqrt(1 - x0);
	double y = 2 * x2 * sqrt(1 - x0);
	double z = 1 - 2 * x0;
	vec3f dir(x, y, z); //this random direction is uniformly distributed on the unit sphere;
	return ray(position, dir);
}

double PointLight::getCumulativeIndex() {
	if (m_refractive_index > 0) return m_refractive_index;
	//calculate the refractive index
	std::default_random_engine generator;
	//randomly pick a ray
	ray r = getPhoton(generator);
	//trace the ray
	multiset<const SceneObject*> objs;
	isect i;
	while (scene->intersect(r, i)) {
		if (i.obj->hasInterior()) { //only care about volumetric shapes
			if (r.getDirection().dot(i.N) > RAY_EPSILON) { //going out of the material
				if (objs.find(i.obj) == objs.end()) { //not found in the set
					//that must be one.
					m_refractive_index *= i.getMaterial().index;
					r = ray(r.at(i.t), r.getDirection());
				}
				else { //found the same object already
					objs.erase(i.obj);
					r = ray(r.at(i.t), r.getDirection());
				}
			}
			else { //going into the material
				objs.insert(i.obj);
				r = ray(r.at(i.t), r.getDirection());
			}
		}
	}
	return m_refractive_index;
}

double AmbientLight::distanceAttenuation(const vec3f& P) const
{
	return 1.0;
}

vec3f AmbientLight::getColor(const vec3f& P) const
{
	return color;
}

vec3f AmbientLight::getDirection(const vec3f& P) const
{
	return vec3f(1, 1, 1);
}


vec3f AmbientLight::shadowAttenuation(const vec3f& P) const
{
	return vec3f(1, 1, 1);
}