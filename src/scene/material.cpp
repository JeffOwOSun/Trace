#include "ray.h"
#include "material.h"
#include "light.h"
#include <algorithm>


// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
vec3f Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
    // You will need to call both distanceAttenuation() and shadowAttenuation()
    // somewhere in your code in order to compute shadows and light falloff.

	vec3f viewer = -r.getDirection();
	vec3f normal = i.N; //TODO: check the direction of this norm

	// emission
	vec3f result = ke; // Firt set to emissive
	
	// ambient
	const vec3f point = r.at(i.t);
	const vec3f ambient = scene->getAmbient();
	result += prod(ka, ambient);

	// light sources
	for (Scene::cliter j = scene->beginLights(); j != scene->endLights(); ++j) 
	{
		vec3f lightIntensity = (*j)->getColor(point);
		vec3f light = (*j)->getDirection(point).normalize();
		vec3f R = 2 * (i.N.dot(viewer) * i.N) - viewer;

		//diffuse
		double angle = max(i.N.dot(light), 0.0);
		vec3f diffuse = kd * angle;

		//specular
		vec3f atten = (*j)->distanceAttenuation(point) * (*j)->shadowAttenuation(point);
		
		vec3f specular = ks*pow(max<double>(R.dot(light), 0.0), shininess*128.0);
		result += prod(prod(atten, light), specular+diffuse);
	}
	result = result.clamp();
	
	return result;
}
