#include <Fl/fl_ask.h>

#include "HeightField.h"
#include "bitmap.h"

#include "parse.h"
#include "../SceneObjects/trimesh.h"
#include "../scene/light.h"
#include "../scene/material.h"

Scene *readHeights(char* fn) {
	int width, height;
	unsigned char *height_map;
	height_map = readBMP(fn, width, height);
	if (!height_map)
	{
		fl_alert("Error loading height map\n");
		return false;
	}

	Scene * ret = new Scene();
	//TODO: customize mat
	Material * mat = new Material();
	mat->kd = vec3f(1.0, 1.0, 1.0);
	//extract the points
	Trimesh * trimesh = new Trimesh(ret, mat, &ret->transformRoot);

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			int pos = y * width + x;
			unsigned char pixel[3];
			memcpy(pixel, height_map + pos * 3, 3);
			double height = double(pixel[0] + pixel[1] + pixel[2]) / 3 / 128;
			vec3f point(x, y, height);
			trimesh->addVertex(point);
			if (x > 0 && y > 0) { //link the points
				trimesh->addFace(pos, pos - 1, pos - 1 - width);
				trimesh->addFace(pos, pos - 1 - width, pos - width);
			}
		}
	}
	
	char *error;
	if (error = trimesh->doubleCheck())
		throw ParseError(error);

	//add a trimesh
	ret->add(trimesh);

	//add a pointlight
	PointLight* point_light = new PointLight(ret, vec3f(width, height, 10), vec3f(1.0, 1.0, 1.0));
	ret->add(point_light);

	//set the camerea
	//TODO: calculate the correct viewing distance;
	vec3f map_center((double)width / 2 - 0.5, (double)height / 2 - 0.5, 0.5);
	double camera_distance = (double)width + 3.0;
	vec3f camera_pos(0, -camera_distance, 2 * camera_distance);
	camera_pos += map_center;
	ret->getCamera()->setEye(camera_pos);
	ret->getCamera()->setLook((map_center - camera_pos).normalize(), vec3f(0, 0, 1).normalize());

	return ret;
}