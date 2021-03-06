// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <string.h> // for memset

#include <iostream>
#include <fstream>
#include <omp.h>


using namespace std;
extern TraceUI* traceUI;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through pixel(i,j), i.e. normalized window coordinates (x,y),
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.

glm::dvec3 RayTracer::trace(double x, double y)
{
	// Clear out the ray cache in the scene for debugging purposes,
	if (TraceUI::m_debug)
		scene->intersectCache.clear();

	ray r(glm::dvec3(0,0,0), glm::dvec3(0,0,0), glm::dvec3(1,1,1), ray::VISIBILITY);
	scene->getCamera().rayThrough(x,y,r);
	double dummy = 0;
	glm::dvec3 ret = traceRay(r, glm::dvec3(1.0,1.0,1.0), traceUI->getDepth(), dummy);
	ret = glm::clamp(ret, 0.0, 1.0);
	return ret;
}

glm::dvec3 RayTracer::tracePixel(int i, int j)
{
	glm::dvec3 col(0,0,0);

	if( ! sceneLoaded() ) return col;
	//deal with anti-aliasing in here
	float aa4Matrix[4 * 2] = {
	    -1.0/4.0,  3.0/4.0,
	     3.0/4.0,  1.0/3.0,
	    -3.0/4.0, -1.0/4.0,
	     1.0/4.0, -3.0/4.0,
	};

	float aa9Matrix[9 * 2] = {
	    0, 0,
	    1.0, 0,
	    -1.0, 0,
	    0, -1.0,
	    0, 1.0,
	    0.707, 0.707,
	    0.707, -0.707,
	    -0.707, -0.707,
	    -0.707, 0.707
	};

	float aa16Matrix[16 * 2] = {
		1.0, 0, -1.0, 0,
	    0, -1.0, 0, 1.0,
	    0.707, 0.707, 0.707, -0.707,
	    -0.707, -0.707, -0.707, 0.707,
	    0.97, 0.25, 0.97, -0.25,
	    -0.97, -0.25, -0.97, 0.25,
	    0.25, 0.97, 0.25, -0.97,
	    -0.25, -0.97, -0.25, 0.97,
	};

	if (samples == 1) {
		double x = double(i)/double(buffer_width);
		double y = double(j)/double(buffer_height);
		col = trace(x, y);
	} else if (samples == 4) {
		for(int count = 0; count < samples; count++) {
			double x = double(i + aa4Matrix[2 * count])/double(buffer_width);
			double y = double(j + aa4Matrix[2 * count + 1])/double(buffer_height);
			col += trace(x, y);
		}
		col /= samples;
	} else if (samples == 9) {
		for(int count = 0; count < samples; count++) {
			double x = double(i + aa9Matrix[2 * count])/double(buffer_width);
			double y = double(j + aa9Matrix[2 * count + 1])/double(buffer_height);
			col += trace(x, y);
		}
		col /= samples;
	} else if (samples == 16) {
		for(int count = 0; count < samples; count++) {
			double x = double(i + aa16Matrix[2 * count])/double(buffer_width);
			double y = double(j + aa16Matrix[2 * count + 1])/double(buffer_height);
			col += trace(x, y);
		}
		col /= samples;
	} else {
		double x = double(i)/double(buffer_width);
		double y = double(j)/double(buffer_height);
		col = trace(x, y);
	}

	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
	return col;
}

