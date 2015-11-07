// The main ray tracer.

#include <Fl/fl_ask.h>

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "fileio/read.h"
#include "fileio/parse.h"
#include "fileio/HeightField.h"
#include "ui/TraceUI.h"
#include "fileio/bitmap.h"

extern TraceUI* traceUI;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
vec3f RayTracer::trace( Scene *scene, double x, double y )
{
    ray r( vec3f(0,0,0), vec3f(0,0,0) );
    scene->getCamera()->rayThrough( x,y,r );
	
	mediaHistory.clear();
	return traceRay( scene, r, vec3f(1.0,1.0,1.0), 0 ).clamp();
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay( Scene *scene, const ray& r, 
	const vec3f& thresh, int depth )
{
	isect i;

	if( scene->intersect( r, i ) ) {
		vec3f shade;

		if (m_bCaustic) {
			//photon mapping mode
			//shade += m_photon_map.shadeCaustic(r.at(i.t));
			shade += m_photon_map.shade(r.at(i.t));
		}

		if (m_bTrace) {

			const Material& m = i.getMaterial();
			shade += m.shade(scene, r, i);
			if (depth >= traceUI->getDepth()) 
				return shade;

			vec3f conPoint = r.at(i.t); 
			vec3f normal;
			vec3f Rdir = 2 * (i.N*-r.getDirection()) * i.N - (-r.getDirection());
			ray R = ray(conPoint, Rdir);
		
			const double fresnel_coeff = getFresnelCoeff(i, r);
			//cout << fresnel_coeff << endl;
			// Reflection part
			if (!i.getMaterial().kr.iszero()) 
			{
				shade += (fresnel_coeff*prod(i.getMaterial().kr, traceRay(scene, R, thresh, depth + 1)));
			}

			// Refraction part
			// We maintain a map, this map has order so it can be simulated as a extended stack		  
			if (!i.getMaterial().kt.iszero())
			{
				// take account total refraction effect
				bool TotalRefraction = false;
				// opposite ray
				ray oppR(conPoint, r.getDirection()); //without refraction

				// marker to simulate a stack
				bool toAdd = false, toErase = false;

				// For now, the interior is just hardcoded
				// That is, we judge it according to cap and whether it is box
				if (i.obj->hasInterior())
				{
					// refractive index
					double indexA, indexB;

					// For ray go out of an object
					if (i.N*r.getDirection() > RAY_EPSILON)
					{
						if (mediaHistory.empty())
						{
							indexA = 1.0;
						}
						else
						{
							// return the refractive index of last object
							indexA = mediaHistory.rbegin()->second.index;
						}

						mediaHistory.erase(i.obj->getOrder());
						toAdd = true;
						if (mediaHistory.empty())
						{
							indexB = 1.0;
						}
						else
						{
							indexB = mediaHistory.rbegin()->second.index;
						}
						normal = -i.N;
					}
					// For ray get in the object
					else
					{
						if (mediaHistory.empty())
						{
							indexA = 1.0;
						}
						else
						{
							indexA = mediaHistory.rbegin()->second.index;
						}
						mediaHistory.insert(make_pair(i.obj->getOrder(), i.getMaterial()));
						toErase = true;
						indexB = mediaHistory.rbegin()->second.index;
						normal = i.N;
					}

					double indexRatio = indexA / indexB;
					double cos_i = max(min(normal*((-r.getDirection()).normalize()), 1.0), -1.0); //SYSNOTE: min(x, 1.0) to prevent cos_i becomes bigger than 1
					double sin_i = sqrt(1 - cos_i*cos_i);
					double sin_t = sin_i * indexRatio;

					if (sin_t > 1.0)
					{
						TotalRefraction = true;
					}
					else
					{
						TotalRefraction = false;
						double cos_t = sqrt(1 - sin_t*sin_t);
						vec3f Tdir = (indexRatio*cos_i - cos_t)*normal - indexRatio*-r.getDirection();
						oppR = ray(conPoint, Tdir);
						shade += (fresnel_coeff*prod(i.getMaterial().kt, traceRay(scene, oppR, thresh, depth + 1)));
					}
				}

				if (toAdd)
				{
					mediaHistory.insert(make_pair(i.obj->getOrder(), i.getMaterial()));
				}
				if (toErase)
				{
					mediaHistory.erase(i.obj->getOrder());
				}
			}
		}
		
		shade = shade.clamp();
		return shade;
	}
	else {
		// when the light go to infinity
		// If already set a background image, return the image pixel
		// otherwise just black
		if (useBackground)
		{
			vec3f x = scene->getCamera()->getU();
			vec3f y = scene->getCamera()->getV();
			vec3f z = scene->getCamera()->getLook();
			double dis_x = r.getDirection() * x;
			double dis_y = r.getDirection() * y;
			double dis_z = r.getDirection() * z;
			return getBackgroundImage(dis_x / dis_z + 0.5, dis_y / dis_z + 0.5);
		}
		else
		{
			return vec3f(0.0, 0.0, 0.0);
		}
	}
}

double RayTracer::getFresnelCoeff(isect& i, const ray& r)
{
	if (i.obj->hasInterior())
	{
		double indexA, indexB;
		if (i.N*r.getDirection() > RAY_EPSILON)
		{
			if (mediaHistory.empty())
			{
				indexA = 1.0;
			}
			else
			{
				indexA = mediaHistory.rbegin()->second.index;
			}
			mediaHistory.erase(i.obj->getOrder());
			if (mediaHistory.empty())
			{
				indexB = 1.0;
			}
			else
			{
				indexB = mediaHistory.rbegin()->second.index;
			}
			mediaHistory.insert(make_pair(i.obj->getOrder(), i.getMaterial()));
		}
		// For ray get in the object
		else
		{
			if (mediaHistory.empty())
			{
				indexA = 1.0;
			}
			else
			{
				indexA = mediaHistory.rbegin()->second.index;
			}
			mediaHistory.insert(make_pair(i.obj->getOrder(), i.getMaterial()));
			indexB = mediaHistory.rbegin()->second.index;
			mediaHistory.erase(i.obj->getOrder());
		}

		double r0 = (indexA - indexB) / (indexA + indexB);
		r0 = r0 * r0;
		const double dot_rn = i.N.dot(-r.getDirection());

		if (indexA <= indexB)
		{
			return r0 + (1 - r0)*pow(1 - dot_rn, 5);
		}
		else
		{
			cout << "aaa" << endl;
			return 1.0;
			/*const double nr = indexA / indexB;
			const double root = 1 - nr * nr * (1 - dot_rn * dot_rn);
			if (root <= 0.0)
			{
				return 1.0;
			}
			else
			{
				cout << "now index is " << indexA << " " << indexB << endl;
				
				const double cos_theta_t = sqrt(1 - (indexA / indexB));
				printf("And the ratio is %.25f\n", cos_theta_t);
				return r0 + (1 - r0) * pow(1 - cos_theta_t, 5);
			}*/
		}
	}
	else
	{
		return 1.0;
	}
}

void RayTracer::loadBackground(char* fn)
{
	unsigned char* data = NULL;
	data = readBMP(fn, background_width, background_height);
	if (data){
		if (backgroundImage) delete[] backgroundImage;
		useBackground = true;
		backgroundImage = data;
	}
}

RayTracer::RayTracer() : 
mediaHistory(), m_bCaustic(false), m_bTrace(false), backgroundImage(NULL), useBackground(false)
{
	buffer = NULL;
	buffer_width = buffer_height = 256;
	scene = NULL;

	m_bSceneLoaded = false;
}

void RayTracer::clearBackground(){
	if (backgroundImage) delete[] backgroundImage;
	backgroundImage = NULL;
	useBackground = false;
	background_height = background_width = 0;
}

vec3f RayTracer::getBackgroundImage(double x, double y){
	if (!useBackground) return vec3f(0, 0, 0);
	int xGrid = int(x*background_width);
	int yGrid = int(y*background_height);
	if (xGrid < 0 || xGrid >= background_width || yGrid < 0 || yGrid >= background_height) 
	{
		return vec3f(0, 0, 0);
	}
	double val1 = backgroundImage[(yGrid*background_width + xGrid) * 3] / 255.0;
	double val2 = backgroundImage[(yGrid*background_width + xGrid) * 3 + 1] / 255.0;
	double val3 = backgroundImage[(yGrid*background_width + xGrid) * 3 + 2] / 255.0;
	return vec3f(val1, val2, val3);
}


RayTracer::~RayTracer()
{
	if (backgroundImage) delete[] backgroundImage;
	delete [] buffer;
	delete scene;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return scene ? scene->getCamera()->getAspectRatio() : 1;
}

bool RayTracer::sceneLoaded()
{
	return m_bSceneLoaded;
}

bool RayTracer::loadScene( char* fn )
{
	try
	{
		scene = readScene( fn );
	}
	catch( ParseError pe )
	{
		fl_alert( "ParseError: %s\n", pe );
		return false;
	}

	if( !scene )
		return false;
	
	buffer_width = 256;
	buffer_height = (int)(buffer_width / scene->getCamera()->getAspectRatio() + 0.5);

	bufferSize = buffer_width * buffer_height * 3;
	buffer = new unsigned char[ bufferSize ];
	
	// separate objects into bounded and unbounded
	scene->initScene();
	
	// Add any specialized scene loading code here
	
	m_bSceneLoaded = true;

	return true;
}

bool RayTracer::loadHeightMap(char* fn)
{
	try
	{
		scene = readHeights(fn);
	}
	catch (ParseError pe)
	{
		fl_alert("ParseError: %s\n", pe);
		return false;
	}

	if (!scene)
		return false;

	buffer_width = 256;
	buffer_height = (int)(buffer_width / scene->getCamera()->getAspectRatio() + 0.5);

	bufferSize = buffer_width * buffer_height * 3;
	buffer = new unsigned char[bufferSize];

	// separate objects into bounded and unbounded
	scene->initScene();

	// Add any specialized scene loading code here

	m_bSceneLoaded = true;

	return true;
}

void RayTracer::traceSetup(int w, int h, bool trace, bool caustic, int photonNum, int queryNum, double coneAtten, double amplify)
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];
	}
	memset( buffer, 0, w*h*3 );
	m_bTrace = trace;
	m_bCaustic = caustic;
	if (caustic) {
		//initialize the photon map
		m_photon_map.initialize(scene, pow(10, photonNum), queryNum, amplify, coneAtten);
	}
}

void RayTracer::traceLines( int start, int stop )
{
	vec3f col;
	if( !scene )
		return;

	if( stop > buffer_height )
		stop = buffer_height;

	for( int j = start; j < stop; ++j )
		for( int i = 0; i < buffer_width; ++i )
			tracePixel(i,j);
}

void RayTracer::tracePixel( int i, int j )
{
	vec3f col;

	if( !scene )
		return;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	col = trace( scene,x,y );

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}