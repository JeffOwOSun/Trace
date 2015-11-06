#ifndef PHOTON_MAPPING_H
#define PHOTON_MAPPING_H

#include <vector>
#include <random>
#include "include/nanoflann.hpp"
#include "vecmath/vecmath.h"
#include "scene/scene.h"
#include "scene/ray.h"
#include "scene/light.h"

using namespace nanoflann;

//inline function to facilitate max between three elements
template <typename T>
inline T max3(T a, T b, T c) {
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

inline double max3(vec3f v) {
	return max3<double>(v[0], v[1], v[2]);
}

inline double cone_filter(double r, double a = -1000.0, double b = 1.0) {
	return (a * r + b < 0) ? 0.0 : a * r + b;
}

class PhotonMap {
	//collection of photons
	template <typename T>
	struct PointCloud {
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

	template<typename T>
	void generatePhotons(PointCloud<T> &cloud, PointCloud<T> &caustic, const Scene* scene, const size_t N);
	
	Scene* m_scene;
	PointCloud<double> m_cloud;
	PointCloud<double> m_caustic;
	size_t m_nN;
	
	// construct a kd-tree index:
	typedef KDTreeSingleIndexAdaptor<
		L2_Simple_Adaptor<double, PointCloud<double> >,
		PointCloud<double>,
		3 /* dim */
	> my_kd_tree_t;

	my_kd_tree_t* m_index;
	my_kd_tree_t* m_caustic_index;

public:
	PhotonMap();
	void initialize(Scene* scene, const size_t N);
	vec3f shade(const vec3f& point);
	vec3f shadeCaustic(const vec3f& point);
};

#endif //PHOTON_MAPPING_H