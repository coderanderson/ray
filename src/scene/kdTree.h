#pragma once

// Note: you can put kd-tree here
#ifndef __KDTREE_H__
#define __KDTREE_H__

#include <iostream>

#include "scene.h"
#include "bbox.h"
#include "ray.h"
#include "../ui/TraceUI.h"

extern TraceUI* traceUI;

using std::vector;
using namespace std;


template <typename Obj> class KdTree {

public:
	KdTree<Obj>* left, *right;

private:
	vector<Obj> nodeObjects;
	BoundingBox bbox;
	int depth;


public:
	void buildKdTree(vector<Obj>& objects);
	bool intersect(ray& r, isect&i);
	

private:
	void buildKdTreeHelper(vector<Obj>& objects, int depth);
	void intersectHelper(ray& r, isect&i, bool& hasIntersect);
};



template<typename Obj> 
void KdTree<Obj>::buildKdTree(vector<Obj>& objects) {
	buildKdTreeHelper(objects, 0);
}

template<typename Obj>
void KdTree<Obj>::buildKdTreeHelper(vector<Obj>& objects, int depth) {
	cout << "depth: " << depth << " " << objects.size() << " objects remaining " << endl;
	if(objects.size() == 0) return; // stop growing 

	this->bbox = objects[0]->getBoundingBox();
	for(auto object : objects) {
		this->bbox.merge(object->getBoundingBox());
	}

	int leafSize = traceUI->getLeafSize(), maxDepth = traceUI->getMaxDepth();
	if(objects.size() <= leafSize || depth >= maxDepth) {
		this->nodeObjects.assign(objects.begin(), objects.end());
		return;
	}

	glm::dvec3 distance = this->bbox.getMax() - this->bbox.getMin();
	int maxDistance = 0, splitDim = 0;
	for(int i = 0; i <= 2; i++) {
		if(distance[i] > maxDistance) {
			maxDistance = distance[i];
			splitDim = i;
		}
	}

	double splitPoint = 0.0;
	for(int i = 0; i < objects.size(); i++) {
		BoundingBox objBounds = objects[i]->getBoundingBox();
		splitPoint += (objBounds.getCenter()[splitDim]) / (1.0 * objects.size());
	}

	// cout << "split point: " << splitPoint << endl;

	vector<Obj> leftObjects, rightObjects;
	for(int i = 0; i < objects.size(); i++) {
		if(objects[i]->getBoundingBox().getCenter()[splitDim] <= splitPoint) {
			leftObjects.push_back(objects[i]);
		}
		else {
			rightObjects.push_back(objects[i]);
		}
	}

	if(leftObjects.empty()) {
		left = nullptr;
	}
	else {
		left = new KdTree<Obj>();
		left->buildKdTreeHelper(leftObjects, depth + 1);
	}

	if(rightObjects.empty()) {
		right = nullptr;
	}
	else {
		right = new KdTree<Obj>();
		right->buildKdTreeHelper(rightObjects, depth + 1);
	}
}



template<typename Obj>
bool KdTree<Obj>::intersect(ray& r, isect&i) {
	bool hasIntersect = false;
	intersectHelper(r, i, hasIntersect);
	return hasIntersect;
}



template<typename Obj>
void KdTree<Obj>::intersectHelper(ray& r, isect& intersection, bool& hasIntersect) {
	double tMin = 0, tMax = 0;
	// cout << "bbox intersect" << endl;
	if(!bbox.intersect(r, tMin, tMax)) {
		return;
	}
	// cout << "after bbox intersect" << endl;

	if(left == nullptr && right == nullptr) {
		// cout << "search leaf node" << endl;
		for(int i = 0; i < nodeObjects.size(); i++) {
			// cout << "leaf for loop " << i << endl;
			isect tmpIsect;
			if(nodeObjects[i]->intersect(r, tmpIsect)) {
				if(tmpIsect.getT() < 0) continue;
				if(!hasIntersect || tmpIsect.getT() < intersection.getT()) {
					hasIntersect = true;
					intersection = tmpIsect;
				}
			}
		}
		return;
		// cout << "end leaf for loop " << endl;
	}

	if(left != nullptr) {
		left->intersectHelper(r, intersection, hasIntersect);
	}
	if(right != nullptr) {
		right->intersectHelper(r, intersection, hasIntersect);
	}
	
	
}






#endif //__KDTREE_H__