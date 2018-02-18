#include <vector>
#include <stack>



using namespace std;


template<typename Obj> 
KdTree<Obj>* KdTree<Obj>::buildKdTree(vector<obj>& objects) {
	return buildKdTreeHelper(objects, 0);
}

template<typename Obj>
KdTree<Obj>* KdTree<Obj>::buildKdTreeHelper(vector<obj>& objects, int depth) {
	if(objects.size() == 0) return; // stop growing 

	for(int i = 0; i < objects.size(); i++) {
		objects[i]->ComputeBoundingBox();
	}
	this->bbox = objects[0]->getBoundingBox();
	for(auto object : objects) {
		this->bbox.merge(object->getBoundingBox());
	}

	int leafSize = traceUI->getLeafSize(), maxDepth = traceUI->getMaxDepth();
	if(objects.size() <= leafSize || depth >= maxDepth) {
		this->nodeObjects.assign(objects.begin(), objects.end());
		return;
	}

	int splitT = 0, splitDim = depth % 3;
	for(int i = 0; i < objects.size(); i++) {
		BoundingBox objBounds = objects[i]->getBoundingBox();
		splitT += (objBounds.getCenter()[splitDim]) / double(2 * objects.size());
	}

	vector<Obj> leftObjects, rightObjects;
	for(int i = 0; i < objects.size(); i++) {
		if(objects[i]->getBoundingBox().getCenter()[splitDim] <= spiltT) {
			leftObjects.push_back(objecs[i]);
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
bool KdTree<Obj>::intersect(const ray& r, isect& intersection) {
	double tMin = 0, tMax = 0;
	if(!bbox.intersect(r, tMin, tMax)) {
		return false;
	}

	if(left == nullptr && right == nullptr) {
		bool hasOne = false;
		for(int i = 0; i = nodeObjects.size(); j++) {
			isect tmpIsect;
			if(nodeObjects[i].intersect(r, tmpIsect)) {
				if(!hasOne || tmpIsect.getT() < intersection.getT()) {
					hasOne = true;
					intersection = tmpIsect;
				}
			}
		}
		return hasOne;
	}
	
	if(left != nullptr) {
		if(left->intersect(r, intersection)) {
			return true;
		}
	}
	if(right != nullptr) {
		if(right->intersect(r, intersection)) {
			return true;
		}
	}
	return false;
}








































































