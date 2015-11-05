#ifndef PHOTON_MAPPING_H
#define PHOTON_MAPPING_H

#include <vector>
#include <random>
#include "vecmath/vecmath.h"
#include "scene/scene.h"
#include "scene/ray.h"
#include "scene/light.h"

template <typename T>
inline T max(T a, T b, T c) {
	if (a > b) {
		if (b > c) {
			return a;
		}
		else if (a > c) {
			return a;
		}
		else {
			return c;
		}
	}
	else if (a > c) {
		return b;
	}
	else if (b > c) {
		return b;
	}
	else {
		return c;
	}
}

inline float max(vec3f v) {
	return max<float>(v[0], v[1], v[2]);
}
// This is an example of a custom data set class
template <typename T>
struct PhotonMap
{
	struct Photon
	{
		//coordinates for the point
		T  x, y, z;
		//the energy values
		vec3f energy;
	};

	std::vector<Photon>  pts;

	// Must return the number of data points
	inline size_t kdtree_get_point_count() const { return pts.size(); }

	// Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
	inline T kdtree_distance(const T *p1, const size_t idx_p2, size_t /*size*/) const
	{
		const T d0 = p1[0] - pts[idx_p2].x;
		const T d1 = p1[1] - pts[idx_p2].y;
		const T d2 = p1[2] - pts[idx_p2].z;
		return d0*d0 + d1*d1 + d2*d2;
	}

	// Returns the dim'th component of the idx'th point in the class:
	// Since this is inlined and the "dim" argument is typically an immediate value, the
	//  "if/else's" are actually solved at compile time.
	inline T kdtree_get_pt(const size_t idx, int dim) const
	{
		if (dim == 0) return pts[idx].x;
		else if (dim == 1) return pts[idx].y;
		else return pts[idx].z;
	}

	// Optional bounding-box computation: return false to default to a standard bbox computation loop.
	//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
	//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
	template <class BBOX>
	bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }

};

template <typename T>
void generatePhotonMap(PhotonMap<T> &point, const Scene* scene, const size_t N, const T max_range = 10)
{
	std::cout << "Generating " << N << " photon map...";
	point.pts.resize(N);
	
	//for randomized light source choosing
	//vector for lights to facilitate direct access
	std::vector<Light*> lights;
	for (Scene::cliter iter = scene->beginLights(); iter != scene->endLights(); ++iter) {
		lights.push_back(*iter);
	}
	std::default_random_engine generator;
	std::uniform_int_distribution<int> light_random(0, lights.size() - 1);
	std::uniform_real_distribution<double> uniform_dist(0, 1.0);
	size_t count = 0;

	while (count < N);
	{
		//randomly pick a light source
		const Light* light = lights[light_random(generator)];
		//the stack for refracton
		double cumulative_index = light->getCumulativeIndex();
		//emit a photon
		PointLight* point_light = dynamic_cast<PointLight*>(light);
		if (point_light) { //the light is indeed a point_light
			//get the photon ray vector
			ray r = point_light->getPhoton(generator);
			//the intensity of the photon
			vec3f intensity = point_light->getColor();
			isect i;
			//use a loop to try the intersection progressively
			while (scene->intersect(r, i);) {
				//look at the material of the intersection point.
				//use Russian Roulette to determine the fate of this photon
				//reference:page16@https://www.siggraph.org/sites/default/files/sample-course-notesa.pdf
				Material m = i.getMaterial();
				double maxI = max(intensity);
				double pr = max(m.ke.prod(intensity)) / maxI;
				double pt = max(m.kt.prod(intensity)) / maxI;
				double ps = max(m.ks.prod(intensity)) / maxI;
				double pd = max(m.ks.prod(intensity)) / maxI;
				double p = (pr + pt + ps + pd < 1.0) ? 1.0 : pr + pt + ps + pd;
				double epsilon = uniform_dist(generator);
				if (epsilon <= pr / p) {
					//reflected
					//modify the intensity
					intensity = m.kr.prod(intensity) / pr;
					//continue the tracing with new ray
					vec3f dir = r.getDirection();
					dir = dir + 2 * dir.dot(i.N) * i.N;
					r = ray(r.at(i.t), dir);
				}
				else if (epsilon <= (pr + pt) / p) {
					//continue the tracing with new ray
					//consider the refraction
					if (i.obj->hasInterior()) {
						double cos_i = r.getDirection().dot(i.N);
						double relative_index;
						if (cos_i > RAY_EPSILON) { //the ray is going out
							relative_index = i.getMaterial().index;
							//flip the normal to point to the side of incident. Justified for later calculation of direction
							i.N = -i.N; 
						}
						else { //the ray is going in
							relative_index = 1 / i.getMaterial().index;
							cos_i = -cos_i; //make sure it's positive, again for dir calculation
						}
						double sin_i = sqrt(1 - cos_i*cos_i);
						double sin_r = sin_i * relative_index;

						if (sin_r > 1) { //total reflection
							//continue tracing with reflected ray
							vec3f dir = r.getDirection();
							dir = dir + 2 * dir.dot(i.N) * i.N;
							r = ray(r.at(i.t), dir);
						}
						else {
							//continue tracing with refracted ray
							double cos_r = sqrt(1 - sin_r * sin_r);
							//calculate the direction
							vec3f dir = (relative_index * cos_i - cos_r) * i.N + relative_index * r.getDirection();
							r = ray(r.at(i.t), dir);
							//attenune the intensity of refracted ray
							intensity = m.kt.prod(intensity) / pt;
							//accumulate the relative_index
							cumulative_index /= relative_index;
						}
					}
					else { //don't consider thickless shapes
						r = ray(r.at(i.t), r.getDirection());
					}
				}
				else if (epsilon <= (pr + pt + ps) / p) {
					//specular reflected
					intensity = m.ks.prod(intensity) / ps;
					//continue the tracing with new ray
					vec3f dir = r.getDirection();
					dir = dir + 2 * dir.dot(i.N) * i.N;
					r = ray(r.at(i.t), dir);
				}
				else if (epsilon <= (pr + pt + ps + pd) / p) {
					//diffused
					intensity = m.kd.prod(intensity) / pd;
					//remember the intensity
					vec3f loc = r.at(i.t);
					point.pts[count].x = loc[0];
					point.pts[count].y = loc[1];
					point.pts[count].z = loc[2];
					point.pts[count].energy = intensity;
				}
				else {
					//absorbed
					break;
				}
 			}
		}
		else {//other types of light source here
			//ignore them for now!
		}
	}

	std::cout << "done\n";
}


#endif //PHOTON_MAPPING_H