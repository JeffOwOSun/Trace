#include <cmath>

#include "light.h"

double DirectionalLight::distanceAttenuation( const vec3f& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


vec3f DirectionalLight::shadowAttenuation( const vec3f& P ) const
{
	const vec3f &dir = getDirection(P);
	isect i;
	ray R(P, dir);
	if (scene->intersect(R, i))
	{
		return vec3f();
	}
	else
	{
		return vec3f(1.0, 1.0, 1.0);
	}
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
	m_quadratic_atten_coeff(0.0)
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
	// YOUR CODE HERE:  
	// You should implement shadow-handling code here.  
	// direction from P to this light
	vec3f d = getDirection(P);
	
	// distance from P to this light
	double distance = (position - P).length();
	
	// loop to get the attenuation
	vec3f curP = P;
	isect isecP;
	vec3f ret = getColor(P);
	ray r = ray(curP, d);
	bool skip = false; //used to skip the second intersection upon the same geometry
	while (scene->intersect(r, isecP)){
		//prevent going beyond this light
		if ((distance -= isecP.t) < RAY_EPSILON) return ret;
		//if not transparent return black
		if (isecP.getMaterial().kt.iszero())return vec3f(0, 0, 0);
		//use current intersection point as new light source
		curP = r.at(isecP.t);
		r = ray(curP, d);
		if (!skip) {
			ret = prod(ret, isecP.getMaterial().kt);
			skip = true;
		} else {
			skip = false;
		}
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