#define VERBOSE 0

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
glm::dvec3 RayTracer::traceRay(ray& r, const glm::dvec3& thresh, int depth, double& t )
{
	isect i;
	glm::dvec3 colorC;
#if VERBOSE
	std::cerr << "== current depth: " << depth << std::endl;
#endif


	if(scene->intersect(r, i)) {
		// YOUR CODE HERE

		// An intersection occurred!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.

		const Material& m = i.getMaterial();
		colorC = m.shade(scene.get(), r, i);

		if(depth == 0) {
			return colorC;
		} 

		glm::dvec3 normal = i.getN();

		glm::dvec3 omegaIn = r.getDirection();
		glm::dvec3 omegaNormal = glm::dot(omegaIn, normal) * normal;
		glm::dvec3 omegaRef = omegaIn - 2 * glm::dot(omegaIn, normal) * normal;

		ray reflectedR(r.at(i), omegaRef, r.getAtten(), ray::REFLECTION);
		glm::dvec3 reflected = RayTracer::traceRay(reflectedR, thresh, depth - 1, t);
		
		colorC += m.kr(i) * reflected;

		glm::dvec3 v_d = r.getDirection();
		glm::dvec3 v_n = normal;
		v_n = glm::normalize(v_n);
		glm::dvec3 cos = v_n * glm::dot(-v_d, v_n);
		glm::dvec3 sin = cos + v_d;

		double n_incident, n_transmit, n_critical;
		if(glm::dot(v_d, v_n) < 0) {
			n_incident = 1;
			n_transmit = m.index(i);
		} else {
			n_incident = m.index(i);
			n_transmit = 1;
			v_n = -v_n;
		}

		n_critical = n_transmit / n_incident;


		if(glm::length(sin) < n_critical) {
			glm::dvec3 sin_t = (n_incident / n_transmit) * sin;
			//cout << "sint" << sin_t.length() << endl;

			double cos_t_val = sqrt(1 - glm::dot(sin_t, sin_t));
			glm::dvec3 cos_t = - v_n * cos_t_val;
			// cout << "cost" << cos_t.length() <<" " << cos_t_val << endl;

			
			glm::dvec3 v_refract = cos_t + sin_t;
			ray refract_ray(r.at(i), v_refract, r.getAtten(), ray::REFRACTION);

			isect newInsect;
			double attenDistance;
			if (scene->intersect(refract_ray, newInsect)) {
				attenDistance = newInsect.getT() / 10; //glm::length(r.at(i) - refract_ray.at(newInsect));
			} 

			// cout << "Refraction: " << - v_n * v_refract << ' ' << sin_t * cos_t << endl;
			glm::dvec3 refracted = RayTracer::traceRay(refract_ray, thresh, depth - 1, t);
			colorC += glm::pow(m.kt(i), glm::dvec3(0.01,0.01,0.01)) * refracted;
		}

	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.
		//
		// FIXME: Add CubeMap support here.
		// TIPS: CubeMap object can be fetched from traceUI->getCubeMap();
		//       Check traceUI->cubeMap() to see if cubeMap is loaded
		//       and enabled.

		colorC = glm::dvec3(0.0, 0.0, 0.0);
		if(!traceUI->cubeMap()) {
			//cout << "don't have cubmap" << endl;
		}
		else {
			CubeMap* cm = traceUI->getCubeMap();
			colorC = cm->getColor(r);
			//cout << "using cubemap" << endl;
		}






	}
#if VERBOSE
	std::cerr << "== depth: " << depth+1 << " done, returning: " << colorC << std::endl;
#endif
	return colorC;
}

RayTracer::RayTracer()
	: scene(nullptr), buffer(0), thresh(0), buffer_width(256), buffer_height(256), m_bBufferReady(false)
{
}

