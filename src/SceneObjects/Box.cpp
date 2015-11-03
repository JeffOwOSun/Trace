#include <cmath>
#include <assert.h>

#include "Box.h"

bool Box::intersectLocal(const ray& r, isect& i) const
{
	// The Box Intersection algorithm addressed here: http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
	i.obj = this;

	numeric_limits<double> doubleLimit;
	double Tfar = doubleLimit.infinity(), Tnear = -Tfar;
	vec3f Nnear, Nfar;

	vec3f &dir = r.getDirection();
	vec3f &ori = r.getPosition();
	const double lBound = -0.5, uBound = 0.5;

	for (int axis = 0; axis < 3; ++axis) {
		if (abs(dir[axis]) < RAY_EPSILON) {
			//parallel to the planes
			if (ori[axis] < lBound || ori[axis] > uBound) {
				//the light ray is out side of the slab
#ifdef _DEBUG
				printf("out of slab!\n");
#endif
				return false;
			}
		}
		//else there should be intersection with this pair of plane
		vec3f N1, N2;
		double t1 = (lBound - ori[axis]) / dir[axis];
		N1[axis] = -1;
		double t2 = (uBound - ori[axis]) / dir[axis];
		N2[axis] = 1;
		//make sure t1 is the intersection with near plane
		if (t1 > t2) {
			double t = t1;
			t1 = t2;
			t2 = t;
			double n = N1[axis];
			N1[axis] = N2[axis];
			N2[axis] = n;
		}

		//tell if this near intersection is better than Tnear
		if (t1 > Tnear) {
			Tnear = t1;
			Nnear = N1;
		}
		//the same for Tfar
		if (t2 < Tfar) {
			Tfar = t2;
			Nfar = N2;
		}
		//return false if the ray misses the box or is behind the source
		if (Tfar < Tnear || Tfar < RAY_EPSILON) {
#ifdef _DEBUG
			printf("missed the box! %f %f\n", Tnear, Tfar);
#endif
			return false;
		}
	}
#ifdef _DEBUG
	printf("intersects!\n");
#endif
	//congrats! this ray survived
	i.setT(Tnear);
	i.setN(Nnear);
	return true;
}
