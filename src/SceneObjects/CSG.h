#ifndef __CSG_H__
#define __CSG_H__

#include "../scene/scene.h"

enum CSG_RELATION {
	CSG_OR = 1,
	CSG_AND,
	CSG_MINUS
};

class CSGTree {
public:
	CSGTree() :root(NULL) {}
private:
	CSGNode* root;
};

// data structure for CSG geometry, containing a tree structure
// The style is like Cylinder.h
class CSG
	: public MaterialSceneObject
{
public:
	CSG(Scene *scene, Material *mat)
		: MaterialSceneObject(scene, mat)
	{
	}

	virtual bool intersectLocal(const ray& r, isect& i) const;
	virtual bool hasBoundingBoxCapability() const { return true; }
	virtual bool hasInterior() const{ return true; }
	virtual BoundingBox ComputeLocalBoundingBox() {};

private:
	CSGTree* tree;
};
#endif