#pragma once

// Note: you can put kd-tree here
#ifndef __KDTREE_H__
#define __KDTREE_H__

#include <iostream>


#include "bbox.h"
#include "ray.h"
#include "../ui/TraceUI.h"

extern TraceUI* traceUI;

using std::vector;


template <typename Obj> class KdTree {

public:
	KdTree<Obj>* left, *right;

private:
	vector<Obj> nodeObjects;
	BoundingBox bbox;
	int depth;


public:
	KdTree* buildKdTree(vector<Obj>& objects);
	bool intersect(ray& r, isect&i);

private:
	KdTree* buildKdTreeHelper(vector<Obj>& objects, int depth);
};





#endif //__KDTREE_H__