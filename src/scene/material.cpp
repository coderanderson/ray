#include "material.h"
#include "../ui/TraceUI.h"
#include "light.h"
#include "ray.h"
extern TraceUI* traceUI;

#include <glm/gtx/io.hpp>
#include <iostream>
#include "../fileio/images.h"

using namespace std;
extern bool debugMode;

Material::~Material()
{
}

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
glm::dvec3 Material::shade(Scene* scene, const ray& r, const isect& i) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
	// You will need to call both distanceAttenuation() and
	// shadowAttenuation()
	// somewhere in your code in order to compute shadows and light falloff.
	//	if( debugMode )
	//		std::cout << "Debugging Phong code..." << std::endl;

	// When you're iterating through the lights,
	// you'll want to use code that looks something
	// like this:
	//
	// for ( const auto& pLight : scene->getAllLights() )
	// {
	//              // pLight has type unique_ptr<Light>
	// 		.
	// 		.
	// 		.
	// }
	glm::dvec3 diffuse = glm::dvec3(0, 0, 0);
	glm::dvec3 specular = glm::dvec3(0, 0, 0);
	glm::dvec3 ambient = glm::dvec3(0, 0, 0);
	for(const auto& pLight: scene->getAllLights()) {
		glm::dvec3 lightDirect = pLight->getDirection(r.at(i));
		glm::dvec3 normal = i.getN();
		glm::dvec3 lightColor = pLight->getColor() * pLight->distanceAttenuation(r.at(i));
		lightColor *= pLight->shadowAttenuation(r, r.at(i));
		double coefficientD = glm::dot(lightDirect, normal);
		if(coefficientD > 0) {
			diffuse += coefficientD * lightColor * kd(i);
		} else {
			diffuse += (-coefficientD) * lightColor * kd(i);
		}

		glm::dvec3 omegaIn = -1.0 * lightDirect;
		glm::dvec3 omegaNormal = glm::dot(omegaIn, normal) * normal;
		glm::dvec3 omegaRef = omegaIn - 2 * glm::dot(omegaIn, normal) * normal;
		double coefficientS = glm::dot(-r.getDirection(), omegaRef);
		
		if(coefficientS > 0) {
			specular += pow(coefficientS, shininess(i)) * lightColor * ks(i);
		} 

		ambient += ka(i) * scene->ambient();
	}
	return diffuse + specular + ambient + ke(i);
}

TextureMap::TextureMap(string filename)
{
	data = readImage(filename.c_str(), width, height);
	if (data.empty()) {
		width = 0;
		height = 0;
		string error("Unable to load texture map '");
		error.append(filename);
		error.append("'.");
		throw TextureMapException(error);
	}
}

glm::dvec3 TextureMap::getMappedValue(const glm::dvec2& coord) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.
	// What this function should do is convert from
	// parametric space which is the unit square
	// [0, 1] x [0, 1] in 2-space to bitmap coordinates,
	// and use these to perform bilinear interpolation
	// of the values.

	double xReal = coord[0] * width, yReal = coord[1] * height;
	int x0 = int(xReal), y0 = int(yReal);
	double dx = xReal - x0, dy = yReal - y0, omdx = 1 - dx, omdy = 1 - dy;
	// cout << "texture mapping x0 = " << x0 << "y0 = " << y0 << "dx = " << dx << " dy = " << dy << endl;
	glm::dvec3 bilinear = omdx * omdy * getPixelAt(x0, y0) +
						omdx * dy * getPixelAt(x0, y0+1) +
						dx * omdy * getPixelAt(x0+1, y0) +
						dx * dy * getPixelAt(x0+1, y0+1);
	// cout << "texture mapping result: " << bilinear << endl;
	return bilinear;
	// return glm::dvec3(1, 1, 1);
	
}

glm::dvec3 TextureMap::getPixelAt(int x, int y) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.

	if(data.empty()) {
		//cout << "don't have a texture map here" << endl;
		return glm::dvec3(1, 1, 1);
	}
	// cout << "texture map size: " << "width = " << width << "height = " << height << endl;
	x = std::min(x, width - 1);
	y = std::min(y, height - 1);

	int idx = (y * width + x) * 3;
	// cout << "texture mapping, data length: " << data.size() << endl;
	// cout << "idx: " << idx << endl;
	return glm::dvec3(data[idx] / 255.0, data[idx + 1] / 255.0, data[idx + 2] / 255.0);
}

glm::dvec3 MaterialParameter::value(const isect& is) const
{
	if (0 != _textureMap)
		return _textureMap->getMappedValue(is.getUVCoordinates());
	else
		return _value;
}

double MaterialParameter::intensityValue(const isect& is) const
{
	if (0 != _textureMap) {
		glm::dvec3 value(
		        _textureMap->getMappedValue(is.getUVCoordinates()));
		return (0.299 * value[0]) + (0.587 * value[1]) +
		       (0.114 * value[2]);
	} else
		return (0.299 * _value[0]) + (0.587 * _value[1]) +
		       (0.114 * _value[2]);
}
