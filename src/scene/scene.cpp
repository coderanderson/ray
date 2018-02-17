#include <cmath>
#include <iostream>

#include "scene.h"
#include "light.h"
#include "kdTree.h"
#include "../ui/TraceUI.h"
#include <glm/gtx/extended_min_max.hpp>
#include <iostream>
#include <glm/gtx/io.hpp>
#include <stack>

#include "../SceneObjects/trimesh.h"

using namespace std;
using std::unique_ptr;
using std::stack;
using std::vector;

bool Geometry::intersect(ray& r, isect& i) const {
	double tmin, tmax;
	if (hasBoundingBoxCapability() && !(bounds.intersect(r, tmin, tmax))) return false;
	// Transform the ray into the object's local coordinate space
	glm::dvec3 pos = transform->globalToLocalCoords(r.getPosition());
	glm::dvec3 dir = transform->globalToLocalCoords(r.getPosition() + r.getDirection()) - pos;
	double length = glm::length(dir);
	dir = glm::normalize(dir);
	// Backup World pos/dir, and switch to local pos/dir
	glm::dvec3 Wpos = r.getPosition();
	glm::dvec3 Wdir = r.getDirection();
	r.setPosition(pos);
	r.setDirection(dir);
	bool rtrn = false;
	if (intersectLocal(r, i))
	{
		// Transform the intersection point & normal returned back into global space.
		i.setN(transform->localToGlobalCoordsNormal(i.getN()));
		i.setT(i.getT()/length);
		rtrn = true;
	}
	// Restore World pos/dir
	r.setPosition(Wpos);
	r.setDirection(Wdir);
	return rtrn;
}

bool Geometry::hasBoundingBoxCapability() const {
	// by default, primitives do not have to specify a bounding box.
	// If this method returns true for a primitive, then either the ComputeBoundingBox() or
    // the ComputeLocalBoundingBox() method must be implemented.

	// If no bounding box capability is supported for an object, that object will
	// be checked against every single ray drawn.  This should be avoided whenever possible,
	// but this possibility exists so that new primitives will not have to have bounding
	// boxes implemented for them.
	return false;
}

