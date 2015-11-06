#include "PhotonMapping.h"
using namespace std;

template <typename T>
void PhotonMap::generatePhotons(PointCloud<T> &point, PointCloud<T> &caustic, const Scene* scene, const size_t N)
{
	std::cout << "Generating " << N << " photon map...";
	point.pts.resize(N); //this is good in terms of re-initializing
	caustic.pts.clear();
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

	while (count < N)
	{
		//randomly pick a light source
		const Light* light = lights[light_random(generator)];
		//the stack for refracton
		double cumulative_index = light->getCumulativeIndex();
		//emit a photon
		const PointLight* point_light = dynamic_cast<const PointLight*>(light);
		if (point_light) { //the light is indeed a point_light
			//get the photon ray vector
			ray r = point_light->getPhoton(generator);
			//the intensity of the photon
			vec3f intensity = point_light->getColor(vec3f());
			isect i;
			//use this boolean to identify whether a photon has been reflected once.
			//Only store those reflected at least once and then diffused into the caustic map
			bool reflected_once = false;
			//use a loop to try the intersection progressively
			while (scene->intersect(r, i)) {
				//look at the material of the intersection point.
				//use Russian Roulette to determine the fate of this photon
				//reference:page16@https://www.siggraph.org/sites/default/files/sample-course-notesa.pdf
				Material m = i.getMaterial();
				double maxI = max3(intensity);
				double pr = max3(prod(m.ke, intensity)) / maxI;
				double pt = max3(prod(m.kt, intensity)) / maxI;
				double ps = max3(prod(m.ks, intensity)) / maxI;
				double pd = max3(prod(m.kd, intensity)) / maxI;
				double p = (pr + pt + ps + pd < 1.0) ? 1.0 : pr + pt + ps + pd;
				double epsilon = uniform_dist(generator);
				//=====reflection=====
				if (epsilon <= pr / p) {
					//reflected
					//modify the intensity
					intensity = prod(m.kr, intensity) / pr;
					//continue the tracing with new ray
					vec3f dir = r.getDirection();
					dir = dir + 2 * dir.dot(i.N) * i.N;
					r = ray(r.at(i.t), dir);
					reflected_once = true;
				}
				//=====transmission=====
				else if (epsilon <= (pr + pt) / p) {
					//continue the tracing with new ray
					//consider the refraction
					if (i.obj->hasInterior()) {
						double cos_i = max(min(r.getDirection().dot(i.N), 1.0), -1.0);
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

							reflected_once = true;
						}
						else {
							//continue tracing with refracted ray
							double cos_r = sqrt(1 - sin_r * sin_r);
							//calculate the direction
							vec3f dir = (relative_index * cos_i - cos_r) * i.N + relative_index * r.getDirection();
							r = ray(r.at(i.t), dir);
							//attenune the intensity of refracted ray
							intensity = prod(m.kt, intensity) / pt;
							//accumulate the relative_index
							cumulative_index /= relative_index;

							reflected_once = true;
						}
					}
					else { //don't consider thickless shapes
						r = ray(r.at(i.t), r.getDirection());
					}
				}
				//=====specular reflection=====
				else if (epsilon <= (pr + pt + ps) / p) {
					//specular reflected
					intensity = prod(m.ks, intensity) / ps;
					//continue the tracing with new ray
					vec3f dir = r.getDirection();
					dir = dir + 2 * dir.dot(i.N) * i.N;
					r = ray(r.at(i.t), dir);

					reflected_once = true;
				}
				//=====diffuse=====
				else if (epsilon <= (pr + pt + ps + pd) / p) {
					//diffused
					intensity = prod(m.kd, intensity) / pd;
					//remember the intensity
					vec3f loc = r.at(i.t);
					point.pts[count].x = loc[0];
					point.pts[count].y = loc[1];
					point.pts[count].z = loc[2];
					point.pts[count].energy = intensity;
					if (reflected_once) { //not direct illumination
						PhotonMap::PointCloud<double>::Photon photon;
						photon.x = loc[0];
						photon.y = loc[1];
						photon.z = loc[2];
						photon.energy = intensity;
						caustic.pts.push_back(photon);
					}
					++count;
					if (!(count % 1000))
						std::cout << "generated " << count << " photons\n";
					break;
				}
				//=====absorption=====
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
	//average the energy value out
	std::cout << "averaging the energy ...\n";
	for (int i = 0; i < count; ++i) {
		point.pts[i].energy /= count;
	}
	for (int i = 0; i < caustic.pts.size(); ++i) {
		caustic.pts[i].energy /= count;
	}
	std::cout << "done\n";
}


PhotonMap::PhotonMap() : m_index(NULL), m_caustic_index(NULL), m_scene(NULL), m_nN(0) {}

void PhotonMap::initialize(Scene* scene, const size_t N) {
	//generate the photons into point cloud
	if (m_scene != scene || m_nN != N) {
		generatePhotons(m_cloud, m_caustic, scene, N);
		m_scene = scene;
		m_nN = N;
		if (m_index) delete m_index; //delete previous copy of kd_tree
		m_index = new my_kd_tree_t(3 /*dim*/, m_cloud, KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
		m_index->buildIndex();
		if (m_caustic_index) delete m_caustic_index; //delete previous copy of kd_tree
		m_caustic_index = new my_kd_tree_t(3 /*dim*/, m_caustic, KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
		m_caustic_index->buildIndex();
	}
}

//find the color of the given point
vec3f PhotonMap::shade(const vec3f& point) {
	//find the nearest n points
	const size_t num_results = 3; //N = 10
	const double atten_coeff = 1.0; //3.0
	std::vector<size_t>   ret_index(num_results);
	std::vector<double> out_dist_sqr(num_results);
	m_index->knnSearch(point.n, num_results, &ret_index[0], &out_dist_sqr[0]);
	//find the sphere that covers them
	double radius = out_dist_sqr.back();
	//calculate the intensity
	vec3f ret;
	for (int i = 0; i < ret_index.size(); ++i) {
		ret += m_cloud.pts[ret_index[i]].energy;
	}
#define PI 3.14159265358979
	return ret / (PI * radius * radius) / atten_coeff;
}

//find the color of the given point
vec3f PhotonMap::shadeCaustic(const vec3f& point) {
	//find the nearest n points
	const size_t num_results = 50; //N = 10
	//const double max_radius = 0.0001;
	const double atten_coeff = 1.0; //3.0
	std::vector<size_t>   ret_index(num_results);
	std::vector<double> out_dist_sqr(num_results);
	//std::vector<std::pair<size_t, double> >   ret_matches;
	//nanoflann::SearchParams params;
	//m_caustic_index->radiusSearch(point.n, max_radius, ret_matches, params);
	//if (ret_matches.size() > num_results) {
		m_caustic_index->knnSearch(point.n, num_results, &ret_index[0], &out_dist_sqr[0]);
			//find the sphere that covers them
			double radius = out_dist_sqr.back();
			//calculate the intensity
			vec3f ret;
			for (int i = 0; i < ret_index.size(); ++i) {
				ret += m_caustic.pts[ret_index[i]].energy;
			}
		#define PI 3.14159265358979
			return ret / (PI * radius * radius) / atten_coeff;
	//}
	//else {
	//	vec3f ret;
	//	for (int i = 0; i < ret_matches.size(); ++i) {
//			ret += m_caustic.pts[ret_matches[i].first].energy;
	//	}
//#define PI 3.14159265358979
	//	return ret / (PI * max_radius * max_radius) / atten_coeff;
	//}
	
}