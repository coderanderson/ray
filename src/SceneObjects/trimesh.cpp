#include "trimesh.h"
#include <assert.h>
#include <float.h>
#include <string.h>
#include <algorithm>
#include <cmath>
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;

using namespace std;

Trimesh::~Trimesh()
{
	for (auto m : materials)
		delete m;
	for (auto f : faces)
		delete f;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex(const glm::dvec3& v)
{
	vertices.emplace_back(v);
}

void Trimesh::addMaterial(Material* m)
{
	materials.emplace_back(m);
}

void Trimesh::addNormal(const glm::dvec3& n)
{
	normals.emplace_back(n);
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace(int a, int b, int c)
{
	int vcnt = vertices.size();

	if (a >= vcnt || b >= vcnt || c >= vcnt)
		return false;

	TrimeshFace* newFace = new TrimeshFace(
	        scene, new Material(*this->material), this, a, b, c);
	newFace->setTransform(this->transform);
	if (!newFace->degen)
		faces.push_back(newFace);
	else
		delete newFace;

	// Don't add faces to the scene's object list so we can cull by bounding
	// box
	return true;
}

// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
const char* Trimesh::doubleCheck()
{
	if (!materials.empty() && materials.size() != vertices.size())
		return "Bad Trimesh: Wrong number of materials.";
	if (!normals.empty() && normals.size() != vertices.size())
		return "Bad Trimesh: Wrong number of normals.";

	return 0;
}

bool Trimesh::intersectLocal(ray& r, isect& i) const
{
	bool have_one = false;
	for (auto face : faces) {
		isect cur;
		if (face->intersectLocal(r, cur)) {
			if (!have_one || (cur.getT() < i.getT())) {
				i = cur;
				have_one = true;
			}
		}
	}
	if (!have_one)
		i.setT(1000.0);
	return have_one;
}

bool TrimeshFace::intersect(ray& r, isect& i) const
{
	return intersectLocal(r, i);
}

// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).
bool TrimeshFace::intersectLocal(ray& r, isect& i) const
{
	// YOUR CODE HERE
	//
	// FIXME: Add ray-trimesh intersection

	double RAY_EPSILON = 0.00000001;

    const glm::dvec3& a = parent->vertices[ids[0]];
    const glm::dvec3& b = parent->vertices[ids[1]];
    const glm::dvec3& c = parent->vertices[ids[2]];

    // YOUR CODE HERE

    glm::dvec3 v_n = this->getNormal();
    
    double d = - glm::dot(v_n, a);

    const glm::dvec3& v_p = r.getPosition();
    const glm::dvec3& v_d = r.getDirection();

    double denominator = glm::dot(v_n, v_d);
    double numerator = glm::dot(v_n, v_p) + d;
    // cout << "t is " << numerator << "/" << denominator;
    // cout << "a is " << a << "; b is " << b << "; c is " << c << endl;
    if (denominator) {
        double t = - numerator / denominator;
        // cout << " = " << t << endl;
        if (t > RAY_EPSILON) {
            glm::dvec3 p = r.at(t);
            //cout << "; p is " << p << "; deter is " << ((b - a) ^ (c - a)) << endl;
            //To calculate the value, we need to make sure that we need fetch the 
            //value of cross product.
            double deter = glm::dot(glm::cross(b - a, c - a), v_n);
            double alpha = glm::dot(glm::cross(b - p, c - p), v_n) / deter;
            double beta = glm::dot(glm::cross(p - a, c - a), v_n) / deter;
            double gamma = glm::dot(glm::cross(b - a, p - a), v_n) / deter;

            if (alpha >= - RAY_EPSILON && 
                beta >= - RAY_EPSILON && 
                alpha + beta <= 1 + RAY_EPSILON) {

                glm::dvec3 normal_isect;
                // Calculate the phong interpolation of normal vector
                if (parent -> vertNorms) {
                    glm::dvec3& normal_a = parent->normals[ids[0]];
                    glm::dvec3& normal_b = parent->normals[ids[1]];
                    glm::dvec3& normal_c = parent->normals[ids[2]];
                    normal_isect = alpha * normal_a + beta * normal_b + gamma * normal_c;
                    normal_isect = glm::normalize(normal_isect);
                } else {
                    normal_isect = v_n;
                }

                i.setT(t);
                i.setN(normal_isect);
                i.setObject(this);
                if(parent->materials.size()>0){
                    Material am = *(parent->materials[ids[0]]);
                    Material bm = *(parent->materials[ids[1]]);
                    Material cm = *(parent->materials[ids[2]]);
                    Material im;
                    im += alpha * am;
                    im += beta * bm;
                    im += gamma * cm;
                    i.setMaterial(im);
                } else {
                    i.setMaterial(parent->getMaterial());
                }
                i.setBary(alpha, beta, gamma); //
                // cout << "new info is: t"<< i.t << "; bary:" << i.bary << endl;
                return true;    
            }
            // cout << "out of triangle\n";
        }
        // cout << "t < RAY_EPSILON\n";
    }
    // cout << "denominator == 0\n";
    return false;










    /*
	glm::dvec3 rayPos =  r.getPosition();
	glm::dvec3 rayDir = r.getDirection();
	glm::dvec3 faceNormal = this->getNormal();
	glm::dvec3 a_coords = parent->vertices[ids[0]];	// vertice A of trangle
	
	double vDotN = glm::dot(rayDir, faceNormal);
	if(vDotN == 0) {	// ray parallel to trimeshFace
		return false;
	}
	double t = - glm::dot(a_coords - rayPos, faceNormal) / vDotN;
	i.setObject(this);
	i.setMaterial(this->getMaterial());
	i.setT(t);
	i.setN(faceNormal);
	return true;
	*/
}

// Once all the verts and faces are loaded, per vertex normals can be
// generated by averaging the normals of the neighboring faces.
void Trimesh::generateNormals()
{
	int cnt = vertices.size();
	normals.resize(cnt);
	std::vector<int> numFaces(cnt, 0);

	for (auto face : faces) {
		glm::dvec3 faceNormal = face->getNormal();

		for (int i = 0; i < 3; ++i) {
			normals[(*face)[i]] += faceNormal;
			++numFaces[(*face)[i]];
		}
	}

	for (int i = 0; i < cnt; ++i) {
		if (numFaces[i])
			normals[i] /= numFaces[i];
	}

	vertNorms = true;
}

