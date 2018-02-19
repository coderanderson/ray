#include "cubeMap.h"
#include "ray.h"
#include "../ui/TraceUI.h"
#include "../scene/material.h"
extern TraceUI* traceUI;


/* algorithm from wiki */

glm::dvec3 CubeMap::getColor(ray r) const
{
	// YOUR CODE HERE
	// FIXME: Implement Cube Map here
	int axis, mapIdx;
	double u,v;
	glm::dvec3 direct = r.getDirection();
	
	if (fabs(direct[0]) > fabs(direct[1])) {
		if (fabs(direct[0]) > fabs(direct[2])) {
			axis = 0;
			if (direct[0] > 0.0) {
				mapIdx = 0;
			}
			else {
				mapIdx = 1;
			}
		}
		else {
			axis = 2;
			if (direct[2] > 0.0) {
				mapIdx = 5;
			}
			else {
				mapIdx = 4;
			}
		}
	}
	else {
		if (fabs(direct[1]) > fabs(direct[2])) {
			axis = 1;
			if (direct[1] > 0.0) {
				mapIdx = 2;
			}
			else {
				mapIdx = 3;
			}
		}
		else {
			axis = 2;
			if (direct[2] > 0.0) {
				mapIdx = 5;
			}
			else {
				mapIdx = 4;
			}
		}
	}

	if (axis == 0) {
		u = direct[2]/direct[0];
		if (direct[0] > 0.0) v = direct[1]/direct[0];
		else v = -direct[1]/direct[0];
	}
	else if (axis == 1) {
		if (direct[1] > 0.0) u = direct[0]/direct[1];
		else u = -direct[0]/direct[1];
		v = direct[2]/direct[1];
	}
	else if (axis == 2) {
		u = -direct[0]/direct[2];
		if (direct[2] > 0.0) v = direct[1]/direct[2];
		else v = -direct[1]/direct[2];
	}

	u = (u + 1.0)/2.0;
	v = (v + 1.0)/2.0;
	return tMap[mapIdx]->getMappedValue(glm::dvec2(u, v));
}

CubeMap::CubeMap()
{
}

CubeMap::~CubeMap()
{
}

void CubeMap::setNthMap(int n, TextureMap* m)
{
	if (m != tMap[n].get())
		tMap[n].reset(m);
}
