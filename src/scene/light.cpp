#include <cmath>
#include <iostream>

#include "light.h"
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>


using namespace std;

double DirectionalLight::distanceAttenuation(const glm::dvec3& P) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


glm::dvec3 DirectionalLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.
	isect i;
  	ray shadow_ray(p, -orientation, r.getAtten(), ray::SHADOW);
  	if (this->getScene()->intersect(shadow_ray, i)) {
      	const Material& m = i.getMaterial();
      	return m.kt(i);
  	}
	return glm::dvec3(1.0, 1.0, 1.0);
}

glm::dvec3 DirectionalLight::getColor() const
{
	return color;
}

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3& P) const
{
	return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3& P) const
{

	// YOUR CODE HERE
	double distance = glm::length(position - P);
	double attenuation = constantTerm + linearTerm * distance + quadraticTerm * distance * distance;
	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, we assume no attenuation and just return 1.0
	return attenuation < 1.0 ? attenuation : 1.0;
}

glm::dvec3 PointLight::getColor() const
{
	return color;
}

glm::dvec3 PointLight::getDirection(const glm::dvec3& P) const
{
	return glm::normalize(position - P);
}


glm::dvec3 PointLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.
	isect i;
  	ray shadow_ray(p, glm::normalize(position - p), r.getAtten(), ray::SHADOW);
  	if (this->getScene()->intersect(shadow_ray, i)) {
  		if ( i.getT() < glm::length(p - position)) {
	      	const Material& m = i.getMaterial();
	      	return m.kt(i);
	    }
  	}
	return glm::dvec3(1.0, 1.0, 1.0);
}

#define VERBOSE 0

