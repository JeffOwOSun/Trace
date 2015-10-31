#include <cmath>

#include "light.h"

double DirectionalLight::distanceAttenuation( const vec3f& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


vec3f DirectionalLight::shadowAttenuation( const vec3f& P ) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.
    return vec3f(1,1,1);
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
    return vec3f(1,1,1);
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