#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

// The main ray tracer.
#include "PhotonMapping.h"
#include "scene/scene.h"
#include "scene/ray.h"
#include <map>

class RayTracer
{
public:
    RayTracer();
    ~RayTracer();

    vec3f trace( Scene *scene, double x, double y );
	vec3f traceRay( Scene *scene, const ray& r, const vec3f& thresh, int depth );


	void getBuffer( unsigned char *&buf, int &w, int &h );
	double aspectRatio();
	void traceSetup( int w, int h, bool caustic = false);
	void traceLines( int start = 0, int stop = 10000000 );
	void tracePixel( int i, int j );

	bool loadScene( char* fn );

	bool sceneLoaded();

private:
	unsigned char *buffer;
	int buffer_width, buffer_height;
	int bufferSize;
	Scene *scene;
	std::map<int, Material> mediaHistory;
	bool m_bSceneLoaded;
	PhotonMap m_photon_map;
	bool m_bCaustic;
};

#endif // __RAYTRACER_H__
