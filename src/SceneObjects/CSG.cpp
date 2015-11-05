#include <cmath>
#include <assert.h>
#include <cfloat>

#include "CSG.h"

Segments& Segments::Merge(const Segments& another, int relation){
	return *this;
}

CSGTree* CSGTree::merge(const CSGTree* pTree, CSG_RELATION relation){
	CSGNode* temp = new CSGNode;
	temp->lchild = root;
	temp->rchild = pTree->root;
	temp->isLeaf = false;
	temp->relation = relation;
	return this;
}

SegmentPoint Segments::firstPositive()
{
	vector<SegmentPoint>::iterator j;
	for (j = points.begin(); j != points.end() ++j);
	return *j;
}

bool CSGTree::intersect(const ray& r, isect& i) const {
	Segments* inters = intersectLocal(r);
	return true;
}

Segments* CSGTree::intersectLocal(const ray& r) const {
	return new Segments();
}

bool CSG::intersectLocal(const ray& r, isect& i) const
{
	return false;
}

BoundingBox CSG::ComputeLocalBoundingBox(){
	BoundingBox localbounds;
	localbounds.max = vec3f(0.5, 0.5, 0.5);
	localbounds.min = vec3f(-0.5, -0.5, -0.5);
	return localbounds;
}