#include "ray.h"
#include "material.h"
#include "light.h"
#include <algorithm>


// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
vec3f Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
	// Initial light get set to ke
	vec3f result = ke;
	vec3f normal = i.N;
	vec3f P = r.at(i.t);
	vec3f out_P = P + i.N*RAY_EPSILON;

	// Handle transparency by transmissive light
	vec3f transparency = vec3f(1, 1, 1) - kt;

	// Handle ambient light effect
	vec3f ambient = prod(ka, scene->getAmbient());

	result += prod(transparency, ambient);
	
	// iterate over ray
	for (Scene::cliter j = scene->beginLights(); j != scene->endLights(); j++){
		// shadow and distance attenuation, the color part is handled in light
		// Note use out_P is important, to see the effect, select recur_depth and look at the red one
		vec3f atten = (*j)->distanceAttenuation(P)*(*j)->shadowAttenuation(out_P);

		vec3f light = ((*j)->getDirection(P)).normalize();
		double angle = maximum(normal.dot(light), 0.0);
		vec3f diffuse = prod(kd * angle, transparency);

		vec3f R = ((2 * (normal.dot(light)) * normal) - light).normalize();
		vec3f specular = ks*(pow(maximum(R*(-r.getDirection()), 0), shininess*128.0));
		result += prod(atten, diffuse + specular);
	}
	result = result.clamp();
	return result;
}
