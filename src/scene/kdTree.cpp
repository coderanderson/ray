#include <vector>
#include <stack>

#include "kdTree.h"

using namespace std;


// template<typename Obj> 
// KdTree<Obj>* KdTree<Obj>::buildKdTree(vector<Obj>& objects) {
// 	return buildKdTreeHelper(objects, 0);
// }

// template<typename Obj>
// KdTree<Obj>* KdTree<Obj>::buildKdTreeHelper(vector<Obj>& objects, int depth) {
// 	if(objects.size() == 0) return; // stop growing 

// 	for(int i = 0; i < objects.size(); i++) {
// 		objects[i]->ComputeBoundingBox();
// 	}
// 	this->bbox = objects[0]->getBoundingBox();
// 	for(auto object : objects) {
// 		this->bbox.merge(object->getBoundingBox());
// 	}

// 	int leafSize = traceUI->getLeafSize(), maxDepth = traceUI->getMaxDepth();
// 	if(objects.size() <= leafSize || depth >= maxDepth) {
// 		this->nodeObjects.assign(objects.begin(), objects.end());
// 		return;
// 	}

// 	glm::dvec3 distance = this->bbox.getMax() - this->bbox.getMin();
// 	int maxDistance = 0, splitDim = 0;
// 	for(int i = 0; i <= 2; i++) {
// 		if(distance[i] > maxDistance) {
// 			maxDistance = distance[i];
// 			splitDim = i;
// 		}
// 	}

// 	int splitPoint = 0;
// 	for(int i = 0; i < objects.size(); i++) {
// 		BoundingBox objBounds = objects[i]->getBoundingBox();
// 		splitPoint += (objBounds.getCenter()[splitDim]) / (1.0 * objects.size());
// 	}

// 	vector<Obj> leftObjects, rightObjects;
// 	for(int i = 0; i < objects.size(); i++) {
// 		if(objects[i]->getBoundingBox().getCenter()[splitDim] <= splitPoint) {
// 			leftObjects.push_back(objects[i]);
// 		}
// 		else {
// 			rightObjects.push_back(objects[i]);
// 		}
// 	}

// 	if(leftObjects.empty()) {
// 		left = nullptr;
// 	}
// 	else {
// 		left = new KdTree<Obj>();
// 		left->buildKdTreeHelper(leftObjects, depth + 1);
// 	}

// 	if(rightObjects.empty()) {
// 		right = nullptr;
// 	}
// 	else {
// 		right = new KdTree<Obj>();
// 		right->buildKdTreeHelper(rightObjects, depth + 1);
// 	}
// }


// template<typename Obj>
// bool KdTree<Obj>::intersect(const ray& r, isect& intersection) {
// 	double tMin = 0, tMax = 0;
// 	if(!bbox.intersect(r, tMin, tMax)) {
// 		return false;
// 	}

// 	if(left == nullptr && right == nullptr) {
// 		bool hasOne = false;
// 		for(int i = 0; i = nodeObjects.size(); i++) {
// 			isect tmpIsect;
// 			if(nodeObjects[i].intersect(r, tmpIsect)) {
// 				if(!hasOne || tmpIsect.getT() < intersection.getT()) {
// 					hasOne = true;
// 					intersection = tmpIsect;
// 				}
// 			}
// 		}
// 		return hasOne;
// 	}
	
// 	if(left != nullptr) {
// 		if(left->intersect(r, intersection)) {
// 			return true;
// 		}
// 	}
// 	if(right != nullptr) {
// 		if(right->intersect(r, intersection)) {
// 			return true;
// 		}
// 	}
// 	return false;
// }








































































