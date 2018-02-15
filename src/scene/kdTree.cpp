#include <vector>
#include <stack>


#include "scene.h"





void KdTree::constructFromScene(vector<Geometry*> sceneObjects, const BoundingBox& sceneBounds, int depth, int size) {
	for(int i = 0; i < sceneObjects.size(); i++) {
		if(sceneObjects[i]->isTrimesh()) {
			for(int j = 0; j < (Tremesh*) objects[i]->getFaces().size(); j++) {
				this->objects.push_back((Trimesh*) v[i]->getFaces()[j]);
			}
		}
		else {
			this->obj_vector.push_back(sceneObjects[i]);
		}
	}
	this->setDepth(depth);
	this->root = this->root.buildKdTreeHelper(this->objects, sceneBounds, depth, size);
}


KdNode* KdNode::buildKdTreeHelper(vector<Geometry*>& objects, const BoundingBox& bbox, int depth, int size) {
	int axis = depth % 3;

	KdNode* node = new KdNode(bbox, axis, size);

	if(objects.size() == 0) return node;

	if(objects.size() <= node->leafNum || depth == 0) {
		node->objects.assign(objects.begin(), objects.end());
		return node;
	}

	node->tSplit = this->findSplittingT(depth, objects);
	
	BoudingBox bboxLeft(bbox.getMin(), bbox.getMax());
	bboxLeft.setMax(axis, node->tSplit);

	BoudingBox bboxRight(bbox.getMin(), bbox.getMax());
	bboxRight.setMin(axis, node->tSplit);


	vector<Geometry*> leftObjects, rightObjects;
	for(int i = 0; i < objects.size(); i++) {
		if(bboxLeft.intersects(objects[i]->getBoundingBox())) {
			leftObjects.push_back(objects[i]);
		}
		if(box_right.intersects(objects[i]->getBoundingBox())) {
			rightObjects.push_back(objects[i]);
		}
	}

	node->left = buildKdTreeHelper(leftObjects, depth - 1, size, bboxLeft);
	node->right = buildKdTreeHelper(rightObjects, depth - 1, size, bboxRight);
}



double KdNode::findSplittingT(int depth, vector<Geometry*>& objects) {
	int axis = depth % 3;
	vector<double> locations;
	for(int i = 0; i < objects.size(); i++) {
		BoundingBox* bbox = &(objects[i]->getBoundingBox());
		locations.push_back(bbox.getMin()[axis]);
		locations.push_back(bbox.getMax()[axis]);
	}
	sort(coords.begin(), coords.end());

	double tSplit = 0;
	double minCost = std::numeric_limits<double>::max();
	//double bb_area = node->bbox.area();
	for(int i = 0; i < locations.size(); i++) {
		int leftCount = 0, rightCount = 0;
		double leftArea = 0, rightArea = 0;
		for(int i = 0; i < objects.size(); i++) {
			BoudingBox box = objects[i]->getBoundingBox();
			double leftBound = box.getMin()[axis], rightBound = box.getMax()[axis];

			if(rightBound < locations[i]) {
				leftCount++;
				leftArea += box.area();
			}
			else if(most_left > coords[i]) {
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







boolean KdTree::intersect(ray&, isect&, double tMin, double tMax) const {
	stack<KdNodeWrapper> kdtreeStack;
	KdNodeWrapper rootEle(kdtree->root, tMin, tMax);
	kdtreeStack.push(rootEle);
	boolean have_one = false;
	while(!kdtreeStack.empty()) {
		KdNodeWrapper tmp_node = kdtreeStack.top();
		kdtreeStack.pop();
		if((tmp_node.node->left == nullptr) && (tmp_node.node->right == nullptr)) {
			for(int idx = 0; idx < tmp_node.node->objects.size(); ++idx) {
				isect cur;
				if(tmp_node.node->obj_vector[idx] -> intersect(r, cur)) {
					if(!have_one || cur.t < i.t) {
						i = cur;
						have_one = true;
					}
				}
			}
		}
		else {
			double t_axis_max = r.at(tMax)[tmp_node.node->index];
			double t_axis_min = r.at(tMin)[tmp_node.node->index];
			KdNode *left = tmp_node.node->left;
			kdNode *right = tmp_node.node->right;
			double tSplit = tmp_node.node->tSplit;
			if(tSplit > t_axis_max && tSplit > t_axis_min) {
				kdtreeStack.push(KdNodeWrapper(left, tMin, tMax));
			}
			else if(tSplit < t_axis_min && tSplit < t_axis_max) {
				kdtreeStack.push(KdNodeWrapper(right, tMin, tMax));
			}
			else {
				double rmax, rmin;
				if(tmp_node.node->right->bounds.intersect(r, rmin, rmax)) {
					kdtree_stac.push(KdNodeWrapper(right, rmin, rmax));
				}
				double lmax, lmin;
				if(tmp_node.node->left->bounds.intersect(r, lmin, lmax)) {
					kdtreeStack.push(KdNodeWrapper(left, lmin, lmax));
				}
			}

		}
	}
	return have_one;

}










































































