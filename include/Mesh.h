#define GLM_FORCE_RADIANS
#include "Obj.h"
#include "Geometry.h"
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

struct HalfEdge {
    HalfEdge* next;
    HalfEdge* flip;
    Point* src;
    Face* face;
    Edge* parent;
};

struct Point {
    HalfEdge* he;
    glm::vec3 pos;
    glm::vec3 normal;
    unsigned int index;
    // newpos
    // isnew
};

struct Face {
    HalfEdge* he;
};

struct Edge {
    HalfEdge* he;
    // newpos for the new point
};

class Mesh : public Geometry {
    public:
        std::vector<Point*> pts;
        std::vector<HalfEdge*> hes;
        std::vector<Face*> faces;
        std::vector<Edge*> edges;

        //void init(const char* filename);

        Edge flip(Edge* edge);
        Edge split(Edge* edge);
};