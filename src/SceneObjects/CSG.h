#ifndef __CSG_H__
#define __CSG_H__

#include "../scene/scene.h"

enum CSG_RELATION {
	CSG_OR = 1,
	CSG_AND,
	CSG_MINUS
};

struct SegmentPoint{
	double t;
	vec3f normal;
	bool isRight;
	int contri;
};

class Segments{
private:
	vector<SegmentPoint> points;
public:
	Segments& Merge(const Segments& another, int relation);	
	SegmentPoint firstPositive();
};

class CSGNode 
{
public:
	CSGNode() : lchild(NULL), rchild(NULL), object(NULL), relation(CSG_AND), isLeaf(0) {}
	void setObject(Geometry* obj)
	{
		this->object = obj;
	}
	void setIsLeaf(bool p)
	{
		this->isLeaf = p;
	}
	CSGNode* lchild;
	CSGNode* rchild;
	Geometry* object;
	CSG_RELATION relation;
	bool isLeaf;
};

class CSGTree {
public:
	CSGTree() :root(NULL) {}
	CSGTree(CSGNode* nodes) : root(nodes) {}
	CSGTree* merge(const CSGTree* pTree, CSG_RELATION relation);
	bool intersect(const ray& r, isect& i) const;
	CSGNode* getRoot() { return root; }
private:
	Segments* intersectLocal(const ray& r) const;
	CSGNode* root;
};

// data structure for CSG geometry, containing a tree structure
// The style is like Cylinder.h
class CSG
	: public MaterialSceneObject
{
public:
	CSG(Scene *scene, Material *mat, CSGTree* tr)
		: MaterialSceneObject(scene, mat), tree(tr)
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