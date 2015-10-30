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


	// Iteration 0
	vec3f result = ke; // Firt set to emissive
	
	// Iteration 1
	const vec3f point = r.at(i.t);
	const vec3f ambient = scene->getAmbient();
	result += vec3f(ka[0] * ambient[0], ka[1] * ambient[1], ka[2] * ambient[2]);

	// Iteration 2
	for (Scene::cliter j = scene->beginLights(); j != scene->endLights; ++j) 
	{
		vec3f light = (*j)->getColor(point);
		vec3f diffuse(kd[0] * light[0], kd[1] * light[1], kd[2] * light[2]);
		double angle = max(i.N.dot((*j)->getDirection(point)), 0.0);
		result += diffuse * angle;
		break;
	}

	return result;
}