RayTracer::~RayTracer()
{
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer.data();
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene(const char* fn)
{
	ifstream ifs(fn);
	if( !ifs ) {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}

	// Strip off filename, leaving only the path:
	string path( fn );
	if (path.find_last_of( "\\/" ) == string::npos)
		path = ".";
	else
		path = path.substr(0, path.find_last_of( "\\/" ));

	// Call this with 'true' for debug output from the tokenizer
	Tokenizer tokenizer( ifs, false );
	Parser parser( tokenizer, path );
	try {
		scene.reset(parser.parseScene());
	}
	catch( SyntaxErrorException& pe ) {
		traceUI->alert( pe.formattedMessage() );
		return false;
	} catch( ParserException& pe ) {
		string msg( "Parser: fatal exception " );
		msg.append( pe.message() );
		traceUI->alert( msg );
		return false;
	} catch( TextureMapException e ) {
		string msg( "Texture mapping exception: " );
		msg.append( e.message() );
		traceUI->alert( msg );
		return false;
	}

	if (!sceneLoaded())
		return false;

	// To initialize kdtree
	if(traceUI->kdSwitch()) {
		scene->buildKdTree();
	}
	return true;
}

void RayTracer::traceSetup(int w, int h)
{
	if (buffer_width != w || buffer_height != h)
	{
		buffer_width = w;
		buffer_height = h;
		bufferSize = buffer_width * buffer_height * 3;
		buffer.resize(bufferSize);
	}
	std::fill(buffer.begin(), buffer.end(), 0);
	m_bBufferReady = true;

	/*
	 * Sync with TraceUI
	 */

	threads = traceUI->getThreads();
	block_size = traceUI->getBlockSize();
	thresh = traceUI->getThreshold();
	samples = traceUI->getSuperSamples();
	aaThresh = traceUI->getAaThreshold();

	// YOUR CODE HERE
	// FIXME: Additional initializations
	this->threadPool = new ThreadPool(threads);
	std::cout << "init thread pool, size: " << threads << std::endl;

}

/*
 * RayTracer::traceImage
 *
 *	Trace the image and store the pixel data in RayTracer::buffer.
 *
 *	Arguments:
 *		w:	width of the image buffer
 *		h:	height of the image buffer
 *
 */
void RayTracer::traceImage(int w, int h)
{
	// Always call traceSetup before rendering anything.
	traceSetup(w,h);

	// YOUR CODE HERE
	// FIXME: Start one or more threads for ray tracing
	//
	// TIPS: Ideally, the traceImage should be executed asynchronously,
	//       i.e. returns IMMEDIATELY after working threads are launched.
	//
	//       An asynchronous traceImage lets the GUI update your results
	//       while rendering.

	int threadNum = traceUI->getThreads();

	if(threadNum > 1) {
		omp_set_num_threads(threadNum);
	#pragma omp parallel for
		for(int i = 0; i < w; i++) {
			for(int j = 0; j < h; j++) {
				this->tracePixel(i, j);
			}
		}
		
	}
	else {
		for(int i = 0; i < w; i++) {
			for(int j = 0; j < h; j++) {
				// Task* task = new Task();
				// task->func = [this, i, j]{this->tracePixel(i, j);};
				// this->threadPool->addTask(task);
				this->tracePixel(i, j);
			}
		}


	}
	
}

int RayTracer::aaImage()
{
	// YOUR CODE HERE
	// FIXME: Implement Anti-aliasing here
	//
	// TIP: samples and aaThresh have been synchronized with TraceUI by
	//      RayTracer::traceSetup() function

	//implemented in tracePixel()

	return 0;
}

bool RayTracer::checkRender()
{
	// YOUR CODE HERE
	// FIXME: Return true if tracing is done.
	//        This is a helper routine for GUI.
	//
	// TIPS: Introduce an array to track the status of each worker thread.
	//       This array is maintained by the worker threads.
}

void RayTracer::waitRender()
{
	

	// YOUR CODE HERE
	// FIXME: Wait until the rendering process is done.
	//        This function is essential if you are using an asynchronous
	//        traceImage implementation.
	//
	// TIPS: Join all worker threads here.
}


glm::dvec3 RayTracer::getPixel(int i, int j)
{
	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;
	return glm::dvec3((double)pixel[0]/255.0, (double)pixel[1]/255.0, (double)pixel[2]/255.0);
}

void RayTracer::setPixel(int i, int j, glm::dvec3 color)
{
	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * color[0]);
	pixel[1] = (int)( 255.0 * color[1]);
	pixel[2] = (int)( 255.0 * color[2]);
}

