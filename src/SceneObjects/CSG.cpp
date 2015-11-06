#include <cmath>
#include <assert.h>
#include <cfloat>

#include "CSG.h"

typedef vector<SegmentPoint>::iterator SegIter;

Segments& Segments::Merge(const Segments& pSegments, int relation){
	vector<SegmentPoint> interval;

	for (SegIter it = mPoints.begin(); it != mPoints.end(); ++it)
	{
		it->contri = 1;
		interval.push_back(*it);
	}

	vector<SegmentPoint> pPoints(pSegments.mPoints);
	for (SegIter it = pPoints.begin(); it != pPoints.end(); ++it)
	{
		it->contri = (relation == CSG_MINUS) ? -1 : 1;
		interval.push_back(*it);
	}

	int before = 0, after = 0;
	int require = (relation == CSG_AND) ? 2 : 1;
	sort(interval.begin(), interval.end());
	mPoints.clear();
	
	for (SegIter it = interval.begin(); it != interval.end(); ++it)
	{
		if (it->isRight)
		{
			after -= it->contri;
		}
		else
		{
			after += it->contri;
		}

		if (before < require && after >= require){
			it->isRight = false;
			mPoints.push_back(*it);
		}
		else if (before >= require&&after < require){
			it->isRight = true;
			mPoints.push_back(*it);
		}
		before = after;
	}
	return *this;
}

CSGTree* CSGTree::merge(const CSGTree* pTree, CSG_RELATION relation){
	CSGNode* temp = new CSGNode;
	temp->lchild = root;
	temp->rchild = pTree->root;
	temp->isLeaf = false;
	temp->relation = relation;
	temp->computeBoundingBox();
	root = temp;
	return this;
}

bool Segments::firstPositive(SegmentPoint& p)
{
	bool hasOne = false;
	for (SegIter it = mPoints.begin(); it != mPoints.end(); ++it)
	{
		if (it->t > RAY_EPSILON)
		{
			if (hasOne)
			{
				if (it->t < p.t) p = *it;
			}
			else
			{
				hasOne = true;
				p = (*it);
			}
		}
	}
	return hasOne;
}

bool CSGTree::intersect(const ray& r, isect& i) const 
{
	if (!root) return false;
	Segments* inters = root->intersectLocal(r);
	SegmentPoint sp;
	if (!inters->firstPositive(sp)) return false;
	i.t = sp.t;
	if (sp.isRight)
	{
		i.N = (sp.normal * r.getDirection() > RAY_EPSILON) ? sp.normal : -sp.normal;
	}
	else
	{
		i.N = (sp.normal * r.getDirection() > RAY_EPSILON) ? -sp.normal : sp.normal;
	}
	return true;
}

Segments* CSGNode::intersectLocal(const ray& r) const
{
	Segments* result = new Segments();
	if (isLeaf)
	{
		SegmentPoint pNear, pFar;
		isect i;
		ray backR(r.at(-10000), r.getDirection());
		if (!object->intersect(backR, i)) return result;
		pNear.t = i.t - 10000;
		pNear.normal = i.N;
		pNear.isRight = false;
		ray contiR(r.at(pNear.t + RAY_EPSILON * 10), r.getDirection());
		if (!object->intersect(contiR, i))
		{
			pFar = pNear;
		}
		else 
		{
			pFar.t = i.t + pNear.t;
			pFar.normal = i.N;
		}
		pFar.isRight = true;
		result->addPoint(pNear);
		result->addPoint(pFar);
		return result;
	}
	else
	{
		if (!lchild || !rchild) return result;
		Segments* leftSeg = new Segments();
		Segments* rightSeg = new Segments();
		leftSeg = lchild->intersectLocal(r);
		rightSeg = rchild->intersectLocal(r);
		leftSeg->Merge(*rightSeg, relation);
		return leftSeg;
	}
}

BoundingBox CSGNode::getBoundingBox() const 
{
	return bound;
}

void CSGNode::computeBoundingBox()
{
	if (isLeaf)
	{
		bound = object->getBoundingBox();
	}
	else
	{
		if (!lchild || !rchild)
		{
			bound = BoundingBox();
		}
		bound = lchild->getBoundingBox();
		bound.plus(rchild->getBoundingBox());
	}
}

bool CSG::intersectLocal(const ray& r, isect& i) const
{
	if (!tree->intersect(r, i)) return false;
	i.obj = this;
	return true;
}

BoundingBox CSG::ComputeLocalBoundingBox(){
	CSGNode* rt = tree->getRoot();
	if (!rt) return BoundingBox();
	return rt->getBoundingBox();
}