void Geometry::ComputeBoundingBox() {
    // take the object's local bounding box, transform all 8 points on it,
    // and use those to find a new bounding box.

    BoundingBox localBounds = ComputeLocalBoundingBox();
        
    glm::dvec3 min = localBounds.getMin();
    glm::dvec3 max = localBounds.getMax();

    glm::dvec4 v, newMax, newMin;

    v = transform->localToGlobalCoords( glm::dvec4(min[0], min[1], min[2], 1) );
    newMax = v;
    newMin = v;
    v = transform->localToGlobalCoords( glm::dvec4(max[0], min[1], min[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = transform->localToGlobalCoords( glm::dvec4(min[0], max[1], min[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = transform->localToGlobalCoords( glm::dvec4(max[0], max[1], min[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = transform->localToGlobalCoords( glm::dvec4(min[0], min[1], max[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = transform->localToGlobalCoords( glm::dvec4(max[0], min[1], max[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = transform->localToGlobalCoords( glm::dvec4(min[0], max[1], max[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = transform->localToGlobalCoords( glm::dvec4(max[0], max[1], max[2], 1) );
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
		
    bounds.setMax(glm::dvec3(newMax));
    bounds.setMin(glm::dvec3(newMin));
}

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::add(Geometry* obj) {
	obj->ComputeBoundingBox();
	sceneBounds.merge(obj->getBoundingBox());
	objects.emplace_back(obj);
}

void Scene::add(Light* light)
{
	lights.emplace_back(light);
}


// Get any intersection with an object.  Return information about the 
// intersection through the reference parameter.
bool Scene::intersect(ray& r, isect& i) const {
	// double tMin = 0.0, tMax = 0.0;
	// bool haveOne = this->intersectKdTree(r, i, tMin, tMax);
	// if(!haveOne)
	// 	i.setT(1000.0);
	// // if debugging,
	// if (TraceUI::m_debug)
	// 	intersectCache.push_back(std::make_pair(new ray(r), new isect(i)));
	// return haveOne;



	double tmin = 0.0;
	double tmax = 0.0;
	bool have_one = false;
	for(const auto& obj : objects) {
		isect cur;
		if( obj->intersect(r, cur) ) {
			if(!have_one || (cur.getT() < i.getT())) {
				i = cur;
				have_one = true;
			}
		}
	}
	if(!have_one)
		i.setT(1000.0);
	// if debugging,
	if (TraceUI::m_debug)
		intersectCache.push_back(std::make_pair(new ray(r), new isect(i)));
	return have_one;
}

TextureMap* Scene::getTexture(string name) {
	auto itr = textureCache.find(name);
	if (itr == textureCache.end()) {
		textureCache[name].reset(new TextureMap(name));
		return textureCache[name].get();
	}
	return itr->second.get();
}


// move this method to scene.h
void Scene::buildKdTree(int depth, int size) {
	// this->kdtree = new KdTree<unique_ptr<Geometry>>();
	this->kdtree = new KdTree<Geometry*>();
	vector<Geometry*> kdObjects;
	for(int i = 0; i < this->objects.size(); i++) {
		kdObjects.push_back(this->objects[i].get());
	}
	this->kdtree->constructFromScene(kdObjects, this->sceneBounds, depth, size);
}

bool Scene::intersectKdTree(ray& r, isect& i, double tMin, double tMax) const {
	return this->kdtree->intersect(r, i, tMin, tMax);
}

template<typename Obj>
void KdTree<Obj>::constructFromScene(vector<Obj> objects, BoundingBox sceneBounds, int depth, int size) {
	for(int i = 0; i < objects.size(); i++) {
		if(objects[i]->isTrimesh()) {
			//cout << "trimesh copying faces..." << endl; // this copy is quick enough. should be no problem.
			for(int j = 0; j < ((Trimesh*) objects[i])->getFaces().size(); j++) {
				this->objects.push_back( ((Trimesh*) objects[i])->getFaces()[j]);
			}
			//cout << "copy faces done" << endl;
		}
		else {
			this->objects.push_back(objects[i]);
		}
	}
	this->setDepth(depth);
	this->root = this->root->buildKdTreeHelper(this->objects, sceneBounds, depth, size);
}


template<typename Obj>
KdNode<Obj>* KdNode<Obj>::buildKdTreeHelper(vector<Obj>& objects, const BoundingBox& bbox, int depth, int size) {
	cout << "build kdtree level: " << depth
		<< ", leafSize: " << size 
		<< ", current objects number: " << objects.size() << endl;
	int axis = depth % 3;

	KdNode<Obj>* node = new KdNode<Obj>(bbox, axis, size);

	if(objects.size() == 0) return node;

	if(objects.size() <= node->leafSize || depth == 0) {
		node->objects.assign(objects.begin(), objects.end());
		return node;
	}

	node->tSplit = this->findSplittingT(depth, objects);
	cout << "find tsplit: " << node->tSplit << endl;
	
	BoundingBox bboxLeft(bbox.getMin(), bbox.getMax());
	bboxLeft.setMax(axis, node->tSplit);

	BoundingBox bboxRight(bbox.getMin(), bbox.getMax());
	bboxRight.setMin(axis, node->tSplit);


	vector<Obj> leftObjects, rightObjects;
	for(int i = 0; i < objects.size(); i++) {
		if(!objects[i]->hasBoundingBoxCapability()) {
			cout << "object " << i << " don't have bounding box!" << endl;
		}
		if(bboxRight.intersects(objects[i]->getBoundingBox())) {
			rightObjects.push_back(objects[i]);
		}
		if(bboxLeft.intersects(objects[i]->getBoundingBox())) {
			leftObjects.push_back(objects[i]);
		}
		
	}

	node->left = buildKdTreeHelper(leftObjects, bboxLeft, depth - 1, size);
	node->right = buildKdTreeHelper(rightObjects, bboxRight, depth - 1, size);
	return node;
}


template<typename Obj>
double KdNode<Obj>::findSplittingT(int depth, vector<Obj>& objects) {
	int axis = depth % 3;
	vector<double> locations;
	for(int i = 0; i < objects.size(); i++) {
		const BoundingBox& bbox = objects[i]->getBoundingBox();
		locations.push_back(bbox.getMin()[axis]);
		locations.push_back(bbox.getMax()[axis]);
	}
	// cout << "sort start" << endl; //quick enough
	sort(locations.begin(), locations.end());
	// cout << "sort end" << endl;

	double tSplit = 0;
	double minCost = std::numeric_limits<double>::max();
	//double bb_area = node->bbox.area();
	for(int i = 0; i < locations.size(); i++) {
		int leftCount = 0, rightCount = 0;
		double leftArea = 0, rightArea = 0;
		for(int objIdx = 0; objIdx < objects.size(); objIdx++) {
			BoundingBox box = objects[objIdx]->getBoundingBox();
			double leftBound = box.getMin()[axis], rightBound = box.getMax()[axis];

			if(rightBound < locations[i]) {
				leftCount++;
				leftArea += box.area();
			}
			else if(leftBound > locations[i]) {
				rightCount++;
				rightArea += box.area();
			}
			else {
				leftCount++;
				rightCount++;
				leftArea += box.area();
				rightArea += box.area();
			}
		}

		double cost = (rightArea * rightCount + leftArea * leftCount) / (leftArea + rightArea);
		if(cost < minCost) {
			minCost = cost;
			tSplit = locations[i];
		}
	}
	return tSplit;
}






template<typename Obj>
bool KdTree<Obj>::intersect(ray& r, isect& i, double tMin, double tMax) const {
	stack<KdNodeWrapper<Obj>> kdtreeStack;
	KdNodeWrapper<Obj> rootEle(this->root, tMin, tMax);
	kdtreeStack.push(rootEle);
	bool have_one = false;
	while(!kdtreeStack.empty()) {
		KdNodeWrapper<Obj> tmp_node = kdtreeStack.top();
		kdtreeStack.pop();
		if((tmp_node.node->left == nullptr) && (tmp_node.node->right == nullptr)) {
			for(int idx = 0; idx < tmp_node.node->objects.size(); ++idx) {
				isect cur;
				if(tmp_node.node->objects[idx] -> intersect(r, cur)) {
					if(!have_one || cur.getT() < i.getT()) {
						i = cur;
						have_one = true;
					}
				}
			}
		}
		else {
			double t_axis_max = r.at(tMax)[tmp_node.node->axis];
			double t_axis_min = r.at(tMin)[tmp_node.node->axis];
			KdNode<Obj> *left = tmp_node.node->left;
			KdNode<Obj> *right = tmp_node.node->right;
			double tSplit = tmp_node.node->tSplit;
			if(tSplit > t_axis_max && tSplit > t_axis_min) {
				kdtreeStack.push(KdNodeWrapper<Obj>(left, tMin, tMax));
			}
			else if(tSplit < t_axis_min && tSplit < t_axis_max) {
				kdtreeStack.push(KdNodeWrapper<Obj>(right, tMin, tMax));
			}
			else {
				double rmax, rmin;
				if(tmp_node.node->right->bbox.intersect(r, rmin, rmax)) {
					kdtreeStack.push(KdNodeWrapper<Obj>(right, rmin, rmax));
				}
				double lmax, lmin;
				if(tmp_node.node->left->bbox.intersect(r, lmin, lmax)) {
					kdtreeStack.push(KdNodeWrapper<Obj>(left, lmin, lmax));
				}
			}

		}
	}
	return have_one;

}

